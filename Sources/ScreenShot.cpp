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




// スクリーンショット作成
HRESULT ScreenShot::CreateScreenShot(ID3D11Device* device, ID3D11DeviceContext* device_context, IDXGISwapChain* swap_chain, HWND hWnd)
{
	HRESULT hr = E_FAIL;
	TCHAR root[] = _T("ScreenShot");
	TCHAR path[256];
	int maxFileCnt = 50;

	ComPtr<ID3D11Texture2D> pBackBuffer = NULL;
	ComPtr<ID3D11Texture2D> pTexture = NULL;

	//OutputMsg(_T("スクリーンショット作成"), _T(""), _T("開始"));

	// ScreenShot フォルダがありますか？
	if (::PathFileExists(root) == false)
	{
		// ないので ScreenShotフォルダを作成する
		if (::CreateDirectory(root, NULL) == FALSE) return false;
	}

	int i = 0;
	//ファイルを作成しすぎてディスクがパンクすることがないようにするため、ファイルの作成可能数を限定する。
	while (i < maxFileCnt)
	{
		_stprintf_s(path, _T("%s\\%02d.jpg"), root, i);

		//ファイルがありますか？
		if (::PathFileExists(path) == FALSE) break;

		i++;
	}
	// スクリーンショットの作成可能数をオーバーしているため作成できません。
	if (i >= maxFileCnt)
	{
		//OutputMsg(_T("スクリーンショット作成"), _T(""), _T("作成数オーバー"));
		return false;
	}

	// スワップチェーンからバックバッファを取得する
	hr = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr)) return false;

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	swap_chain->GetDesc(&swapchainDesc);

	// バックバッファがマルチサンプル無効で作成されている場合
	if (swapchainDesc.SampleDesc.Count == 1 && swapchainDesc.SampleDesc.Quality == 0)
	{
		// この場合はそのままファイル出力する
		// D3DX11SaveTextureToFile
		hr = D3DX11SaveTextureToFile(device_context, pBackBuffer.Get(), D3DX11_IFF_JPG, path);
		if (FAILED(hr)) return false;
	}
	// バックバッファがマルチサンプルが有効で作成されている場合
	else
	{
		D3D11_TEXTURE2D_DESC texDesc;
		pBackBuffer->GetDesc(&texDesc);
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		// コピー先用のテクスチャーを作成する。
		hr = device->CreateTexture2D(&texDesc, NULL, &pTexture);
		if (FAILED(hr)) return false;

		// マルチサンプリングされたリソースをマルチサンプリングされていないリソースにコピーする
		// ID3D11DeviceContext::ResolveSubresource
		device_context->ResolveSubresource(pTexture.Get(), NULL, pBackBuffer.Get(), NULL, texDesc.Format);

		// ファイル出力
		hr = D3DX11SaveTextureToFile(device_context, pTexture.Get(), D3DX11_IFF_JPG, path);
		if (FAILED(hr)) return false;
	}

	//OutputMsg(_T("スクリーンショット作成"), path, _T("完了"));

	hr = S_OK;

	return hr;
}



// スナップショットをクリップボードから作成
// タイトルバーやウィンドウの枠を出力可能
// (Fn +) Alt + PrtScでクリップボードへイメージを転送する処理が必要
HRESULT ScreenShot::CreateScreenShotFromClipBoard(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hWnd)
{
	HRESULT hr = E_FAIL;
	WCHAR root[] = _T("ScreenShot");
	WCHAR path[256];
	int maxFileCnt = 20;

	// ビットマップのバイナリバッファ
	BYTE* bits = NULL;
	ComPtr<ID3D11Texture2D> texture = NULL;

	// SetClipboardViewer を実行すると WM_DRAWCLIPBOARD イベントが実行されてしまうっぽいので
	// 初期化が完了してないときはスクリーンショットを作成しないようにする。
	if (device == NULL || device_context == NULL)
	{
		hr = S_OK;
		goto EXIT;
	}

	// ScreenShot フォルダがありますか？
	if (::PathFileExists(root) == false)
	{
		// ないので ScreenShotフォルダを作成する
		if (::CreateDirectory(root, NULL) == FALSE) goto EXIT;
	}

	int i = 0;
	// ファイルを作成しすぎてディスクがパンクすることがないようにするため、ファイルの作成可能数を限定する。
	while (i < maxFileCnt)
	{
		_stprintf_s(path, _T("%s\\%02d.jpg"), root, i);

		// ファイルがありますか？
		if (::PathFileExists(path) == FALSE) break;

		i++;
	}
	// スクリーンショットの作成可能数をオーバーしているため作成できません。
	if (i >= maxFileCnt)
	{
		//OutputMsg(_T("スクリーンショット作成"), _T(""), _T("作成数オーバー"));
		DebugWindow(L"作成数オーバー");
		goto EXIT;
	}

	// クリップボードに存在するデータがビットマップか
	if (::IsClipboardFormatAvailable(CF_BITMAP))
	{
		// クリップボードのオープン
		if (!::OpenClipboard(hWnd)) goto EXIT;

		HBITMAP hBitmap = NULL;
		// GetClipboardData
		if ((hBitmap = (HBITMAP)::GetClipboardData(CF_BITMAP)) == NULL) goto EXIT;

		// BITMAP 構造体を作成する
		BITMAP bitmap;
		::GetObject(hBitmap, sizeof(BITMAP), &bitmap);

		// ファイルヘッダ部
		// BITMAPFILEHEADER
		BITMAPFILEHEADER bHeader;
		::ZeroMemory(&bHeader, sizeof(BITMAPFILEHEADER));
		// BM 固定
		bHeader.bfType = ('M' << 8) + 'B';
		// ファイルサイズ
		bHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bitmap.bmWidth * bitmap.bmHeight * bitmap.bmBitsPixel;
		// ファイル先頭から画像データまでのオフセット
		bHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		// 情報ヘッダ部
		// BITMAPINFO
		// BITMAPINFOHEADER
		BITMAPINFO info;
		::ZeroMemory(&info, sizeof(BITMAPINFO));
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biWidth = bitmap.bmWidth;
		info.bmiHeader.biHeight = bitmap.bmHeight;
		info.bmiHeader.biPlanes = 1;                        // 常に 1
		info.bmiHeader.biBitCount = bitmap.bmBitsPixel;       // 1 ピクセルあたりのビット数を指定する。ディスプレイのカラービット深度に依存する。
		info.bmiHeader.biCompression = BI_RGB;                   // 非圧縮

		bits = new BYTE[bHeader.bfSize];

		// ファイルヘッダ部をコピー
		CopyMemory(bits, &bHeader, sizeof(BITMAPFILEHEADER));
		// 情報ヘッダ部をコピー(カラーテーブルは作成しない)
		CopyMemory(&bits[sizeof(BITMAPFILEHEADER)], &info, sizeof(BITMAPINFOHEADER));

		HDC hDC = NULL;
		hDC = GetDC(hWnd);

		// 指定されたビットマップのビットを取得し、指定された形式でバッファへコピーする。
		// GetDIBits
		GetDIBits(
			hDC,                        // デバイスコンテキスト
			hBitmap,                    // ビットマップハンドル
			0,                          // 取得する最初の走査行
			bitmap.bmHeight,            // 取得する走査行の数
			&bits[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)], // 保存するバッファのポインタ
			&info,                      // BITMAPINFO構造体のポインタ
			DIB_RGB_COLORS              // カラーテーブルを取得しない
		);
		ReleaseDC(NULL, hDC);

		// システム メモリー内にあるバイナリバッファからテクスチャー リソースを作成する。
		// D3DX11CreateTextureFromMemory
		hr = D3DX11CreateTextureFromMemory(device, bits, bHeader.bfSize, NULL, NULL, (ID3D11Resource**)texture.GetAddressOf(), NULL);
		if (FAILED(hr)) goto EXIT;

		// ウィンドウモードのとき、画像をウィンドウにあわせてくりぬく

		// ファイル出力
		// D3DX11SaveTextureToFile
		hr = D3DX11SaveTextureToFile(device_context, texture.Get(), D3DX11_IFF_JPG, path);
		if (FAILED(hr)) {
			DebugWindow(L"スクリーンショット作成失敗");

			goto EXIT;
		}

		//OutputMsg(_T("スクリーンショット作成"), path, _T("完了"));
	}

	hr = S_OK;

EXIT:
	::CloseClipboard();
	SAFE_DELETE_ARRAY(bits);

	return hr;
}