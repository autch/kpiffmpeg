
#pragma once

#include "kmp_pi.h"

void WINAPI kpiInit();
void WINAPI kpiDeinit();
HKMP WINAPI kpiOpen(const char* cszFileName, SOUNDINFO* pInfo);
void WINAPI kpiClose(HKMP hKMP);
DWORD WINAPI kpiRender(HKMP hKMP, BYTE* Buffer, DWORD dwSize);
DWORD WINAPI kpiSetPosition(HKMP hKMP, DWORD dwPos);

UINT GetMyProfileInt(LPSTR szSectionName, LPSTR szKeyName, INT nDefault = 0);
DWORD GetMyProfileString(LPSTR szSectionName, LPSTR szKeyName, LPCSTR cszDefault, LPSTR szResult, DWORD dwSize);
DWORD GetMyProfileString(LPSTR szSectionName, LPSTR szKeyName, LPCSTR cszDefault, DWORD dwMaxSize, std::string& result);

BOOL WINAPI kpiGetTagInfo(const char *cszFileName, IKmpTagInfo *pTagInfo);
