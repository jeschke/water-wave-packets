//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

#include "GlobalDefs.h"

#pragma warning( disable : 3571)

float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // View * Projection matrix
float	g_diffX;					// x size of grid texture
float	g_diffY;					// y size of grid texture

Texture2D g_waterTerrainTex;	// water depth and terrain height texture
Texture2D g_waterPosTex;		// position of water samples
Texture2D g_waterHeightTex;		// displacement of water samples

#define PI 3.14159265359

//--------------------------------------------------------------------------------------
// shader input and output structures
//--------------------------------------------------------------------------------------

//------ Vertex shader structures

struct VS_INPUT
{
    float2 pos		: POSITION;
};

struct VS_QUAD_INPUT
{
    float2 Pos		: POSITION;
    float2 Tex		: TEXTURE0;
};

struct VS_INPUT_PACKET
{
	float4 vPos	: POSITION;  // (x,y) = world position of this vertex, z,w = direction of traveling
	float4 Att	: TEXTURE0;  // x = amplitude, w = time this ripple was initialized
	float4 Att2	: TEXTURE1;
};

//------- Pixel shader structures

struct PS_INPUT
{
	float4 oPosition : SV_POSITION;
    float2 Tex		 : TEXTURE0;
};

struct PS_INPUT_POS
{
	float4 oPosition : SV_POSITION;
	float2 Tex		 : TEXTURE0;
	float3 Pos		 : TEXTURE1; // world space position
};

struct PS_INPUT_PACKET
{
	float4 oPosition	 : SV_POSITION;
	float4 pos			 : TEXTURE0;
	float4 Att			 : TEXTURE1; 
	float4 Att2			 : TEXTURE2; 
};

struct PS_OUTPUT
{
	float4 oColor	: SV_Target0;
};


//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState PointSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

SamplerState LinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

//--------------------------------------------------------------------------------------
// Vetex shaders
//--------------------------------------------------------------------------------------

// this rasterizes position of the wave mesh on screel
VS_INPUT_PACKET PacketVS(VS_INPUT_PACKET In)
{
    return In;
}


PS_INPUT_POS DisplayWaveMeshVS(VS_INPUT In )
{
	PS_INPUT_POS Out;
	Out.Tex = float2(In.pos.x, In.pos.y);
	Out.Pos = 0.5*SCENE_EXTENT*float3(In.pos.x, 0, In.pos.y);
	Out.oPosition = mul( float4(Out.Pos.xyz,1.0), g_mWorldViewProjection);
	return Out;
}



// takes a simple 2D vertex on the ground plane, offsets it along y by the land or water offset and projects it on screen
PS_INPUT_POS DisplayMicroMeshVS(VS_INPUT In)
{
	PS_INPUT_POS Out;
	Out.Tex = In.pos.xy;
	float4 pos = g_waterPosTex.SampleLevel(PointSampler, In.pos.xy, 0.0).xyzw;
	if (pos.w <= 0.0)  // if no position data has been rendered to this texel, there is no water surface here
	{
		Out.Pos = float3(0, 0, 0);
		Out.oPosition = float4(0, 0, 0, -1.0);
	}
	else
	{
		Out.Pos = pos.xyz + g_waterHeightTex.SampleLevel(LinearSampler, In.pos.xy, 0).xyz;
		Out.oPosition = mul(float4(Out.Pos.xyz, 1.0), g_mWorldViewProjection);
	}
	return Out;
}



// takes a simple 2D vertex on the ground plane, offsets it along y by the land or water offset and projects it on screen
PS_INPUT_POS DisplayTerrainVS(VS_INPUT In)
{
	PS_INPUT_POS Out;
	Out.Tex = 0.5*(In.pos.xy+float2(1.0,1.0));
	float h = 0.001*(-3.5+80.0*g_waterTerrainTex.SampleLevel(LinearSampler, Out.Tex, 0).y);
	Out.Pos = SCENE_EXTENT*0.5*float3(In.pos.x, h, In.pos.y);
	Out.oPosition = mul(float4(Out.Pos,1.0), g_mWorldViewProjection);
	return Out;
}



// this rasterizes a quad for full screen AA
PS_INPUT RenderQuadVS(VS_QUAD_INPUT In)
{
	PS_INPUT Out;
	Out.Tex = In.Tex;
	Out.oPosition = float4(In.Pos.xy, 0.5, 1.0);
	return Out;
}



// ================================================================================
//
// Geometry shader
//
// ================================================================================

[maxvertexcount(4)]
void PacketGS( point VS_INPUT_PACKET input[1], inout TriangleStream<PS_INPUT_PACKET> tStream )
{
	if (input[0].vPos.x <-9000)
		return;
	PS_INPUT_PACKET p0;
	p0.Att = input[0].Att.xyzw; 
	p0.Att2 = input[0].Att2.xyzw;
	float3 cPos = float3(input[0].vPos.x, 0, input[0].vPos.y );	// "dangling patch" center
	float3 depth = float3(input[0].vPos.z, 0, input[0].vPos.w );// vector in traveling direction (orthogonal to crest direction)
	float3 width =  float3(depth.z, 0, -depth.x);				// vector along wave crest (orthogonal to travel direction)
	float dThickness = input[0].Att.w;							// envelope size of packet (=depthspread)
	float wThickness = input[0].Att.w;							// rectangular constant sidewidth patches (but individual thickness = envelope size)
	float3 p1 = cPos + 0.0*depth		- wThickness*width;		// neighboring packet patches overlap by 50%
	float3 p2 = cPos - dThickness*depth - wThickness*width;
	float3 p3 = cPos + 0.0*depth		+ wThickness*width;
	float3 p4 = cPos - dThickness*depth + wThickness*width;
	p0.pos = float4(-1,1, -input[0].Att.w, 0);
	p0.oPosition = mul(float4(p1,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	p0.pos = float4(-1,-1, -input[0].Att.w, -input[0].Att.w);
	p0.oPosition = mul(float4(p2,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	p0.pos = float4(1,1, input[0].Att.w, 0);
	p0.oPosition = mul(float4(p3,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	p0.pos = float4(1,-1, input[0].Att.w, -input[0].Att.w);
	p0.oPosition = mul(float4(p4,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	tStream.RestartStrip();
}



[maxvertexcount(5)]
void PacketOutlinedGS( point VS_INPUT_PACKET input[1], inout LineStream<PS_INPUT> tStream )
{
	if (input[0].vPos.x <-9000)
		return;
	PS_INPUT p0;
	p0.Tex = float2(0,0);
	float dThickness = input[0].Att.w;  // envelope size of packet (=depthspread)
	const float wThickness = input[0].Att.w; // rectangular constant sidewidth patches (but individual thickness = envelope size)
	float3 cPos = float3(input[0].vPos.x, 0, input[0].vPos.y );	// "dangling patch" center
	float3 dVec = float3(input[0].vPos.z, 0, input[0].vPos.w ); // vector in traveling direction (orthogonal to crest direction)
	float3 dVec2 =  float3(dVec.z, 0, -dVec.x);					// vector along wave crest (orthogonal to travel direction)
	float3 p1 = cPos - wThickness*dVec2;						// neighboring packet patches overlap by 50%
	float3 p2 = cPos - wThickness*dVec2 - dThickness*dVec;		
	float3 p3 = cPos + wThickness*dVec2;						
	float3 p4 = cPos + wThickness*dVec2 - dThickness*dVec;		
	p0.oPosition = mul(float4(p1,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	p0.oPosition = mul(float4(p2,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	p0.oPosition = mul(float4(p4,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	p0.oPosition = mul(float4(p3,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	p0.oPosition = mul(float4(p1,1.0), g_mWorldViewProjection);
	tStream.Append( p0 );
	tStream.RestartStrip();
}



[maxvertexcount(3)]
void DisplayMicroMeshGS( triangle PS_INPUT_POS input[3], inout TriangleStream<PS_INPUT_POS> tStream )
{
	if ((input[0].oPosition.w<0.01) || (input[1].oPosition.w<0.01) || (input[2].oPosition.w<0.01))
		return;
	tStream.Append(input[0]);
	tStream.Append(input[1]);
	tStream.Append(input[2]);
}


// ================================================================================
//
// Pixel Shaders
//
// ================================================================================


// rasterize wave packet quad
// wave packet data:
// position vector: x,y = [-1..1], position in envelope
// attribute vector: x=amplitude, y=wavelength, z=time phase, w=envelope size
// attribute2 vector: (x,y)=position of bending point, z=central distance to ref point, 0
PS_OUTPUT PacketDisplacementPS(PS_INPUT_PACKET In)
{
	PS_OUTPUT Out;
	float centerDiff = length(In.pos.zw - float2(0.0f, In.Att2.x)) - abs(In.pos.w - In.Att2.x);   //	centerDiff = 0; -> straight waves
	float phase = -In.Att.z + (In.pos.w + centerDiff)*2.0*PI / In.Att.y;
	float3 rippleAdd =	1.0*(1.0 + cos(In.pos.x*PI)) *(1.0 + cos(In.pos.y*PI))*In.Att.x // gaussian envelope
						* float3(0, cos(phase), 0);									 	 // ripple function
	Out.oColor.xyzw = float4(rippleAdd, 1.0);
	return Out;
}



PS_OUTPUT WavePositionPS(PS_INPUT_POS In)
{
	PS_OUTPUT Out; 
	Out.oColor = float4(In.Pos.xyz, 1.0);
	return Out;
}



//water rendering
PS_OUTPUT DisplayWaterPS(PS_INPUT_POS In)
{
	PS_OUTPUT Out;
	// the derivative of the water displacement texture gives us the water surface normal
	float3 pos = g_waterPosTex.SampleLevel(LinearSampler, In.Tex.xy, 0).xyz + g_waterHeightTex.SampleLevel(LinearSampler, In.Tex.xy, 0).xyz;
	float3 nVec = cross(ddx(pos), -ddy(pos));
	if (dot(nVec, nVec) <= 0)
		nVec = float3(0, -1, 0);
	else
		nVec = normalize(nVec);
	float3 vDir = normalize(In.Pos - g_mWorld[3].xyz);	// view vector
	float3 rDir = vDir - (2.0*dot(vDir, nVec))*nVec;	// reflection vector
	// diffuse/reflective lighting
	float3 color = float3(0.5, 0.6, 0.8);
	float fac = 1.0 - (1.0 - abs(nVec.y) + abs(rDir.y))*(1.0 - abs(nVec.y) + abs(rDir.y));
	Out.oColor.xyz = fac*fac*float3(0.5, 0.6, 0.8);
	// add few specular glares
	const float3 glareDir1 = normalize(float3(-1, -0.75, 1));
	const float3 glareDir2 = normalize(float3(1, -0.75, -1));
	const float3 glareDir3 = normalize(float3(1, -0.75, 1));
	const float3 glareDir4 = normalize(float3(-1, -0.75, -1));
	const float3 glareDir5 = normalize(float3(0, -1, 0));
	Out.oColor.xyz += 100.0*pow(max(dot(-rDir, glareDir5), max(dot(-rDir, glareDir4), max(dot(-rDir, glareDir3), max(dot(-rDir, glareDir2), max(0.0, dot(-rDir, glareDir1)))))), 5000);
	// grid overlay
	float floorFactor = 1.0;
	float sth = 0.06;
	float posfac = 1.2*80.0 / SCENE_EXTENT;
	if (frac(posfac*pos.x) < sth)
		floorFactor = 0.5 - 0.5*cos(-PI + 2.0*PI*frac(posfac*pos.x)/ sth);
	if (frac(posfac*pos.z) < sth)
		floorFactor = min(floorFactor, 0.5 - 0.5*cos(-PI + 2.0*PI*frac(posfac*pos.z) / sth));
	Out.oColor.xyz *= (0.75+0.25*floorFactor);
	float waterDepth = 1.0 + 0.9*pow(g_waterTerrainTex.SampleLevel(LinearSampler, In.Pos.xz / SCENE_EXTENT + float2(0.5, 0.5), 0).z, 4);
	Out.oColor.xyz = waterDepth*Out.oColor.xyz;
	Out.oColor.w = 1.0;
	return Out;
}



// display the landscape
PS_OUTPUT DisplayTerrainPS(PS_INPUT_POS In)
{
	PS_OUTPUT Out; 
	if (In.Pos.y < -0.1)
		clip(-1);
	Out.oColor.xyz = (0.25+0.75*In.Pos.y)*float3(0.75, 0.75, 0.75);
	Out.oColor.w = 1.0;
	return Out;
}



// mesh overlay
PS_OUTPUT DisplayWaveMeshOverlayPS(PS_INPUT In)
{
	PS_OUTPUT Out; 
	Out.oColor = float4(0,0,0,1);
	return Out;
}



// downsampling (anti aliasing)
PS_OUTPUT RenderAAPS(PS_INPUT In) 
{ 
    PS_OUTPUT Out;
	const float w[25] = {	0.00296901674395065, 0.013306209891014005, 0.02193823127971504, 0.013306209891014005, 0.00296901674395065, 
							0.013306209891014005, 0.05963429543618023, 0.09832033134884507, 0.05963429543618023, 0.013306209891014005, 
							0.02193823127971504, 0.09832033134884507, 0.16210282163712417, 0.09832033134884507, 0.02193823127971504, 
							0.013306209891014005, 0.05963429543618023, 0.09832033134884507, 0.05963429543618023, 0.013306209891014005, 
							0.00296901674395065, 0.013306209891014005, 0.02193823127971504, 0.013306209891014005, 0.00296901674395065, 
						};
	Out.oColor = float4(0,0,0,1.0);
	for (float y=-2; y<2.5; y++)
		for (float x=-2; x<2.5; x++)
		    Out.oColor += w[((int)(y)+2)*5+(int)(x)+2]*g_waterHeightTex.SampleLevel(LinearSampler, float2(In.Tex.x,In.Tex.y)+float2(x*g_diffX,y*g_diffY), 0);
	return Out;
}



//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState soliddepth
{
	DepthEnable = FALSE;
};


RasterizerState state
{
	FillMode = Solid;
	CullMode = None;
	MultisampleEnable = FALSE;
};


RasterizerState AAstate
{
	FillMode = Solid;
	CullMode = None;
	MultisampleEnable = TRUE;
};


RasterizerState stateOutline
{
	FillMode = WireFrame;
	DepthBias = -1.0;
	CullMode = none;
	MultisampleEnable = FALSE;
};


BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};


BlendState BlendAdd
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
	SrcBlend = ONE;
	DestBlend = ONE;
  	BlendOp = ADD;
	SrcBlendAlpha = ONE;
	DestBlendAlpha = ONE;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------


technique11 RasterizeWaveMeshPosition
{
	pass P1
	{
		SetVertexShader(CompileShader(vs_4_0, DisplayWaveMeshVS( )));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, WavePositionPS( )));
		SetRasterizerState(state);
		SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), 0xFFFFFFFF);
		SetDepthStencilState(soliddepth, 0);
	}
}

technique11 AddPacketDisplacement
{
	pass P1
	{
        SetVertexShader( CompileShader( vs_4_0, PacketVS( ) ) );
        SetGeometryShader( CompileShader( gs_4_0, PacketGS( ) ) );
        SetPixelShader( CompileShader( ps_4_0, PacketDisplacementPS( ) ) );
        SetRasterizerState( state );
        SetBlendState( BlendAdd, float4( 1.0f, 1.0f, 1.0f, 1.0f ), 0xFFFFFFFF );
        SetDepthStencilState( soliddepth, 0 );
    }
}


technique11 DisplayPacketsOutlined
{
	pass P1
	{
        SetVertexShader( CompileShader( vs_4_0, PacketVS( ) ) );
        SetGeometryShader( CompileShader( gs_4_0, PacketOutlinedGS( ) ) );
        SetPixelShader( CompileShader( ps_4_0, DisplayWaveMeshOverlayPS( ) ) );
        SetRasterizerState( stateOutline );
        SetBlendState( NoBlending, float4( 1.0f, 1.0f, 1.0f, 1.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 0 );
    }
}



technique11 DisplayMicroMesh
{
	pass P1
	{
        SetVertexShader( CompileShader( vs_4_0, DisplayMicroMeshVS( ) ) );
        SetGeometryShader( CompileShader( gs_4_0, DisplayMicroMeshGS()) );
        SetPixelShader( CompileShader( ps_4_0, DisplayWaterPS() ) );
        SetRasterizerState( state );
        SetBlendState( NoBlending, float4( 1.0f, 1.0f, 1.0f, 1.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 0 );
    }
}


technique11 DisplayTerrain
{
	pass P1
	{
        SetVertexShader( CompileShader( vs_4_0, DisplayTerrainVS( ) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, DisplayTerrainPS( ) ) );
        SetRasterizerState( state );
        SetBlendState( NoBlending, float4( 1.0f, 1.0f, 1.0f, 1.0f ), 0xFFFFFFFF );
        SetDepthStencilState( EnableDepth, 0 );
    }
}


technique11 RenderAA
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, RenderQuadVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderAAPS() ) );
		SetRasterizerState( state );
        SetBlendState( NoBlending, float4( 1.0f, 1.0f, 1.0f, 1.0f ), 0xFFFFFFFF );
        SetDepthStencilState( soliddepth, 0 );
    }
}