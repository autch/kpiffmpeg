// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

HMODULE g_hModule;
extern char g_szIniFileName[MAX_PATH];

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
		::DisableThreadLibraryCalls(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

