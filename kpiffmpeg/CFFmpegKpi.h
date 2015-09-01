#pragma once

#include "CAbstractKpi.h"

struct Pipe
{
	HANDLE hPipeRead;
	HANDLE hPipeWrite;

	Pipe() : hPipeRead(INVALID_HANDLE_VALUE), hPipeWrite(INVALID_HANDLE_VALUE)
	{
	}

	~Pipe()
	{
	}

	BOOL Create(LPSECURITY_ATTRIBUTES lpSA, DWORD dwSize)
	{
		return ::CreatePipe(&hPipeRead, &hPipeWrite, lpSA, dwSize);
	}

	void Close()
	{
		CloseRead();
		CloseWrite();
	}
	void CloseRead()
	{
		if (hPipeRead != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hPipeRead);
			hPipeRead = INVALID_HANDLE_VALUE;
		}
	}
	void CloseWrite()
	{
		if (hPipeWrite != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hPipeWrite);
			hPipeWrite = INVALID_HANDLE_VALUE;
		}
	}
};

class CriticalSection
{
private:
	CRITICAL_SECTION cs;
public:
	CriticalSection()
	{
		InitializeCriticalSection(&cs);
	}
	~CriticalSection()
	{
		DeleteCriticalSection(&cs);
	}

	void Enter() { EnterCriticalSection(&cs); }
	void Leave() { LeaveCriticalSection(&cs); }
};

class ScopedCriticalSection
{
private:
	CriticalSection& my_cs;
public:
	ScopedCriticalSection(CriticalSection& cs) : my_cs(cs)
	{
		my_cs.Enter();
	}
	~ScopedCriticalSection()
	{
		my_cs.Leave();
	}
};

class CFFmpegKpi : public CAbstractKpi
{
public:
	typedef std::map<std::string, std::string> taginfo;
private:
	PROCESS_INFORMATION procInfo;
	SOUNDINFO info;
	Pipe stdoutPipe;
	std::string filename;
	CriticalSection cs;

	std::string CFFmpegKpi::getPCMFormat(int bps);

	std::string createFFmpegCommandLine(LPSTR szFileName, SOUNDINFO* pInfo, DWORD dwPos);
	BOOL startFFmpeg(std::string command);

	static std::string createFFprobeCommandLine(LPSTR szFileName);
	static std::string captureFFprobe(std::string command);
	static taginfo getTagMap(const char* cszFileName);
	static std::string DecideWhichFFToUse(LPSTR iniKeyName, LPSTR defaultExeName, std::string & localOverridePath);
	static bool FileExists(const char* szFileName);

public:
	CFFmpegKpi();
	virtual ~CFFmpegKpi();

	virtual BOOL Open(LPSTR szFileName, SOUNDINFO* pInfo);
	virtual void Close();
	virtual DWORD Render(BYTE* buffer, DWORD dwSize);
	virtual DWORD SetPosition(DWORD dwPos);


	void Stop();

	static BOOL GetTagInfo(const char *cszFileName, IKmpTagInfo *pInfo);
};
