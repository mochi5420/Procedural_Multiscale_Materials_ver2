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
//	�}�N���̒�`
//------------------------------------------------------------------------------------------
#define ALIGN( alignment ) _declspec(align(alignment))


//------------------------------------------------------------------------------------------
//	�萔��`
//------------------------------------------------------------------------------------------
static const int WINDOW_WIDTH = 512;	//�E�B���h�E��
static const int WINDOW_HEIGHT = 512;	//�E�B���h�E����
static const wchar_t* const APP_NAME = L"Window";	//Window�̃^�C�g��
static const int DRAW_GLINT = 0;


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

private:

	//=============================================================================================
	//	�\����
	//=============================================================================================
	struct Vertex
	{
		D3DXVECTOR4 Position;
		D3DXVECTOR3 Normal;
		D3DXVECTOR2 UV;

	};


	//=============================================================================================
	//	�R���X�^���g�o�b�t�@
	//=============================================================================================
	struct ConstantBuffer
	{
		ALIGN(16) D3DXMATRIX  WVP;
		ALIGN(16) D3DXVECTOR3  lightPos;
		//ALIGN(16) float		  time;
		//ALIGN(16) D3DXVECTOR2 mouse;
		//ALIGN(16) D3DXVECTOR2 roughness;
		//ALIGN(16) D3DXVECTOR2 microRoughness;
		//ALIGN(16) float variation;
		//ALIGN(16) float density;
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
	ComPtr<ID3D11RasterizerState>	m_pRasterizerState;
	ComPtr<ID3D11SamplerState>		m_pSamLinear;
	ComPtr<ID3D11InputLayout>		m_pVertexLayout[1];
	ComPtr<ID3D11VertexShader>		m_pVertexShader[1];
	ComPtr<ID3D11PixelShader>		m_pPixelShader[1];
	ComPtr<ID3D11Buffer>			m_pConstantBuffer[1];
	ComPtr<ID3D11Buffer>			m_pVertexBuffer;	

	D3DXMATRIX m_ViewMatrix;
	D3DXMATRIX m_ProjectionMatrix;

	//=============================================================================================
	//	private methods
	//=============================================================================================
	bool InitWindow();
	bool InitD3D();
	bool InitShader();
	void MainLoop();
	void OnRender();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif // !_APP_H_