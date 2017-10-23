#ifndef _APP_H_
#define _APP_H_

//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include <stdio.h>
#include <windows.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <wrl.h>
#include <memory>


//------------------------------------------------------------------------------------------
//	Linker
//------------------------------------------------------------------------------------------
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")


//------------------------------------------------------------------------------------------
//	マクロの定義
//------------------------------------------------------------------------------------------
#define ALIGN( alignment ) _declspec(align(alignment))


//------------------------------------------------------------------------------------------
//	定数定義
//------------------------------------------------------------------------------------------
static const int WINDOW_WIDTH = 720;	//ウィンドウ幅
static const int WINDOW_HEIGHT = 720;	//ウィンドウ高さ
static const wchar_t* const APP_NAME = L"Window";	//Windowのタイトル
static const int DRAW_GLINT = 0;
static const int DRAW_SKY = 1;
static const float PI = 3.1415926536f;
//static const LPSTR OBJ_FILE = "Resources/Obj/chess.obj";
static const LPSTR OBJ_FILE = "Resources/Obj/VWUP.obj";
static const wchar_t* const CUBE_MAP = L"Resources/Textures/sunsetcube1024.dds";


//------------------------------------------------------------------------------------------
//	App class
//------------------------------------------------------------------------------------------
using namespace Microsoft::WRL;

class App
{
public:
	App();
	~App();

	int Run();
	

protected:
	static LRESULT CALLBACK WndProcWrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


private:
	//=============================================================================================
	//	構造体
	//=============================================================================================
	struct Vertex
	{
		D3DXVECTOR4 Position;
		D3DXVECTOR3 Normal;
	};

	struct Vertex3
	{
		Vertex3() {}
		Vertex3(float x, float y, float z,
			float nx, float ny, float nz,
			float u, float v)
			: pos(x, y, z), normal(nx, ny, nz), texCoord(u, v) {}

		D3DXVECTOR3 pos;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texCoord;
	};

	//=============================================================================================
	//	コンスタントバッファ
	//=============================================================================================
	struct ConstantBufferGlint
	{
		ALIGN(16) D3DXMATRIX  WVP;
		ALIGN(16) D3DXMATRIX  W;
		ALIGN(16) D3DXVECTOR3 cameraPos;
		ALIGN(16) D3DXVECTOR3 lightPos;
		ALIGN(16) D3DXVECTOR2 roughness;
		ALIGN(16) D3DXVECTOR2 microRoughness;
		ALIGN(16) float variation;
		ALIGN(16) float density;
		ALIGN(16) float searchConeAngle;
		ALIGN(16) float dynamicRange;
		ALIGN(16) float glintsBrightness;
		ALIGN(16) float shadingBribhtness;
	};	

	struct ConstantBufferSky
	{
		ALIGN(16) D3DXMATRIX  WVP;
		ALIGN(16) D3DXMATRIX  W;
		ALIGN(16) D3DXVECTOR3  cameraPos;
		ALIGN(16) D3DXVECTOR3  lightPos;
	};
		
	//=============================================================================================
	//	データ
	//=============================================================================================
	HWND m_hWnd;
	
	ComPtr<ID3D11Device>			m_pDevice;			
	ComPtr<ID3D11DeviceContext>		m_pDeviceContext;	
	ComPtr<IDXGISwapChain>			m_pSwapChain;			
	ComPtr<ID3D11RenderTargetView>	m_pRenderTargetView;
	ComPtr<ID3D11DepthStencilView>	m_pDepthStencilView;
	ComPtr<ID3D11Texture2D>			m_pDepthStencil;
	ComPtr<ID3D11RasterizerState>	m_pRasterizerState[2];
	ComPtr<ID3D11SamplerState>		m_pSamLinear;
	ComPtr<ID3D11InputLayout>		m_pVertexLayout[2];
	ComPtr<ID3D11VertexShader>		m_pVertexShader[2];
	ComPtr<ID3D11PixelShader>		m_pPixelShader[2];
	ComPtr<ID3D11Buffer>			m_pConstantBuffer[2];
	ComPtr<ID3D11Buffer>			m_pVertexBuffer[2];	
	ComPtr<ID3D11Buffer>            m_pIndexBuffer[2];
	DWORD							NumFace[2];

	//環境マップ
	ComPtr<ID3D11Texture2D>			 m_pCubeMapTexture;
	ComPtr<ID3D11ShaderResourceView> m_pCubeMapSRV;
	ComPtr<ID3D11SamplerState>		 m_pCubeMapSamplerState;

	//Matrix
	D3DXMATRIX m_ViewMatrix;
	D3DXMATRIX m_ProjectionMatrix;

	//=============================================================================================
	//	private methods
	//=============================================================================================
	bool InitWindow();
	bool InitAntTweakBar();
	bool InitD3D();
	bool InitShader();
	void ResizeWindow(LPARAM lParam);
	void MainLoop();
	void OnRender();

	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif // !_APP_H_