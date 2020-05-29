#include "test_100oj_readgamedata.h"
#include<TlHelp32.h>
#include<d3dx9.h>

//������ͬһ���̣����Բ���ʹ��Query***��ReadProcessMemory������ȡ���ݣ�ֱ���þ����ָ����ʼ��ɣ����ܣ���

//���ͼƬ���л���ʾģʽ����ϸ/���ҵ���ֵ/����ʾ
//���ƶ�ͼƬλ��

//INI:
//6��ͼƬ��ͼƬ����XY�ڴ��ڵİٷֱ�λ�ã�ͼƬ�������
//6������ͼƬ
//���ӵ��С�������ı���XY�ٷֱ�λ�ã�����ͼƬXY�ٷֱ�λ�ã��ı����ϸ��Բ�ǰ뾶���߽���ɫ�������ɫ���ı����壬�ı����أ��ı���ɫ���ı��ֺ�
//��ʾģʽ
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
		hm = QueryBaseAddrOfPIDModule(pidOrange, TEXT("100orange.exe"));//ͬһ���̵Ļ�ַ������GetModuleHandle(NULL)���
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
	//TODO:�Ѳ鵽����һ�����飬����֪����ж����鳤�ȣ��Ҹ���ֵ���ܱ�ʾս���е����ӵ�������Ҫ�������
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
