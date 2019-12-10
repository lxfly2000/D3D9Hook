#include"custom_present.h"
#include <d3dx9.h>
#include<map>
#include<string>
#include<ctime>

#pragma comment(lib,"d3dx9.lib")

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif

class D3DXCustomPresent
{
private:
	ID3DXFont* pFont;
	unsigned t1, t2, fcount;
	std::wstring display_text;
	int current_fps;
	TCHAR time_text[32], fps_text[32];

	TCHAR font_name[256], font_size[16],text_x[16],text_y[16],text_align[16],text_valign[16],display_text_fmt[256],fps_fmt[32],time_fmt[32];
	TCHAR font_red[16], font_green[16], font_blue[16], font_alpha[16];
	TCHAR font_shadow_red[16], font_shadow_green[16], font_shadow_blue[16], font_shadow_alpha[16], font_shadow_distance[16];
	UINT font_weight,period_frames;
	RECT rText, rTextShadow;
	int formatFlag;
	D3DCOLOR color_text, color_shadow;
public:
	D3DXCustomPresent():pFont(nullptr),t1(0),t2(0),fcount(0),formatFlag(0)
	{
	}
	D3DXCustomPresent(D3DXCustomPresent &&other)
	{
		t1 = std::move(other.t1);
		t2 = std::move(other.t2);
		fcount = std::move(other.fcount);
		formatFlag = std::move(other.formatFlag);
	}
	~D3DXCustomPresent()
	{
		Uninit();
	}
	BOOL Init(LPDIRECT3DDEVICE9 pDev)
	{
		TCHAR szConfPath[MAX_PATH];
		GetDLLPath(szConfPath, ARRAYSIZE(szConfPath));
		lstrcpy(wcsrchr(szConfPath, '.'), TEXT(".ini"));
#define GetInitConfStr(key,def) GetPrivateProfileString(TEXT("Init"),TEXT(_STRINGIZE(key)),def,key,ARRAYSIZE(key),szConfPath)
#define GetInitConfInt(key,def) key=GetPrivateProfileInt(TEXT("Init"),TEXT(_STRINGIZE(key)),def,szConfPath)
#define F(_i_str) (float)_wtof(_i_str)
		GetInitConfStr(font_name, TEXT("����"));
		GetInitConfStr(font_size, TEXT("48"));
		GetInitConfStr(font_red, TEXT("1"));
		GetInitConfStr(font_green, TEXT("1"));
		GetInitConfStr(font_blue, TEXT("0"));
		GetInitConfStr(font_alpha, TEXT("1"));
		GetInitConfStr(font_shadow_red, TEXT("0.5"));
		GetInitConfStr(font_shadow_green, TEXT("0.5"));
		GetInitConfStr(font_shadow_blue, TEXT("0"));
		GetInitConfStr(font_shadow_alpha, TEXT("1"));
		GetInitConfStr(font_shadow_distance, TEXT("2"));
		GetInitConfInt(font_weight, 400);
		GetInitConfStr(text_x, TEXT("0"));
		GetInitConfStr(text_y, TEXT("0"));
		GetInitConfStr(text_align, TEXT("left"));
		GetInitConfStr(text_valign, TEXT("top"));
		GetInitConfInt(period_frames, 60);
		GetInitConfStr(time_fmt, TEXT("%H:%M:%S"));
		GetInitConfStr(fps_fmt, TEXT("FPS:%3d"));
		GetInitConfStr(display_text_fmt, TEXT("{fps}"));

		D3DXFONT_DESC df;
		ZeroMemory(&df, sizeof(D3DXFONT_DESC));
		df.Height = -MulDiv(_wtoi(font_size), GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);//�˴�����ֱ���������С����Ҫ�������С�����GDI���߼���Ԫ��С
		df.Width = 0;
		df.Weight = font_weight;
		df.MipLevels = D3DX_DEFAULT;
		df.Italic = false;
		df.CharSet = DEFAULT_CHARSET;
		df.OutputPrecision = 0;
		df.Quality = 0;
		df.PitchAndFamily = 0;
		lstrcpy(df.FaceName, font_name);

		C(D3DXCreateFontIndirect(pDev, &df, &pFont));
		IDirect3D9* pD3D9;
		C(pDev->GetDirect3D(&pD3D9));
		D3DDISPLAYMODE dm;
		C(pD3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm));
		if (lstrcmpi(text_align, TEXT("right")) == 0)
		{
			formatFlag |= DT_RIGHT;
			rText.left = 0;
			rText.right = (LONG)(F(text_x)*dm.Width);
		}
		else if (lstrcmpi(text_align, TEXT("center")) == 0)
		{
			formatFlag |= DT_CENTER;
			if (F(text_x) > 0.5f)
			{
				rText.left = 0;
				rText.right = (LONG)(2.0f*dm.Width*F(text_x));
			}
			else
			{
				rText.left = (LONG)(2.0f*dm.Width*F(text_x) - dm.Width);
				rText.right = (LONG)dm.Width;
			}
		}
		else
		{
			formatFlag |= DT_LEFT;
			rText.left = (LONG)(F(text_x)*dm.Width);
			rText.right = (LONG)dm.Width;
		}
		if (lstrcmpi(text_valign, TEXT("bottom")) == 0)
		{
			formatFlag |= DT_BOTTOM;
			rText.top = 0;
			rText.bottom = (LONG)(F(text_y)*dm.Height);
		}
		else if (lstrcmpi(text_valign, TEXT("center")) == 0)
		{
			formatFlag |= DT_VCENTER;
			if (F(text_y) > 0.5f)
			{
				rText.top = 0;
				rText.bottom = (LONG)(2.0f*dm.Height*F(text_y));
			}
			else
			{
				rText.top = (LONG)(2.0f*dm.Height*F(text_y) - dm.Height);
				rText.bottom = (LONG)dm.Height;
			}
		}
		else
		{
			formatFlag |= DT_TOP;
			rText.top = (LONG)(F(text_y)*dm.Height);
			rText.bottom = (LONG)dm.Height;
		}
		rTextShadow.left = rText.left + (LONG)F(font_shadow_distance);
		rTextShadow.top = rText.top + (LONG)F(font_shadow_distance);
		rTextShadow.right = rText.right + (LONG)F(font_shadow_distance);
		rTextShadow.bottom = rText.bottom + (LONG)F(font_shadow_distance);
		color_shadow = D3DCOLOR_ARGB((DWORD)(255.0f*F(font_shadow_alpha)),
			(DWORD)(255.0f*F(font_shadow_red)),
			(DWORD)(255.0f*F(font_shadow_green)),
			(DWORD)(255.0f*F(font_shadow_blue)));
		color_text = D3DCOLOR_ARGB((DWORD)(255.0f*F(font_alpha)),
			(DWORD)(255.0f*F(font_red)),
			(DWORD)(255.0f*F(font_green)),
			(DWORD)(255.0f*F(font_blue)));

		return TRUE;
	}
	void Uninit()
	{
		if(pFont)
			pFont->Release();
	}
	void Draw()
	{
		if (fcount--==0)
		{
			fcount = period_frames;
			t1 = t2;
			t2 = GetTickCount();
			if (t1 == t2)
				t1--;
			current_fps = period_frames * 1000 / (t2 - t1);
			wsprintf(fps_text, fps_fmt, current_fps);//ע��wsprintf��֧�ָ�������ʽ��
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			wcsftime(time_text, ARRAYSIZE(time_text), time_fmt, &tm1);
			display_text = display_text_fmt;
			size_t pos = display_text.find(TEXT("\\n"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 2, TEXT("\n"));
			pos = display_text.find(TEXT("{fps}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 5, fps_text);
			pos = display_text.find(TEXT("{time}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 6, time_text);
		}
		pFont->DrawText(NULL, display_text.c_str(), (int)display_text.length(), &rTextShadow, formatFlag, color_shadow);
		pFont->DrawText(NULL, display_text.c_str(), (int)display_text.length(), &rText, formatFlag, color_text);
	}
};

static std::map<LPDIRECT3DDEVICE9, D3DXCustomPresent> cp;

void CustomPresent(LPDIRECT3DDEVICE9 p)
{
	if (cp.find(p) == cp.end())
	{
		cp.insert(std::make_pair(p, D3DXCustomPresent()));
		cp[p].Init(p);
	}
	cp[p].Draw();
}