// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

HMODULE g_hModule;
extern char g_szIniFileName[MAX_PATH];
std::string defaultFFmpegPath, defaultFFprobePath;

namespace
{
	std::string GetDefaultFFPath(const char* cszBaseName)
	{
		char pluginPath[MAX_PATH];
		char* pSlash;

		::GetModuleFileName(g_hModule, pluginPath, sizeof pluginPath);
		while (1)
		{
			pSlash = strrchr(pluginPath, '\\');
			if (pSlash == NULL) break;
			if (pSlash > pluginPath && ::IsDBCSLeadByte(*(pSlash - 1)))
			{
				*pSlash = '\0';
				*(pSlash - 1) = '\0';
			}
			else
			{
				break;
			}
		}

		strcpy_s(pSlash + 1, (sizeof pluginPath) - ((pSlash + 1) - pluginPath), (char*)cszBaseName);

		return std::string(pluginPath);
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	char* pDot;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hModule = hModule;

		GetModuleFileName(::g_hModule, ::g_szIniFileName, MAX_PATH);
		pDot = strrchr(g_szIniFileName, '.');
		strncpy_s(pDot, MAX_PATH - (pDot - g_szIniFileName), ".ini", 4);

		defaultFFmpegPath = GetDefaultFFPath("ffmpeg.exe");
		defaultFFprobePath = GetDefaultFFPath("ffprobe.exe");

		::DisableThreadLibraryCalls(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

