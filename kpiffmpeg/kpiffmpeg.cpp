// kpiffmpeg.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "kpiffmpeg.h"

#include "kpi.h"
#include "kpi_version.h"

extern char g_szIniFileName[MAX_PATH];

char* pszNULL[] = { NULL };
char** ppExts = pszNULL;
std::vector<std::string> exts;

extern "C"
KMPMODULE* APIENTRY kmp_GetTestModule()
{
	char szExts[4096];

	DWORD dwIniRet = GetMyProfileString("kpiffmpeg", "exts", "", szExts, sizeof szExts);
	if (dwIniRet != 0)
	{
		if (ppExts != pszNULL)
		{
			delete[] ppExts;
			exts.clear();
		}
		char* p = szExts;
		char* pe = szExts + dwIniRet + 1;
		while (p < pe)
		{
			char* slash = strchr(p, '/');
			if (slash != NULL)
			{
				*slash = '\0';
				exts.push_back("." + std::string(p));
				p = slash + 1;
			}
			else
			{
				exts.push_back("." + std::string(p));
				break;
			}
		}
		ppExts = new char*[exts.size() + 1];
		char** pp = ppExts;
		for (auto && e : exts)
		{
			*pp++ = (char*)e.c_str();
		}
		*pp++ = NULL;
	}
	else
	{
		if (ppExts != pszNULL)
		{
			delete[] ppExts;
			exts.clear();
		}
		ppExts = pszNULL;
	}

	static KMPMODULE kpiModule =
	{
		KMPMODULE_VERSION,		// DWORD dwVersion;
		KPI_VERSION,    		// DWORD dwPluginVersion;
		KPI_COPYRIGHT, 			// const char	*pszCopyright;
		KPI_DESC,				// const char	*pszDescription;
		(const char**)ppExts,	// const char	**ppszSupportExts;
		1,						// DWORD dwReentrant;
		NULL,					// void (WINAPI *Init)(void);
		NULL,					// void (WINAPI *Deinit)(void);
		kpiOpen,				// HKMP (WINAPI *Open)(const char *cszFileName, SOUNDINFO *pInfo);
		NULL,	            	// HKMP (WINAPI *OpenFromBuffer)(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo);
		kpiClose,				// void (WINAPI *Close)(HKMP hKMP);
		kpiRender,				// DWORD (WINAPI *Render)(HKMP hKMP, BYTE* Buffer, DWORD dwSize);
		kpiSetPosition			// DWORD (WINAPI *SetPosition)(HKMP hKMP, DWORD dwPos);
	};
	return &kpiModule;
}

extern "C"
BOOL WINAPI kmp_GetTestTagInfo(const char *cszFileName, IKmpTagInfo *pTagInfo)
{
	char szExts[4096];
	std::vector<std::string> exts;

	DWORD dwIniRet = GetMyProfileString("kpiffmpeg", "disabletags", "", szExts, sizeof szExts);
	if (dwIniRet != 0)
	{
		if (strcmp(szExts, "*") == 0)
		{
			return TRUE;
		}
		char* p = szExts;
		char* pe = szExts + dwIniRet + 1;
		while (p < pe)
		{
			char* slash = strchr(p, '/');
			if (slash != NULL)
			{
				*slash = '\0';
				exts.push_back("." + std::string(p));
				p = slash + 1;
			}
			else
			{
				exts.push_back("." + std::string(p));
				break;
			}
		}
		std::string ext(strrchr((char*)cszFileName, '.'));
		for (auto && e : exts)
		{
			if (e == ext)
			{
				return TRUE;
			}
		}
	}

	return kpiGetTagInfo(cszFileName, pTagInfo);
}
