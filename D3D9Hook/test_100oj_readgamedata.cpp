#include "test_100oj_readgamedata.h"
#include "custom_present.h"
#include<TlHelp32.h>
#include<d3dx9.h>
#include<string>

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif

//������ͬһ���̣����Բ���ʹ��Query***��ReadProcessMemory������ȡ���ݣ�ֱ���þ����ָ����ʼ��ɣ����ܣ���

//���ͼƬ���л���ʾģʽ����ϸ/���ҵ���ֵ/����ʾ
//���ƶ�ͼƬλ��

//INI:
//6��ͼƬ��ͼƬ����XY�ڴ��ڵİٷֱ�λ�ã�ͼƬ�������
//6������ͼƬ
//���ӵ��С�������ı���XY�ٷֱ�λ�ã�����ͼƬXY�ٷֱ�λ�ã��ı����ϸ��Բ�ǰ뾶���߽���ɫ�������ɫ���ı����壬�ı����أ��ı���ɫ���ı��ֺ�
//��ʾģʽ

FLOAT GetPrivateProfileFloatW(_In_ LPCWSTR lpAppName, _In_ LPCWSTR lpKeyName, _In_ FLOAT nDefault, _In_opt_ LPCWSTR lpFileName)
{
	WCHAR buf[32]{};
	GetPrivateProfileStringW(lpAppName, lpKeyName, L"", buf, ARRAYSIZE(buf) - 1, lpFileName);
	return lstrlenW(buf) == 0 ? nDefault : _wtof(buf);
}

struct Ini100OJ
{
	TCHAR szConfPath[MAX_PATH];
	TCHAR szDllDir[MAX_PATH];

	TCHAR characters[6][MAX_PATH];
	TCHAR dices[6][MAX_PATH];
	TCHAR boxImage[MAX_PATH];
	float characterPosX, characterPosY,characterOriginX,characterOriginY;
	int group;
	float cornerToBoxX, cornerToBoxY,cornerToCharacterX[6],cornerToCharacterY[6];
	TCHAR boxFontName[64];
	int boxFontWeight;//100,200,300...
	int boxFontColorARGB;
	int boxFontSize;
	int displayMode;//0:����ʾ 1-4:��P1-P4����ֵ 5:������ҵ���ֵ 6:���ҵ���ֵ 7:��ϸ
	int calcAvgCount;

	std::wstring GetDllRelativePath(LPCWSTR fileRelativePath)
	{
		std::wstring path(szDllDir);
		path.append(fileRelativePath);
		return path;
	}
	void LoadSettings()
	{
		GetDLLPath(szConfPath, ARRAYSIZE(szConfPath));
		lstrcpy(wcsrchr(szConfPath, '.'), TEXT(".ini"));
		lstrcpy(szDllDir, szConfPath);
		wcsrchr(szDllDir, '\\')[1] = 0;
#define GetInitConfStr(key,def) GetPrivateProfileString(TEXT("100oj"),TEXT(_CRT_STRINGIZE(key)),def,key,ARRAYSIZE(key),szConfPath)
#define GetInitConfInt(key,def) key=GetPrivateProfileInt(TEXT("100oj"),TEXT(_CRT_STRINGIZE(key)),def,szConfPath)
#define F(_i_str) (float)_wtof(_i_str)
#define GetInitConfFloat(key,def) key=GetPrivateProfileFloatW(TEXT("100oj"),TEXT(_CRT_STRINGIZE(key)),def,szConfPath)
		for (int i = 0; i < 6; i++)
		{
			TCHAR kn[32], defn[MAX_PATH];
			wsprintf(kn, TEXT("character%d"), i);
			wsprintf(defn, TEXT("c%d.png"), i);
			GetPrivateProfileString(TEXT("100oj"), kn, defn, characters[i], ARRAYSIZE(characters[i]), szConfPath);
			wsprintf(kn, TEXT("dice%d"), i);
			wsprintf(defn, TEXT("d%d.png"), i);
			GetPrivateProfileString(TEXT("100oj"), kn, defn, dices[i], ARRAYSIZE(dices[i]), szConfPath);
			TCHAR fbuf[32];
			wsprintf(kn, TEXT("cornerToCharacterX%d"), i);
			wsprintf(defn, TEXT("0"), i);
			GetPrivateProfileString(TEXT("100oj"), kn, defn, fbuf, ARRAYSIZE(fbuf), szConfPath);
			cornerToCharacterX[i] = F(fbuf);
			wsprintf(kn, TEXT("cornerToCharacterY%d"), i);
			wsprintf(defn, TEXT("0"), i);
			GetPrivateProfileString(TEXT("100oj"), kn, defn, fbuf, ARRAYSIZE(fbuf), szConfPath);
			cornerToCharacterY[i] = F(fbuf);
		}
		GetInitConfFloat(characterPosX, 0.8f);
		GetInitConfFloat(characterPosY, 1.0f);
		GetInitConfFloat(characterOriginX, 0.5f);
		GetInitConfFloat(characterOriginY, 1.0f);
		GetInitConfInt(group, 2);
		GetInitConfFloat(cornerToBoxX, 0.8f);
		GetInitConfFloat(cornerToBoxY, 1.0f);
		GetInitConfStr(boxFontName, TEXT("SimHei"));
		GetInitConfInt(boxFontWeight, 300);
		GetInitConfInt(boxFontColorARGB, 0xFF000000);
		GetInitConfInt(boxFontSize, 18);
		GetInitConfInt(displayMode, 7);
		GetInitConfStr(boxImage, TEXT("box.png"));
		GetInitConfInt(calcAvgCount, 6);
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

LPDIRECT3DTEXTURE9 texCharacters[6]{}, texDices[6]{}, texBox{};
D3DXIMAGE_INFO imginfoCharacters[6]{}, imginfoDices[6]{}, imginfoBox{};
ID3DXSprite* g_pSprite = nullptr;
ID3DXFont* g_pFont = nullptr;
D3DVIEWPORT9 viewport;

Ini100OJ ini;

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
		pCurPlayer = reinterpret_cast<int*>(hm + 0x57EC64);
		pCurChapter = reinterpret_cast<int*>(hm + 0x57EC60);
		pMapNum = reinterpret_cast<int*>(hm + 0x550E60);
		pLastDiceIndex = reinterpret_cast<int*>(hm + 0x57ED50);
	}
	g_pDevice->GetViewport(&viewport);
	if (!g_pSprite)
		C(D3DXCreateSprite(g_pDevice, &g_pSprite));
	ini.LoadSettings();

	for (int i = 0; i < ARRAYSIZE(ini.characters); i++)
	{
		if (D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, ini.characters[i], 0, 0, 0, 0,
			D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, imginfoCharacters + i, NULL, texCharacters + i))
		{
			texCharacters[i] = nullptr;
		}
	}
	for (int i = 0; i < ARRAYSIZE(ini.dices); i++)
	{
		if (D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, ini.dices[i], 0, 0, 0, 0,
			D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, imginfoDices + i, NULL, texDices + i))
		{
			texDices[i] = nullptr;
		}
	}
	if (D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, ini.boxImage, 0, 0, 0, 0,
		D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, &imginfoBox, NULL, &texBox))
	{
		texBox = nullptr;
	}

	return TRUE;
}

#define SAFE_RELEASE(p) if(p){p->Release();p=nullptr;}
void Test100OJReadGameDataUninit()
{
	for (int i = 0; i < ARRAYSIZE(texCharacters); i++)
	{
		if (texCharacters[i])
		{
			texCharacters[i]->Release();
			texCharacters[i] = nullptr;
		}
	}
	for (int i = 0; i < ARRAYSIZE(texDices); i++)
	{
		if (texDices[i])
		{
			texDices[i]->Release();
			texDices[i] = nullptr;
		}
	}
	SAFE_RELEASE(g_pFont)
	SAFE_RELEASE(g_pSprite)
	CloseHandle(hOrange);
}

#include<vector>

std::vector<int>dice_records[5];//0-3�ֱ��ӦP1-P4��4��ӦBOSS������
bool inGame = false;
int lastPlayer = -1, lastChapter = -1;

void UpdateData()
{
	if (!hOrange)
		return;
	//TODO:�Ѳ鵽����һ�����飬����֪����ж����鳤�ȣ��Ҹ���ֵ���ܱ�ʾս���е����ӵ�������Ҫ�������
	lastDice = *(int*)pLastDice;// ReadProcessMemory(hOrange, pLastDice, &lastDice, sizeof(lastDice), NULL);

	lastDiceIndex = *(int*)pLastDiceIndex;// ReadProcessMemory(hOrange, pLastDiceIndex, &lastDiceIndex, sizeof(lastDiceIndex), NULL);

	pMyPlayer = *(LPBYTE*)ppMyPlayer + 0xF8;// ReadProcessMemory(hOrange, ppMyPlayer, &pMyPlayer, sizeof(pMyPlayer), NULL);
	//0��ʼ
	myPlayer = *(int*)pMyPlayer;// ReadProcessMemory(hOrange, (LPVOID)pMyPlayer, &myPlayer, sizeof(myPlayer), NULL);

	//0��ʼ
	curPlayer = *(int*)pCurPlayer;// ReadProcessMemory(hOrange, pCurPlayer, &curPlayer, sizeof(curPlayer), NULL);

	curChapter = *(int*)pCurChapter;// ReadProcessMemory(hOrange, pCurChapter, &curChapter, sizeof(curChapter), NULL);

	//0��6ʱΪ��Ϸ��
	mapNum = *(int*)pMapNum;// ReadProcessMemory(hOrange, pMapNum, &mapNum, sizeof(mapNum), NULL);

	if (inGame)
	{
		if (mapNum <= 6)
		{
			inGame = false;
		}
		else
		{
			if (curPlayer != lastPlayer)
			{
				/*TODO*/
				lastPlayer = curPlayer;
			}
		}
	}
	else
	{
		if (mapNum > 6)
		{
			inGame = true;
			for (int i = 0; i < ARRAYSIZE(dice_records); i++)
			{
				dice_records[i].clear();
			}
		}
	}
}

void Test100OJDraw()
{
	UpdateData();
	C(g_pSprite->Begin(D3DXSPRITE_ALPHABLEND));
	if (texCharacters[lastDiceIndex % 6])
		C(g_pSprite->Draw(texCharacters[lastDiceIndex % 6], NULL, &D3DXVECTOR3(imginfoCharacters[lastDiceIndex % 6].Width * ini.characterOriginX,
			imginfoCharacters[lastDiceIndex % 6].Height * ini.characterOriginY, 0), &D3DXVECTOR3(viewport.X + viewport.Width * ini.characterPosX,
				viewport.Y + viewport.Height * ini.characterPosY, 0), -1));
	C(g_pSprite->End());
}
