#ifndef _SCREEN_SHOT_H_
#define _SCREEN_SHOT_H_

//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <tchar.h>
#include <shlwapi.h>
#include <wrl.h>


//------------------------------------------------------------------------------------------
//	Linker
//------------------------------------------------------------------------------------------
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment( lib, "Shlwapi.lib" ) // PathFileExists Ç»Ç«ÇÃ API ä÷êîÇégÇ¢ÇΩÇ¢


//------------------------------------------------------------------------------------------
//	ObjMeshLoader class
//------------------------------------------------------------------------------------------
class ScreenShot
{
public:
	ScreenShot();
	~ScreenShot();
	HRESULT CreateScreenShot(ID3D11Device* device, ID3D11DeviceContext* device_context, IDXGISwapChain* swap_chain, HWND hWnd);
	HRESULT CreateScreenShotFromClipBoard(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hWnd);

private:


};
#endif // !_SCREEN_SHOT_H_