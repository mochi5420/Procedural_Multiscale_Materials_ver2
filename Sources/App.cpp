//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include "App.h"
#include "ObjMeshLoader.h"
#include "Sphere.h"
#include "FPS.h"

//------------------------------------------------------------------------------------------
//	constructor & destructor
//------------------------------------------------------------------------------------------
App::App()
{
	SecureZeroMemory(this, sizeof(App));	//クラス内のすべての変数をゼロ初期化
}
App::~App()
{
}


//------------------------------------------------------------------------------------------
//	アプリケーションの実行
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
	
	if (FAILED(InitShader()))	//Initialize Shader
	{
		return 0;
	}

	MainLoop();	//メインループへ

	return 0;	//アプリ終了
}


//------------------------------------------------------------------------------------------
//	ウィンドウプロシージャー
//------------------------------------------------------------------------------------------
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		switch ((char)wParam)
		{
		case VK_ESCAPE:	//ESCキーで修了
			PostQuitMessage(0);
			break;
		}

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//------------------------------------------------------------------------------------------
//	ウィンドウ作成
//------------------------------------------------------------------------------------------
bool App::InitWindow()
{
	// インスタンスハンドルを取得.
	HINSTANCE hInst = GetModuleHandle(nullptr);
	if (!hInst)
	{
		return FALSE;
	}

	// ウィンドウの定義
	WNDCLASSEX  wc;
	SecureZeroMemory(&wc, sizeof(wc));
	wc.cbSize			= sizeof(WNDCLASSEX);					// 構造体の大きさ
	wc.style			= CS_HREDRAW | CS_VREDRAW;				// スタイル 
	wc.lpfnWndProc		= WndProc;								// メッセージ処理関数
	wc.hInstance		= hInst;								// プログラムのハンドル
	wc.hIcon			= LoadIcon(nullptr, IDI_APPLICATION);	// アイコン
	wc.hCursor			= LoadCursor(nullptr, IDC_ARROW);		// カーソル
	wc.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);	// ブラシ
	wc.lpszMenuName		= nullptr;								// メニュー
	wc.lpszClassName	= APP_NAME;								// クラス名
	wc.hIconSm			= LoadIcon(nullptr, IDI_APPLICATION);

	RegisterClassEx(&wc);	//ウインドウクラスを登録

	// ウィンドウの作成
	m_hWnd = CreateWindow(
		APP_NAME,				// ウインドウクラス名 
		APP_NAME,				// ウインドウのタイトル
		WS_OVERLAPPEDWINDOW,	// ウインドウスタイル 
		150, 150,				// ウインドウ表示位置 
		WINDOW_WIDTH, 			// ウインドウの大きさ 
		WINDOW_HEIGHT,			// ウインドウの大きさ 
		nullptr,				// 親ウインドウのハンドル 
		nullptr,				// メニューのハンドル 
		hInst,					// インスタンスのハンドル
		nullptr);				// 作成時の引数保存用ポインタ 

	// エラーチェック
	if (!m_hWnd)
	{
		return E_FAIL;
	}

	//ウインドウの表示
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}


//------------------------------------------------------------------------------------------
//	Initializes Direct3D
//------------------------------------------------------------------------------------------
bool App::InitD3D()
{
	HRESULT hr = S_OK;

	// デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;
	SecureZeroMemory(&sd, sizeof(sd)); //初期化
	sd.BufferCount							= 1;
	sd.BufferDesc.Width						= WINDOW_WIDTH;						//サイズ指定
	sd.BufferDesc.Height					= WINDOW_HEIGHT;					//サイズ指定
	sd.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;		//32bit color
	sd.BufferDesc.RefreshRate.Numerator		= 60;								//リフレッシュレート
	sd.BufferDesc.RefreshRate.Denominator	= 1;								//バックバッファの数
	sd.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow							= m_hWnd;
	sd.SampleDesc.Count						= 1;
	sd.SampleDesc.Quality					= 0;
	sd.Windowed								= TRUE;								//Fullscreen表示の指定


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


	//レンダーターゲットビューの作成
	ComPtr<ID3D11Texture2D> pBackBuffer;	//受け皿
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)pBackBuffer.GetAddressOf());
	m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf());


	//深度ステンシルビューの作成
	D3D11_TEXTURE2D_DESC descDepth;
	SecureZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width					= WINDOW_WIDTH;
	descDepth.Height				= WINDOW_HEIGHT;
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


	//ラスタライズ設定
	D3D11_RASTERIZER_DESC rdc;
	SecureZeroMemory(&rdc, sizeof(rdc));
	rdc.CullMode = D3D11_CULL_BACK;	
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = FALSE;

	m_pDevice->CreateRasterizerState(&rdc, m_pRasterizerState[DRAW_GLINT].GetAddressOf());

	rdc.CullMode = D3D11_CULL_NONE;
	m_pDevice->CreateRasterizerState(&rdc, m_pRasterizerState[DRAW_SKY].GetAddressOf());


	//Objの読み込み
	{
		ObjMeshLoader obj;

		hr = obj.CreateMesh(m_pDevice.Get(), m_pVertexBuffer[DRAW_GLINT].GetAddressOf(),
			m_pIndexBuffer[DRAW_GLINT].GetAddressOf(), NumFace[DRAW_GLINT], OBJ_FILE);

		if (FAILED(hr))
		{
			return false;
		}
	}
	

	//Debug用の板ポリ
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


	//環境マップの作成
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
	//DRAW_GLINT のシェーダーの初期化
	// ---------------------------------------------------------------------	
	//ブロブからDRAW_GLINTバーテックスシェーダー作成
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


	//DRAW_GLINTシェーダー用　頂点インプットレイアウトを作成
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


	//ブロブからDRAW_GLINTピクセルシェーダー作成
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


	//コンスタントバッファー作成
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
	//DRAW_SKY　シェーダーの初期化
	// ---------------------------------------------------------------------	
	//ブロブからDRAW_SKYバーテックスシェーダー作成
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


	//DRAW_SKYシェーダー用　頂点インプットレイアウトを作成
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


	//ブロブからDRAW_SKYピクセルシェーダー作成
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

	//コンスタントバッファー作成
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
//	アプリケーション処理。アプリのメイン関数。
//------------------------------------------------------------------------------------------
void App::MainLoop()
{
	// メッセージループ
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
//	描画処理
//------------------------------------------------------------------------------------------
void App::OnRender()
{
	//cameraの移動
	static float cameraRotX = PI * 0.1f;
	static float cameraRotY = -PI * 0.15f;
	static float camZoom = 0.0f;
	if (GetKeyState('C') & 0x80) {

		//Rotation
		if (GetKeyState(VK_UP) & 0x80) {
			cameraRotX += 0.005f;
		}
		if (GetKeyState(VK_DOWN) & 0x80) {
			cameraRotX -= 0.005f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80) {
			cameraRotY -= 0.005f;
		}
		if (GetKeyState(VK_LEFT) & 0x80) {
			cameraRotY += 0.005f;
		}

		//Zoom
		if (GetKeyState('I') & 0x80) {
			camZoom += 0.01f;
		}
		if (GetKeyState('O') & 0x80) {
			camZoom -= 0.01f;
		}
	}


	//cameraの移動変換行列
	D3DXMATRIX camWorldMatrix, camRotXMatrix, camRotYMatrix, camZoomMatrix;
	D3DXMatrixTranslation(&camZoomMatrix, 0, 0, camZoom);
	D3DXMatrixRotationX(&camRotXMatrix, cameraRotX);
	D3DXMatrixRotationY(&camRotYMatrix, cameraRotY);

	camWorldMatrix = camZoomMatrix* camRotXMatrix * camRotYMatrix;


	//ビュー行列
	D3DXVECTOR3 cameraPos(0.0f, 0.0f, -5.0f);	//カメラ位置
	D3DXVec3TransformCoord(&cameraPos, &cameraPos, &camWorldMatrix);
	D3DXVECTOR3 lookAtPos(0.0f, 0.25f, 0.0f);	//注視位置
	D3DXVECTOR3 upVec(0.0f, 1.0f, 0.0f);		//上方位置
	//D3DXVec3TransformCoord(&upVec, &upVec, &camWorldMatrix);
	D3DXMatrixLookAtLH(&m_ViewMatrix, &cameraPos, &lookAtPos, &upVec);


	//プロジェクション行列（射影変換）
	D3DXMatrixPerspectiveFovLH(
		&m_ProjectionMatrix,
		(float)D3DX_PI / 4.0,							//視野角
		(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,		//アスペクト比
		0.1f,											//near clip
		1000.0f);										//far clip

	float ClearColor[4] = { 0.178, 0.178, 0.178, 1 };// クリア色作成　RGBAの順


	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width	= WINDOW_WIDTH;
	vp.Height	= WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	m_pDeviceContext->RSSetViewports(1, &vp);


	//------------------------------------------------------------------------------------------
	//	Draw Object
	//------------------------------------------------------------------------------------------

	//レンダーターゲットビューと深度ステンシルビューをパイプラインにバインド
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);	//画面クリア
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);	//深度バッファクリア


	//キーボードによる入力はとりあえずここで
	char str[60];

	//ObjectのRotation
	static float roll = 0.0f;
	static float pitch = 0.0f;
	if (GetKeyState('O') & 0x80) {
		if (GetKeyState(VK_UP) & 0x80) {
			pitch += 0.05f;
		}
		if (GetKeyState(VK_DOWN) & 0x80) {
			pitch -= 0.05f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80) {
			roll -= 0.05f;
		}
		if (GetKeyState(VK_LEFT) & 0x80) {
			roll += 0.05f;
		}
	}


	//LightのRotation
	static float lightRotX = PI * 0.2f;
	static float lightRotY = PI * 0.8f;
	if (GetKeyState('L') & 0x80) {

		if (GetKeyState(VK_UP) & 0x80) {
			lightRotX += 0.01f;
		}
		if (GetKeyState(VK_DOWN) & 0x80) {
			lightRotX -= 0.01f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80) {
			lightRotY += 0.01f;
		}
		if (GetKeyState(VK_LEFT) & 0x80) {
			lightRotY -= 0.01f;
		}
	}


	//Global Roughness
	static D3DXVECTOR2 roughness(0.25f, 0.25f);
	if (GetKeyState('R') & 0x80) {
		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			roughness.x += 0.001f;
		}
		else if ((GetKeyState(VK_LEFT) & 0x80) && (roughness.x > 0.0f))
		{
			roughness.x -= 0.001f;
		}
		if (GetKeyState(VK_UP) & 0x80)
		{
			roughness.y += 0.001f;
		}
		else if ((GetKeyState(VK_DOWN) & 0x80) && (roughness.y > 0.0f))
		{
			roughness.y -= 0.001f;
		}

		sprintf(str, "roughness=%f, %f", roughness.x, roughness.y);
		SetWindowTextA(m_hWnd, str);
	}

	//Micro Roughness
	static D3DXVECTOR2 microRoughness(0.02f, 0.02f);
	if (GetKeyState('M') & 0x80) {
		if ((GetKeyState(VK_RIGHT) & 0x80) && (microRoughness.x < roughness.x))
		{
			microRoughness.x += 0.001f;
		}
		else if ((GetKeyState(VK_LEFT) & 0x80) && (microRoughness.x > 0.0f))
		{
			microRoughness.x -= 0.001f;
		}
		if ((GetKeyState(VK_UP) & 0x80) && (microRoughness.y < roughness.y))
		{
			microRoughness.y += 0.001f;
		}
		else if ((GetKeyState(VK_DOWN) & 0x80) && (microRoughness.y > 0.0f))
		{
			microRoughness.y -= 0.001f;
		}
		sprintf(str, "microRoughness=%f , %f", microRoughness.x, microRoughness.y);
		SetWindowTextA(m_hWnd, str);
	}

	//Variation
	static float variation = 860.0f;
	if (GetKeyState('V') & 0x80) {
		if (GetKeyState(VK_UP) & 0x80)
		{
			variation += 10.0f;
		}
		if (GetKeyState(VK_DOWN) & 0x80)
		{
			variation -= 10.0f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			variation += 0.1f;
		}
		if (GetKeyState(VK_LEFT) & 0x80)
		{
			variation -= 0.1f;
		}
		sprintf(str, "variation=%f", variation);
		SetWindowTextA(m_hWnd, str);
	}

	//Density
	static float density = 5.0e13;
	static float exponet = 13.0f;
	if (GetKeyState('D') & 0x80) {
		if (GetKeyState(VK_UP) & 0x80)
		{
			exponet += 0.1f;
		}
		if (GetKeyState(VK_DOWN) & 0x80)
		{
			exponet -= 0.1f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			exponet += 0.01f;
		}
		if (GetKeyState(VK_LEFT) & 0x80)
		{
			exponet-= 0.01f;
		}

		density = pow(10, exponet);
		sprintf(str, "density=%e", density);

		SetWindowTextA(m_hWnd, str);
	}
	
	//searchConeAngle
	static float searchConeAngle = 0.01f;
	if (GetKeyState('S') & 0x80) {
		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			searchConeAngle += 0.001f;
		}
		if (GetKeyState(VK_LEFT) & 0x80)
		{
			searchConeAngle -= 0.001;
		}
		sprintf(str, "searchConeAngle=%f", searchConeAngle);
		SetWindowTextA(m_hWnd, str);
	}

	//dynamicRange
	static float dynamicRange = 10.0f;
	if (GetKeyState('Q') & 0x80) {
		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			dynamicRange += 10.0f;
		}
		if (GetKeyState(VK_LEFT) & 0x80)
		{
			dynamicRange -= 10.0f;
		}
		sprintf(str, "dynamicRange=%f", dynamicRange);
		SetWindowTextA(m_hWnd, str);
	}

	//glints Blightness & shading Blightness
	static float glintsBrightness = 2.0f;
	static float shadingBribhtness = 7.0f;
	if (GetKeyState('B') & 0x80) {
		if (GetKeyState(VK_UP) & 0x80)
		{
			shadingBribhtness += 0.1f;
		}
		if (GetKeyState(VK_DOWN) & 0x80)
		{
			shadingBribhtness -= 0.1f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			glintsBrightness += 0.1f;
		}
		if (GetKeyState(VK_LEFT) & 0x80)
		{
			glintsBrightness -= 0.1f;
		}

		sprintf(str, "shadingBribhtness=%f / glintsBrightness=%f", shadingBribhtness, glintsBrightness);

		SetWindowTextA(m_hWnd, str);
	}

	//モデルのWorld変換行列
	D3DXMATRIX WorldMatrix, RollMatrix, PitchMatrix, ScallMatrix;
	D3DXMatrixRotationX(&PitchMatrix, pitch);
	D3DXMatrixRotationY(&RollMatrix, roll);
	//D3DXMatrixScaling(&ScallMatrix, 2.0, 2.0f, 2.0f);
	D3DXMatrixScaling(&ScallMatrix, 1.0f / 200.0f, 1.0f / 200.0f, 1.0f / 200.0f);
	WorldMatrix = ScallMatrix* RollMatrix * PitchMatrix;


	//LightのWorld変換行列
	D3DXVECTOR3 lightPos(0.0f, 0.0f, -5.0f);
	D3DXMATRIX lightWorldMatrix, lightRotXMatrix, lightRotYMatrix, lightTranslationMatrix;
	D3DXMatrixRotationX(&lightRotXMatrix, lightRotX);
	D3DXMatrixRotationY(&lightRotYMatrix, lightRotY);
	//D3DXMatrixTranslation(&lightTranslationMatrix, 0.0f, 0.0, 0.0f);
	lightWorldMatrix = lightRotXMatrix * lightRotYMatrix;
	D3DXVec3TransformCoord(&lightPos, &lightPos, &lightWorldMatrix);


	//シェーダーのコンスタントバッファーに各種データを渡す
	ConstantBufferGlint cbg;
	D3D11_MAPPED_SUBRESOURCE pData;

	if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBuffer[DRAW_GLINT].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//WVP行列をシェーダーに渡す
		cbg.WVP = WorldMatrix * m_ViewMatrix*m_ProjectionMatrix;
		D3DXMatrixTranspose(&cbg.WVP, &cbg.WVP);

		cbg.W = WorldMatrix;
		D3DXMatrixTranspose(&cbg.W, &cbg.W);

		cbg.cameraPos = cameraPos;
		cbg.lightPos = lightPos;
		  
		cbg.roughness = roughness;
		cbg.microRoughness = microRoughness;
		cbg.variation = variation;
		cbg.density = density;
		cbg.searchConeAngle = searchConeAngle;
		cbg.dynamicRange = dynamicRange;
		cbg.glintsBrightness = glintsBrightness;
		cbg.shadingBribhtness = shadingBribhtness;


		memcpy_s(pData.pData, pData.RowPitch, (void*)(&cbg), sizeof(cbg));
		m_pDeviceContext->Unmap(m_pConstantBuffer[DRAW_GLINT].Get(), 0);
	}

	//バーテックスバッファーをセット
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer[DRAW_GLINT].GetAddressOf(), &stride, &offset);


	//インデックスバッファーをセット
	stride = sizeof(int);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer[DRAW_GLINT].Get(), DXGI_FORMAT_R32_UINT, 0);


	//このコンスタントバッファーを使うシェーダーの登録
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_GLINT].GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_GLINT].GetAddressOf());


	//頂点インプットレイアウトをセット
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout[DRAW_GLINT].Get());
	
	
	//プリミティブ・トポロジーをセット
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ラスタライザーのセット
	m_pDeviceContext->RSSetState(m_pRasterizerState[DRAW_GLINT].Get());

	//使用するシェーダーの登録	
	m_pDeviceContext->VSSetShader(m_pVertexShader[DRAW_GLINT].Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader[DRAW_GLINT].Get(), nullptr, 0);
	
	
	//環境マップをセット
	m_pDeviceContext->PSSetShaderResources(0, 1, m_pCubeMapSRV.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(0, 1, m_pCubeMapSamplerState.GetAddressOf());

	//プリミティブをレンダリング
	m_pDeviceContext->DrawIndexed(NumFace[DRAW_GLINT] * 3, 0, 0);
	//m_pDeviceContext->Draw(4, 0);		//Debugで使う板ポリ用


	//------------------------------------------------------------------------------------------
	//	Draw Sky
	//------------------------------------------------------------------------------------------
	//モデルのWorld変換行列
	D3DXMatrixScaling(&ScallMatrix, 20.0, 20.0f, 20.0f);
	//D3DXMatrixScaling(&ScallMatrix, 1.0f / 150.0f, 1.0f / 150.0f, 1.0f / 150.0f);
	WorldMatrix = ScallMatrix* RollMatrix * PitchMatrix;
	
	//シェーダーのコンスタントバッファーに各種データを渡す
	ConstantBufferSky cbs;

	if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBuffer[DRAW_SKY].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//WVP行列をシェーダーに渡す
		cbs.WVP = WorldMatrix * m_ViewMatrix*m_ProjectionMatrix;
		D3DXMatrixTranspose(&cbs.WVP, &cbs.WVP);

		cbs.W = WorldMatrix;
		D3DXMatrixTranspose(&cbs.W, &cbs.W);

		cbs.cameraPos = cameraPos;
		cbs.lightPos = lightPos;
		
		memcpy_s(pData.pData, pData.RowPitch, (void*)(&cbs), sizeof(cbs));
		m_pDeviceContext->Unmap(m_pConstantBuffer[DRAW_SKY].Get(), 0);
	}

	//バーテックスバッファーをセット
	stride = sizeof(Vertex3);
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer[DRAW_SKY].GetAddressOf(), &stride, &offset);


	//インデックスバッファーをセット
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer[DRAW_SKY].Get(), DXGI_FORMAT_R32_UINT, 0);


	//このコンスタントバッファーを使うシェーダーの登録
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_SKY].GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_SKY].GetAddressOf());


	//頂点インプットレイアウトをセット
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout[DRAW_SKY].Get());


	//プリミティブ・トポロジーをセット
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ラスタライザーのセット
	m_pDeviceContext->RSSetState(m_pRasterizerState[DRAW_SKY].Get());

	//使用するシェーダーの登録	
	m_pDeviceContext->VSSetShader(m_pVertexShader[DRAW_SKY].Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader[DRAW_SKY].Get(), nullptr, 0);

	//テクスチャをセット（環境マップと同じ）
	m_pDeviceContext->PSSetShaderResources(0, 1, m_pCubeMapSRV.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(0, 1, m_pCubeMapSamplerState.GetAddressOf());

	//プリミティブをレンダリング
	m_pDeviceContext->DrawIndexed(NumFace[DRAW_SKY] * 3, 0, 0);

	m_pSwapChain->Present(0, 0);	//画面更新（バックバッファをフロントバッファに）	
}