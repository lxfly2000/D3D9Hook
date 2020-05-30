#include "test_100oj_readgamedata.h"
#include "custom_present.h"
#include<TlHelp32.h>
#include<d3dx9.h>
#include<string>
#include<sstream>
#include<vector>

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif

//由于是同一进程，可以不必使用Query***及ReadProcessMemory函数获取数据，直接用句柄及指针访问即可（可能？）

//点击图片可切换显示模式：详细/仅我的数值/不显示
//可移动图片位置

//INI:
//6个图片，图片中心XY在窗口的百分比位置，图片随机分组
//6个骰子图片
//连接点大小，连接文本框XY百分比位置，连接图片XY百分比位置，文本框粗细，圆角半径，边界颜色，填充颜色，文本字体，文本字重，文本颜色，文本字号
//显示模式

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
	TCHAR boxImageExpandRect[64], boxImageContentRect[64];
	float characterPosX, characterPosY,characterOriginX,characterOriginY;
	float cornerToBoxX, cornerToBoxY,cornerToCharacterX[6],cornerToCharacterY[6];
	TCHAR boxFontName[64];
	int boxFontWeight;//100,200,300...
	int boxFontColorARGB;
	int boxFontSize;
	int displayMode;//0:不显示 1-4:仅P1-P4的数值 5:仅非玩家的数值 6:仅我的数值 7:仅当前玩家的数值 8:详细
	int calcAvgCount;
	std::vector<int>avgUpImages, avgDownImages;
	TCHAR textNotInGame[64];

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
		GetInitConfFloat(cornerToBoxX, 0.8f);
		GetInitConfFloat(cornerToBoxY, 1.0f);
		GetInitConfStr(boxFontName, TEXT("SimHei"));
		GetInitConfInt(boxFontWeight, 300);
		GetInitConfInt(boxFontColorARGB, 0xFF000000);
		GetInitConfInt(boxFontSize, 18);
		GetInitConfInt(displayMode, 8);
		GetInitConfStr(boxImage, TEXT("box.png"));
		GetInitConfInt(calcAvgCount, 6);
		TCHAR avgbuf[32];
		GetPrivateProfileString(TEXT("100oj"), TEXT(_CRT_STRINGIZE(avgUpImages)), TEXT("0 1 2"), avgbuf, ARRAYSIZE(avgbuf), szConfPath);
		std::wstringstream ssbuf(avgbuf);
		while (!ssbuf.eof())
		{
			int n;
			ssbuf >> n;
			avgUpImages.push_back(n);
		}
		GetPrivateProfileString(TEXT("100oj"), TEXT(_CRT_STRINGIZE(avgDownImages)), TEXT("3 4 5"), avgbuf, ARRAYSIZE(avgbuf), szConfPath);
		ssbuf.clear();
		ssbuf.str(avgbuf);
		while (!ssbuf.eof())
		{
			int n;
			ssbuf >> n;
			avgDownImages.push_back(n);
		}
		GetInitConfStr(textNotInGame, TEXT("游戏外"));
		GetInitConfStr(boxImageExpandRect, TEXT("0 0 -1 -1"));
		GetInitConfStr(boxImageContentRect, TEXT("0 0 -1 -1"));
	}
	void SetNewXY(float x, float y,bool save)//0.0-1.0
	{
		characterPosX = x;
		characterPosY = y;
		if (save)
		{
			WritePrivateProfileString(TEXT("100oj"), TEXT(_CRT_STRINGIZE(characterPosX)), std::to_wstring(characterPosX).c_str(), szConfPath);
			WritePrivateProfileString(TEXT("100oj"), TEXT(_CRT_STRINGIZE(characterPosY)), std::to_wstring(characterPosY).c_str(), szConfPath);
		}
	}
	void SetMode(int mode,bool save)
	{
		displayMode = mode;
		if (save)
		{
			WritePrivateProfileString(TEXT("100oj"), TEXT(_CRT_STRINGIZE(displayMode)), std::to_wstring(displayMode).c_str(), szConfPath);
		}
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
LPVOID pLastDice,pCurPlayer,pCurChapter,pMapNum,pLastDiceIndex,pGameStatus;
LPVOID ppMyPlayer;
LPBYTE pMyPlayer;
int lastDice;//1-6
int myPlayer;//0开始
int curPlayer;//0开始
int curChapter;//1开始
int mapNum;//0或6为游戏外
int lastDiceIndex;//0-5
int gameStatus;

LPDIRECT3DTEXTURE9 texCharacters[6]{}, texDices[6]{};
D3DXIMAGE_INFO imginfoCharacters[6]{}, imginfoDices[6]{};
ID3DXSprite* g_pSprite = nullptr;
D3DVIEWPORT9 viewport;

Ini100OJ ini;

class Dot9ImageTexture
{
private:
	LPDIRECT3DTEXTURE9 img;
	D3DXIMAGE_INFO imginfo;
	RECT boxExpandRect, boxContentRect;
	RECT lastDrawContentRect;
public:
	ID3DXFont* m_pFont = nullptr;
	BOOL LoadFromFile(LPCWSTR file)
	{
		if (D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, file, 0, 0, 0, 0,
			D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, &imginfo, NULL, &img))
			return FALSE;
		return TRUE;
	}
	void SetExpandRect(LONG left, LONG top, LONG right, LONG bottom)
	{
		boxExpandRect.left = left;
		boxExpandRect.top = top;
		boxExpandRect.right = right == -1 ? imginfo.Width : right;
		boxExpandRect.bottom = bottom == -1 ? imginfo.Height : bottom;
	}
	void SetContentRect(LONG left, LONG top, LONG right, LONG bottom)
	{
		boxContentRect.left = left;
		boxContentRect.top = top;
		boxContentRect.right = right == -1 ? imginfo.Width : right;
		boxContentRect.bottom = bottom == -1 ? imginfo.Height : bottom;
	}
	void Release()
	{
		if (m_pFont)
		{
			m_pFont->Release();
			m_pFont = nullptr;
		}
	}
	void DrawExpandContent(LONG rectContentWidth,LONG rectContentHeight,const D3DXVECTOR3 *pos,float originXfactor,float originYfactor)
	{
		DrawExpandRect(boxExpandRect.right - boxExpandRect.left + rectContentWidth - boxContentRect.right + boxContentRect.left,
			boxExpandRect.bottom - boxExpandRect.top + rectContentHeight - boxContentRect.bottom + boxContentRect.top, pos, originXfactor, originYfactor);
	}
	void DrawExpandRect(LONG rectExpandWidth,LONG rectExpandHeight,const D3DXVECTOR3 *pos,float originXfactor,float originYfactor)
	{
		LONG deltaWidth = rectExpandWidth - boxExpandRect.right + boxExpandRect.left;
		LONG deltaHeight = rectExpandHeight - boxExpandRect.bottom + boxExpandRect.top;
		UINT boxNewWidth = imginfo.Width + deltaWidth;
		UINT boxNewHeight = imginfo.Height + deltaHeight;
		LONG originX,originY;
		if (originXfactor > 1.0f)
			originX = originXfactor;
		else if (originXfactor < 0.0f)
			originX = boxNewWidth - originXfactor;
		else
			originX = boxNewWidth * originXfactor;
		if (originYfactor > 1.0f)
			originY = originYfactor;
		else if (originYfactor < 0.0f)
			originY = boxNewHeight - originYfactor;
		else
			originY = boxNewHeight * originYfactor;
		RECT boxOuter{ pos->x - originX,pos->y - originY,0,0 };
		boxOuter.right = boxOuter.left + boxNewWidth;
		boxOuter.bottom = boxOuter.top + boxNewHeight;
		RECT tmp;
		D3DXMATRIX mtx,mtxs,mtxt;
		//先平移后缩放时，平移矩阵在右，缩放矩阵在左
		C(g_pSprite->SetTransform(D3DXMatrixMultiply(&mtx,
			D3DXMatrixScaling(&mtxs, (FLOAT)(deltaWidth + boxExpandRect.right - boxExpandRect.left) / (boxExpandRect.right - boxExpandRect.left), 1.0f, 1.0f),
			D3DXMatrixTranslation(&mtxt,boxOuter.left + boxExpandRect.left, boxOuter.top, 0))));
		tmp = { boxExpandRect.left,0,boxExpandRect.right,boxExpandRect.top };
		C(g_pSprite->Draw(img, &tmp, NULL, NULL, -1));
		C(g_pSprite->SetTransform(D3DXMatrixMultiply(&mtx,&mtxs,
			D3DXMatrixTranslation(&mtxt,boxOuter.left + boxExpandRect.left, boxOuter.top+boxExpandRect.bottom+deltaHeight, 0))));
		tmp.top = boxExpandRect.bottom;
		tmp.bottom = imginfo.Height;
		C(g_pSprite->Draw(img, &tmp, NULL, NULL, -1));
		C(g_pSprite->SetTransform(D3DXMatrixMultiply(&mtx,
			D3DXMatrixScaling(&mtxs, (FLOAT)(deltaWidth + boxExpandRect.right - boxExpandRect.left) / (boxExpandRect.right - boxExpandRect.left),
			(FLOAT)(deltaHeight + boxExpandRect.bottom - boxExpandRect.top) / (boxExpandRect.bottom - boxExpandRect.top), 1.0f),
			D3DXMatrixTranslation(&mtxt,boxOuter.left + boxExpandRect.left, boxOuter.top + boxExpandRect.top, 0))));
		tmp.top = boxExpandRect.top;
		tmp.bottom = boxExpandRect.bottom;
		C(g_pSprite->Draw(img, &tmp, NULL, NULL, -1));
		C(g_pSprite->SetTransform(D3DXMatrixMultiply(&mtx,
			D3DXMatrixScaling(&mtxs, 1.0f, (FLOAT)(deltaHeight + boxExpandRect.bottom - boxExpandRect.top) / (boxExpandRect.bottom - boxExpandRect.top), 1.0f),
			D3DXMatrixTranslation(&mtxt,boxOuter.left, boxOuter.top + boxExpandRect.top, 0))));
		tmp.left = 0;
		tmp.right = boxExpandRect.left;
		C(g_pSprite->Draw(img, &tmp, NULL, NULL, -1));
		C(g_pSprite->SetTransform(D3DXMatrixMultiply(&mtx, &mtxs,
			D3DXMatrixTranslation(&mtxt,boxOuter.left+boxExpandRect.right+deltaWidth, boxOuter.top + boxExpandRect.top, 0))));
		tmp.left = boxExpandRect.right;
		tmp.right = imginfo.Width;
		C(g_pSprite->Draw(img, &tmp, NULL, NULL, -1));
		C(g_pSprite->SetTransform(D3DXMatrixIdentity(&mtx)));
		tmp.top = 0;
		tmp.bottom = boxExpandRect.top;
		C(g_pSprite->Draw(img, &tmp, NULL, &D3DXVECTOR3(boxOuter.left+boxExpandRect.right+deltaWidth, boxOuter.top, 0), -1));
		tmp.left = 0;
		tmp.right = boxExpandRect.left;
		C(g_pSprite->Draw(img, &tmp, NULL, &D3DXVECTOR3(boxOuter.left, boxOuter.top, 0), -1));
		tmp.top = boxExpandRect.bottom;
		tmp.bottom = imginfo.Height;
		C(g_pSprite->Draw(img, &tmp, NULL, &D3DXVECTOR3(boxOuter.left, boxOuter.top+boxExpandRect.bottom+deltaHeight, 0), -1));
		tmp.left = boxExpandRect.right;
		tmp.right = imginfo.Width;
		C(g_pSprite->Draw(img, &tmp, NULL, &D3DXVECTOR3(boxOuter.left + boxExpandRect.right + deltaWidth, boxOuter.top + boxExpandRect.bottom + deltaHeight, 0), -1));
		lastDrawContentRect.left = boxOuter.left + boxContentRect.left;
		lastDrawContentRect.top = boxOuter.top + boxContentRect.top;
		lastDrawContentRect.right = boxOuter.left + boxContentRect.right + deltaWidth;
		lastDrawContentRect.bottom = boxOuter.top + boxContentRect.bottom + deltaHeight;
	}
	void DrawExpandRectString(LPCWSTR text, int length, const RECT* rect, const D3DXVECTOR3* pos, float originXfactor, float originYfactor)
	{
		DrawExpandContent(rect->right - rect->left, rect->bottom - rect->top, pos, originXfactor, originYfactor);
		C(g_pSprite->End());
		C(g_pSprite->Begin(D3DXSPRITE_ALPHABLEND));
		C(m_pFont->DrawText(NULL, text, length, &lastDrawContentRect, DT_CENTER | DT_VCENTER, ini.boxFontColorARGB));
	}
}dot9imgBox;

D3DXFONT_DESC df;
void CalcDrawTextWidth(LPCWSTR str);
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
		pCurPlayer = reinterpret_cast<int*>(hm + 0x57EC64);
		pCurChapter = reinterpret_cast<int*>(hm + 0x57EC60);
		pMapNum = reinterpret_cast<int*>(hm + 0x550E60);
		pLastDiceIndex = reinterpret_cast<int*>(hm + 0x57ED50);
		pGameStatus = reinterpret_cast<int*>(hm + 0x57EC68);
	}
	g_pDevice->GetViewport(&viewport);
	if (!g_pSprite)
		C(D3DXCreateSprite(g_pDevice, &g_pSprite));
	ini.LoadSettings();
	if (!dot9imgBox.m_pFont)
	{
		ZeroMemory(&df, sizeof(D3DXFONT_DESC));
		df.Height = -MulDiv(ini.boxFontSize, USER_DEFAULT_SCREEN_DPI, 72);//此处不能直接用字体大小，需要将字体大小换算成GDI的逻辑单元大小
		df.Width = 0;
		df.Weight = ini.boxFontWeight;
		df.MipLevels = D3DX_DEFAULT;
		df.Italic = false;
		df.CharSet = DEFAULT_CHARSET;
		df.OutputPrecision = 0;
		df.Quality = 0;
		df.PitchAndFamily = 0;
		lstrcpy(df.FaceName, ini.boxFontName);
		C(D3DXCreateFontIndirect(g_pDevice, &df, &dot9imgBox.m_pFont));
	}

	for (int i = 0; i < ARRAYSIZE(ini.characters); i++)
	{
		if (D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, ini.characters[i], 0, 0, 0, 0,
			D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, imginfoCharacters + i, NULL, texCharacters + i)&&
			D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, ini.GetDllRelativePath(ini.characters[i]).c_str(), 0, 0, 0, 0,
				D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, imginfoCharacters + i, NULL, texCharacters + i))
		{
			texCharacters[i] = nullptr;
		}
	}
	for (int i = 0; i < ARRAYSIZE(ini.dices); i++)
	{
		if (D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, ini.dices[i], 0, 0, 0, 0,
			D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, imginfoDices + i, NULL, texDices + i)&&
			D3D_OK != D3DXCreateTextureFromFileEx(g_pDevice, ini.GetDllRelativePath(ini.dices[i]).c_str(), 0, 0, 0, 0,
				D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, imginfoDices + i, NULL, texDices + i))
		{
			texDices[i] = nullptr;
		}
	}
	//加载BG图片
	if (!dot9imgBox.LoadFromFile(ini.boxImage) && !dot9imgBox.LoadFromFile(ini.GetDllRelativePath(ini.boxImage).c_str()))
		dot9imgBox.Release();
	else
	{
		LONG a, b, c, d;
		std::wstringstream ssbuf(ini.boxImageExpandRect);
		ssbuf >> a >> b >> c >> d;
		dot9imgBox.SetExpandRect(a, b, c, d);
		ssbuf.clear();
		ssbuf.str(ini.boxImageContentRect);
		ssbuf >> a >> b >> c >> d;
		dot9imgBox.SetContentRect(a, b, c, d);
	}
	CalcDrawTextWidth(ini.textNotInGame);
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
	dot9imgBox.Release();
	SAFE_RELEASE(g_pSprite)
	CloseHandle(hOrange);
}

//0-3分别对应P1-P4，4对应BOSS或其他
std::vector<int>dice_records[5];
float dice_avg[5]{};

bool inGame = false;
int lastPlayer = -1, lastChapter = -1;
int lastDice1 = 0, lastDice2 = 0;
bool rollingDice = false;
int displayingCharacter = 0;
std::wstring detailedData;
RECT calcDrawTextRect{};
int lastGameStatus = 0;
int lastRecordDice = 0;
RECT characterRect;

BOOL IsMouseOnCharacter(const POINT *p)
{
	characterRect = { (LONG)(viewport.X + viewport.Width * ini.characterPosX - imginfoCharacters[displayingCharacter].Width * ini.characterOriginX),
	(LONG)(viewport.Y + viewport.Height * ini.characterPosY - imginfoCharacters[displayingCharacter].Height * ini.characterOriginY),0,0 };
	characterRect.right = characterRect.left + imginfoCharacters[displayingCharacter].Width;
	characterRect.bottom = characterRect.top + imginfoCharacters[displayingCharacter].Height;

	if (p->x >= characterRect.left && p->x < characterRect.right && p->y >= characterRect.top && p->y < characterRect.bottom)
		return TRUE;
	else
		return FALSE;
}

void CalcDrawTextWidth(LPCWSTR str)
{
	calcDrawTextRect.left = calcDrawTextRect.top = calcDrawTextRect.right = 0;
	calcDrawTextRect.bottom = dot9imgBox.m_pFont->DrawText(NULL, str, -1,
		&calcDrawTextRect, DT_SINGLELINE | DT_LEFT | DT_CALCRECT, ini.boxFontColorARGB);
}

int lastLeftMousePressed = 0;
POINT lastMouseUpPoint{};
float lastCharacterPointX = 0.0f, lastCharacterPointY = 0.0f;
POINT currentMousePoint{};
bool mouseOnCharacter = false;
int dragCharacter = 0;//0=No 1=OnStart 2=Dragging 3=End

void UpdateData()
{
	if (!hOrange)
		return;
	//TODO:已查到这是一个数组，但不知如何判断数组长度，且该数值不能表示战斗中的骰子点数，需要另外查找
	lastDice = *(int*)pLastDice;// ReadProcessMemory(hOrange, pLastDice, &lastDice, sizeof(lastDice), NULL);

	lastDiceIndex = *(int*)pLastDiceIndex;// ReadProcessMemory(hOrange, pLastDiceIndex, &lastDiceIndex, sizeof(lastDiceIndex), NULL);

	/*pMyPlayer = *(LPBYTE*)ppMyPlayer + 0xF8;*/ ReadProcessMemory(hOrange, ppMyPlayer, &pMyPlayer, sizeof(pMyPlayer), NULL);
	/*myPlayer = *(int*)pMyPlayer;*/ ReadProcessMemory(hOrange, (LPVOID)(pMyPlayer+0xF8), &myPlayer, sizeof(myPlayer), NULL);

	curPlayer = *(int*)pCurPlayer;// ReadProcessMemory(hOrange, pCurPlayer, &curPlayer, sizeof(curPlayer), NULL);

	curChapter = *(int*)pCurChapter;// ReadProcessMemory(hOrange, pCurChapter, &curChapter, sizeof(curChapter), NULL);

	mapNum = *(int*)pMapNum;// ReadProcessMemory(hOrange, pMapNum, &mapNum, sizeof(mapNum), NULL);

	gameStatus = *(int*)pGameStatus;

	if (inGame)
	{
		if (mapNum <= 6)
		{
			inGame = false;
			CalcDrawTextWidth(ini.textNotInGame);
		}
		else
		{
			//状态变化
			if (lastGameStatus != gameStatus)
			{
				lastGameStatus = gameStatus;
				lastRecordDice = 0;
			}
			//游戏中
			if (lastDice2 != lastDice1 && lastDice1 != lastDiceIndex&&lastRecordDice==0)
			{
				rollingDice = true;
			}
			else if (lastDice2 == lastDice1 && lastDice1 == lastDiceIndex)
			{
				if (rollingDice == true)
				{
					rollingDice = false;//BUG:有时一次投骰会记录多次
					//记录骰子
					if ((UINT)curPlayer > 4)
						curPlayer = 4;
					lastRecordDice = lastDice;
					dice_records[curPlayer].push_back(lastDice);
					float backupAvg = dice_avg[curPlayer];
					if (ini.calcAvgCount == 0)
						dice_avg[curPlayer] = lastDice;
					else
					{
						dice_avg[curPlayer] = 0;
						int i = 0;
						for (; i < min((int)dice_records[curPlayer].size(), ini.calcAvgCount); i++)
						{
							dice_avg[curPlayer] += dice_records[curPlayer][dice_records[curPlayer].size() - 1 - i];
						}
						dice_avg[curPlayer] /= i;
					}
					if (curPlayer == myPlayer)
					{
						if (dice_avg[curPlayer] < backupAvg)
							displayingCharacter = ini.avgDownImages[(UINT)rand() % ini.avgDownImages.size()];
						else
							displayingCharacter = ini.avgUpImages[(UINT)rand() % ini.avgUpImages.size()];
					}
					detailedData.clear();
					TCHAR fbuf[16];
					for (int i = 0; i < 4; i++)
					{
						if (i != 0)
							detailedData += TEXT("\n");
						detailedData += TEXT("P") + std::to_wstring(i + 1);
						int pointCount[6]{};
						for (int j = 0; j < dice_records[i].size(); j++)
							pointCount[dice_records[i][j] - 1]++;
						for (int j = 0; j < 6; j++)
						{
							TCHAR fmtbuf[8];
							wsprintf(fmtbuf, TEXT(" %2d"), pointCount[j]);
							detailedData += fmtbuf;
						}
						detailedData+=TEXT("|");
						for (int j = (int)dice_records[i].size() - ini.calcAvgCount; j < (int)dice_records[i].size(); j++)
						{
							if (detailedData.back() != '|')
								detailedData.push_back(' ');
							if (j < 0)
								detailedData += TEXT("0");
							else
								detailedData += std::to_wstring(dice_records[i][j]);
						}
						swprintf_s(fbuf, TEXT(" %3.1f"), dice_avg[i]);
						detailedData += fbuf;
						if (i == 0)
						{
							CalcDrawTextWidth(detailedData.c_str());
						}
					}
					calcDrawTextRect.bottom *= 4;
				}
			}
			lastDice2 = lastDice1;
			lastDice1 = lastDiceIndex;
			lastPlayer = curPlayer;
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
				dice_avg[i] = 0;
			}
			detailedData.clear();
			CalcDrawTextWidth(detailedData.c_str());
		}
	}
	int leftMousePressed = GetAsyncKeyState(VK_LBUTTON);
	GetCursorPos(&currentMousePoint);
	ScreenToClient(g_hwnd, &currentMousePoint);
	mouseOnCharacter = IsMouseOnCharacter(&currentMousePoint);
	static float backupOriginX, backupOriginY;
	switch (dragCharacter)
	{
	case 0:
		if (!lastLeftMousePressed && leftMousePressed && mouseOnCharacter)
			dragCharacter = 1;
		break;
	case 1:
		backupOriginX = ini.characterOriginX;
		backupOriginY = ini.characterOriginY;
		ini.characterOriginX = (float)(currentMousePoint.x - characterRect.left) / imginfoCharacters[displayingCharacter].Width;
		ini.characterOriginY = (float)(currentMousePoint.y - characterRect.top) / imginfoCharacters[displayingCharacter].Height;
		ini.SetNewXY((float)currentMousePoint.x / viewport.Width, (float)currentMousePoint.y / viewport.Height, false);
		dragCharacter = 2;
		break;
	case 2:
		ini.SetNewXY((float)currentMousePoint.x / viewport.Width, (float)currentMousePoint.y / viewport.Height, false);
		if (!leftMousePressed)
			dragCharacter = 3;
		break;
	case 3:
		ini.characterOriginX = backupOriginX;
		ini.characterOriginY = backupOriginY;
		ini.SetNewXY((float)(characterRect.left + imginfoCharacters[displayingCharacter].Width * ini.characterOriginX) / viewport.Width,
			(float)(characterRect.top + imginfoCharacters[displayingCharacter].Height * ini.characterOriginY) / viewport.Height, true);
		dragCharacter = 0;
		break;
	}
	lastLeftMousePressed = leftMousePressed;
}

void Test100OJDraw()
{
	UpdateData();
	C(g_pSprite->Begin(D3DXSPRITE_ALPHABLEND));
	if (mouseOnCharacter)
	{
		if (mapNum <= 6)
		{
			dot9imgBox.DrawExpandRectString(ini.textNotInGame, -1, &calcDrawTextRect,
				&D3DXVECTOR3(viewport.X + viewport.Width * ini.characterPosX - imginfoCharacters[displayingCharacter].Width * ini.characterOriginX +
					imginfoCharacters[displayingCharacter].Width * ini.cornerToCharacterX[displayingCharacter],
					viewport.Y + viewport.Height * ini.characterPosY - imginfoCharacters[displayingCharacter].Height * ini.characterOriginY +
					imginfoCharacters[displayingCharacter].Height * ini.cornerToCharacterY[displayingCharacter], 0),
				ini.cornerToBoxX, ini.cornerToBoxY);
		}
		else
		{
			switch (ini.displayMode)
			{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				//TODO
				break;
			case 8:
				dot9imgBox.DrawExpandRectString(detailedData.c_str(), detailedData.length(), &calcDrawTextRect,
					&D3DXVECTOR3(viewport.X + viewport.Width * ini.characterPosX - imginfoCharacters[displayingCharacter].Width * ini.characterOriginX +
						imginfoCharacters[displayingCharacter].Width * ini.cornerToCharacterX[displayingCharacter],
						viewport.Y + viewport.Height * ini.characterPosY - imginfoCharacters[displayingCharacter].Height * ini.characterOriginY +
						imginfoCharacters[displayingCharacter].Height * ini.cornerToCharacterY[displayingCharacter], 0),
					ini.cornerToBoxX, ini.cornerToBoxY);
				break;
			default:
				break;
			}
		}
	}
	if (texCharacters[displayingCharacter])
		C(g_pSprite->Draw(texCharacters[displayingCharacter], NULL, &D3DXVECTOR3(imginfoCharacters[displayingCharacter].Width * ini.characterOriginX,
			imginfoCharacters[displayingCharacter].Height * ini.characterOriginY, 0), &D3DXVECTOR3(viewport.X + viewport.Width * ini.characterPosX,
				viewport.Y + viewport.Height * ini.characterPosY, 0), -1));
	C(g_pSprite->End());
}
