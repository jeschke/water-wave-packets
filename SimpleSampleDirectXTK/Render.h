#pragma once

#include <iostream>
#include "GlobalDefs.h"
#include "SDKmisc.h"
#include "d3dx11effect.h"

using namespace std;
using namespace DirectX;


struct SIMPLE_Vertex
{
	XMFLOAT2 pos; // Position
};

struct QUAD_Vertex
{
	XMFLOAT2 pos; // Position
	XMFLOAT2 tex; // Texture coords
};

struct PACKET_Vertex
{
	XMFLOAT4 posDir;	// Position and direction of this wave packet
	XMFLOAT4 att;		// wave packet attributes needed for rendering: x=amplitude, y=wavelength, z=phase offset within packed, w=envelope size
	XMFLOAT4 att2;		// wave packet attributes needed for rendering: x = center of wave bending circle
};


__declspec(align(16)) class Render
{
public:
	ID3D11Buffer	*m_pQuadMesh;
	ID3D11Buffer	*m_pDisplayMesh;		// fine mesh that gets projected on the ocean surface
	ID3D11Buffer	*m_pDisplayMeshIndex;
	ID3D11Buffer	*m_pHeightfieldMesh;
	ID3D11Buffer	*m_ppacketPointMesh;

	ID3D11Texture2D	*m_heightTexture;
	ID3D11Texture2D	*m_posTexture; 
	ID3D11Texture2D	*m_AATexture; 
	ID3D11Texture2D	*m_pAADepthStencil; 

	ID3D11InputLayout* m_pVertexLayout;

	ID3DX11Effect* g_pEffect11;

	ID3D11ShaderResourceView	*m_heightTextureRV;
	ID3D11RenderTargetView		*m_heightTextureTV;
	ID3D11ShaderResourceView	*m_posTextureRV;
	ID3D11RenderTargetView		*m_posTextureTV;
	ID3D11ShaderResourceView	*m_AATextureRV;
	ID3D11RenderTargetView		*m_AATextureTV;
	ID3D11DepthStencilView		*m_pAADepthStencilView;
	ID3D11ShaderResourceView	*m_waterTerrainTextureRV;

	ID3DX11EffectTechnique *m_pAddPacketsDisplacementTechnique;
	ID3DX11EffectTechnique *m_pRasterizeWaveMeshPositionTechnique;
	ID3DX11EffectTechnique *m_pDisplayPacketQuadsOutlined;
	ID3DX11EffectTechnique *m_pDisplayMicroMeshTechnique;
	ID3DX11EffectTechnique *m_pDisplayTerrain;
	ID3DX11EffectTechnique *m_pDisplayAATechnique;

	ID3DX11EffectShaderResourceVariable* m_pOutputTex;
	ID3DX11EffectShaderResourceVariable* m_pWaterPosTex;
	ID3DX11EffectShaderResourceVariable* m_pWaterTerrainTex;

	ID3DX11EffectScalarVariable*	m_pDiffX;
	ID3DX11EffectScalarVariable*	m_pDiffY;

	ID3DX11EffectMatrixVariable*	g_pmWorldViewProjection = nullptr;
	ID3DX11EffectMatrixVariable*	g_pmWorld = nullptr;

	// viewports for rendering 
	D3D11_VIEWPORT m_vp,m_vpAA;

	// GPU packets 
	PACKET_Vertex *m_packetData;

	// terrain and objects
	int			m_heightfieldVertNum;
	int			m_displayMeshVertNum;
	int			m_displayMeshIndexNum;

	static D3D11_INPUT_ELEMENT_DESC SimpleVertexElements[];
	static D3D11_INPUT_ELEMENT_DESC QuadElements[];
	static D3D11_INPUT_ELEMENT_DESC PacketElements[];
	static int SimpleVertexElementCount;
	static int QuadElementCount;
	static int PacketElementCount;

	virtual ~Render()
	{
	}
	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}

	Render();
	int Release(void);
	void UpdateDisplayMesh();
	void InitiateWavefield(XMMATRIX &mWorld, XMMATRIX &mWorldViewProjectionWide);
	void EvaluatePackets(int usedpackets);
	void DisplayScene(bool showPacketQuads, int usedpackets, XMMATRIX &mWorldViewProjection);
};
