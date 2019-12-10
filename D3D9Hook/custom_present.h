#pragma once
#include<d3d9.h>
#ifdef __cplusplus
extern "C" {
#endif
//自定义Present的附加操作
void CustomPresent(LPDIRECT3DDEVICE9);
DWORD GetDLLPath(LPTSTR path, DWORD max_length);
#ifdef __cplusplus
}
#endif
