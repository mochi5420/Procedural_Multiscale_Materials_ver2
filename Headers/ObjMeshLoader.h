#ifndef _ObjMeshLoader_H_
#define _ObjMeshLoader_H_

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
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


//------------------------------------------------------------------------------------------
//	Linker
//------------------------------------------------------------------------------------------
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")
#pragma comment(lib,"d3dx11.lib")


//------------------------------------------------------------------------------------------
//	ObjMeshLoader class
//------------------------------------------------------------------------------------------
class ObjMeshLoader 
{
public:
	ObjMeshLoader();
	~ObjMeshLoader();
	HRESULT CreateMesh(ID3D11Device* device, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, DWORD& numFace, LPSTR FileName);

private:

	//=============================================================================================
	//	ç\ë¢ëÃ
	//=============================================================================================
	struct Triplet {
		Triplet(unsigned int a_ = 0u, unsigned int b_ = 0u, unsigned int c_ = 0u)
			: a(a_), b(b_), c(c_) {}
		unsigned int a, b, c;
	};

	struct Vertex
	{
		D3DXVECTOR4 Position;
		D3DXVECTOR3 Normal;
	};

};


#endif // !_ObjMeshLoader_H_