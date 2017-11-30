#ifndef _APP_H_
#define _APP_H_

//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>
#include <wrl.h>


//------------------------------------------------------------------------------------------
//	Linker
//------------------------------------------------------------------------------------------
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")


//------------------------------------------------------------------------------------------
//	�}�N���̒�`
//------------------------------------------------------------------------------------------
#define ALIGN( alignment ) _declspec(align(alignment))


//------------------------------------------------------------------------------------------
//	�萔��`
//------------------------------------------------------------------------------------------
static const int WINDOW_WIDTH = 720;	//�E�B���h�E��
static const int WINDOW_HEIGHT = 720;	//�E�B���h�E����
static const wchar_t* const APP_NAME = L"Window";	//Window�̃^�C�g��
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
	//	�\����
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

	enum Objects
	{
		OBJ,
		SKY,
		QUAD,
		CUBE,
		OBJECT_NUM
	};

	enum Shaders
	{
		DRAW_GLINT,
		DRAW_SKY,
		SHADER_NUM
	};

	//=============================================================================================
	//	�R���X�^���g�o�b�t�@
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
		ALIGN(16) int  cubeMap;
	};	

	struct ConstantBufferSky
	{
		ALIGN(16) D3DXMATRIX  WVP;
		ALIGN(16) D3DXMATRIX  W;
		ALIGN(16) D3DXVECTOR3  cameraPos;
		ALIGN(16) D3DXVECTOR3  lightPos;
	};
		
	//=============================================================================================
	//	�f�[�^
	//=============================================================================================
	HWND m_hWnd;
	
	ComPtr<ID3D11Device>			m_pDevice;			
	ComPtr<ID3D11DeviceContext>		m_pDeviceContext;	
	ComPtr<IDXGISwapChain>			m_pSwapChain;			
	ComPtr<ID3D11RenderTargetView>	m_pRenderTargetView;
	ComPtr<ID3D11DepthStencilView>	m_pDepthStencilView;
	ComPtr<ID3D11Texture2D>			m_pDepthStencil;
	ComPtr<ID3D11BlendState>		m_pBlendState[SHADER_NUM];
	ComPtr<ID3D11RasterizerState>	m_pRasterizerState[SHADER_NUM];
	ComPtr<ID3D11SamplerState>		m_pSamLinear;
	ComPtr<ID3D11InputLayout>		m_pVertexLayout[SHADER_NUM];
	ComPtr<ID3D11VertexShader>		m_pVertexShader[SHADER_NUM];
	ComPtr<ID3D11PixelShader>		m_pPixelShader[SHADER_NUM];
	ComPtr<ID3D11Buffer>			m_pConstantBuffer[SHADER_NUM];
	ComPtr<ID3D11Buffer>			m_pVertexBuffer[OBJECT_NUM];
	ComPtr<ID3D11Buffer>            m_pIndexBuffer[OBJECT_NUM];
	DWORD							NumFace[OBJECT_NUM];

	//���}�b�v
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