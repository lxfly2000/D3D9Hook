#include "test_100oj_readgamedata.h"
#include<TlHelp32.h>
#include<d3dx9.h>

//由于是同一进程，可以不必使用Query***及ReadProcessMemory函数获取数据，直接用句柄及指针访问即可（可能？）

//点击图片可切换显示模式：详细/仅我的数值/不显示
//可移动图片位置

//INI:
//6个图片，图片中心XY在窗口的百分比位置，图片随机分组
//6个骰子图片
//连接点大小，连接文本框XY百分比位置，连接图片XY百分比位置，文本框粗细，圆角半径，边界颜色，填充颜色，文本字体，文本字重，文本颜色，文本字号
//显示模式
#define GetInitConfStr(key,def) GetPrivateProfileString(TEXT("100oj"),TEXT(_STRINGIZE(key)),def,key,ARRAYSIZE(key),szConfPath)
#define GetInitConfInt(key,def) key=GetPrivateProfileInt(TEXT("100oj"),TEXT(_STRINGIZE(key)),def,szConfPath)
#define F(_i_str) (float)_wtof(_i_str)

class Ini100OJ
{
public:
	void LoadSettings()
	{
		//TODO
	}
	void SetNewXY(float x, float y)//0.0-1.0
	{
		//TODO
	}
	void SetMode(int mode)
	{
		//TODO
	}
};

DWORD QueryFirstPIDOfProcessName(LPCWSTR pn)
{
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof pe;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	for (BOOL notend = Process32First(hProcessSnap, &pe); notend; notend = Process32Next(hProcessSnap, &pe))
	{
		if (lstrcmp(pn, pe.szExeFile) == 0)
			return pe.th32ProcessID;
	}
	return 0;
}
LPBYTE QueryBaseAddrOfPIDModule(DWORD pid, LPCWSTR mn)
{
	MODULEENTRY32 me;
	me.dwSize = sizeof me;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	for (BOOL notend = Module32First(hProcessSnap, &me); notend; notend = Module32Next(hProcessSnap, &me))
	{
		if (lstrcmp(mn, me.szModule) == 0)
			return me.modBaseAddr;
	}
	return 0;
}

HWND g_hwnd;
LPDIRECT3DDEVICE9 g_pDevice;

DWORD pidOrange;
HANDLE hOrange;
LPBYTE hm;
LPVOID pLastDice,pCurPlayer,pCurChapter,pMapNum,pLastDiceIndex;
LPVOID ppMyPlayer;
LPBYTE pMyPlayer;
int lastDice, myPlayer,curPlayer,curChapter,mapNum,lastDiceIndex;

BOOL Test100OJReadGameDataInit(HWND hwnd,LPDIRECT3DDEVICE9 pDevice)
{
	g_hwnd = hwnd;
	g_pDevice = pDevice;
	pidOrange = QueryFirstPIDOfProcessName(TEXT("100orange.exe"));
	hOrange = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pidOrange);
	if (hOrange)
	{
		hm = QueryBaseAddrOfPIDModule(pidOrange, TEXT("100orange.exe"));//同一进程的基址可以用GetModuleHandle(NULL)获得
		pLastDice = reinterpret_cast<int*>(hm + 0x57ED0C);
		ppMyPlayer = reinterpret_cast<int**>(hm + 0x57E14C);
		pCurPlayer= reinterpret_cast<int*>(hm + 0x57EC64);
		pCurChapter = reinterpret_cast<int*>(hm + 0x57EC60);
		pMapNum = reinterpret_cast<int*>(hm + 0x550E60);
		pLastDiceIndex = reinterpret_cast<int*>(hm + 0x57ED50);
		return TRUE;
	}
	return FALSE;
}

void Test100OJReadGameDataUninit()
{
	CloseHandle(hOrange);
}

void UpdateData()
{
	//TODO:已查到这是一个数组，但不知如何判断数组长度，且该数值不能表示战斗中的骰子点数，需要另外查找
	lastDice = *(int*)pLastDice;// ReadProcessMemory(hOrange, pLastDice, &lastDice, sizeof(lastDice), NULL);

	lastDiceIndex = *(int*)pLastDiceIndex;// ReadProcessMemory(hOrange, pLastDiceIndex, &lastDiceIndex, sizeof(lastDiceIndex), NULL);

	pMyPlayer = *(LPBYTE*)ppMyPlayer + 0xF8;// ReadProcessMemory(hOrange, ppMyPlayer, &pMyPlayer, sizeof(pMyPlayer), NULL);
	myPlayer = *(int*)pMyPlayer;// ReadProcessMemory(hOrange, (LPVOID)pMyPlayer, &myPlayer, sizeof(myPlayer), NULL);

	curPlayer = *(int*)pCurPlayer;// ReadProcessMemory(hOrange, pCurPlayer, &curPlayer, sizeof(curPlayer), NULL);

	curChapter = *(int*)pCurChapter;// ReadProcessMemory(hOrange, pCurChapter, &curChapter, sizeof(curChapter), NULL);

	mapNum = *(int*)pMapNum;// ReadProcessMemory(hOrange, pMapNum, &mapNum, sizeof(mapNum), NULL);
}

void Test100OJDraw()
{
	if (!hOrange)
		return;

}
