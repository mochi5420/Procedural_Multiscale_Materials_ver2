//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include "Quad.h"

//------------------------------------------------------------------------------------------
//	constructor & destructor
//------------------------------------------------------------------------------------------
Quad::Quad()
{
	SecureZeroMemory(this, sizeof(Quad));
}
Quad::~Quad()
{
}


HRESULT Quad::Create(ID3D11Device* device, ID3D11Buffer** vertexBuffer)
{
	Vertex vertices[] =
	{
		D3DXVECTOR4(-1.0, -1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
		D3DXVECTOR4(-1.0, 1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
		D3DXVECTOR4(1.0, -1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
		D3DXVECTOR4(1.0, 1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
		D3DXVECTOR4(1.0, -1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
		D3DXVECTOR4(-1.0, 1.0, 0.0, 1.0), D3DXVECTOR3(0.0, 0.0, -1.0),
	};

	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;

	
	if (FAILED(device->CreateBuffer(&bd, &InitData, vertexBuffer)))
	{
		return false;
	}

	return S_OK;
}


