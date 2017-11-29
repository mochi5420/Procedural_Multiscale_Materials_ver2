//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include "App.h"
#include "ObjMeshLoader.h"
#include "Sphere.h"
#include "FPS.h"
#include <vector>
#include <AntTweakBar.h>

//------------------------------------------------------------------------------------------
//	global variables
//------------------------------------------------------------------------------------------
static D3DXVECTOR3 g_LightDir(-2.82f, 2.24f, 3.35f);
static D3DXQUATERNION g_ObjRotation;
static float g_ObjScale = 1.0f / 200.0f;

static D3DXVECTOR2 g_Roughness(0.25f, 0.25f);	//Global Roughness
static D3DXVECTOR2 g_MicroRoughness(0.02f, 0.02f);	//Micro Roughness
static float g_Variation = 860.0f;	//Variation
static float g_Density = 13.0f;	//Density
static float g_SearchConeAngle = 0.01f; 	//SearchConeAngle
static float g_DynamicRange = 10.0f;	//dynamicRange
static float g_GlintsBrightness = 2.0f;
static float g_ShadingBribhtness = 7.0f;


//------------------------------------------------------------------------------------------
//	constructor & destructor
//------------------------------------------------------------------------------------------
App::App()
{
	SecureZeroMemory(this, sizeof(App));	//�N���X���̂��ׂĂ̕ϐ����[��������
}
App::~App()
{
}


//------------------------------------------------------------------------------------------
//	�A�v���P�[�V�����̎��s
//------------------------------------------------------------------------------------------
int App::Run()
{
	if (FAILED(InitWindow()))	//Initialize the window
	{
		return 0;
	}

	if (FAILED(InitD3D()))	//Initialize Direct3D
	{
		return 0;
	}
	
	if (FAILED(InitAntTweakBar()))	//Initialize AntTweakBar
	{
		return 0;
	}

	if (FAILED(InitShader()))	//Initialize Shader
	{
		return 0;
	}

	MainLoop();	//���C�����[�v��

	TwTerminate();	//AntTweakBar�̏I��

	return 0;	//�A�v���I��
}


//------------------------------------------------------------------------------------------
//	�E�B���h�E�v���V�[�W���[
//------------------------------------------------------------------------------------------
LRESULT App::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Send event message to AntTweakBar
	if (TwEventWin(hWnd, uMsg, wParam, lParam))
	{
		return 0; // Event has been handled by AntTweakBar

	}

	switch (uMsg)
	{
		case WM_SIZE: // Window size has been changed
		{
			ResizeWindow(lParam);
			break;
		}
		case WM_KEYDOWN:
		{
			switch ((char)wParam)
			{
				case VK_ESCAPE:	//ESC�L�[�ŏC��
					PostQuitMessage(0);
					break;
			}

			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;

		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK App::WndProcWrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//static�Ȋ֐����烁���o���ĂԂ��Ƃ͂ł��Ȃ��B
	
	//�����Ō��X�̃v���V�[�W�A�̃A�h���X���擾����B
	App* instance = (App*)(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (!instance) {//�擾�ł��Ȃ������ꍇ�i�܂���肽�Ăł���j
		if (message == WM_CREATE) {
			// Save the DXSample* passed in to CreateWindow.
			LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(pCreateStruct->lpCreateParams));
			
		}
	}
	else {//�擾�ł����ꍇ�i�����������j
		LRESULT ret = instance->WndProc(hWnd, message, wParam, lParam);
		return ret;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


//------------------------------------------------------------------------------------------
//	�E�B���h�E�쐬
//------------------------------------------------------------------------------------------
bool App::InitWindow()
{
	// �C���X�^���X�n���h�����擾.
	HINSTANCE hInst = GetModuleHandle(nullptr);
	if (!hInst)
	{
		return FALSE;
	}

	// �E�B���h�E�̒�`
	WNDCLASSEX  wc;
	SecureZeroMemory(&wc, sizeof(wc));
	wc.cbSize			= sizeof(WNDCLASSEX);					// �\���̂̑傫��
	wc.style			= CS_HREDRAW | CS_VREDRAW;				// �X�^�C�� 
	wc.lpfnWndProc		= WndProcWrapper;						// ���b�Z�[�W�����֐�
	wc.hInstance		= hInst;								// �v���O�����̃n���h��
	wc.hIcon			= LoadIcon(nullptr, IDI_APPLICATION);	// �A�C�R��
	wc.hCursor			= LoadCursor(nullptr, IDC_ARROW);		// �J�[�\��
	wc.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);	// �u���V
	wc.lpszMenuName		= nullptr;								// ���j���[
	wc.lpszClassName	= APP_NAME;								// �N���X��
	wc.hIconSm			= LoadIcon(nullptr, IDI_APPLICATION);

	RegisterClassEx(&wc);	//�E�C���h�E�N���X��o�^

	// �E�B���h�E�̍쐬
	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	m_hWnd = CreateWindow(
		APP_NAME,						// �E�C���h�E�N���X�� 
		APP_NAME,						// �E�C���h�E�̃^�C�g��
		WS_OVERLAPPEDWINDOW,			// �E�C���h�E�X�^�C�� 
		CW_USEDEFAULT, CW_USEDEFAULT,	// �E�C���h�E�\���ʒu 
		rc.right - rc.left, 			// �E�C���h�E�̑傫�� 
		rc.bottom - rc.top,				// �E�C���h�E�̑傫�� 
		nullptr,						// �e�E�C���h�E�̃n���h�� 
		nullptr,						// ���j���[�̃n���h�� 
		hInst,							// �C���X�^���X�̃n���h��
		this);							// �쐬���̈����ۑ��p�|�C���^ 

	// �G���[�`�F�b�N
	if (!m_hWnd)
	{
		return E_FAIL;
	}

	//�E�C���h�E�̕\��
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}


bool App::InitAntTweakBar() {

	// Initialize AntTweakBar
	if (!TwInit(TW_DIRECT3D11, m_pDevice.Get()))
	{
		return false;
	}

	// Create a tweak bar
	TwBar *bar = TwNewBar("TweakBar");
	TwDefine(" TweakBar label='Glints UI' ");
	TwDefine(" GLOBAL help='Real-time Rendering of Procedural Multiscale Materials (2016)' "); // Message added to the help bar.
	TwDefine(" TweakBar size='300 400' color='41 126 231' alpha=0"); // change default tweak bar size and color
	TwDefine(" GLOBAL fontsize=2 ");
	TwDefine(" TweakBar valueswidth=130 "); // column width fits content

	// Add variables to the tweak bar
	TwAddVarRW(bar, "Global Roughness U", TW_TYPE_FLOAT, &g_Roughness.x, "min=0 max=1 step=0.001 group=Paramaters ");
	TwAddVarRW(bar, "Global Roughness V", TW_TYPE_FLOAT, &g_Roughness.y, "min=0 max=1 step=0.001 group=Paramaters ");
	TwAddVarRW(bar, "Micro Roughness U", TW_TYPE_FLOAT, &g_MicroRoughness.x, "min=0 max=1 step=0.001 group=Paramaters ");
	TwAddVarRW(bar, "Micro Roughness V", TW_TYPE_FLOAT, &g_MicroRoughness.y, "min=0 max=1 step=0.001 group=Paramaters ");
	TwAddVarRW(bar, "Variation", TW_TYPE_FLOAT, &g_Variation, "min=0 max=10000 keyincr=V keydecr=v group=Paramaters ");
	TwAddVarRW(bar, "Log Density", TW_TYPE_FLOAT, &g_Density, "min=0 max=20 step=0.01 keyincr=D keydecr=d group=Paramaters ");
	TwAddVarRW(bar, "Search Cone Angle", TW_TYPE_FLOAT, &g_SearchConeAngle, "min=0 max=1 step=0.001 keyincr=S keydecr=s group=Paramaters ");
	TwAddVarRW(bar, "Dynamic Range", TW_TYPE_FLOAT, &g_DynamicRange, "min=0 max=10000 step=10.0 group=Paramaters ");
	TwAddVarRW(bar, "Shading Bribhtness", TW_TYPE_FLOAT, &g_ShadingBribhtness, "min=0 max=100 step=0.1 group=Paramaters ");
	TwAddVarRW(bar, "Glints Brightness", TW_TYPE_FLOAT, &g_GlintsBrightness, "min=0 max=100 step=0.1 group=Paramaters ");

	TwAddVarRW(bar, "Light direction", TW_TYPE_DIR3F, &g_LightDir, "opened=true axisz=-z showval=false");
	TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &g_ObjRotation, "opened=true axisz=-z group=Object ");
	TwAddVarRW(bar, "Scale", TW_TYPE_FLOAT, &g_ObjScale, "min=0.0 max=100.0 step=0.001 group=Object ");

}

//------------------------------------------------------------------------------------------
//	Initializes Direct3D
//------------------------------------------------------------------------------------------
bool App::InitD3D()
{
	HRESULT hr = S_OK;

	// Get window size
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// �f�o�C�X�ƃX���b�v�`�F�[���̍쐬
	DXGI_SWAP_CHAIN_DESC sd;
	SecureZeroMemory(&sd, sizeof(sd)); //������
	sd.BufferCount							= 1;
	sd.BufferDesc.Width						= width;							//�T�C�Y�w��
	sd.BufferDesc.Height					= height;							//�T�C�Y�w��
	sd.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;		//32bit color
	sd.BufferDesc.RefreshRate.Numerator		= 60;								//���t���b�V�����[�g
	sd.BufferDesc.RefreshRate.Denominator	= 1;								//�o�b�N�o�b�t�@�̐�
	sd.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow							= m_hWnd;
	sd.SampleDesc.Count						= 1;
	sd.SampleDesc.Quality					= 0;
	sd.Windowed								= TRUE;								//Fullscreen�\���̎w��


	D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
	
	hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			&pFeatureLevels,
			1,
			D3D11_SDK_VERSION,
			&sd,
			m_pSwapChain.GetAddressOf(),
			m_pDevice.GetAddressOf(),
			nullptr,
			&m_pDeviceContext);

	if (FAILED(hr))
	{
		return false;
	}


	//�����_�[�^�[�Q�b�g�r���[�̍쐬
	ComPtr<ID3D11Texture2D> pBackBuffer;	//�󂯎M
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)pBackBuffer.GetAddressOf());
	m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf());


	//�[�x�X�e���V���r���[�̍쐬
	D3D11_TEXTURE2D_DESC descDepth;
	SecureZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width					= width;
	descDepth.Height				= height;
	descDepth.MipLevels				= 1;
	descDepth.ArraySize				= 1;
	descDepth.Format				= DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count		= 1;
	descDepth.SampleDesc.Quality	= 0;
	descDepth.Usage					= D3D11_USAGE_DEFAULT;
	descDepth.BindFlags				= D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags		= 0;
	descDepth.MiscFlags				= 0;

	m_pDevice->CreateTexture2D(&descDepth, nullptr, m_pDepthStencil.GetAddressOf());
	m_pDevice->CreateDepthStencilView(m_pDepthStencil.Get(), nullptr, m_pDepthStencilView.GetAddressOf());


	//���X�^���C�Y�ݒ�
	D3D11_RASTERIZER_DESC rdc;
	SecureZeroMemory(&rdc, sizeof(rdc));
	rdc.CullMode = D3D11_CULL_BACK;	
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = FALSE;

	m_pDevice->CreateRasterizerState(&rdc, m_pRasterizerState[DRAW_GLINT].GetAddressOf());

	rdc.CullMode = D3D11_CULL_NONE;
	m_pDevice->CreateRasterizerState(&rdc, m_pRasterizerState[DRAW_SKY].GetAddressOf());


	//Obj�̓ǂݍ���
	{
		ObjMeshLoader obj;

		hr = obj.CreateMesh(m_pDevice.Get(), m_pVertexBuffer[DRAW_GLINT].GetAddressOf(),
			m_pIndexBuffer[DRAW_GLINT].GetAddressOf(), NumFace[DRAW_GLINT], OBJ_FILE);

		if (FAILED(hr))
		{
			return false;
		}
	}
	

	//Debug�p�̔|��
	//{
	//	Vertex vertices[] =
	//	{
	//		D3DXVECTOR4(-1.0, -1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
	//		D3DXVECTOR4(-1.0, 1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
	//		D3DXVECTOR4(1.0, -1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
	//		D3DXVECTOR4(1.0, 1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
	//	};

	//	D3D11_BUFFER_DESC bd;
	//	bd.Usage = D3D11_USAGE_DEFAULT;
	//	bd.ByteWidth = sizeof(Vertex) * 4;
	//	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//	bd.CPUAccessFlags = 0;
	//	bd.MiscFlags = 0;

	//	D3D11_SUBRESOURCE_DATA InitData;
	//	InitData.pSysMem = vertices;

	//	hr = m_pDevice->CreateBuffer(&bd, &InitData, m_pVertexBuffer.GetAddressOf());

	//	if (FAILED(hr))
	//	{
	//		return false;
	//	}
	//}


	//���}�b�v�̍쐬
	//Tell D3D we will be loading a cube texture
	D3DX11_IMAGE_LOAD_INFO loadinfo;
	loadinfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//Load the texture
	hr = D3DX11CreateTextureFromFile(m_pDevice.Get(), CUBE_MAP,
		&loadinfo, 0, (ID3D11Resource**)m_pCubeMapTexture.GetAddressOf(), 0);

	if (FAILED(hr))
	{
		return false;
	}

	//Create the textures description
	D3D11_TEXTURE2D_DESC tdesc;
	m_pCubeMapTexture->GetDesc(&tdesc);

	//Tell D3D We have a cube texture, which is an array of 2D textures
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.Format = tdesc.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvd.TextureCube.MipLevels = tdesc.MipLevels;
	srvd.TextureCube.MostDetailedMip = 0;

	//Create the Resource view
	hr = m_pDevice->CreateShaderResourceView(m_pCubeMapTexture.Get(), &srvd, m_pCubeMapSRV.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	//Describe the Sample State
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Create the Sample State
	hr = m_pDevice->CreateSamplerState(&sampDesc, m_pCubeMapSamplerState.GetAddressOf());

	//Create Sky Sphere
	{
		Sphere sphere;

		hr = sphere.CreateSphere(m_pDevice.Get(), m_pVertexBuffer[DRAW_SKY].GetAddressOf(),
			m_pIndexBuffer[DRAW_SKY].GetAddressOf(), NumFace[DRAW_SKY], 10, 10);

		if (FAILED(hr))
		{
			return false;
		}
	}

	return true;
}


//------------------------------------------------------------------------------------------
//	Initialize Shader
//------------------------------------------------------------------------------------------
bool App::InitShader()
{
	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> pCompiledShader = nullptr;
	ComPtr<ID3DBlob> pErrors = nullptr;

	UINT Flag1 = D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR | D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	Flag1 |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION | D3D10_SHADER_OPTIMIZATION_LEVEL0;
#else
	Flag1 |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
#endif


	// ---------------------------------------------------------------------
	//DRAW_GLINT �̃V�F�[�_�[�̏�����
	// ---------------------------------------------------------------------	
	//�u���u����DRAW_GLINT�o�[�e�b�N�X�V�F�[�_�[�쐬
	hr = D3DX11CompileFromFile(
			L"Resources/Shaders/DrawGlint.hlsl",
			nullptr,
			nullptr,
			"VS",
			"vs_5_0",
			Flag1,
			0,
			nullptr,
			pCompiledShader.GetAddressOf(),
			pErrors.GetAddressOf(),
			nullptr);

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (LPSTR)pErrors->GetBufferPointer(), nullptr, MB_OK);
		return false;
	}

	hr = m_pDevice->CreateVertexShader(
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			nullptr,
			m_pVertexShader[DRAW_GLINT].GetAddressOf());

	if (FAILED(hr))
	{
		return false;
	}


	//DRAW_GLINT�V�F�[�_�[�p�@���_�C���v�b�g���C�A�E�g���쐬
	D3D11_INPUT_ELEMENT_DESC layoutGlint[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	int numElements = sizeof(layoutGlint) / sizeof(layoutGlint[0]);

	hr = m_pDevice->CreateInputLayout(
			layoutGlint,
			numElements,
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			m_pVertexLayout[DRAW_GLINT].GetAddressOf());

	if (FAILED(hr))
	{
		return false;
	}


	//�u���u����DRAW_GLINT�s�N�Z���V�F�[�_�[�쐬
	hr = D3DX11CompileFromFile(
			L"Resources/Shaders/DrawGlint.hlsl",
			nullptr,
			nullptr,
			"PS",
			"ps_5_0",
			Flag1,
			0,
			nullptr,
			pCompiledShader.GetAddressOf(),
			pErrors.GetAddressOf(),
			nullptr);

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (LPSTR)pErrors->GetBufferPointer(), nullptr, MB_OK);
		return false;
	}

	hr = m_pDevice->CreatePixelShader(
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			nullptr,
			m_pPixelShader[DRAW_GLINT].GetAddressOf());

	if (FAILED(hr))
	{
		return false;
	}


	//�R���X�^���g�o�b�t�@�[�쐬
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = sizeof(ConstantBufferGlint);
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pConstantBuffer[DRAW_GLINT]);

	if (FAILED(hr))
	{
		return false;
	}


	// ---------------------------------------------------------------------
	//DRAW_SKY�@�V�F�[�_�[�̏�����
	// ---------------------------------------------------------------------	
	//�u���u����DRAW_SKY�o�[�e�b�N�X�V�F�[�_�[�쐬
	hr = D3DX11CompileFromFile(
		L"Resources/Shaders/DrawSky.hlsl",
		nullptr,
		nullptr,
		"VS",
		"vs_5_0",
		Flag1,
		0,
		nullptr,
		pCompiledShader.GetAddressOf(),
		pErrors.GetAddressOf(),
		nullptr);

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (LPSTR)pErrors->GetBufferPointer(), nullptr, MB_OK);
		return false;
	}

	hr = m_pDevice->CreateVertexShader(
		pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(),
		nullptr,
		m_pVertexShader[DRAW_SKY].GetAddressOf());

	if (FAILED(hr))
	{
		return false;
	}


	//DRAW_SKY�V�F�[�_�[�p�@���_�C���v�b�g���C�A�E�g���쐬
	D3D11_INPUT_ELEMENT_DESC layoutSky[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	numElements = sizeof(layoutSky) / sizeof(layoutSky[0]);

	hr = m_pDevice->CreateInputLayout(
		layoutSky,
		numElements,
		pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(),
		m_pVertexLayout[DRAW_SKY].GetAddressOf());

	if (FAILED(hr))
	{
		return false;
	}


	//�u���u����DRAW_SKY�s�N�Z���V�F�[�_�[�쐬
	hr = D3DX11CompileFromFile(
		L"Resources/Shaders/DrawSky.hlsl",
		nullptr,
		nullptr,
		"PS",
		"ps_5_0",
		Flag1,
		0,
		nullptr,
		pCompiledShader.GetAddressOf(),
		pErrors.GetAddressOf(),
		nullptr);

	if (FAILED(hr))
	{
		MessageBoxA(m_hWnd, (LPSTR)pErrors->GetBufferPointer(), nullptr, MB_OK);
		return false;
	}

	hr = m_pDevice->CreatePixelShader(
		pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(),
		nullptr,
		m_pPixelShader[DRAW_SKY].GetAddressOf());

	if (FAILED(hr))
	{
		return false;
	}

	//�R���X�^���g�o�b�t�@�[�쐬
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = sizeof(ConstantBufferSky);
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pConstantBuffer[DRAW_SKY]);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}


//------------------------------------------------------------------------------------------
//	�E�B���h�E�T�C�Y���ς�������̏���
//------------------------------------------------------------------------------------------
void App::ResizeWindow(LPARAM lParam)
{

	if (m_pDevice.Get()) // Resize D3D render target
	{
		// Release render target and depth-stencil view
		ID3D11RenderTargetView *nullRTV = NULL;
		m_pDeviceContext->OMSetRenderTargets(1, &nullRTV, NULL);
		if (m_pRenderTargetView.Get())
		{
			m_pRenderTargetView.Reset();
			m_pRenderTargetView = nullptr;
		}
		if (m_pDepthStencilView.Get())
		{
			m_pDepthStencilView.Reset();
			m_pDepthStencilView = nullptr;
		}

		if (m_pSwapChain.Get())
		{
			// Resize swap chain
			DXGI_SWAP_CHAIN_DESC sd;
			SecureZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 1;
			sd.BufferDesc.Width = LOWORD(lParam);				
			sd.BufferDesc.Height = HIWORD(lParam);				
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	
			sd.BufferDesc.RefreshRate.Numerator = 60;			
			sd.BufferDesc.RefreshRate.Denominator = 1;			
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = m_hWnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;							

			m_pSwapChain->ResizeBuffers(sd.BufferCount, sd.BufferDesc.Width,
										sd.BufferDesc.Height, sd.BufferDesc.Format,
										sd.Flags);

			// Re-create a render target and depth-stencil view
			ID3D11Texture2D *backBuffer = NULL, *dsBuffer = NULL;
			m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
			m_pDevice->CreateRenderTargetView(backBuffer, NULL, m_pRenderTargetView.GetAddressOf());
			backBuffer->Release();

			D3D11_TEXTURE2D_DESC descDepth;
			SecureZeroMemory(&descDepth, sizeof(descDepth));
			descDepth.Width = sd.BufferDesc.Width;
			descDepth.Height = sd.BufferDesc.Height;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = DXGI_FORMAT_D32_FLOAT;
			descDepth.SampleDesc.Count = 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags = 0;
			descDepth.MiscFlags = 0;

			m_pDevice->CreateTexture2D(&descDepth, NULL, &dsBuffer);
			m_pDevice->CreateDepthStencilView(dsBuffer, NULL, m_pDepthStencilView.GetAddressOf());
			dsBuffer->Release();
			m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

			// Setup the viewport
			D3D11_VIEWPORT vp;
			vp.Width = (float)sd.BufferDesc.Width;
			vp.Height = (float)sd.BufferDesc.Height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			m_pDeviceContext->RSSetViewports(1, &vp);
		}
	}
}



//------------------------------------------------------------------------------------------
//	�A�v���P�[�V���������B�A�v���̃��C���֐��B
//------------------------------------------------------------------------------------------
void App::MainLoop()
{
	// ���b�Z�[�W���[�v
	MSG msg = { 0 };
	SecureZeroMemory(&msg, sizeof(msg));

	FPS fps;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//render
			OnRender();
			fps.COUNTER(m_hWnd);
		}
	}	
}


//------------------------------------------------------------------------------------------
//	�`�揈��
//------------------------------------------------------------------------------------------
void App::OnRender()
{
	//camera�̈ړ�
	static float cameraRotX = PI * 0.1f;
	static float cameraRotY = -PI * 0.15f;
	static float camZoom = 0.0f;
	if (GetKeyState('C') & 0x80) {

		//Rotation
		if (GetKeyState(VK_UP) & 0x80) {
			cameraRotX += 0.0075f;
		}
		if (GetKeyState(VK_DOWN) & 0x80) {
			cameraRotX -= 0.0075f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80) {
			cameraRotY -= 0.0075f;
		}
		if (GetKeyState(VK_LEFT) & 0x80) {
			cameraRotY += 0.0075f;
		}

		//Zoom
		if (GetKeyState('I') & 0x80) {
			camZoom += 0.01f;
		}
		if (GetKeyState('O') & 0x80) {
			camZoom -= 0.01f;
		}
	}


	//camera�̈ړ��ϊ��s��
	D3DXMATRIX camWorldMatrix, camRotXMatrix, camRotYMatrix, camZoomMatrix;
	D3DXMatrixTranslation(&camZoomMatrix, 0, 0, camZoom);
	D3DXMatrixRotationX(&camRotXMatrix, cameraRotX);
	D3DXMatrixRotationY(&camRotYMatrix, cameraRotY);

	camWorldMatrix = camZoomMatrix* camRotXMatrix * camRotYMatrix;


	//�r���[�s��
	D3DXVECTOR3 cameraPos(0.0f, 0.0f, -5.0f);	//�J�����ʒu
	D3DXVec3TransformCoord(&cameraPos, &cameraPos, &camWorldMatrix);
	D3DXVECTOR3 lookAtPos(0.0f, 0.25f, 0.0f);	//�����ʒu
	D3DXVECTOR3 upVec(0.0f, 1.0f, 0.0f);		//����ʒu
	//D3DXVec3TransformCoord(&upVec, &upVec, &camWorldMatrix);
	D3DXMatrixLookAtLH(&m_ViewMatrix, &cameraPos, &lookAtPos, &upVec);

	// Get window size
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	//�v���W�F�N�V�����s��i�ˉe�ϊ��j
	D3DXMatrixPerspectiveFovLH(
		&m_ProjectionMatrix,
		(float)D3DX_PI / 4.0,							//����p
		(float)width / (float)height,		//�A�X�y�N�g��
		0.1f,											//near clip
		1000.0f);										//far clip

	float ClearColor[4] = { 0.178, 0.178, 0.178, 1 };// �N���A�F�쐬�@RGBA�̏�


	//�r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT vp;
	vp.Width	= (float)width;
	vp.Height	= (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	m_pDeviceContext->RSSetViewports(1, &vp);


	//------------------------------------------------------------------------------------------
	//	Draw Object
	//------------------------------------------------------------------------------------------

	//�����_�[�^�[�Q�b�g�r���[�Ɛ[�x�X�e���V���r���[���p�C�v���C���Ƀo�C���h
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);	//��ʃN���A
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);	//�[�x�o�b�t�@�N���A


	//���f����World�ϊ��s��
	D3DXMATRIX WorldMatrix, ScallMatrix, RotateMatrix;
	//D3DXMatrixScaling(&ScallMatrix, 2.0, 2.0f, 2.0f);
	D3DXMatrixScaling(&ScallMatrix, g_ObjScale, g_ObjScale, g_ObjScale);
	D3DXMatrixRotationQuaternion(&RotateMatrix, &g_ObjRotation);
	WorldMatrix = ScallMatrix * RotateMatrix;

	//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�[�Ɋe��f�[�^��n��
	ConstantBufferGlint cbg;
	D3D11_MAPPED_SUBRESOURCE pData;

	if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBuffer[DRAW_GLINT].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//WVP�s����V�F�[�_�[�ɓn��
		cbg.WVP = WorldMatrix * m_ViewMatrix*m_ProjectionMatrix;
		D3DXMatrixTranspose(&cbg.WVP, &cbg.WVP);

		cbg.W = WorldMatrix;
		D3DXMatrixTranspose(&cbg.W, &cbg.W);

		cbg.cameraPos = cameraPos;
		cbg.lightPos = g_LightDir;
		  
		cbg.roughness = g_Roughness;
		cbg.microRoughness = g_MicroRoughness;
		cbg.variation = g_Variation;
		cbg.density = pow(10, g_Density);
		cbg.searchConeAngle = g_SearchConeAngle;
		cbg.dynamicRange = g_DynamicRange;
		cbg.glintsBrightness = g_GlintsBrightness;
		cbg.shadingBribhtness = g_ShadingBribhtness;


		memcpy_s(pData.pData, pData.RowPitch, (void*)(&cbg), sizeof(cbg));
		m_pDeviceContext->Unmap(m_pConstantBuffer[DRAW_GLINT].Get(), 0);
	}

	//�o�[�e�b�N�X�o�b�t�@�[���Z�b�g
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer[DRAW_GLINT].GetAddressOf(), &stride, &offset);


	//�C���f�b�N�X�o�b�t�@�[���Z�b�g
	stride = sizeof(int);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer[DRAW_GLINT].Get(), DXGI_FORMAT_R32_UINT, 0);


	//���̃R���X�^���g�o�b�t�@�[���g���V�F�[�_�[�̓o�^
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_GLINT].GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_GLINT].GetAddressOf());


	//���_�C���v�b�g���C�A�E�g���Z�b�g
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout[DRAW_GLINT].Get());
	
	
	//�v���~�e�B�u�E�g�|���W�[���Z�b�g
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//���X�^���C�U�[�̃Z�b�g
	m_pDeviceContext->RSSetState(m_pRasterizerState[DRAW_GLINT].Get());

	//�g�p����V�F�[�_�[�̓o�^	
	m_pDeviceContext->VSSetShader(m_pVertexShader[DRAW_GLINT].Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader[DRAW_GLINT].Get(), nullptr, 0);
	
	
	//���}�b�v���Z�b�g
	m_pDeviceContext->PSSetShaderResources(0, 1, m_pCubeMapSRV.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(0, 1, m_pCubeMapSamplerState.GetAddressOf());

	//�v���~�e�B�u�������_�����O
	m_pDeviceContext->DrawIndexed(NumFace[DRAW_GLINT] * 3, 0, 0);
	//m_pDeviceContext->Draw(4, 0);		//Debug�Ŏg���|���p


	//------------------------------------------------------------------------------------------
	//	Draw Sky
	//------------------------------------------------------------------------------------------
	//���f����World�ϊ��s��
	D3DXMatrixScaling(&ScallMatrix, 20.0, 20.0f, 20.0f);
	WorldMatrix = ScallMatrix* RotateMatrix;
	
	//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�[�Ɋe��f�[�^��n��
	ConstantBufferSky cbs;

	if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBuffer[DRAW_SKY].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//WVP�s����V�F�[�_�[�ɓn��
		cbs.WVP = WorldMatrix * m_ViewMatrix*m_ProjectionMatrix;
		D3DXMatrixTranspose(&cbs.WVP, &cbs.WVP);

		cbs.W = WorldMatrix;
		D3DXMatrixTranspose(&cbs.W, &cbs.W);

		cbs.cameraPos = cameraPos;
		cbs.lightPos = g_LightDir;
		
		memcpy_s(pData.pData, pData.RowPitch, (void*)(&cbs), sizeof(cbs));
		m_pDeviceContext->Unmap(m_pConstantBuffer[DRAW_SKY].Get(), 0);
	}

	//�o�[�e�b�N�X�o�b�t�@�[���Z�b�g
	stride = sizeof(Vertex3);
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer[DRAW_SKY].GetAddressOf(), &stride, &offset);


	//�C���f�b�N�X�o�b�t�@�[���Z�b�g
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer[DRAW_SKY].Get(), DXGI_FORMAT_R32_UINT, 0);


	//���̃R���X�^���g�o�b�t�@�[���g���V�F�[�_�[�̓o�^
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_SKY].GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_SKY].GetAddressOf());


	//���_�C���v�b�g���C�A�E�g���Z�b�g
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout[DRAW_SKY].Get());


	//�v���~�e�B�u�E�g�|���W�[���Z�b�g
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//���X�^���C�U�[�̃Z�b�g
	m_pDeviceContext->RSSetState(m_pRasterizerState[DRAW_SKY].Get());

	//�g�p����V�F�[�_�[�̓o�^	
	m_pDeviceContext->VSSetShader(m_pVertexShader[DRAW_SKY].Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader[DRAW_SKY].Get(), nullptr, 0);

	//�e�N�X�`�����Z�b�g�i���}�b�v�Ɠ����j
	m_pDeviceContext->PSSetShaderResources(0, 1, m_pCubeMapSRV.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(0, 1, m_pCubeMapSamplerState.GetAddressOf());

	//�v���~�e�B�u�������_�����O
	m_pDeviceContext->DrawIndexed(NumFace[DRAW_SKY] * 3, 0, 0);

	//------------------------------------------------------------------------------------------
	// Draw tweak bars
	//------------------------------------------------------------------------------------------
	TwDraw();


	m_pSwapChain->Present(0, 0);	//��ʍX�V�i�o�b�N�o�b�t�@���t�����g�o�b�t�@�Ɂj	
}