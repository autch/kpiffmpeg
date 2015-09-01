
#include "stdafx.h"
#include "kmp_pi.h"
#include "CFFmpegKpi.h"

char g_szIniFileName[MAX_PATH];
extern HMODULE g_hModule;

void WINAPI kpiInit()
{
}

void WINAPI kpiDeinit()
{
}

HKMP WINAPI kpiOpen(const char* cszFileName, SOUNDINFO* pInfo)
{
	CAbstractKpi* d = new CFFmpegKpi();
	if (d == NULL) return NULL;

	if (d->Open((LPSTR)cszFileName, pInfo))
		return (HKMP)d;
	delete d;
	return NULL;
}

void WINAPI kpiClose(HKMP hKMP)
{
	CAbstractKpi* d = (CAbstractKpi*)hKMP;
	if (d)
	{
		d->Close();
		delete d;
	}
}

DWORD WINAPI kpiRender(HKMP hKMP, BYTE* Buffer, DWORD dwSize)
{
	CAbstractKpi* d = (CAbstractKpi*)hKMP;
	if (d)
		return d->Render(Buffer, dwSize);
	return 0;
}

DWORD WINAPI kpiSetPosition(HKMP hKMP, DWORD dwPos)
{
	CAbstractKpi* d = (CAbstractKpi*)hKMP;
	if (d)
		return d->SetPosition(dwPos);
	return 0;
}

UINT GetMyProfileInt(LPSTR szSectionName, LPSTR szKeyName, INT nDefault)
{
	return ::GetPrivateProfileInt(szSectionName, szKeyName, nDefault, ::g_szIniFileName);
}

DWORD GetMyProfileString(LPSTR szSectionName, LPSTR szKeyName, LPCSTR cszDefault, LPSTR szResult, DWORD dwSize)
{
	return ::GetPrivateProfileString(szSectionName, szKeyName, cszDefault, szResult, dwSize, ::g_szIniFileName);
}

DWORD GetMyProfileString(LPSTR szSectionName, LPSTR szKeyName, LPCSTR cszDefault, DWORD dwMaxSize, std::string& result)
{
	char* buffer = new char[dwMaxSize];
	DWORD dwSize;

	dwSize = GetMyProfileString(szSectionName, szKeyName, cszDefault, buffer, dwMaxSize);
	result = std::string(buffer);
	delete[] buffer;
	return dwSize;
}

BOOL WINAPI kpiGetTagInfo(const char *cszFileName, IKmpTagInfo *pTagInfo)
{
	return CFFmpegKpi::GetTagInfo(cszFileName, pTagInfo);
}
