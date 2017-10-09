//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include "App.h"
#include "ObjMeshLoader.h"
#include "FPS.h"


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

	if (FAILED(InitShader()))	//Initialize Shader
	{
		return 0;
	}

	MainLoop();	//���C�����[�v��

	return 0;	//�A�v���I��
}


//------------------------------------------------------------------------------------------
//	�E�B���h�E�v���V�[�W���[
//------------------------------------------------------------------------------------------
LRESULT CALLBACK App::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
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
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
	wc.lpfnWndProc		= WndProc;								// ���b�Z�[�W�����֐�
	wc.hInstance		= hInst;								// �v���O�����̃n���h��
	wc.hIcon			= LoadIcon(nullptr, IDI_APPLICATION);	// �A�C�R��
	wc.hCursor			= LoadCursor(nullptr, IDC_ARROW);		// �J�[�\��
	wc.hbrBackground	= (HBRUSH)GetStockObject(LTGRAY_BRUSH);	// �u���V
	wc.lpszMenuName		= nullptr;								// ���j���[
	wc.lpszClassName	= APP_NAME;								// �N���X��
	wc.hIconSm			= LoadIcon(nullptr, IDI_APPLICATION);

	RegisterClassEx(&wc);	//�E�C���h�E�N���X��o�^

	// �E�B���h�E�̍쐬
	m_hWnd = CreateWindow(
		APP_NAME,				// �E�C���h�E�N���X�� 
		APP_NAME,				// �E�C���h�E�̃^�C�g��
		WS_OVERLAPPEDWINDOW,	// �E�C���h�E�X�^�C�� 
		100, 100,				// �E�C���h�E�\���ʒu 
		WINDOW_WIDTH, 			// �E�C���h�E�̑傫�� 
		WINDOW_HEIGHT,			// �E�C���h�E�̑傫�� 
		nullptr,				// �e�E�C���h�E�̃n���h�� 
		nullptr,				// ���j���[�̃n���h�� 
		hInst,					// �C���X�^���X�̃n���h��
		nullptr);				// �쐬���̈����ۑ��p�|�C���^ 

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


//------------------------------------------------------------------------------------------
//	Initializes Direct3D
//------------------------------------------------------------------------------------------
bool App::InitD3D()
{
	HRESULT hr = S_OK;

	// �f�o�C�X�ƃX���b�v�`�F�[���̍쐬
	DXGI_SWAP_CHAIN_DESC sd;
	SecureZeroMemory(&sd, sizeof(sd)); //������
	sd.BufferCount							= 1;
	sd.BufferDesc.Width						= WINDOW_WIDTH;						//�T�C�Y�w��
	sd.BufferDesc.Height					= WINDOW_HEIGHT;					//�T�C�Y�w��
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


	//���X�^���C�Y�ݒ�
	D3D11_RASTERIZER_DESC rdc;
	SecureZeroMemory(&rdc, sizeof(rdc));
	rdc.CullMode = D3D11_CULL_BACK;	
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = FALSE;

	m_pDevice->CreateRasterizerState(&rdc, &m_pRasterizerState);
	m_pDeviceContext->RSSetState(m_pRasterizerState.Get());


	//Obj�̓ǂݍ���
	{
		ObjMeshLoader obj;

		hr = obj.CreateMesh(m_pDevice.Get(), m_pVertexBuffer.GetAddressOf(),
			m_pIndexBuffer.GetAddressOf(), NumFace, OBJ_FILE);

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

	//�u���u����DRAW_GLINT�o�[�e�b�N�X�V�F�[�_�[�쐬
	hr = D3DX11CompileFromFile(
			L"Resources/Shaders/DrawTexture.hlsl",
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
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	int numElements = sizeof(layout) / sizeof(layout[0]);

	hr = m_pDevice->CreateInputLayout(
			layout,
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
			L"Resources/Shaders/DrawTexture.hlsl",
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
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pConstantBuffer[DRAW_GLINT]);

	if (FAILED(hr))
	{
		return false;
	}


	return true;
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
	static float cameraRotX = 0.0f;
	static float cameraRotY = 0.0f;
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


	//camera�̈ړ��ϊ��s��
	D3DXMATRIX camWorldMatrix, camRotXMatrix, camRotYMatrix, camZoomMatrix;
	D3DXMatrixTranslation(&camZoomMatrix, 0, 0, camZoom);
	D3DXMatrixRotationX(&camRotXMatrix, cameraRotX);
	D3DXMatrixRotationY(&camRotYMatrix, cameraRotY);

	camWorldMatrix = camZoomMatrix* camRotXMatrix * camRotYMatrix;


	//�r���[�s��
	D3DXVECTOR3 cameraPos(0.0f, 0.0f, -5.0f);	//�J�����ʒu
	D3DXVec3TransformCoord(&cameraPos, &cameraPos, &camWorldMatrix);
	D3DXVECTOR3 lookAtPos(0.0f, 0.0f, 0.0f);	//�����ʒu
	D3DXVECTOR3 upVec(0.0f, 1.0f, 0.0f);		//����ʒu
	//D3DXVec3TransformCoord(&upVec, &upVec, &camWorldMatrix);
	D3DXMatrixLookAtLH(&m_ViewMatrix, &cameraPos, &lookAtPos, &upVec);


	//�v���W�F�N�V�����s��i�ˉe�ϊ��j
	D3DXMatrixPerspectiveFovLH(
		&m_ProjectionMatrix,
		(float)D3DX_PI / 4.0,							//����p
		(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,		//�A�X�y�N�g��
		0.1f,											//near clip
		100.0f);										//far clip

	float ClearColor[4] = { 0.178, 0.178, 0.178, 1 };// �N���A�F�쐬�@RGBA�̏�


	//�r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT vp;
	vp.Width	= WINDOW_WIDTH;
	vp.Height	= WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	m_pDeviceContext->RSSetViewports(1, &vp);


	//�����_�[�^�[�Q�b�g�r���[�Ɛ[�x�X�e���V���r���[���p�C�v���C���Ƀo�C���h
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);	//��ʃN���A
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);	//�[�x�o�b�t�@�N���A


	//�o�[�e�b�N�X�o�b�t�@�[���Z�b�g
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);


	//�C���f�b�N�X�o�b�t�@�[���Z�b�g
	stride = sizeof(int);
	offset = 0;
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	//�L�[�{�[�h�ɂ����͂͂Ƃ肠����������
	//�I�u�W�F�N�g��Rotation
	static float roll = 0.0f;
	static float pitch = 0.0f;
	if (GetKeyState('O') & 0x80) {
		if (GetKeyState(VK_UP) & 0x80) {
			pitch += 0.01f;
		}
		if (GetKeyState(VK_DOWN) & 0x80) {
			pitch -= 0.01f;
		}
		if (GetKeyState(VK_RIGHT) & 0x80) {
			roll -= 0.01f;
		}
		if (GetKeyState(VK_LEFT) & 0x80) {
			roll += 0.01f;
		}
	}


	//���̑��e��p�����[�^�ύX
	char str[60];

	static D3DXVECTOR2 roughness(0.07f, 0.07f);
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

	static D3DXVECTOR2 microRoughness(0.06f, 0.06f);
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

	static float variation = 0.1f;
	if (GetKeyState('V') & 0x80) {
		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			variation += 1.0f;
		}
		if (GetKeyState(VK_LEFT) & 0x80)
		{
			variation -= 1.0f;
		}
		sprintf(str, "variation=%f", variation);
		SetWindowTextA(m_hWnd, str);
	}

	static float density = 1.5e5;
	if (GetKeyState('D') & 0x80) {
		if (GetKeyState(VK_UP) & 0x80)
		{
			density += 1.e5;
		}
		if (GetKeyState(VK_DOWN) & 0x80)
		{
			density -= 1.e5f;
		}
		sprintf(str, "density=%e", density);

		if (GetKeyState(VK_RIGHT) & 0x80)
		{
			density += 1.e4;
		}
		if (GetKeyState(VK_LEFT) & 0x80)
		{
			density -= 1.e4f;
		}
		sprintf(str, "density=%e", density);

		SetWindowTextA(m_hWnd, str);
	}
	

	//���f���̉�]�s��
	D3DXMATRIX WorldMatrix, RollMatrix, PitchMatrix, ScallMatrix;
	D3DXMatrixRotationX(&PitchMatrix, pitch);
	D3DXMatrixRotationY(&RollMatrix, roll);
	//D3DXMatrixScaling(&ScallMatrix, 2.0, 2.0f, 2.0f);
	D3DXMatrixScaling(&ScallMatrix, 1.0f / 150.0f, 1.0f / 150.0f, 1.0f / 150.0f);
	WorldMatrix = ScallMatrix* RollMatrix * PitchMatrix;


	//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�[�Ɋe��f�[�^��n��
	ConstantBuffer cb;
	D3D11_MAPPED_SUBRESOURCE pData;

	if (SUCCEEDED(m_pDeviceContext->Map(m_pConstantBuffer[DRAW_GLINT].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//WVP�s����V�F�[�_�[�ɓn��
		cb.WVP = WorldMatrix * m_ViewMatrix*m_ProjectionMatrix;
		D3DXMatrixTranspose(&cb.WVP, &cb.WVP);

		cb.W = WorldMatrix;
		D3DXMatrixTranspose(&cb.W, &cb.W);

		cb.lightPos = D3DXVECTOR3(0.0, 0.0f, -5.0f);
		
		cb.roughness = roughness;
		cb.microRoughness = microRoughness;
		cb.variation = variation;
		cb.density = density;

		cb.cameraPos = cameraPos;

		memcpy_s(pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb));
		m_pDeviceContext->Unmap(m_pConstantBuffer[DRAW_GLINT].Get(), 0);
	}

	
	//���̃R���X�^���g�o�b�t�@�[���g���V�F�[�_�[�̓o�^
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_GLINT].GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer[DRAW_GLINT].GetAddressOf());


	//���_�C���v�b�g���C�A�E�g���Z�b�g
	m_pDeviceContext->IASetInputLayout(m_pVertexLayout[DRAW_GLINT].Get());
	
	
	//�v���~�e�B�u�E�g�|���W�[���Z�b�g
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�g�p����V�F�[�_�[�̓o�^	
	m_pDeviceContext->VSSetShader(m_pVertexShader[DRAW_GLINT].Get(), nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader[DRAW_GLINT].Get(), nullptr, 0);
	
	
	//�v���~�e�B�u�������_�����O
	m_pDeviceContext->DrawIndexed(NumFace * 3, 0, 0);
	//m_pDeviceContext->Draw(4, 0);		//Debug�Ŏg���|���p

	m_pSwapChain->Present(0, 0);	//��ʍX�V�i�o�b�N�o�b�t�@���t�����g�o�b�t�@�Ɂj	
}