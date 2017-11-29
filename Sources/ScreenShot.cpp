//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include "ScreenShot.h"
#include "Debug.h"


#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif  

using namespace Microsoft::WRL;


//------------------------------------------------------------------------------------------
//	constructor & destructor
//------------------------------------------------------------------------------------------
ScreenShot::ScreenShot()
{

}
ScreenShot::~ScreenShot()
{

}




// �X�N���[���V���b�g�쐬
HRESULT ScreenShot::CreateScreenShot(ID3D11Device* device, ID3D11DeviceContext* device_context, IDXGISwapChain* swap_chain, HWND hWnd)
{
	HRESULT hr = E_FAIL;
	TCHAR root[] = _T("ScreenShot");
	TCHAR path[256];
	int maxFileCnt = 50;

	ComPtr<ID3D11Texture2D> pBackBuffer = NULL;
	ComPtr<ID3D11Texture2D> pTexture = NULL;

	//OutputMsg(_T("�X�N���[���V���b�g�쐬"), _T(""), _T("�J�n"));

	// ScreenShot �t�H���_������܂����H
	if (::PathFileExists(root) == false)
	{
		// �Ȃ��̂� ScreenShot�t�H���_���쐬����
		if (::CreateDirectory(root, NULL) == FALSE) return false;
	}

	int i = 0;
	//�t�@�C�����쐬�������ăf�B�X�N���p���N���邱�Ƃ��Ȃ��悤�ɂ��邽�߁A�t�@�C���̍쐬�\�������肷��B
	while (i < maxFileCnt)
	{
		_stprintf_s(path, _T("%s\\%02d.jpg"), root, i);

		//�t�@�C��������܂����H
		if (::PathFileExists(path) == FALSE) break;

		i++;
	}
	// �X�N���[���V���b�g�̍쐬�\�����I�[�o�[���Ă��邽�ߍ쐬�ł��܂���B
	if (i >= maxFileCnt)
	{
		//OutputMsg(_T("�X�N���[���V���b�g�쐬"), _T(""), _T("�쐬���I�[�o�["));
		return false;
	}

	// �X���b�v�`�F�[������o�b�N�o�b�t�@���擾����
	hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr)) return false;

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	swap_chain->GetDesc(&swapchainDesc);

	// �o�b�N�o�b�t�@���}���`�T���v�������ō쐬����Ă���ꍇ
	if (swapchainDesc.SampleDesc.Count == 1 && swapchainDesc.SampleDesc.Quality == 0)
	{
		// ���̏ꍇ�͂��̂܂܃t�@�C���o�͂���
		// D3DX11SaveTextureToFile
		hr = D3DX11SaveTextureToFile(device_context, pBackBuffer.Get(), D3DX11_IFF_JPG, path);
		if (FAILED(hr)) return false;
	}
	// �o�b�N�o�b�t�@���}���`�T���v�����L���ō쐬����Ă���ꍇ
	else
	{
		D3D11_TEXTURE2D_DESC texDesc;
		pBackBuffer->GetDesc(&texDesc);
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		// �R�s�[��p�̃e�N�X�`���[���쐬����B
		hr = device->CreateTexture2D(&texDesc, NULL, &pTexture);
		if (FAILED(hr)) return false;

		// �}���`�T���v�����O���ꂽ���\�[�X���}���`�T���v�����O����Ă��Ȃ����\�[�X�ɃR�s�[����
		// ID3D11DeviceContext::ResolveSubresource
		device_context->ResolveSubresource(pTexture.Get(), NULL, pBackBuffer.Get(), NULL, texDesc.Format);

		// �t�@�C���o��
		hr = D3DX11SaveTextureToFile(device_context, pTexture.Get(), D3DX11_IFF_JPG, path);
		if (FAILED(hr)) return false;
	}

	//OutputMsg(_T("�X�N���[���V���b�g�쐬"), path, _T("����"));

	hr = S_OK;

	return hr;
}



// �X�i�b�v�V���b�g���N���b�v�{�[�h����쐬
// �^�C�g���o�[��E�B���h�E�̘g���o�͉\
// (Fn +) Alt + PrtSc�ŃN���b�v�{�[�h�փC���[�W��]�����鏈�����K�v
HRESULT ScreenShot::CreateScreenShotFromClipBoard(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hWnd)
{
	HRESULT hr = E_FAIL;
	WCHAR root[] = _T("ScreenShot");
	WCHAR path[256];
	int maxFileCnt = 20;

	// �r�b�g�}�b�v�̃o�C�i���o�b�t�@
	BYTE* bits = NULL;
	ComPtr<ID3D11Texture2D> texture = NULL;

	// SetClipboardViewer �����s����� WM_DRAWCLIPBOARD �C�x���g�����s����Ă��܂����ۂ��̂�
	// ���������������ĂȂ��Ƃ��̓X�N���[���V���b�g���쐬���Ȃ��悤�ɂ���B
	if (device == NULL || device_context == NULL)
	{
		hr = S_OK;
		goto EXIT;
	}

	// ScreenShot �t�H���_������܂����H
	if (::PathFileExists(root) == false)
	{
		// �Ȃ��̂� ScreenShot�t�H���_���쐬����
		if (::CreateDirectory(root, NULL) == FALSE) goto EXIT;
	}

	int i = 0;
	// �t�@�C�����쐬�������ăf�B�X�N���p���N���邱�Ƃ��Ȃ��悤�ɂ��邽�߁A�t�@�C���̍쐬�\�������肷��B
	while (i < maxFileCnt)
	{
		_stprintf_s(path, _T("%s\\%02d.jpg"), root, i);

		// �t�@�C��������܂����H
		if (::PathFileExists(path) == FALSE) break;

		i++;
	}
	// �X�N���[���V���b�g�̍쐬�\�����I�[�o�[���Ă��邽�ߍ쐬�ł��܂���B
	if (i >= maxFileCnt)
	{
		//OutputMsg(_T("�X�N���[���V���b�g�쐬"), _T(""), _T("�쐬���I�[�o�["));
		DebugWindow(L"�쐬���I�[�o�[");
		goto EXIT;
	}

	// �N���b�v�{�[�h�ɑ��݂���f�[�^���r�b�g�}�b�v��
	if (::IsClipboardFormatAvailable(CF_BITMAP))
	{
		// �N���b�v�{�[�h�̃I�[�v��
		if (!::OpenClipboard(hWnd)) goto EXIT;

		HBITMAP hBitmap = NULL;
		// GetClipboardData
		if ((hBitmap = (HBITMAP)::GetClipboardData(CF_BITMAP)) == NULL) goto EXIT;

		// BITMAP �\���̂��쐬����
		BITMAP bitmap;
		::GetObject(hBitmap, sizeof(BITMAP), &bitmap);

		// �t�@�C���w�b�_��
		// BITMAPFILEHEADER
		BITMAPFILEHEADER bHeader;
		::ZeroMemory(&bHeader, sizeof(BITMAPFILEHEADER));
		// BM �Œ�
		bHeader.bfType = ('M' << 8) + 'B';
		// �t�@�C���T�C�Y
		bHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bitmap.bmWidth * bitmap.bmHeight * bitmap.bmBitsPixel;
		// �t�@�C���擪����摜�f�[�^�܂ł̃I�t�Z�b�g
		bHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		// ���w�b�_��
		// BITMAPINFO
		// BITMAPINFOHEADER
		BITMAPINFO info;
		::ZeroMemory(&info, sizeof(BITMAPINFO));
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biWidth = bitmap.bmWidth;
		info.bmiHeader.biHeight = bitmap.bmHeight;
		info.bmiHeader.biPlanes = 1;                        // ��� 1
		info.bmiHeader.biBitCount = bitmap.bmBitsPixel;       // 1 �s�N�Z��������̃r�b�g�����w�肷��B�f�B�X�v���C�̃J���[�r�b�g�[�x�Ɉˑ�����B
		info.bmiHeader.biCompression = BI_RGB;                   // �񈳏k

		bits = new BYTE[bHeader.bfSize];

		// �t�@�C���w�b�_�����R�s�[
		CopyMemory(bits, &bHeader, sizeof(BITMAPFILEHEADER));
		// ���w�b�_�����R�s�[(�J���[�e�[�u���͍쐬���Ȃ�)
		CopyMemory(&bits[sizeof(BITMAPFILEHEADER)], &info, sizeof(BITMAPINFOHEADER));

		HDC hDC = NULL;
		hDC = GetDC(hWnd);

		// �w�肳�ꂽ�r�b�g�}�b�v�̃r�b�g���擾���A�w�肳�ꂽ�`���Ńo�b�t�@�փR�s�[����B
		// GetDIBits
		GetDIBits(
			hDC,                        // �f�o�C�X�R���e�L�X�g
			hBitmap,                    // �r�b�g�}�b�v�n���h��
			0,                          // �擾����ŏ��̑����s
			bitmap.bmHeight,            // �擾���鑖���s�̐�
			&bits[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)], // �ۑ�����o�b�t�@�̃|�C���^
			&info,                      // BITMAPINFO�\���̂̃|�C���^
			DIB_RGB_COLORS              // �J���[�e�[�u�����擾���Ȃ�
		);
		ReleaseDC(NULL, hDC);

		// �V�X�e�� �������[���ɂ���o�C�i���o�b�t�@����e�N�X�`���[ ���\�[�X���쐬����B
		// D3DX11CreateTextureFromMemory
		hr = D3DX11CreateTextureFromMemory(device, bits, bHeader.bfSize, NULL, NULL, (ID3D11Resource**)texture.GetAddressOf(), NULL);
		if (FAILED(hr)) goto EXIT;

		// �E�B���h�E���[�h�̂Ƃ��A�摜���E�B���h�E�ɂ��킹�Ă���ʂ�

		// �t�@�C���o��
		// D3DX11SaveTextureToFile
		hr = D3DX11SaveTextureToFile(device_context, texture.Get(), D3DX11_IFF_JPG, path);
		if (FAILED(hr)) {
			DebugWindow(L"�X�N���[���V���b�g�쐬���s");

			goto EXIT;
		}

		//OutputMsg(_T("�X�N���[���V���b�g�쐬"), path, _T("����"));
	}

	hr = S_OK;

EXIT:
	::CloseClipboard();
	SAFE_DELETE_ARRAY(bits);

	return hr;
}