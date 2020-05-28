#include "test_100oj_readgamedata.h"
#include<TlHelp32.h>

//由于是同一进程，可以不必使用Query***及ReadProcessMemory函数获取数据，直接用句柄及指针访问即可（可能？）

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

DWORD pidOrange;
HANDLE hOrange;
LPBYTE hm;
LPVOID pLastDice,pCurPlayer,pCurChapter,pMapNum,pLastDiceIndex;
LPVOID ppMyPlayer;
LPBYTE pMyPlayer;
int lastDice, myPlayer,curPlayer,curChapter,mapNum,lastDiceIndex;

void Test100OJReadGameDataInit()
{
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
	}
}

void Test100OJReadGameDataUninit()
{
	CloseHandle(hOrange);
}

int Test100OJGetGameDataOutput(LPWSTR str, int length)
{
	if (!hOrange)
		return str[0] = 0;

	int offset = wsprintf(str, TEXT("LastDice:"));
	//TODO:已查到这是一个数组，但不知如何判断数组长度，且该数值不能表示战斗中的骰子点数，需要另外查找
	if (!ReadProcessMemory(hOrange, pLastDice, &lastDice, sizeof(lastDice), NULL))
		offset += wsprintf(str + offset, TEXT("[%d]"), GetLastError());
	offset += wsprintf(str + offset, TEXT("%d"), lastDice);

	if (!ReadProcessMemory(hOrange, pLastDiceIndex, &lastDiceIndex, sizeof(lastDiceIndex), NULL))
		offset += wsprintf(str + offset, TEXT("[%d]"), GetLastError());
	offset += wsprintf(str + offset, TEXT("(%d)"), lastDiceIndex);

	offset += wsprintf(str + offset, TEXT(" Me:P"));
	if (!ReadProcessMemory(hOrange, ppMyPlayer, &pMyPlayer, sizeof(pMyPlayer), NULL))
		offset += wsprintf(str + offset, TEXT("[%d]"), GetLastError());
	pMyPlayer += 0xF8;
	if (!ReadProcessMemory(hOrange, (LPVOID)pMyPlayer, &myPlayer, sizeof(myPlayer), NULL))
		offset += wsprintf(str + offset, TEXT("[%d]"), GetLastError());
	offset += wsprintf(str + offset, TEXT("%d"), myPlayer + 1);

	offset += wsprintf(str + offset, TEXT(" Turn:P"));
	if (!ReadProcessMemory(hOrange, pCurPlayer, &curPlayer, sizeof(curPlayer), NULL))
		offset += wsprintf(str + offset, TEXT("[%d]"), GetLastError());
	offset += wsprintf(str + offset, TEXT("%d"), curPlayer + 1);

	offset += wsprintf(str + offset, TEXT(" Chpt:"));
	if (!ReadProcessMemory(hOrange, pCurChapter, &curChapter, sizeof(curChapter), NULL))
		offset += wsprintf(str + offset, TEXT("[%d]"), GetLastError());
	offset += wsprintf(str + offset, TEXT("%d"), curChapter);

	offset += wsprintf(str + offset, TEXT(" Map:"));
	if (!ReadProcessMemory(hOrange, pMapNum, &mapNum, sizeof(mapNum), NULL))
		offset += wsprintf(str + offset, TEXT("[%d]"), GetLastError());
	offset += wsprintf(str + offset, TEXT("%d"), mapNum);

	if (offset >= length)
		return str[0] = 0;
	return offset;
}
