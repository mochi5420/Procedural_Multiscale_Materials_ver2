#ifndef _SPHERE_H_
#define _SPHERE_H_


//------------------------------------------------------------------------------------------
//	Includes
//------------------------------------------------------------------------------------------
#include <d3d11.h>
#include <d3dx10.h>
#include <vector>


//------------------------------------------------------------------------------------------
//	Linker
//------------------------------------------------------------------------------------------
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx10.lib")


//------------------------------------------------------------------------------------------
//	ObjMeshLoader class
//------------------------------------------------------------------------------------------
class Sphere
{
public:
	Sphere();
	~Sphere();
	HRESULT CreateSphere(ID3D11Device* device, ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, DWORD& numFace, int LatLines, int LongLines);

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
		Vertex() {}
		Vertex(float x, float y, float z,
			float nx, float ny, float nz,
			float u, float v)
			: pos(x, y, z), normal(nx, ny, nz), texCoord(u, v){}

		D3DXVECTOR3 pos;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texCoord;
	};

};

#endif // !_SPHERE_H_