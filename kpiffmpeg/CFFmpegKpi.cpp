#include "stdafx.h"
#include "CFFmpegKpi.h"
#include "kpi.h"

CFFmpegKpi::CFFmpegKpi() : stdoutPipe(), filename()
{
	::ZeroMemory(&procInfo, sizeof procInfo);
	::ZeroMemory(&info, sizeof info);
}

CFFmpegKpi::~CFFmpegKpi()
{
	Close();
}

BOOL CFFmpegKpi::Open(LPSTR szFileName, SOUNDINFO * pInfo)
{
	int bps;

	ScopedCriticalSection myCs(cs);

	Close();

	pInfo->dwBitsPerSample = pInfo->dwBitsPerSample == 0 ? 16 : pInfo->dwBitsPerSample;
	pInfo->dwChannels = 2;
	pInfo->dwLength = -1;
	pInfo->dwReserved1 = pInfo->dwReserved2 = 0;
	pInfo->dwSamplesPerSec = pInfo->dwSamplesPerSec == 0 ? 44100 : pInfo->dwSamplesPerSec;
	pInfo->dwSeekable = 1;

	auto tagMap = getTagMap(szFileName);
	taginfo::iterator it;
	if ((it = tagMap.find("format.duration")) != tagMap.end())
	{
		double duration = std::stod(it->second) * 1000.0;
		pInfo->dwLength = (int)duration;
	}
	//if ((it = tagMap.find("streams.stream.0.channels")) != tagMap.end())
	//{
	//	pInfo->dwChannels = std::stoi(it->second);
	//}

	std::string cmdline = createFFmpegCommandLine(szFileName, pInfo, 0);

	bps = pInfo->dwBitsPerSample;
	if (bps < 0)
	{
		bps = -bps;
	}
	else if (bps == 0)
	{
		bps = 16;
	}

	pInfo->dwUnitRender = (bps >> 3) * pInfo->dwChannels * pInfo->dwSamplesPerSec;

	if (!startFFmpeg(cmdline))
	{
		return FALSE;
	}

	filename = szFileName;
	info = *pInfo;

	return TRUE;
}

void CFFmpegKpi::Stop()
{
	if (procInfo.hProcess != 0)
	{
		::TerminateProcess(procInfo.hProcess, 0);
		::WaitForSingleObject(procInfo.hProcess, 2000);
		::CloseHandle(procInfo.hProcess);
		::CloseHandle(procInfo.hThread);
		procInfo.hProcess = 0;
		procInfo.hThread = 0;
	}
	stdoutPipe.Close();
}

void CFFmpegKpi::Close()
{
	Stop();
	filename = "";
}

DWORD CFFmpegKpi::Render(BYTE * buffer, DWORD dwSize)
{
	DWORD dwBytesRead = 0;

	ScopedCriticalSection myCs(cs);

	if (filename.length() == 0)
		return 0;

	::ZeroMemory(buffer, dwSize);

	PBYTE p = buffer;
	while (dwSize > 0)
	{
		BOOL r = ::ReadFile(stdoutPipe.hPipeRead, p, dwSize, &dwBytesRead, NULL);
		if (!r) break;

		dwSize -= dwBytesRead;
		p += dwBytesRead;
	}

	return p - buffer;
}

DWORD CFFmpegKpi::SetPosition(DWORD dwPos)
{
	ScopedCriticalSection myCs(cs);

	if (filename.length() == 0)
		return -1;

	Stop();

	std::string cmdline = createFFmpegCommandLine((LPSTR)filename.c_str(), &info, dwPos);

	if (!startFFmpeg(cmdline))
	{
		return 0;
	}

	return dwPos;
}

int CFFmpegKpi::get_pcmformat(const char* prefix, int bps, char* buffer, int size)
{
	return _snprintf_s(buffer, size, size, "%s%c%d%s",
		prefix,
		bps < 0 ? 'f' : 's',
		bps < 0 ? -bps : bps,
		bps == 8 ? "" : "le");
}


std::string CFFmpegKpi::createFFmpegCommandLine(LPSTR szFileName, SOUNDINFO* pInfo, DWORD dwPos)
{
	char pos[256];
	std::vector<std::string> cmdline;
	int bps = 16;
	int seekPrecision = GetMyProfileInt("kpiffmpeg", "SeekPrecision", 0);

	switch (pInfo->dwBitsPerSample)
	{
	case -64:
	case -32:
	case 32:
	case 24:
	case 16:
	case 8:
		bps = pInfo->dwBitsPerSample;
		break;
	default:
		bps = pInfo->dwBitsPerSample = 16;
	}

	_snprintf_s(pos, sizeof pos, "%d:%02d:%02d.%04d",
		dwPos / (1000 * 60 * 60),
		(dwPos / (1000 * 60)) % 60,
		(dwPos / 1000) % 60,
		dwPos % 1000);

	cmdline.push_back("ffmpeg");
	cmdline.push_back("-hide_banner");
	cmdline.push_back("-y");
	//#ifdef _DEBUG
	//	cmdline.push_back("-report");
	//#endif
	cmdline.push_back("-v");
	cmdline.push_back("-8");
	if (seekPrecision == 0) {
		cmdline.push_back("-ss");
		cmdline.push_back(pos);
	}
	cmdline.push_back("-i");
	cmdline.push_back(szFileName);
	if (seekPrecision != 0) {
		cmdline.push_back("-ss");
		cmdline.push_back(pos);
	}
	cmdline.push_back("-vn");
	cmdline.push_back("-f");
	{
		char format[64];
		get_pcmformat("", bps, format, sizeof format);
		cmdline.push_back(format);
	}
	cmdline.push_back("-ac");
	{
		char channels[8];
		_snprintf_s(channels, sizeof channels, "%d", pInfo->dwChannels);
		cmdline.push_back(channels);
	}
	cmdline.push_back("-ar");
	{
		char fs[16];
		_snprintf_s(fs, sizeof fs, "%d", pInfo->dwSamplesPerSec);
		cmdline.push_back(fs);
	}
	cmdline.push_back("-acodec");
	{
		char format[64];
		get_pcmformat("pcm_", bps, format, sizeof format);
		cmdline.push_back(format);
	}
	cmdline.push_back("-");

	std::string result;
	for (auto i = cmdline.begin(); i != cmdline.end(); i++)
	{
		if (i->find_first_of(' ') != std::string::npos)
			result += "\"" + *i + "\" ";
		else
			result += *i + " ";
	}

	return result;
}

std::string CFFmpegKpi::createFFprobeCommandLine(LPSTR szFileName)
{
	std::vector<std::string> cmdline;

	cmdline.push_back("ffprobe");
	cmdline.push_back("-hide_banner");
	//#ifdef _DEBUG
	//	cmdline.push_back("-report");
	//#endif
	cmdline.push_back("-v");
	cmdline.push_back("-8");
	cmdline.push_back("-show_format");
	cmdline.push_back("-show_streams");
	cmdline.push_back("-of");
	cmdline.push_back("flat");
	cmdline.push_back(szFileName);

	std::string result;
	for (auto i = cmdline.begin(); i != cmdline.end(); i++)
	{
		if (i->find_first_of(' ') != std::string::npos)
			result += "\"" + *i + "\" ";
		else
			result += *i + " ";
	}

	return result;
}

BOOL CFFmpegKpi::startFFmpeg(std::string command)
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;
	HANDLE hNUL;

	::ZeroMemory(&pi, sizeof pi);
	::ZeroMemory(&startupInfo, sizeof startupInfo);
	::ZeroMemory(&sa, sizeof sa);

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	hNUL = ::CreateFile("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hNUL == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if (!stdoutPipe.Create(&sa, info.dwUnitRender))
	{
		::CloseHandle(hNUL);
		return FALSE;
	}

	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdInput = hNUL;
	startupInfo.hStdOutput = stdoutPipe.hPipeWrite;
	startupInfo.hStdError = NULL;
	startupInfo.wShowWindow = SW_HIDE;
	startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	BOOL r = ::CreateProcess(NULL, (LPSTR)command.c_str(), &sa, NULL, TRUE, 0, NULL, NULL, &startupInfo, &pi);
	if (!r)
	{
		stdoutPipe.Close();
		::CloseHandle(hNUL);
		return FALSE;
	}

	WaitForInputIdle(pi.hProcess, 2000);

	stdoutPipe.CloseWrite();
	::CloseHandle(hNUL);

	procInfo = pi;

	return r;
}

std::string CFFmpegKpi::captureFFprobe(std::string command)
{
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;
	Pipe pipe;
	HANDLE hNUL;

	::ZeroMemory(&pi, sizeof pi);
	::ZeroMemory(&startupInfo, sizeof startupInfo);
	::ZeroMemory(&sa, sizeof sa);

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	hNUL = ::CreateFile("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hNUL == INVALID_HANDLE_VALUE)
	{
		return std::string();
	}

	if (!pipe.Create(&sa, 0))
	{
		::CloseHandle(hNUL);
		return std::string();
	}

	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdInput = hNUL;
	startupInfo.hStdOutput = pipe.hPipeWrite;
	startupInfo.hStdError = NULL;
	startupInfo.wShowWindow = SW_HIDE;
	startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	BOOL r = ::CreateProcess(NULL, (LPSTR)command.c_str(), &sa, NULL, TRUE, 0, NULL, NULL, &startupInfo, &pi);
	if (!r)
	{
		pipe.Close();
		::CloseHandle(hNUL);
		return std::string();
	}

	WaitForInputIdle(pi.hProcess, 2000);

	pipe.CloseWrite();
	::CloseHandle(hNUL);

	std::string result;
	char buffer[4096];
	DWORD dwBytesRead = 0;
	while (1)
	{
		::ZeroMemory(buffer, sizeof buffer);
		BOOL r = ::ReadFile(pipe.hPipeRead, buffer, sizeof buffer, &dwBytesRead, NULL);
		if (!r) break;

		result.append(buffer, dwBytesRead);
	}

	::TerminateProcess(pi.hProcess, 0);
	::WaitForSingleObject(pi.hProcess, 2000);
	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);
	pipe.Close();

	return result;
}

CFFmpegKpi::taginfo CFFmpegKpi::getTagMap(const char * cszFileName)
{
	taginfo tagMap;

	std::string cmdline_probe = createFFprobeCommandLine((LPSTR)cszFileName);
	std::string probe = captureFFprobe(cmdline_probe);
	if (probe.length() > 0) {
		std::regex re("^([^=]+)=\"?(.*?)\"?$");
		std::smatch match;
		size_t offset = 0;
		while (offset < probe.length())
		{
			auto pe = probe.find_first_of("\r\n", offset);
			std::string line;
			if (pe != std::string::npos)
			{
				line = probe.substr(offset, pe - offset);
				offset = pe + 2;
			}
			else
			{
				line = probe.substr(offset);
				offset = probe.length();
			}

			if (std::regex_match(line, match, re))
			{
				tagMap[match[1].str()] = match[2].str();
			}
		}
	}

	return tagMap;
}

BOOL CFFmpegKpi::GetTagInfo(const char * cszFileName, IKmpTagInfo * pInfo)
{
	auto tagMap = getTagMap(cszFileName);

	if (tagMap.size() == 0)
		return FALSE;

	std::regex re("^format\\.tags\\.(.*)$");
	std::smatch match;
	taginfo::iterator it;

	for (auto && info : tagMap)
	{
		if (std::regex_match(info.first, match, re))
		{
			pInfo->SetValueU8(match[1].str().c_str(), info.second.c_str());
		}
		else
		{
			pInfo->SetValueU8(info.first.c_str(), info.second.c_str());
		}
	}
	if ((it = tagMap.find("format.bit_rate")) != tagMap.end())
	{
		pInfo->SetValueU8(SZ_KMP_TAGINFO_NAME_BITRATE, (it->second + "bps").c_str());
	}

	return TRUE;
}

