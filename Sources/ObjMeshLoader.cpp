//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include "ObjMeshLoader.h"
#include "Debug.h"

//------------------------------------------------------------------------------------------
//	constructor & destructor
//------------------------------------------------------------------------------------------
ObjMeshLoader::ObjMeshLoader()
{
	SecureZeroMemory(this, sizeof(ObjMeshLoader));
}
ObjMeshLoader::~ObjMeshLoader()
{
}


//------------------------------------------------------------------------------------------
//	.objファイルから、頂点・法線情報をロードする
//------------------------------------------------------------------------------------------
HRESULT ObjMeshLoader::CreateMesh(ID3D11Device* device, 
	ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, DWORD& numFace, LPSTR FileName)
{
	// ---------------------------------------------------------------------
	//  VertexとNormalを.objファイルから読み込む処理 
	// ---------------------------------------------------------------------
	std::ifstream ifs(FileName, std::ios::in);
	if (!ifs.is_open()) {
		DebugWindow(L"Failed to load file: %s", FileName);
		std::exit(1);
	}

	std::vector<D3DXVECTOR4> pvCoord;
	std::vector<D3DXVECTOR3> pvNormal;
	std::vector<Triplet> vIndices;
	std::vector<Triplet> nIndices;

	std::string line;
	while (!ifs.eof())
	{
		std::getline(ifs, line);

		std::stringstream ss;
		ss << line;

		std::string identifier;
		ss >> identifier;

		if (identifier == "v") {
			float x, y, z;
			ss >> x >> y >> z;
			pvCoord.push_back(D3DXVECTOR4(x, y, z, 1.0f));
		}
		else if (identifier == "vn") {
			float x, y, z;
			ss >> x >> y >> z;
			pvNormal.push_back(D3DXVECTOR3(x, y, z));
		}
		else if (identifier == "f") {
			std::string s1, s2, s3;
			ss >> s1 >> s2 >> s3;

			int v1, v2, v3, vn1, vn2, vn3;

			if (sscanf(s1.c_str(), "%d//%d", &v1, &vn1) != 2 ||
				sscanf(s2.c_str(), "%d//%d", &v2, &vn2) != 2 ||
				sscanf(s3.c_str(), "%d//%d", &v3, &vn3) != 2) {
				DebugWindow(L"Failed to parse file!!");
				std::exit(1);
			}
			vIndices.push_back(Triplet(v1 - 1, v2 - 1, v3 - 1));
			nIndices.push_back(Triplet(vn1 - 1, vn2 - 1, vn3 - 1));
		}
	}

	std::vector<unsigned int> piFaceBuffer;
	std::vector<Vertex> pvVertexBuffer(pvCoord.size());
	for (int i = 0; i < vIndices.size(); i++) {
		Triplet vt = vIndices[i];
		Triplet nt = nIndices[i];

		pvVertexBuffer[vt.a].Position = pvCoord[vt.a];
		pvVertexBuffer[vt.a].Normal = pvNormal[nt.a];
		pvVertexBuffer[vt.b].Position = pvCoord[vt.b];
		pvVertexBuffer[vt.b].Normal = pvNormal[nt.b];
		pvVertexBuffer[vt.c].Position = pvCoord[vt.c];
		pvVertexBuffer[vt.c].Normal = pvNormal[nt.c];

		piFaceBuffer.push_back(vt.a);
		piFaceBuffer.push_back(vt.b);
		piFaceBuffer.push_back(vt.c);
	}
	ifs.close();

	// ---------------------------------------------------------------------
	//  描画用のVertex Bufferを作成
	// ---------------------------------------------------------------------
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * pvVertexBuffer.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = (void*)&pvVertexBuffer[0];
	if (FAILED(device->CreateBuffer(&bd, &InitData, vertexBuffer))) {
		DebugWindow(L"Failed to create vertex buffer!!");
		return FALSE;
	}

	// ---------------------------------------------------------------------
	//  描画用のIndex Bufferを作成
	// ---------------------------------------------------------------------	
	numFace = vIndices.size();
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(int) * numFace * 3;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	InitData.pSysMem = (void*)&piFaceBuffer[0];
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	if (FAILED(device->CreateBuffer(&bd, &InitData, indexBuffer))) {
		DebugWindow(L"Failed to create index buffer!!");
		return FALSE;
	}

	return S_OK;
}