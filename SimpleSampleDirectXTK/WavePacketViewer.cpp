//--------------------------------------------------------------------------------------
// File: WavePacketViewer.cpp
//
// Direct3D 11 Win32 desktop using DirectX Tool Kit and DXUT.
//
//--------------------------------------------------------------------------------------
#pragma warning( disable : 4100 )

#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "resource.h"
#include "Packets.h"
#include "Render.h"

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CFirstPersonCamera					g_Camera;               // A model viewing camera
CDXUTDialogResourceManager			g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg						g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*					g_pTxtHelper = nullptr;
CDXUTDialog							g_HUD;                  // dialog for standard controls
CDXUTDialog							g_SampleUI;             // dialog for sample specific controls

// simulation and rendering variables 
float								m_waveSpeedUI = 0.07f;	// timestep for 60Hz: 16ms = 0.016s
bool								g_showMenue = true;
Packets*							g_packets;
Render*								g_render;
XMVECTOR							m_cursPos;
int									m_displayedPackets = 0;
LARGE_INTEGER						StartingTimeSim, EndingTimeSim, ElapsedMicrosecondsSim, Frequency;  // simulation timing-related 


//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------

#define IDC_WAVESPEED	        10
#define IDC_WAVESPEED_STATIC	11
#define IDC_PACKETBUDGET		12
#define IDC_PACKETBUDGET_STATIC 13
#define IDC_SHOWENVELOPES		20
#define IDC_RESET   			30
#define IDC_NEWWAVE   			31

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
                                       const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext );

void InitApp();
void RenderText(float fElapsedTime);

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    // DXUT will create and use the best device
    // that is available on the system depending on which D3D callbacks are set below
    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    InitApp();
    DXUTInit( true, true, nullptr ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"Wave packets viewer" );
    // Only require 10-level hardware, change to D3D_FEATURE_LEVEL_11_0 to require 11-class hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 720 );
    DXUTMainLoop(); // Enter into the DXUT render loop
	return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	g_SettingsDlg.Init(&g_DialogResourceManager);
	g_HUD.Init(&g_DialogResourceManager);
	g_SampleUI.Init(&g_DialogResourceManager);
	g_HUD.SetCallback(OnGUIEvent);
	g_SampleUI.SetCallback(OnGUIEvent);
	WCHAR sz[100];
	int iY = -300;
	g_SampleUI.AddButton(IDC_RESET, L"Reset", 0, iY += 48, 125, 22, 'N');
	g_SampleUI.AddButton(IDC_NEWWAVE, L"New wave", 0, iY += 28, 125, 22, 'R');
	swprintf_s(sz, 100, L"Simulation speed");
	g_SampleUI.AddStatic(IDC_WAVESPEED_STATIC, sz, 0, iY += 28, 125, 22);
	g_SampleUI.AddSlider(IDC_WAVESPEED, 15, iY += 18, 100, 22, 0, 10000, (int)(m_waveSpeedUI*10000.0f));
	swprintf_s(sz, 100, L"Packet budget: 100000");
	g_SampleUI.AddStatic(IDC_PACKETBUDGET_STATIC, sz, 0, iY += 28, 125, 22);
	g_SampleUI.AddSlider(IDC_PACKETBUDGET, 15, iY += 18, 100, 22, 1000, 1000000, 100000);
	g_SampleUI.AddCheckBox(IDC_SHOWENVELOPES, L"Show envelopes", 0, iY += 40, 125, 22, false, 'L', false);
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text.
//--------------------------------------------------------------------------------------
void RenderText(float fElapsedTime)
{
	g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos(2, 0);
	FXMVECTOR fgColor = 0.75*Colors::Red + 0.5*Colors::Black;
	g_pTxtHelper->SetForegroundColor(fgColor);
	g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
	g_pTxtHelper->DrawFormattedTextLine(L"Time: %0.9fs  (timestep: %0.9fs)", g_packets->m_time, g_packets->m_elapsedTime);
	g_pTxtHelper->DrawFormattedTextLine(L"Simtime: %0.0fms Rendertime: %0.0fms", (double)(ElapsedMicrosecondsSim.QuadPart) / 1000.0, (double)(fElapsedTime)*1000.0 - (double)(ElapsedMicrosecondsSim.QuadPart) / 1000.0);
	float c = 5.0f*std::max<float>(0.0f, (float)(g_packets->m_usedPackets) / (float)(g_packets->m_packetBudget) - 1.0f);
	g_pTxtHelper->SetForegroundColor(max(0.0f,(1.0f - c))*fgColor + c*Colors::Red);
	g_pTxtHelper->DrawFormattedTextLine(L"%i packets simulated and %i ghost packets active", g_packets->m_usedPackets, g_packets->m_usedGhosts);
	g_pTxtHelper->SetForegroundColor(fgColor);
	g_pTxtHelper->DrawFormattedTextLine(L"Damping factor: %0.4f", g_packets->m_softDampFactor);
	g_pTxtHelper->End();
}



//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
                                       const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;
    auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_SettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );
	static const XMVECTORF32 vecPos = { 10.17f, 22.13f, 59.49f, 0.0f };
	static const XMVECTORF32 lookAt = { 10.17f, 21.66f, 58.61f, 0.0f };
	g_Camera.SetViewParams( vecPos, lookAt );
	g_render = new Render();
	g_packets = new Packets(g_SampleUI.GetSlider(IDC_PACKETBUDGET)->GetValue());
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( XM_PI / 4.0, fAspectRatio, 0.5f, 4000.0f );
	g_Camera.SetEnablePositionMovement(true);
	g_Camera.SetResetCursorAfterMove(false);
	g_Camera.SetScalers(0.003, 30);
    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300 );
    g_SampleUI.SetSize( 170, 300 );
	g_render->UpdateDisplayMesh();
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext )
{
	// Packet simulation (and timing)
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTimeSim);
	g_packets->AdvectWavePackets(m_waveSpeedUI);
	QueryPerformanceCounter(&EndingTimeSim);
	ElapsedMicrosecondsSim.QuadPart = EndingTimeSim.QuadPart - StartingTimeSim.QuadPart;
	ElapsedMicrosecondsSim.QuadPart *= 1000000;
	ElapsedMicrosecondsSim.QuadPart /= Frequency.QuadPart;

	// scene and packet display
	// get projection & view matrix from the camera class
	XMMATRIX mWorld = g_Camera.GetWorldMatrix();
	XMMATRIX mView = g_Camera.GetViewMatrix();
	XMMATRIX mProj = g_Camera.GetProjMatrix();
	XMMATRIX mWorldViewProjection = XMMatrixMultiply(mView, mProj);
	CFirstPersonCamera   camWide = g_Camera;
	float fAspectRatio = DXUTGetDXGIBackBufferSurfaceDesc()->Width / (FLOAT)DXUTGetDXGIBackBufferSurfaceDesc()->Height;
	camWide.SetProjParams(XM_PI / 3.0, fAspectRatio, 0.1f, 15000.0f);
	XMMATRIX  mWorldViewProjectionWide = XMMatrixMultiply(mView, camWide.GetProjMatrix());
	g_render->InitiateWavefield(mWorld, mWorldViewProjectionWide);   
	// transfer and rasterize wave packets and ghost packets to GPU datastructure
	m_displayedPackets = 0;
	int packetChunk = 0;
	for (int uP = 0; uP<g_packets->m_usedPackets; uP++)  // regular wave packets
		if (!g_packets->m_packet[g_packets->m_usedPacket[uP]].use3rd)
		{
			int i1 = g_packets->m_usedPacket[uP];
			g_render->m_packetData[packetChunk].posDir = XMFLOAT4(g_packets->m_packet[i1].midPos.x(), g_packets->m_packet[i1].midPos.y(), g_packets->m_packet[i1].travelDir.x(), g_packets->m_packet[i1].travelDir.y());
			g_render->m_packetData[packetChunk].att = XMFLOAT4(g_packets->m_packet[i1].ampOld, (float)(2.0*XM_PI / g_packets->m_packet[i1].k), (float)(g_packets->m_packet[i1].phase), (float)(g_packets->m_packet[i1].envelope));
			g_render->m_packetData[packetChunk].att2 = XMFLOAT4(g_packets->m_packet[i1].bending, 0.0f, 0.0f, 0.0f);
			m_displayedPackets++;
			packetChunk++;
			if (packetChunk >= PACKET_GPU_BUFFER_SIZE)  // send the wave packet data to the GPU
			{
				pd3dImmediateContext->UpdateSubresource(g_render->m_ppacketPointMesh, 0, NULL, g_render->m_packetData, 0, 0);
				g_render->EvaluatePackets(packetChunk);
				packetChunk = 0;
			}
		}
	for (int uG = 0; uG<g_packets->m_usedGhosts; uG++)  // ghost packets
	{
		int i1 = g_packets->m_usedGhost[uG];
		g_render->m_packetData[packetChunk].att = XMFLOAT4((float)(g_packets->m_ghostPacket[i1].ampOld), (float)(2.0*XM_PI / g_packets->m_ghostPacket[i1].k), (float)(g_packets->m_ghostPacket[i1].phase), (float)(g_packets->m_ghostPacket[i1].envelope));
		g_render->m_packetData[packetChunk].posDir = XMFLOAT4((float)(g_packets->m_ghostPacket[i1].pos.x()), (float)(g_packets->m_ghostPacket[i1].pos.y()), (float)(g_packets->m_ghostPacket[i1].dir.x()), (float)(g_packets->m_ghostPacket[i1].dir.y()));
		g_render->m_packetData[packetChunk].att2 = XMFLOAT4(g_packets->m_ghostPacket[i1].bending, 0.0f, 0.0f, 0.0f);
		m_displayedPackets++;
		packetChunk++;
		if (packetChunk >= PACKET_GPU_BUFFER_SIZE)  // send the wave packet data to the GPU
		{
			pd3dImmediateContext->UpdateSubresource(g_render->m_ppacketPointMesh, 0, NULL, g_render->m_packetData, 0, 0); 
			g_render->EvaluatePackets(packetChunk);
			packetChunk = 0;
		}
	}
	pd3dImmediateContext->UpdateSubresource(g_render->m_ppacketPointMesh, 0, NULL, g_render->m_packetData, 0, 0);  // send the wave packet data to the GPU
	g_render->EvaluatePackets(packetChunk);
	g_render->DisplayScene(g_SampleUI.GetCheckBox(IDC_SHOWENVELOPES)->GetChecked(), min(m_displayedPackets, PACKET_GPU_BUFFER_SIZE), mWorldViewProjection);

	// update scene cursor position
	XMVECTOR cPos = g_Camera.GetEyePt();
	XMVECTOR lPos = g_Camera.GetLookAtPt();
	XMVECTOR viewVec = lPos - cPos;
	m_cursPos = XMVECTORF32{ 0, 0 };  // if the camera points up, point to scene center
	if ((XMVectorGetY(viewVec)<0.0) && (XMVectorGetY(cPos)>0.0))
		m_cursPos = XMVectorSet(XMVectorGetX(cPos) - XMVectorGetX(viewVec)*XMVectorGetY(cPos) / XMVectorGetY(viewVec), XMVectorGetZ(cPos) - XMVectorGetZ(viewVec)*XMVectorGetY(cPos) / XMVectorGetY(viewVec), 0, 0);

	// Render HUD
	if (g_showMenue)
	{
		g_HUD.OnRender(fElapsedTime);
		g_SampleUI.OnRender(fElapsedTime);
		RenderText(fElapsedTime);
	}
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	delete(g_packets);
	g_render->Release();
	delete(g_render);

	g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if (bKeyDown)
	{
		switch (nChar)
		{
			case 'M':	{
							g_showMenue = !g_showMenue; 
							break;
						}
		}
	}
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
		case IDC_NEWWAVE:		{
									g_packets->CreateCircularWavefront(XMVectorGetX(m_cursPos), XMVectorGetY(m_cursPos), 2.0, 0.2, 1.0, 10000);	// "all"-wavelength impact
									break;
								}
		case IDC_RESET:			{
									g_packets->Reset();
									break;
								}
		case IDC_WAVESPEED:		{
									m_waveSpeedUI = (float)(g_SampleUI.GetSlider(IDC_WAVESPEED)->GetValue()) / 10000.0f;
									break;
								}
		case IDC_PACKETBUDGET:	{
									WCHAR sz[100];
									swprintf_s(sz, 100, L"Packet budget: %i", g_SampleUI.GetSlider(IDC_PACKETBUDGET)->GetValue());
									g_SampleUI.GetStatic(IDC_PACKETBUDGET_STATIC)->SetText(sz);
									g_packets->m_packetBudget = g_SampleUI.GetSlider(IDC_PACKETBUDGET)->GetValue();
									break;
								}
    }
}
