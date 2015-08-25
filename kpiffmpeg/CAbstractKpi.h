
#pragma once

class CAbstractKpi
{
public:
	virtual BOOL Open(LPSTR szFileName, SOUNDINFO* pInfo) = 0;
	virtual void Close() = 0;
	virtual DWORD Render(BYTE* buffer, DWORD dwSize) = 0;
	virtual DWORD SetPosition(DWORD dwPos) = 0;

	static BOOL GetTagInfo(const char *cszFileName, IKmpTagInfo *pInfo)
	{
		return FALSE;
	}
};
