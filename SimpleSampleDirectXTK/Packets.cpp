// Include the OS headers
//-----------------------
#include <windows.h>
#include <atlbase.h>
#include <strsafe.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include "Packets.h"
#pragma warning( disable: 4996 )


inline float rational_tanh(float x)
{
    if (x < -3.0f)
        return -1.0f;
    else if (x > 3.0f)
        return 1.0f;
    else
        return x*(27.0f + x*x) / (27.0f+9.0f*x*x);
}


float Packets::GetIntersectionDistance(Vector2f pos1, Vector2f dir1, Vector2f pos2, Vector2f dir2)
{
	ParametrizedLine<float, 2> line1(pos1, dir1);
	Hyperplane<float, 2> line2 = Hyperplane<float, 2>::Through(pos2, pos2+dir2);
	float intPointDist = line1.intersectionParameter(line2);
	if (abs(intPointDist) > 10000.0f)
		intPointDist = 10000.0f;
	return intPointDist;
}



inline float Packets::GetGroundVal(Vector2f &p)
{
	Vector2f pTex = Vector2f(p.x()/SCENE_EXTENT+0.5f,p.y()/SCENE_EXTENT+0.5f);		// convert from world space to texture space
	float val1 = m_ground[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	float val2 = m_ground[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	float val3 = m_ground[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	float val4 = m_ground[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	float xOffs = (pTex.x()*m_groundSizeX) - (int)(pTex.x()*m_groundSizeX);
	float yOffs = (pTex.y()*m_groundSizeY) - (int)(pTex.y()*m_groundSizeY);
	float valH1 = (1.0f-xOffs)*val1 + xOffs*val2;
	float valH2 = (1.0f-xOffs)*val3 + xOffs*val4;
	return( (1.0f-yOffs)*valH1 + yOffs*valH2 );
}

inline Vector2f Packets::GetGroundNormal(Vector2f &p)
{
	Vector2f pTex = Vector2f(p.x()/SCENE_EXTENT+0.5f,p.y()/SCENE_EXTENT+0.5f);		// convert from world space to texture space
	Vector2f val1 = m_gndDeriv[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	Vector2f val2 = m_gndDeriv[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	Vector2f val3 = m_gndDeriv[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	Vector2f val4 = m_gndDeriv[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	float xOffs = (pTex.x()*m_groundSizeX) - (int)(pTex.x()*m_groundSizeX);
	float yOffs = (pTex.y()*m_groundSizeY) - (int)(pTex.y()*m_groundSizeY);
	Vector2f valH1 = (1.0f-xOffs)*val1 + xOffs*val2;
	Vector2f valH2 = (1.0f-xOffs)*val3 + xOffs*val4;
	Vector2f res = (1.0f-yOffs)*valH1 + yOffs*valH2;
	Vector2f resN = Vector2f(0,1);
	if (abs(res.x()) + abs(res.y()) > 0.0)
		resN = res.normalized();
	return( resN );
}

inline float Packets::GetBoundaryDist(Vector2f &p)
{
	Vector2f pTex = Vector2f(p.x()/SCENE_EXTENT+0.5f,p.y()/SCENE_EXTENT+0.5f);		// convert from world space to texture space
	float val1 = m_distMap[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	float val2 = m_distMap[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	float val3 = m_distMap[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	float val4 = m_distMap[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	float xOffs = (pTex.x()*m_groundSizeX) - (int)(pTex.x()*m_groundSizeX);
	float yOffs = (pTex.y()*m_groundSizeY) - (int)(pTex.y()*m_groundSizeY);
	float valH1 = (1.0f-xOffs)*val1 + xOffs*val2;
	float valH2 = (1.0f-xOffs)*val3 + xOffs*val4;
	return( (1.0f-yOffs)*valH1 + yOffs*valH2 );
}

inline Vector2f Packets::GetBoundaryNormal(Vector2f &p)
{
	Vector2f pTex = Vector2f(p.x()/SCENE_EXTENT+0.5f,p.y()/SCENE_EXTENT+0.5f);		// convert from world space to texture space
	Vector2f val1 = m_bndDeriv[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	Vector2f val2 = m_bndDeriv[(int)(max(0,min(m_groundSizeY-1,pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	Vector2f val3 = m_bndDeriv[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,pTex.x()*m_groundSizeX)))];
	Vector2f val4 = m_bndDeriv[(int)(max(0,min(m_groundSizeY-1,1+pTex.y()*m_groundSizeY)))*m_groundSizeX + (int)(max(0,min(m_groundSizeX-1,1+pTex.x()*m_groundSizeX)))];
	float xOffs = (pTex.x()*m_groundSizeX) - (int)(pTex.x()*m_groundSizeX);
	float yOffs = (pTex.y()*m_groundSizeY) - (int)(pTex.y()*m_groundSizeY);
	Vector2f valH1 = (1.0f-xOffs)*val1 + xOffs*val2;
	Vector2f valH2 = (1.0f-xOffs)*val3 + xOffs*val4;
	Vector2f res = (1.0f-yOffs)*valH1 + yOffs*valH2;
	Vector2f resN = Vector2f(0,1);
	if (abs(res.x())+abs(res.y()) > 0.0)
		resN = res.normalized();
	return( resN );
}




#pragma warning( push )
#pragma warning( disable : 4996)
Packets::Packets(int packetBudget)
{
	WCHAR wcInfo[512];

	//load ground/boundary texture for CPU processing
	LPCWSTR groundTexFile = WATER_TERRAIN_FILE;
	tagBITMAPFILEHEADER bmpheader;
	tagBITMAPINFOHEADER bmpinfo;
	DWORD bytesread;
	HANDLE file = CreateFile( groundTexFile , GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if (file == NULL)
	{
		throw std::exception("Media file not found");
		return;
	}
	if (   (ReadFile ( file, &bmpheader, sizeof ( BITMAPFILEHEADER ), &bytesread, NULL ) == false)
		|| (ReadFile(file, &bmpinfo, sizeof(BITMAPINFOHEADER), &bytesread, NULL) == false)
		|| (bmpheader.bfType != 'MB')
		|| (bmpinfo.biCompression != BI_RGB)
		|| (bmpinfo.biBitCount != 24) )
	{
		CloseHandle ( file );
		throw std::exception("Error reading media file");
		return;
	}
	m_groundSizeX  = abs(bmpinfo.biWidth);
	m_groundSizeY  = abs(bmpinfo.biHeight);
	long size = bmpheader.bfSize-bmpheader.bfOffBits;
	SetFilePointer ( file, bmpheader.bfOffBits, NULL, FILE_BEGIN );
	BYTE* Buffer = new BYTE[size];
	if ( ReadFile ( file, Buffer, size, &bytesread, NULL ) == false )
	{
		delete[](Buffer);
		CloseHandle(file);
		throw std::exception("Media file not found");
		return;
	}
	CloseHandle(file);
	// convert read buffer to our rgb datastructure
	int padding = 0;
	int scanlinebytes = m_groundSizeX*3;
	while ( ( scanlinebytes + padding ) % 4 != 0 )
		padding++;
	int psw = scanlinebytes + padding;
	m_ground = new float[m_groundSizeX*m_groundSizeY];
	float *bound = new float[m_groundSizeX*m_groundSizeY];
	for (int y=0; y<m_groundSizeY; y++)
		for (int x=0; x<m_groundSizeX; x++)
		{
			int bufpos = (m_groundSizeY-y-1)*psw + x*3;
			m_ground[y*m_groundSizeX + x] = (float)(Buffer[bufpos + 0]) / 255.0f;  // read the blue channel (which is the smoothed depth for directional Tessendorf shader)
			float v = (float)(Buffer[bufpos + 1])/255.0f;									   // read the green channel (contains terrain heightfield)
			if (v > 11.1f/255.0f) 
				bound[y*m_groundSizeX+x] = 1.0f;
			else
				bound[y*m_groundSizeX+x] = 0.0f;
		}
	delete[](Buffer);

	// boundary texture distance transform 
	// init helper distance map (pMap)
	StringCchPrintf(wcInfo, 512, L"Computing boundary distance transform..");
	OutputDebugString(wcInfo);
	int *pMap = new int[m_groundSizeX*m_groundSizeY];
	#pragma omp parallel for
	for (int y = 0; y < m_groundSizeY; y++)
		for (int x = 0; x < m_groundSizeX; x++)
		{
			// if we are at the boundary, intialize the distance function with 0, otherwise with maximum value
			if ((bound[y*m_groundSizeX + x] > 0.5f) &&
				((bound[max(0, min(m_groundSizeY - 1, y + 1))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x + 0))] <= 0.5f)
					|| (bound[max(0, min(m_groundSizeY - 1, y + 0))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x + 1))] <= 0.5f)
					|| (bound[max(0, min(m_groundSizeY - 1, y - 1))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x + 0))] <= 0.5f)
					|| (bound[max(0, min(m_groundSizeY - 1, y + 0))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x - 1))] <= 0.5f)))
				pMap[y*m_groundSizeX + x] = 0;  // initialize with maximum x distance
			else if ((bound[y*m_groundSizeX + x] <= 0.5f) &&
				((bound[max(0, min(m_groundSizeY - 1, y + 1))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x + 0))] > 0.5f)
					|| (bound[max(0, min(m_groundSizeY - 1, y + 0))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x + 1))] > 0.5f)
					|| (bound[max(0, min(m_groundSizeY - 1, y - 1))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x + 0))] > 0.5f)
					|| (bound[max(0, min(m_groundSizeY - 1, y + 0))*m_groundSizeX + max(0, min(m_groundSizeX - 1, x - 1))] > 0.5f)))
				pMap[y*m_groundSizeX + x] = 0;  // initialize with maximum x distance
			else
				pMap[y*m_groundSizeX + x] = m_groundSizeX*m_groundSizeX;  // initialize with maximum x distance
		}
	m_distMap = new float[m_groundSizeX*m_groundSizeY];
	#pragma omp parallel for
	for (int y=0; y<m_groundSizeY; y++)
		for (int x=0; x<m_groundSizeX; x++)
			m_distMap[y*m_groundSizeX+x] = (float)(m_groundSizeX*m_groundSizeX + m_groundSizeY*m_groundSizeY);
	#pragma omp parallel for
	for (int y=0; y<m_groundSizeY; y++)   // horizontal scan forward
	{
		int lastBoundX = -m_groundSizeX;
		for (int x=0; x<m_groundSizeX; x++)
		{
			if (pMap[y*m_groundSizeX+x] == 0)
				lastBoundX = x;
			pMap[y*m_groundSizeX+x] = min(pMap[y*m_groundSizeX+x], (x-lastBoundX)*(x-lastBoundX));
		}
	}
	#pragma omp parallel for
	for (int y=0; y<m_groundSizeY; y++)  // horizontal scan backward
	{
		int lastBoundX = 2*m_groundSizeX;
		for (int x=m_groundSizeX-1; x>=0; x--)
		{
			if (pMap[y*m_groundSizeX+x] == 0)
				lastBoundX = x;
			pMap[y*m_groundSizeX+x] = min(pMap[y*m_groundSizeX+x], (lastBoundX-x)*(lastBoundX-x));
		}
	}
	#pragma omp parallel for
	for (int x=0; x<m_groundSizeX; x++)  // vertical scan forward and backward
		for (int y=0; y<m_groundSizeY; y++)
		{
			int minDist = pMap[y*m_groundSizeX+x];
			for (int yd=1; yd+y<=m_groundSizeY-1; yd++)
			{
				minDist = min(minDist, yd*yd+pMap[(y+yd)*m_groundSizeX+x]);
				if (minDist < yd*yd)
					break;
			}
			for (int yd=-1; yd+y>=0; yd--)
			{
				minDist = min(minDist, yd*yd+pMap[(y+yd)*m_groundSizeX+x]);
				if (minDist < yd*yd)
					break;
			}
			m_distMap[y*m_groundSizeX+x] = (float)(minDist);
		}
	delete[](pMap);
	// m_distMap now contains the _squared_ euklidean distance to closest label boundary, so take the sqroot. And sign the distance
	#pragma omp parallel for
	for (int y=0; y<m_groundSizeY; y++)
		for (int x=0; x<m_groundSizeX; x++)
		{
			m_distMap[y*m_groundSizeX+x] = sqrt(m_distMap[y*m_groundSizeX+x]);
			if (bound[y*m_groundSizeX+x] > 0.5f)
				m_distMap[y*m_groundSizeX+x] = -m_distMap[y*m_groundSizeX+x];		// negative distance INSIDE a boundary regions
			m_distMap[y*m_groundSizeX+x] = m_distMap[y*m_groundSizeX+x]*SCENE_EXTENT / m_groundSizeX;
		}
	StringCchPrintf(wcInfo, 512, L"done!\n");
	OutputDebugString(wcInfo);

	// derivative (2D normal) of the boundary texture
	StringCchPrintf( wcInfo, 512, L"Computing boundary derivatives..");
	OutputDebugString( wcInfo );
	m_bndDeriv = new Vector2f[m_groundSizeX*m_groundSizeY];
	#pragma omp parallel for
	for (int y=0; y<m_groundSizeY; y++)
		for (int x=0; x<m_groundSizeX; x++)
		{
			float dx = m_distMap[y*m_groundSizeX + max(0,min(m_groundSizeX-1,x+1))] - m_distMap[y*m_groundSizeX + x];
			float dy = m_distMap[max(0,min(m_groundSizeY-1,y+1))*m_groundSizeX + x] - m_distMap[y*m_groundSizeX + x];
			Vector2f dV = Vector2f(dx,dy);
			dx = m_distMap[y*m_groundSizeX + x] - m_distMap[y*m_groundSizeX + max(0,min(m_groundSizeX-1,x-1))];
			dy = m_distMap[y*m_groundSizeX + x] - m_distMap[max(0,min(m_groundSizeY-1,y-1))*m_groundSizeX + x];
			dV += Vector2f(dx,dy);
			m_bndDeriv[y*m_groundSizeX+x] = Vector2f(0,0);
			if ((dV.x() != 0) || (dV.y() != 0))
				m_bndDeriv[y*m_groundSizeX+x] = dV.normalized();
		}
	StringCchPrintf( wcInfo, 512, L"done!\n");
	OutputDebugString( wcInfo );
	
	//smooth the derivative to avoid staricase artifacts of the texture
	StringCchPrintf( wcInfo, 512, L"Smoothing boundary derivatives..");
	OutputDebugString( wcInfo );
	Vector2f *m_bndDerivH = new Vector2f[m_groundSizeX*m_groundSizeY];
	for (int i=0; i<2; i++)  //15
	{
		#pragma omp parallel for
		for (int y=0; y<m_groundSizeY; y++)
			for (int x=0; x<m_groundSizeX; x++)
			{
				Vector2f dV = Vector2f(0.0f,0.0f);
				for (int dy=-1; dy<=1; dy++)
					for (int dx=-1; dx<=1; dx++)
						{
							float w = 1.0/16.0;
							if ((abs(dy) == 0) && (abs(dx) == 0))
								w = 4.0/16.0;
							else if ((abs(dy) == 0) || (abs(dx) == 0))
								w = 2.0/16.0;
							dV += w*m_bndDeriv[max(0,min(m_groundSizeY-1,y+dy))*m_groundSizeX + max(0,min(m_groundSizeX-1,x+dx))];
						}
				if ((dV.x() != 0) || (dV.y() != 0))
					m_bndDerivH[y*m_groundSizeX+x] = dV.normalized();
			}
		// copy the result back to derivative map
		memcpy(m_bndDeriv, m_bndDerivH, sizeof(Vector2f)*m_groundSizeX*m_groundSizeY);
	}
	delete[](m_bndDerivH);
	StringCchPrintf( wcInfo, 512, L"done!\n");
	OutputDebugString( wcInfo );

	// derivative (2D normal) of the ground texture
	StringCchPrintf( wcInfo, 512, L"Computing ground derivatives..");
	OutputDebugString( wcInfo );
	m_gndDeriv = new Vector2f[m_groundSizeX*m_groundSizeY];
	#pragma omp parallel for
	for (int y=0; y<m_groundSizeY; y++)
		for (int x=0; x<m_groundSizeX; x++)
		{
			float dx = m_ground[y*m_groundSizeX + max(0,min(m_groundSizeX-1,x+1))] - m_ground[y*m_groundSizeX + x];
			float dy = m_ground[max(0,min(m_groundSizeY-1,y+1))*m_groundSizeX + x] - m_ground[y*m_groundSizeX + x];
			Vector2f dV = Vector2f(dx,dy);
			dx = m_ground[y*m_groundSizeX + x] - m_ground[y*m_groundSizeX + max(0,min(m_groundSizeX-1,x-1))];
			dy = m_ground[y*m_groundSizeX + x] - m_ground[max(0,min(m_groundSizeY-1,y-1))*m_groundSizeX + x];
			dV += Vector2f(dx,dy);
			m_gndDeriv[y*m_groundSizeX+x] = Vector2f(0,0);
			if ((dV.x() != 0) || (dV.y() != 0))
				m_gndDeriv[y*m_groundSizeX+x] = dV.normalized();
		}
	StringCchPrintf( wcInfo, 512, L"done!\n");
	OutputDebugString( wcInfo );

	// init variables
	m_packetBudget = packetBudget;
	m_usedPackets = 0;
	m_freePackets = 0;
	m_usedGhosts = 0;
	m_freeGhosts = 0;
	m_packetNum = 0;
	m_packet = NULL;
	ExpandWavePacketMemory(PACKET_BUFFER_DELTA);
	Reset();
	UpdateTime(0.0);
}
#pragma warning( pop ) 




Packets::~Packets(void)
{
	delete[](m_packet);
	delete[](m_ghostPacket);
	delete[](m_usedPacket);
	delete[](m_freePacket);
	delete[](m_usedGhost);
	delete[](m_freeGhost);
	delete[](m_ground);
	delete[](m_distMap);
	delete[](m_gndDeriv);
	delete[](m_bndDeriv);
}




// initialize all packets as "unused"
void Packets::Reset()
{
	for (int i = 0; i<m_packetNum; i++)
	{
		m_freePacket[i] = i;
		m_freeGhost[i] = i;
	}
	m_usedPackets = 0;			// points BEHIND the last used packet (= first free slot)
	m_freePackets = m_packetNum;// points BEHIND the last free packet
	m_usedGhosts = 0;			// points BEHIND the last used ghost
	m_freeGhosts = m_packetNum;	// points BEHIND the last free ghost packet
	m_time = 0.0f;
	m_oldTime = -1.0;
}




// increase the available packet memory
void Packets::ExpandWavePacketMemory(int targetNum)
{
	if (targetNum < m_packetNum)	// this should never happen
		return;
	WCHAR wcFileInfo[512];
	StringCchPrintf(wcFileInfo, 512, L"(INFO): Expanding packet memory from %i to %i packets (%i MB).\n", m_packetNum, targetNum, (targetNum)*(sizeof(WAVE_PACKET) + sizeof(GHOST_PACKET) + 4 * sizeof(int)) / (1024 * 1024));
	OutputDebugString(wcFileInfo);
	WAVE_PACKET *p = new WAVE_PACKET[targetNum];
	GHOST_PACKET *pG = new GHOST_PACKET[targetNum];
	int *uP = new int[targetNum];
	int *fP = new int[targetNum];
	int *uG = new int[targetNum];
	int *fG = new int[targetNum];
	for (int i=0; i<m_packetNum; i++)
	{
		p[i] = m_packet[i];
		pG[i] = m_ghostPacket[i];
		uP[i] = m_usedPacket[i];
		fP[i] = m_freePacket[i];
		uG[i] = m_usedGhost[i];
		fG[i] = m_freeGhost[i];
	}
	for (int i = 0; i < targetNum - m_packetNum; i++)  
		fP[m_freePackets+i] = m_packetNum + i;
	for (int i = 0; i<targetNum - m_packetNum; i++)  
		fG[m_freeGhosts+i] = m_packetNum + i;
	if (m_packet != NULL)
	{
		delete[](m_packet);
		delete[](m_ghostPacket);
		delete[](m_usedPacket);
		delete[](m_freePacket);
		delete[](m_usedGhost);
		delete[](m_freeGhost);
	}
	m_packet = p;
	m_ghostPacket = pG;
	m_usedPacket = uP;
	m_freePacket = fP;
	m_usedGhost = uG;
	m_freeGhost = fG;
	m_freePackets += (targetNum - m_packetNum);	
	m_freeGhosts += (targetNum - m_packetNum);
	m_packetNum = targetNum;
};




// search a new free slot for a wave packet (must be thread safe in case of parallel computing)
int Packets::GetFreePackedID()
{
	int firstfree;
	#pragma omp critical
	{
		m_freePackets--;
		firstfree = m_freePacket[m_freePackets];	// pop last free packet
		m_usedPacket[m_usedPackets] = firstfree;	// push to used packets 
		m_usedPackets++;
	}
	return firstfree;
}



// search a new free slot for a wave packet (must be thread safe in case of parallel computing)
int Packets::GetFreeGhostID()
{
	int firstghost;
	#pragma omp critical
	{
		m_freeGhosts--;
		firstghost = m_freeGhost[m_freeGhosts];
		m_usedGhost[m_usedGhosts] = firstghost;
		m_usedGhosts++;
	}
	return firstghost;
}


// delete ghost at id
void Packets::DeletePacket(int id)
{
	#pragma omp critical
	{
		m_freePacket[m_freePackets] = m_usedPacket[id];
		m_freePackets++;
		m_usedPackets--;
		m_usedPacket[id] = m_usedPacket[m_usedPackets];
	}
}

// delete ghost at id
void Packets::DeleteGhost(int id)
{
	#pragma omp critical
	{
		m_freeGhost[m_freeGhosts] = m_usedGhost[id];
		m_freeGhosts++;
		m_usedGhosts--;
		m_usedGhost[id] = m_usedGhost[m_usedGhosts];
	}
}



// adds a new packet at given positions, directions and wavenumber interval k_L and k_H
void Packets::CreatePacket(float pos1x, float pos1y, float pos2x, float pos2y, float dir1x, float dir1y, float dir2x, float dir2y, float k_L, float k_H, float E)
{
	// make sure we have enough memory
	if ( max(m_usedPackets, m_usedGhosts) + 10 > m_packetNum)
		ExpandWavePacketMemory(max(m_usedPackets,m_usedGhosts) + PACKET_BUFFER_DELTA);
	float speedDummy, kDummy;
	int	firstfree = GetFreePackedID();
	m_packet[firstfree].pos1 =  Vector2f(pos1x,pos1y);
	m_packet[firstfree].pOld1 = m_packet[firstfree].pos1;
	m_packet[firstfree].pos2 =  Vector2f(pos2x,pos2y);
	m_packet[firstfree].pOld2 = m_packet[firstfree].pos2;
	m_packet[firstfree].dir1  = Vector2f(dir1x,dir1y);
	m_packet[firstfree].dOld1 = m_packet[firstfree].dir1;
	m_packet[firstfree].dir2  = Vector2f(dir2x,dir2y);
	m_packet[firstfree].dOld2 = m_packet[firstfree].dir2;
	m_packet[firstfree].phase = 0.0;			
	m_packet[firstfree].phOld = 0.0;
	m_packet[firstfree].E = E; 
	m_packet[firstfree].use3rd = false;
	m_packet[firstfree].bounced1 = false;
	m_packet[firstfree].bounced2 = false;
	m_packet[firstfree].bounced3 = false;
	m_packet[firstfree].sliding3 = false;
	// set the wavelength/freq interval 
	m_packet[firstfree].k_L = k_L;
	float wd = GetWaterDepth(m_packet[firstfree].pos1);
	m_packet[firstfree].w0_L = sqrt((GRAVITY + k_L*k_L*SIGMA/DENSITY)*k_L*rational_tanh(k_L*wd));	// this take surface tension into account
	m_packet[firstfree].k_H = k_H;
	m_packet[firstfree].w0_H = sqrt((GRAVITY + k_H*k_H*SIGMA/DENSITY)*k_H*rational_tanh(k_H*wd));	// this take surface tension into account
	m_packet[firstfree].d_L = 0.0;
	m_packet[firstfree].d_H = 0.0;
	// set the representative wave as average of interval boundaries
	m_packet[firstfree].k = 0.5f*(m_packet[firstfree].k_L+m_packet[firstfree].k_H);
	m_packet[firstfree].w0 = sqrt((GRAVITY + m_packet[firstfree].k*m_packet[firstfree].k*SIGMA/DENSITY)*m_packet[firstfree].k*rational_tanh(m_packet[firstfree].k*wd));	// this takes surface tension into account
	GetWaveParameters(GetWaterDepth(m_packet[firstfree].pos1), m_packet[firstfree].w0, m_packet[firstfree].k, kDummy, m_packet[firstfree].speed1);
	m_packet[firstfree].sOld1 = m_packet[firstfree].speed1;
	GetWaveParameters(GetWaterDepth(m_packet[firstfree].pos2), m_packet[firstfree].w0, m_packet[firstfree].k, kDummy, m_packet[firstfree].speed2);
	m_packet[firstfree].sOld2 = m_packet[firstfree].speed2;
	m_packet[firstfree].envelope = min(PACKET_ENVELOPE_MAXSIZE, max(PACKET_ENVELOPE_MINSIZE, PACKET_ENVELOPE_SIZE_FACTOR*2.0f*M_PI/m_packet[firstfree].k)); // adjust envelope size to represented wavelength
	m_packet[firstfree].ampOld = 0.0;
	float a1 = min(MAX_SPEEDNESS*2.0f*M_PI / m_packet[firstfree].k, GetWaveAmplitude(m_packet[firstfree].envelope*(m_packet[firstfree].pos1 - m_packet[firstfree].pos2).norm(), m_packet[firstfree].E, m_packet[firstfree].k));
	m_packet[firstfree].dAmp = 0.5f*(m_packet[firstfree].speed1+m_packet[firstfree].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[firstfree].envelope)*a1;

	// Test for wave number splitting -> if the packet interval crosses the slowest waves, divide so that each part has a monotonic speed function (assumed for travel spread/error calculation)
	int i1=firstfree;
	if ((m_packet[i1].w0_L>PACKET_SLOWAVE_W0) && (m_packet[i1].w0_H<PACKET_SLOWAVE_W0))
	{
		firstfree = GetFreePackedID();
		m_packet[firstfree] = m_packet[i1];  // copy the entiry wave packet information
		// set new interval boundaries to the slowest wave and update all influenced parameters
		m_packet[firstfree].k_L = PACKET_SLOWAVE_K;
		m_packet[firstfree].w0_L = PACKET_SLOWAVE_W0;
		GetWaveParameters(wd, m_packet[firstfree].w0_L, m_packet[firstfree].k_L, m_packet[firstfree].k_L, speedDummy);
		m_packet[firstfree].w0 = 0.5f*(m_packet[firstfree].w0_L+m_packet[firstfree].w0_H);  
		m_packet[firstfree].k = 0.5f*(m_packet[firstfree].k_L+m_packet[firstfree].k_H);  
		GetWaveParameters(wd, m_packet[firstfree].w0, m_packet[firstfree].k, m_packet[firstfree].k, m_packet[firstfree].speed1);
		m_packet[firstfree].speed2 = m_packet[firstfree].speed1;
		m_packet[firstfree].envelope = min(PACKET_ENVELOPE_MAXSIZE, max(PACKET_ENVELOPE_MINSIZE, PACKET_ENVELOPE_SIZE_FACTOR*2.0f*M_PI/m_packet[firstfree].k)); // adjust envelope size to represented wavelength
		m_packet[firstfree].ampOld = 0.0;
		a1 = min(MAX_SPEEDNESS*2.0f*M_PI / m_packet[firstfree].k, GetWaveAmplitude(m_packet[firstfree].envelope*(m_packet[firstfree].pos1 - m_packet[firstfree].pos2).norm(), m_packet[firstfree].E, m_packet[firstfree].k));
		m_packet[firstfree].dAmp = 0.5f*(m_packet[firstfree].speed1 + m_packet[firstfree].speed2)*m_elapsedTime / (PACKET_BLEND_TRAVEL_FACTOR*m_packet[firstfree].envelope)*a1;
		// also adjust freq. interval and envelope size of existing wave
		m_packet[i1].k_H = PACKET_SLOWAVE_K;
		m_packet[i1].w0_H = PACKET_SLOWAVE_W0;	
		GetWaveParameters(wd, m_packet[i1].w0_H, m_packet[i1].k_H, m_packet[i1].k_H, speedDummy);
		m_packet[i1].w0 = 0.5f*(m_packet[i1].w0_L+m_packet[i1].w0_H);  
		m_packet[i1].k = 0.5f*(m_packet[i1].k_L+m_packet[i1].k_H);  
		GetWaveParameters(wd, m_packet[i1].w0, m_packet[i1].k, m_packet[i1].k, m_packet[i1].speed1);
		m_packet[i1].speed2 = m_packet[i1].speed1;
		m_packet[i1].envelope = min(PACKET_ENVELOPE_MAXSIZE, max(PACKET_ENVELOPE_MINSIZE, PACKET_ENVELOPE_SIZE_FACTOR*2.0f*M_PI/m_packet[i1].k)); // adjust envelope size to represented wavelength
		m_packet[i1].ampOld = 0.0f;
		a1 = min(MAX_SPEEDNESS*2.0f*M_PI / m_packet[i1].k, GetWaveAmplitude(m_packet[i1].envelope*(m_packet[i1].pos1 - m_packet[i1].pos2).norm(), m_packet[i1].E, m_packet[i1].k));
		m_packet[i1].dAmp = 0.5f*(m_packet[i1].speed1 + m_packet[i1].speed2)*m_elapsedTime / (PACKET_BLEND_TRAVEL_FACTOR*m_packet[i1].envelope)*a1;
	}
}




// adds a new linear wave at normalized position x,y with wavelength boundaries lambda_L and lambda_H (in meters)
void Packets::CreateLinearWavefront(float xPos, float yPos, float dirx, float diry, float crestlength, float lambda_L, float lambda_H, float E)
{
	Vector2f dir = Vector2f(dirx,diry);
	dir.normalize();
	Vector2f wfAlign = 0.5f*crestlength*Vector2f(dir.y(),-dir.x()); 
	CreatePacket( xPos-wfAlign.x(), yPos-wfAlign.y(), xPos+wfAlign.x(), yPos+wfAlign.y(), dir.x(), dir.y(), dir.x(), dir.y(), 2.0f*M_PI/lambda_L, 2.0f*M_PI/lambda_H, E);
}



// adds a new linear wave at position (xPos,yPos) with wavelength boundaries lambda_L and lambda_H (in meters), spreadFactor = 1 -> 45 degrees
void Packets::CreateSpreadingPacket(float xPos, float yPos, float dirx, float diry, float spreadFactor, float crestlength, float lambda_L, float lambda_H, float E)
{
	Vector2f dir = Vector2f(dirx,diry);
	dir.normalize();
	Vector2f wfAlign = 0.5f*crestlength*Vector2f(dir.y(),-dir.x());
	Vector2f dirSpread1 = dir - spreadFactor*Vector2f(dir.y(),-dir.x());
	Vector2f dirSpread2 = dir + spreadFactor*Vector2f(dir.y(),-dir.x());
	CreatePacket( xPos-wfAlign.x(), yPos-wfAlign.y(), xPos+wfAlign.x(), yPos+wfAlign.y(), dirSpread1.x(), dirSpread1.y(), dirSpread2.x(), dirSpread2.y(), 2.0f*M_PI/lambda_L, 2.0f*M_PI/lambda_H, E);
}



// adds a new circular wave at normalized position x,y with wavelength boundaries lambda_L and lambda_H (in meters)
void Packets::CreateCircularWavefront(float xPos, float yPos, float radius, float lambda_L, float lambda_H, float E)
{
	// adapt initial packet crestlength to impact radius and wavelength
	float dAng = min(24.0f, 360.0f * ((0.5f*lambda_L + 0.5f*lambda_H)*3.0f) / (2.0f*M_PI*radius));
	for (float i = 0; i < 360.0f; i += dAng)
		CreatePacket(
			xPos + radius*sin(i*M_PI / 180.0f), yPos + radius*cos(i*M_PI / 180.0f),
			xPos + radius*sin((i + dAng)*M_PI / 180.0f), yPos + radius*cos((i + dAng)*M_PI / 180.0f),
			sin(i*M_PI / 180.0f), cos(i*M_PI / 180.0f),
			sin((i + dAng)*M_PI / 180.0), cos((i + dAng)*M_PI / 180.0f),
			2.0f*M_PI / lambda_L, 2.0f*M_PI / lambda_H, E);
}



// update the simulation time
void Packets::UpdateTime(float dTime)
{
	if (m_oldTime < 0)							// if we are entering the first time, set the current time as time
		m_oldTime = 0.0f;
	else
		m_oldTime = m_time;
	m_time = m_oldTime + dTime;					// time stepping
	m_elapsedTime = abs(m_time - m_oldTime);
}



// returns water depth at position p
float Packets::GetWaterDepth(Vector2f &p)
{
	float v = 1.0f-GetGroundVal(p);
	return(MIN_WATER_DEPTH + (MAX_WATER_DEPTH-MIN_WATER_DEPTH)*v*v*v*v);
}




void Packets::GetWaveParameters(float waterDepth, float w_0, float kIn, float &k_out, float &speed_out)
{
	float k = kIn;
	float dk = 1.0f;
	float kOld;
	int it = 0;
	while ((dk > 1.0e-04) && (it<6))
	{
		kOld = k;
		k = w_0/sqrt((GRAVITY/k+k*SIGMA/DENSITY)*rational_tanh(k*waterDepth));		// this includes surface tension / capillary waves
		dk = abs(k-kOld);
		it++;
	}
	k_out = k;
	float t = rational_tanh(k*waterDepth);
	const float c = SIGMA/DENSITY;
	speed_out = ((c*k*k + GRAVITY)*(t + waterDepth*k*(1.0f-t*t)) + 2.0f*c*k*k*t) / (2.0f*sqrt(k*(c*k*k + GRAVITY)*t));   // this is group speed as dw/dk
	return;
}



float Packets::GetPhaseSpeed(float w_0, float kIn)
{
	return( w_0/kIn );
}



// area = surface area of the wave packet, E = Energy, k = wavenumber
// computing the amplitude from energy flux for a wave packet
float Packets::GetWaveAmplitude(float area, float E, float k)
{
	return( sqrt(abs(E)/(abs(area)*0.5f*(DENSITY*GRAVITY+SIGMA*k*k))) );
}





// advects a single packet vertex with groupspeed, returns 1 if boundary reflection occured
bool Packets::AdvectPacketVertex(float elapsedTime, Vector2f &posIn,  Vector2f &dirIn, float w0, float &kIn, float &speedIn, Vector2f &posOut, Vector2f &dirOut, float &speedOut)
{
	bool bounced = false;
	// intialize the output with the input
	posOut = posIn;
	dirOut = dirIn;
	speedOut = speedIn;

	// compute new direction and speed based on snells law (depending on water depth)
	float speed1, k;
	GetWaveParameters(GetWaterDepth( posIn ), w0, kIn, k, speed1);
	speedOut = speed1;  // the new speed is defined by the speed of this wave at this water depth, this is does not necessarily respect snells law!
	Vector2f nDir = GetGroundNormal( posIn );
	if (abs(nDir.x())+abs(nDir.y()) > 0.1f)	// if there is a change in water depth here, indicated by a non-zero ground normal
	{
		Vector2f pNext = posIn + elapsedTime*speed1*dirIn;
		float speed2;
		GetWaveParameters(GetWaterDepth(pNext), w0, kIn, k, speed2);

		float cos1 = nDir.dot(-dirIn);
		float cos2 = sqrt( max(0.0f, 1.0f - (speed2*speed2)/(speed1*speed1)*(1.0f - cos1*cos1) ));
		Vector2f nRefrac;
		if (cos1 <= 0.0f)
			nRefrac = speed2/speed1*dirIn + (speed2/speed1*cos1 + cos2)*nDir;
		else
			nRefrac = speed2/speed1*dirIn + (speed2/speed1*cos1 - cos2)*nDir;
		if (nRefrac.norm() > 0.000001f)
			dirOut = nRefrac.normalized();
	}
	posOut = posIn + elapsedTime*speed1*dirOut;  // advect wave vertex position

	// if we ran into a boundary -> step back and bounce off
	if (GetBoundaryDist(posOut)<0.0f)   
	{
		Vector2f nor = GetBoundaryNormal(posOut);
		float a = nor.dot(dirOut);
		if (a <= -0.08f)  // a wave reflects if it travels with >4.5 degrees towards a surface. Otherwise, it gets diffracted
		{
			bounced = true;
			// step back until we are outside the boundary
			Vector2f pD = posIn;
			Vector2f vD = elapsedTime*speedIn*dirOut;
			for (int j = 0; j < 16; j++)
			{
				Vector2f pDD = pD + vD;
				if (GetBoundaryDist(pDD) > 0.0f)
					pD = pDD;
				vD = 0.5f*vD;
			}
			Vector2f wayVec = pD - posIn;
			float lGone = wayVec.norm();
			posOut = pD;
			// compute the traveling direction after the bounce
			dirOut = -dirOut;
			Vector2f nor2 = GetBoundaryNormal(posOut);
			float a2 = nor2.dot(dirOut);
			Vector2f bFrac = a2*nor2 - dirOut;
			Vector2f d0 = dirOut + 2.0f*bFrac;
			dirOut = d0.normalized();
			posOut += (elapsedTime*speedOut - lGone)*dirOut;
		}
	}

	// if we got trapped in a boundary (again), just project onto the nearest surface (this approximates multiple bounces)
	if (GetBoundaryDist(posOut) < 0.0)
		for (int i2=0; i2<16; i2++)
			posOut += -0.5f*GetBoundaryDist(posOut)*GetBoundaryNormal(posOut);

	return(bounced);
}






// updates the wavefield using the movin wavefronts and generated an output image from the wavefield
void Packets::AdvectWavePackets(float dTime)
{
	UpdateTime(dTime);
	if (m_elapsedTime <= 0.0)  // if there is no time advancement, do not update anything..
		return;

	// compute the new packet vertex positions, directions and speeds based on snells law
	#pragma omp parallel for
	for (int uP=0; uP<m_usedPackets; uP++)
	{
		int i1 = m_usedPacket[uP];
		m_packet[i1].pOld1 = m_packet[i1].pos1;
		m_packet[i1].dOld1 = m_packet[i1].dir1;
		m_packet[i1].sOld1 = m_packet[i1].speed1;
		m_packet[i1].bounced1 = AdvectPacketVertex(m_elapsedTime, m_packet[i1].pOld1, m_packet[i1].dOld1, m_packet[i1].w0, m_packet[i1].k, m_packet[i1].sOld1, m_packet[i1].pos1, m_packet[i1].dir1, m_packet[i1].speed1);
		m_packet[i1].pOld2 = m_packet[i1].pos2;
		m_packet[i1].dOld2 = m_packet[i1].dir2;
		m_packet[i1].sOld2 = m_packet[i1].speed2;
		m_packet[i1].bounced2 = AdvectPacketVertex(m_elapsedTime, m_packet[i1].pOld2, m_packet[i1].dOld2, m_packet[i1].w0, m_packet[i1].k, m_packet[i1].sOld2, m_packet[i1].pos2, m_packet[i1].dir2, m_packet[i1].speed2);
		// measure new wave k at phase speed at wave packet center, advect all representative waves inside
		m_packet[i1].phOld = m_packet[i1].phase;
		float packetSpeed = 0.5f*(m_packet[i1].speed1 + m_packet[i1].speed2);
		m_packet[i1].phase += m_elapsedTime*(GetPhaseSpeed(m_packet[i1].w0, m_packet[i1].k) - packetSpeed)*m_packet[i1].k;	// advect phase with difference between group speed and wave speed
	}

	// compute the new position, direction and speed of 3rd vertex (if present)
	#pragma omp parallel for
	for (int uP = 0; uP<m_usedPackets; uP++)
		if (m_packet[m_usedPacket[uP]].use3rd)
		{
			int i1 = m_usedPacket[uP];
			m_packet[i1].pOld3 = m_packet[i1].pos3;
			m_packet[i1].dOld3 = m_packet[i1].dir3;
			m_packet[i1].sOld3 = m_packet[i1].speed3;
			m_packet[i1].bounced3 = AdvectPacketVertex(m_elapsedTime, m_packet[i1].pOld3,  m_packet[i1].dOld3,  m_packet[i1].w0, m_packet[i1].k, m_packet[i1].sOld3,  m_packet[i1].pos3,  m_packet[i1].dir3,  m_packet[i1].speed3);
			if (!m_packet[i1].bounced3)	// advect 3rd as sliding vertex (from now on it is)
			{
				Vector2f nDir = GetBoundaryNormal( m_packet[i1].pos3 ); // get sliding direction
				m_packet[i1].dir3  = Vector2f(-nDir.y(), nDir.x());  
				if (m_packet[i1].dir3.dot(m_packet[i1].dOld3) < 0)
					m_packet[i1].dir3 = -m_packet[i1].dir3;
				Vector2f pD = m_packet[i1].pos3;						// project advected point onto closest boundary
				for (int i3=0; i3<16; i3++)
					pD += -0.5f*GetBoundaryDist(pD)*GetBoundaryNormal(pD);
				m_packet[i1].pos3 = pD;
			}
			else if ((!m_packet[i1].sliding3) && (!m_packet[i1].bounced1) && (!m_packet[i1].bounced2))  // "has to bounce"-3rd vertex -> find new "has to bounce" point if no other vertex bounced
			{
				float s = 0.0f;
				float sD = 0.5f;
				Vector2f posOld = m_packet[i1].pOld1;
				Vector2f dirOld = m_packet[i1].dOld1;
				float speedOld = m_packet[i1].sOld1;
				Vector2f pos = m_packet[i1].pos1;
				Vector2f dir = m_packet[i1].dir1;
				float speed = m_packet[i1].speed1;
				float wN = m_packet[i1].k;
				float w0 = m_packet[i1].w0;
				for (int j=0; j<16; j++)		
				{
					Vector2f p = (1.0f-(s+sD))*m_packet[i1].pOld1 + (s+sD)*m_packet[i1].pOld3;
					Vector2f d = (1.0f-(s+sD))*m_packet[i1].dOld1 + (s+sD)*m_packet[i1].dOld3;
					float sp = (1.0f-(s+sD))*m_packet[i1].sOld1 + (s+sD)*m_packet[i1].sOld3;
					Vector2f posD, dirD;
					float speedD;
					if (!AdvectPacketVertex(m_elapsedTime, p, d, w0, wN, sp, posD, dirD, speedD))
					{
						s += sD;
						posOld = p;
						dirOld = d;
						speedOld = sp;
						pos = posD;
						dir = dirD;
						speed = speedD;
					}
					sD = 0.5f*sD;
				}
				m_packet[i1].pOld3 = posOld;
				m_packet[i1].dOld3 = dirOld.normalized();
				m_packet[i1].sOld3 = speedOld;
				m_packet[i1].pos3 = pos;
				m_packet[i1].dir3  = dir;
				m_packet[i1].speed3 = speed;
			}
		}


	// first contact to a boundary -> sent a ghost packet, make packet invisible for now, add 3rd vertex
	if (m_usedGhosts + m_usedPackets > m_packetNum)
		ExpandWavePacketMemory(m_usedGhosts + m_usedPackets + PACKET_BUFFER_DELTA);
	#pragma omp parallel for
	for (int uP = m_usedPackets-1; uP>=0; uP--)
		if ((!m_packet[m_usedPacket[uP]].use3rd) && (m_packet[m_usedPacket[uP]].bounced1 || m_packet[m_usedPacket[uP]].bounced2))
		{
			int i1 = m_usedPacket[uP];
			int firstghost = GetFreeGhostID();
			m_ghostPacket[firstghost].pos = 0.5f*(m_packet[i1].pOld1+m_packet[i1].pOld2);
			m_ghostPacket[firstghost].dir = (m_packet[i1].dOld1+m_packet[i1].dOld2).normalized(); // the new position is wrong after the reflection, so use the old direction instead
			m_ghostPacket[firstghost].speed = 0.5f*(m_packet[i1].sOld1+m_packet[i1].sOld2);
			m_ghostPacket[firstghost].envelope = m_packet[i1].envelope;
			m_ghostPacket[firstghost].ampOld = m_packet[i1].ampOld;
			m_ghostPacket[firstghost].dAmp = m_ghostPacket[firstghost].ampOld* m_ghostPacket[firstghost].speed*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_ghostPacket[firstghost].envelope);
			m_ghostPacket[firstghost].k = m_packet[i1].k;
			m_ghostPacket[firstghost].phase = m_packet[i1].phOld;
			m_ghostPacket[firstghost].dPhase = m_packet[i1].phase-m_packet[i1].phOld;
			m_ghostPacket[firstghost].bending = GetIntersectionDistance(m_ghostPacket[firstghost].pos, m_ghostPacket[firstghost].dir, m_packet[i1].pOld1, m_packet[i1].dOld1);
			// hide this packet from display
			m_packet[i1].ampOld = 0.0;
			m_packet[i1].dAmp = 0.0;  
			// emit all (higher-)frequency waves after a bounce
			if ((PACKET_BOUNCE_FREQSPLIT) && (m_packet[i1].k_L < PACKET_BOUNCE_FREQSPLIT_K))  // split the frequency range if smallest wave is > 20cm
			{
				m_packet[i1].k_L = PACKET_BOUNCE_FREQSPLIT_K;
				m_packet[i1].w0_L = sqrt(GRAVITY/m_packet[i1].k_L)*m_packet[i1].k_L;  // initial guess for angular frequency
				m_packet[i1].w0 = 0.5f*(m_packet[i1].w0_L+m_packet[i1].w0_H);
				// distribute the error according to current speed difference
				float dummySpeed;
				Vector2f pos = 0.5f*(m_packet[i1].pos1+m_packet[i1].pos2);
				float wd = GetWaterDepth(pos);
				GetWaveParameters(wd, m_packet[i1].w0_L, m_packet[i1].k_L, m_packet[i1].k_L, dummySpeed);
				GetWaveParameters(wd, m_packet[i1].w0_H, m_packet[i1].k_H, m_packet[i1].k_H, dummySpeed);
				GetWaveParameters(wd, m_packet[i1].w0, 0.5f*(m_packet[i1].k_L+m_packet[i1].k_H), m_packet[i1].k, dummySpeed);
				m_packet[i1].d_L = 0.0; m_packet[i1].d_H = 0.0; // reset the internally tracked error
				m_packet[i1].envelope = min(PACKET_ENVELOPE_MAXSIZE, max(PACKET_ENVELOPE_MINSIZE, PACKET_ENVELOPE_SIZE_FACTOR*2.0f*M_PI/m_packet[i1].k));
			}
			//if both vertices bounced, the reflected wave needs to smoothly reappear
			if (m_packet[i1].bounced1==m_packet[i1].bounced2)	 
			{				
				m_packet[i1].ampOld = 0.0;
				m_packet[i1].dAmp = 0.5f*(m_packet[i1].speed1+m_packet[i1].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[i1].envelope)*GetWaveAmplitude( m_packet[i1].envelope*(m_packet[i1].pos1-m_packet[i1].pos2).norm(), m_packet[i1].E, m_packet[i1].k);
			}
			if (m_packet[i1].bounced1 != m_packet[i1].bounced2)	  // only one vertex bounced -> insert 3rd "wait for bounce" vertex and reorder such that 1st is waiting for bounce
			{
				if (m_packet[i1].bounced1)  // if the first bounced an the second did not -> exchange the two points, as we assume that the second bounced already and the 3rd will be "ahead" of the first.. 
				{
					WAVE_PACKET *seg = &m_packet[i1];  // use the 3rd vertex as copy element
					seg->pos3 = seg->pos2; seg->pos2 = seg->pos1; seg->pos1 = seg->pos3;
					seg->pOld3 = seg->pOld2; seg->pOld2 = seg->pOld1; seg->pOld1 = seg->pOld3;
					seg->dir3 = seg->dir2; seg->dir2 = seg->dir1; seg->dir1 = seg->dir3;
					seg->dOld3 = seg->dOld2; seg->dOld2 = seg->dOld1; seg->dOld1 = seg->dOld3;
					seg->speed3 = seg->speed2; seg->speed2 = seg->speed1; seg->speed1 = seg->speed3;
					seg->sOld3 = seg->sOld2; seg->sOld2 = seg->sOld1; seg->sOld1 = seg->sOld3;
					seg->bounced3 = seg->bounced2; seg->bounced2 = seg->bounced1; seg->bounced1 = seg->bounced3;
				}
				float s = 0.0;
				float sD = 0.5f;
				Vector2f posOld = m_packet[i1].pOld1;
				Vector2f dirOld = m_packet[i1].dOld1;
				float speedOld = m_packet[i1].sOld1;
				Vector2f pos = m_packet[i1].pos1;
				Vector2f dir = m_packet[i1].dir1;
				float speed = m_packet[i1].speed1;
				float wN = m_packet[i1].k;
				float w0 = m_packet[i1].w0;
				for (int j=0; j<16; j++)				// find the last point before the boundary that does not bounce in this timestep, it becomes the 3rd point
				{
					Vector2f p = (1.0f-(s+sD))*m_packet[i1].pOld1 + (s+sD)*m_packet[i1].pOld2;
					Vector2f d = (1.0f-(s+sD))*m_packet[i1].dOld1 + (s+sD)*m_packet[i1].dOld2;
					float sp = (1.0f-(s+sD))*m_packet[i1].sOld1 + (s+sD)*m_packet[i1].sOld2;
					Vector2f posD, dirD;
					float speedD;
					if (!AdvectPacketVertex(m_elapsedTime, p, d, w0, wN, sp, posD, dirD, speedD))
					{
						s += sD;
						posOld = p;
						dirOld = d;
						speedOld = sp;
						pos = posD;
						dir = dirD;
						speed = speedD;
					}
					sD = 0.5f*sD;
				}
				// the new 3rd vertex has "has to bounce" state (not sliding yet)
				m_packet[i1].pOld3 = posOld;
				m_packet[i1].dOld3 = dirOld.normalized();
				m_packet[i1].sOld3 = speedOld;
				m_packet[i1].pos3 = pos;
				m_packet[i1].dir3  = dir;
				m_packet[i1].speed3 = speed;
			}
		}


	// define new state based on current state and bouncings
	#pragma omp parallel for
	for (int uP = 0; uP<m_usedPackets; uP++)
	{ 
		int i1 = m_usedPacket[uP];
		if (!m_packet[i1].use3rd)  // no 3rd vertex present
		{
			if (m_packet[i1].bounced1!=m_packet[i1].bounced2)  // on point bounced -> case "intiate new 3rd vertex"
			{
				m_packet[i1].use3rd = true;
				m_packet[i1].bounced3 = false;
				m_packet[i1].sliding3 = false;
			}
		}
		else // 3rd vertex already present
		{
			if (!m_packet[i1].sliding3)  // 3rd point was in "waiting for bounce" state
			{
				if (!m_packet[i1].bounced3)								// case: 3rd "has to bounce" vertex did not bounce -> make it sliding in any case
					m_packet[i1].sliding3 = true;
				else if ((m_packet[i1].bounced1) || (m_packet[i1].bounced2))	// case: 3rd "has to bounce" and one other point bounced as well -> release 3rd vertex
					m_packet[i1].use3rd = false;
			}
			else // 3rd point was already in "sliding" state
			{
				if (m_packet[i1].bounced3)				// if sliding 3rd point bounced, release it
					m_packet[i1].use3rd = false;
			}
			// if we released this wave from a boundary (3rd vertex released) -> blend it smoothly in again
			if (!m_packet[i1].use3rd)	
			{
				m_packet[i1].ampOld = 0.0;
				m_packet[i1].dAmp = 0.5f*(m_packet[i1].speed1+m_packet[i1].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[i1].envelope)*GetWaveAmplitude( m_packet[i1].envelope*(m_packet[i1].pos1-m_packet[i1].pos2).norm(), m_packet[i1].E, m_packet[i1].k);
			}
		}
	}


	// wavenumber interval subdivision if travel distance between fastest and slowest wave packets differ more than PACKET_SPLIT_DISPERSION x envelope size
	if ( max(m_usedGhosts + m_usedPackets, 2*m_usedPackets) > m_packetNum)
		ExpandWavePacketMemory(max(m_usedGhosts + m_usedPackets, 2 * m_usedPackets) + PACKET_BUFFER_DELTA);
	#pragma omp parallel for
	for (int uP = m_usedPackets-1; uP>=0; uP--)
		if (!m_packet[m_usedPacket[uP]].use3rd)
		{
			int i1 = m_usedPacket[uP];
			float speedDummy,kDummy;
			Vector2f pos = 0.5f*(m_packet[i1].pos1+m_packet[i1].pos2);  
			float wd = GetWaterDepth(pos);
			GetWaveParameters(wd, m_packet[i1].w0, m_packet[i1].k, kDummy, speedDummy);
			float dist_Ref = m_elapsedTime*speedDummy;
			GetWaveParameters(wd, m_packet[i1].w0_L, m_packet[i1].k_L, kDummy, speedDummy);
			m_packet[i1].k_L = kDummy;
			m_packet[i1].d_L += fabs(m_elapsedTime*speedDummy-dist_Ref);  // taking the abs augments any errors caused by waterdepth independent slowest wave assumption..
			GetWaveParameters(wd, m_packet[i1].w0_H, m_packet[i1].k_H, kDummy, speedDummy);
			m_packet[i1].k_H = kDummy;
			m_packet[i1].d_H += fabs(m_elapsedTime*speedDummy-dist_Ref);
			if (m_packet[i1].d_L+m_packet[i1].d_H > PACKET_SPLIT_DISPERSION*m_packet[i1].envelope)  // if fastest/slowest waves in this packet diverged too much -> subdivide
			{
				// first create a ghost for the old packet
				int firstghost = GetFreeGhostID();
				m_ghostPacket[firstghost].pos = 0.5f*(m_packet[i1].pOld1+m_packet[i1].pOld2);
				m_ghostPacket[firstghost].dir = (0.5f*(m_packet[i1].pos1+m_packet[i1].pos2)-0.5f*(m_packet[i1].pOld1+m_packet[i1].pOld2)).normalized();
				m_ghostPacket[firstghost].speed = 0.5f*(m_packet[i1].sOld1+m_packet[i1].sOld2);
				m_ghostPacket[firstghost].envelope = m_packet[i1].envelope;
				m_ghostPacket[firstghost].ampOld = m_packet[i1].ampOld;
				m_ghostPacket[firstghost].dAmp = m_ghostPacket[firstghost].ampOld* m_ghostPacket[firstghost].speed*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_ghostPacket[firstghost].envelope); 
				m_ghostPacket[firstghost].k = m_packet[i1].k;
				m_ghostPacket[firstghost].phase = m_packet[i1].phOld;
				m_ghostPacket[firstghost].dPhase = m_packet[i1].phase-m_packet[i1].phOld;
				m_ghostPacket[firstghost].bending = GetIntersectionDistance(m_ghostPacket[firstghost].pos, m_ghostPacket[firstghost].dir, m_packet[i1].pOld1, m_packet[i1].dOld1);
				// create new packet and copy ALL parameters
				int firstfree = GetFreePackedID();
				m_packet[firstfree] = m_packet[i1];
				// split the frequency range
				m_packet[firstfree].k_H = m_packet[i1].k;
				m_packet[firstfree].w0_H = m_packet[i1].w0;
				m_packet[firstfree].w0 = 0.5f*(m_packet[firstfree].w0_L+m_packet[firstfree].w0_H);
				// distribute the error according to current speed difference
				float speed_L,speed_M,speed_H; 
				GetWaveParameters( wd, m_packet[firstfree].w0_L, m_packet[firstfree].k_L, m_packet[firstfree].k_L, speed_L); 
				GetWaveParameters( wd, m_packet[firstfree].w0_H, m_packet[firstfree].k_H, m_packet[firstfree].k_H, speed_H); 
				GetWaveParameters( wd, m_packet[firstfree].w0, 0.5f*(m_packet[firstfree].k_L+m_packet[firstfree].k_H), m_packet[firstfree].k, speed_M); 
				float dSL = abs(abs(speed_L)-abs(speed_M));
				float dSH = abs(abs(speed_H)-abs(speed_M));
				float d_All = m_packet[i1].d_L;
				m_packet[firstfree].d_L = dSL*d_All / (dSH + dSL);
				m_packet[firstfree].d_H = d_All - m_packet[firstfree].d_L;
				m_packet[firstfree].envelope = min(PACKET_ENVELOPE_MAXSIZE, max(PACKET_ENVELOPE_MINSIZE, PACKET_ENVELOPE_SIZE_FACTOR*2.0f*M_PI/m_packet[firstfree].k));
				// set the new upper freq. boundary and representative freq.
				m_packet[i1].k_L = m_packet[i1].k;
				m_packet[i1].w0_L = m_packet[i1].w0;
				m_packet[i1].w0 = 0.5f*(m_packet[i1].w0_L+m_packet[i1].w0_H);
				// distribute the error according to current speed difference
				GetWaveParameters( wd, m_packet[i1].w0_L, m_packet[i1].k_L, m_packet[i1].k_L, speed_L); 
				GetWaveParameters( wd, m_packet[i1].w0_H, m_packet[i1].k_H, m_packet[i1].k_H, speed_H); 
				GetWaveParameters( wd, m_packet[i1].w0, 0.5f*(m_packet[i1].k_L+m_packet[i1].k_H), m_packet[i1].k, speed_M); 
				dSL = abs(abs(speed_L)-abs(speed_M));
				dSH = abs(abs(speed_H)-abs(speed_M));
				d_All = m_packet[i1].d_H;
				m_packet[i1].d_L = dSL*d_All / (dSH + dSL);
				m_packet[i1].d_H = d_All - m_packet[i1].d_L;
				m_packet[i1].envelope = min(PACKET_ENVELOPE_MAXSIZE, max(PACKET_ENVELOPE_MINSIZE, PACKET_ENVELOPE_SIZE_FACTOR*2.0f*M_PI/m_packet[i1].k));
				// distribute the energy such that both max. wave gradients are equal -> both get the same wave shape
				m_packet[firstfree].E = abs(m_packet[i1].E)/(1.0f + (m_packet[i1].envelope*m_packet[firstfree].k*m_packet[firstfree].k*(DENSITY*GRAVITY+SIGMA*m_packet[i1].k*m_packet[i1].k))/(m_packet[firstfree].envelope*m_packet[i1].k*m_packet[i1].k*(DENSITY*GRAVITY+SIGMA*m_packet[firstfree].k*m_packet[firstfree].k)));
				m_packet[i1].E = abs(m_packet[i1].E)-m_packet[firstfree].E;
				// smoothly ramp the new waves
				m_packet[i1].phase=0;
				m_packet[i1].ampOld = 0.0f;
				m_packet[i1].dAmp = 0.5f*(m_packet[i1].speed1+m_packet[i1].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[i1].envelope)*GetWaveAmplitude( m_packet[i1].envelope*(m_packet[i1].pos1-m_packet[i1].pos2).norm(), m_packet[i1].E, m_packet[i1].k);
				m_packet[firstfree].phase = 0.0f;
				m_packet[firstfree].ampOld = 0.0f;
				m_packet[firstfree].dAmp = 0.5f*(m_packet[firstfree].speed1+m_packet[firstfree].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[firstfree].envelope)*GetWaveAmplitude( m_packet[firstfree].envelope*(m_packet[firstfree].pos1-m_packet[firstfree].pos2).norm(), m_packet[firstfree].E, m_packet[firstfree].k);
			}
		}


	// crest-refinement of packets of regular packet (not at any boundary, i.e. having no 3rd vertex)
	if (max(m_usedGhosts + m_usedPackets, 2 * m_usedPackets) > m_packetNum)
		ExpandWavePacketMemory(max(m_usedGhosts + m_usedPackets, 2 * m_usedPackets) + PACKET_BUFFER_DELTA);
	#pragma omp parallel for
	for (int uP = m_usedPackets-1; uP>=0; uP--)
		if (!m_packet[m_usedPacket[uP]].use3rd)
		{
			int i1 = m_usedPacket[uP];
			float sDiff = (m_packet[i1].pos2-m_packet[i1].pos1).norm();
			float aDiff = m_packet[i1].dir1.dot(m_packet[i1].dir2);
			if ((sDiff > m_packet[i1].envelope) || (aDiff <= PACKET_SPLIT_ANGLE))  // if the two vertices move towards each other, do not subdivide
			{
				int firstghost = GetFreeGhostID();
				m_ghostPacket[firstghost].pos = 0.5f*(m_packet[i1].pOld1+m_packet[i1].pOld2);
				m_ghostPacket[firstghost].dir = (m_packet[i1].dOld1+m_packet[i1].dOld2).normalized();
				m_ghostPacket[firstghost].speed = 0.5f*(m_packet[i1].sOld1+m_packet[i1].sOld2);
				m_ghostPacket[firstghost].envelope = m_packet[i1].envelope;
				m_ghostPacket[firstghost].ampOld = m_packet[i1].ampOld;
				m_ghostPacket[firstghost].dAmp = m_ghostPacket[firstghost].ampOld* m_ghostPacket[firstghost].speed*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_ghostPacket[firstghost].envelope); 
				m_ghostPacket[firstghost].k = m_packet[i1].k;
				m_ghostPacket[firstghost].phase = m_packet[i1].phOld;
				m_ghostPacket[firstghost].dPhase = m_packet[i1].phase-m_packet[i1].phOld;
				m_ghostPacket[firstghost].bending = GetIntersectionDistance(m_ghostPacket[firstghost].pos, m_ghostPacket[firstghost].dir, m_packet[i1].pOld1, m_packet[i1].dOld1);
				// create new vertex between existing packet vertices
				int firstfree = GetFreePackedID();
				m_packet[firstfree] = m_packet[i1];	// first copy all data
				m_packet[firstfree].pOld1 = 0.5f*(m_packet[i1].pOld1 + m_packet[i1].pOld2);
				m_packet[firstfree].dOld1 = (m_packet[i1].dOld1 + m_packet[i1].dOld2).normalized();
				m_packet[firstfree].sOld1 = 0.5f*(m_packet[i1].sOld1 + m_packet[i1].sOld2);
				m_packet[firstfree].pos1 = 0.5f*(m_packet[i1].pos1 + m_packet[i1].pos2);
				m_packet[firstfree].dir1 = (m_packet[i1].dir1  + m_packet[i1].dir2).normalized();
				m_packet[firstfree].speed1 = 0.5f*(m_packet[i1].speed1 + m_packet[i1].speed2);
				m_packet[firstfree].E = 0.5f*m_packet[i1].E;
				m_packet[firstfree].ampOld = 0.0;
				m_packet[firstfree].dAmp = 0.5f*(m_packet[firstfree].speed1+m_packet[firstfree].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[firstfree].envelope)*GetWaveAmplitude( m_packet[firstfree].envelope*(m_packet[firstfree].pos1-m_packet[firstfree].pos2).norm(), m_packet[firstfree].E, m_packet[firstfree].k);
				// use the same new middle vertex here
				m_packet[i1].pOld2 = m_packet[firstfree].pOld1;
				m_packet[i1].dOld2 = m_packet[firstfree].dOld1;
				m_packet[i1].sOld2 = m_packet[firstfree].sOld1;
				m_packet[i1].pos2 = m_packet[firstfree].pos1;
				m_packet[i1].dir2 = m_packet[firstfree].dir1;
				m_packet[i1].speed2 = m_packet[firstfree].speed1;
				m_packet[i1].E *= 0.5f;
				m_packet[i1].ampOld = 0.0;
				m_packet[i1].dAmp = 0.5f*(m_packet[i1].speed1+m_packet[i1].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[i1].envelope)*GetWaveAmplitude( m_packet[i1].envelope*(m_packet[i1].pos1-m_packet[i1].pos2).norm(), m_packet[i1].E, m_packet[i1].k);
			}
		}



	// crest-refinement of packets with a sliding 3rd vertex
	if ( 3 * m_usedPackets > m_packetNum)
		ExpandWavePacketMemory(3 * m_usedPackets + PACKET_BUFFER_DELTA);
	#pragma omp parallel for
	for (int uP = m_usedPackets-1; uP>=0; uP--)
		if ((m_packet[m_usedPacket[uP]].use3rd) && (m_packet[m_usedPacket[uP]].sliding3))
		{
			int i1 = m_usedPacket[uP];
			float sDiff1 = (m_packet[i1].pos1-m_packet[i1].pos3).norm();
			float aDiff1 = m_packet[i1].dir1.dot(m_packet[i1].dir3);
			if ((sDiff1 >= m_packet[i1].envelope))// || (aDiff1 <= PACKET_SPLIT_ANGLE))  // angle criterion is removed here because this would prevent diffraction
			{
				int firstfree = GetFreePackedID();
				// first vertex becomes first in new wave packet, third one becomes second
				m_packet[firstfree] = m_packet[i1];	// first copy all data
				m_packet[firstfree].pOld2 = 0.5f*(m_packet[i1].pOld1 + m_packet[i1].pOld3);
				m_packet[firstfree].dOld2 = (m_packet[i1].dOld1 + m_packet[i1].dOld3).normalized();
				m_packet[firstfree].sOld2 = 0.5f*(m_packet[i1].sOld1 + m_packet[i1].sOld3);
				m_packet[firstfree].pos2 = 0.5f*(m_packet[i1].pos1 + m_packet[i1].pos3);
				m_packet[firstfree].dir2 = (m_packet[i1].dir1  + m_packet[i1].dir3).normalized();
				m_packet[firstfree].speed2 = 0.5f*(m_packet[i1].speed1 + m_packet[i1].speed3);
				m_packet[firstfree].ampOld = 0.0;
				m_packet[firstfree].dAmp = 0.5f*(m_packet[i1].speed1+m_packet[i1].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[i1].envelope)*GetWaveAmplitude( m_packet[i1].envelope*(m_packet[i1].pos1-m_packet[i1].pos2).norm(), m_packet[i1].E, m_packet[i1].k);
				m_packet[firstfree].bounced1 = false;
				m_packet[firstfree].bounced2 = false;
				m_packet[firstfree].bounced3 = false;
				m_packet[firstfree].use3rd = false;
				m_packet[firstfree].sliding3 = false;
				// use the same new middle vertex here
				m_packet[i1].pOld1 = m_packet[firstfree].pOld2;
				m_packet[i1].dOld1 = m_packet[firstfree].dOld2;
				m_packet[i1].sOld1 = m_packet[firstfree].sOld2;
				m_packet[i1].pos1 = m_packet[firstfree].pos2;
				m_packet[i1].dir1 = m_packet[firstfree].dir2;
				m_packet[i1].speed1 = m_packet[firstfree].speed2;
				// distribute the energy according to length of the two new packets
				float s = (m_packet[firstfree].pos1-m_packet[firstfree].pos2).norm()/((m_packet[firstfree].pos1-m_packet[firstfree].pos2).norm() + (m_packet[i1].pos1-m_packet[i1].pos3).norm() + (m_packet[i1].pos2-m_packet[i1].pos3).norm());
				m_packet[firstfree].E = s*m_packet[i1].E;
				m_packet[i1].E *= (1.0f-s);
			}
			// same procedure for the other end of sliding vertex..
			sDiff1 = (m_packet[i1].pos2-m_packet[i1].pos3).norm();
			aDiff1 = m_packet[i1].dir2.dot(m_packet[i1].dir3);
			if ((sDiff1 >= m_packet[i1].envelope)/* || (aDiff1 <= PACKET_SPLIT_ANGLE)*/)  // angle criterion is removed here because this would prevent diffraction
			{
				int firstfree = GetFreePackedID();
				// first vertex becomes first in new packet, third one becomes second
				m_packet[firstfree] = m_packet[i1];	// first copy all data
				m_packet[firstfree].pOld1 = 0.5f*(m_packet[i1].pOld2 + m_packet[i1].pOld3);
				m_packet[firstfree].dOld1 = (m_packet[i1].dOld2 + m_packet[i1].dOld3).normalized();
				m_packet[firstfree].sOld1 = 0.5f*(m_packet[i1].sOld2 + m_packet[i1].sOld3);
				m_packet[firstfree].pos1 = 0.5f*(m_packet[i1].pos2 + m_packet[i1].pos3);
				m_packet[firstfree].dir1 = (m_packet[i1].dir2  + m_packet[i1].dir3).normalized();
				m_packet[firstfree].speed1 = 0.5f*(m_packet[i1].speed2 + m_packet[i1].speed3);
				m_packet[firstfree].ampOld = 0.0;
				m_packet[firstfree].dAmp = 0.5f*(m_packet[firstfree].speed1+m_packet[firstfree].speed2)*m_elapsedTime/(PACKET_BLEND_TRAVEL_FACTOR*m_packet[firstfree].envelope)*GetWaveAmplitude( m_packet[firstfree].envelope*(m_packet[firstfree].pos1-m_packet[firstfree].pos2).norm(), m_packet[firstfree].E, m_packet[firstfree].k);
				m_packet[firstfree].bounced1 = false;
				m_packet[firstfree].bounced2 = false;
				m_packet[firstfree].bounced3 = false;
				m_packet[firstfree].use3rd = false;
				m_packet[firstfree].sliding3 = false;
				// use the same new middle vertex
				m_packet[i1].pOld2 = m_packet[firstfree].pOld1;
				m_packet[i1].dOld2 = m_packet[firstfree].dOld1;
				m_packet[i1].sOld2 = m_packet[firstfree].sOld1;
				m_packet[i1].pos2 = m_packet[firstfree].pos1;
				m_packet[i1].dir2 = m_packet[firstfree].dir1;
				m_packet[i1].speed2 = m_packet[firstfree].speed1;
				// distribute the energy according to length of the two new packets
				float s = (m_packet[firstfree].pos1-m_packet[firstfree].pos2).norm()/((m_packet[firstfree].pos1-m_packet[firstfree].pos2).norm() + (m_packet[i1].pos1-m_packet[i1].pos3).norm() + (m_packet[i1].pos2-m_packet[i1].pos3).norm());
				m_packet[firstfree].E = s*m_packet[i1].E;
				m_packet[i1].E *= (1.0f-s);
			}
		}


	// delete packets traveling outside the scene
	#pragma omp parallel for
	for (int uP = 0; uP < m_usedPackets; uP++)
	{
		int i1 = m_usedPacket[uP];
		m_packet[i1].toDelete = false;
		if (!m_packet[i1].use3rd)
		{
			Vector2f dir = m_packet[i1].pos1 - m_packet[i1].pOld1;
			Vector2f dir2 = m_packet[i1].pos2 - m_packet[i1].pOld2;
			if ((((m_packet[i1].pos1.x() < -0.5f*SCENE_EXTENT) && (dir.x() < 0.0))
				|| ((m_packet[i1].pos1.x() > +0.5f*SCENE_EXTENT) && (dir.x() > 0.0))
				|| ((m_packet[i1].pos1.y() < -0.5f*SCENE_EXTENT) && (dir.y() < 0.0))
				|| ((m_packet[i1].pos1.y() > +0.5f*SCENE_EXTENT) && (dir.y() > 0.0)))
				&&
				(((m_packet[i1].pos2.x() < -0.5f*SCENE_EXTENT) && (dir2.x() < 0.0))
					|| ((m_packet[i1].pos2.x() > +0.5f*SCENE_EXTENT) && (dir2.x() > 0.0))
					|| ((m_packet[i1].pos2.y() < -0.5f*SCENE_EXTENT) && (dir2.y() < 0.0))
					|| ((m_packet[i1].pos2.y() > +0.5f*SCENE_EXTENT) && (dir2.y() > 0.0))))
				m_packet[i1].toDelete = true;
		}
	}

	// damping, insignificant packet removal (if too low amplitude), reduce energy of steep waves with too high gradient
	m_softDampFactor = 1.0f + 100.0f*pow(max(0.0f, (float)(m_usedPackets)/(float)(m_packetBudget) - 1.0f), 2.0f);
	#pragma omp parallel for
	for (int uP = 0; uP < m_usedPackets; uP++)
		if ((!m_packet[m_usedPacket[uP]].use3rd) && (!m_packet[m_usedPacket[uP]].toDelete))
		{
			int i1 = m_usedPacket[uP];
			float dampViscosity = -2.0f*m_packet[i1].k*m_packet[i1].k*KINEMATIC_VISCOSITY;
			float dampJunkfilm = -0.5f*m_packet[i1].k*sqrt(0.5f*KINEMATIC_VISCOSITY*m_packet[i1].w0);
			m_packet[i1].E *= exp((dampViscosity + dampJunkfilm)*m_elapsedTime*m_softDampFactor);   // wavelength-dependent damping
			// amplitude computation: lower if too steep, delete if too low
			float area = m_packet[i1].envelope*(m_packet[i1].pos2 - m_packet[i1].pos1).norm();
			float a1 = GetWaveAmplitude( area, m_packet[i1].E, m_packet[i1].k);
			if (a1*m_packet[i1].k < PACKET_KILL_AMPLITUDE_DERIV)
				m_packet[i1].toDelete = true;
			else
			{
				// get the biggest wave as reference for energy reduction (conservative but important to not remove too much energy in case of large k intervals)
				float a_L = GetWaveAmplitude(area, m_packet[i1].E, m_packet[i1].k_L);
				float a_H = GetWaveAmplitude(area, m_packet[i1].E, m_packet[i1].k_H);
				float k;
				if (a_L*m_packet[i1].k_L < a_H*m_packet[i1].k_H)   // take the smallest wave steepness (=amplitude derivative)
				{
					a1 = a_L;
					k = m_packet[i1].k_L;
				}
				else
				{
					a1 = a_H;
					k = m_packet[i1].k_H;
				}
				if (a1 > MAX_SPEEDNESS*2.0f*M_PI / k)
				{
					a1 = MAX_SPEEDNESS*2.0f*M_PI / k;
					m_packet[i1].E = a1*a1*(area*0.5f*(DENSITY*GRAVITY + SIGMA*k*k));
				}
			}
			m_packet[i1].ampOld = min(a1, m_packet[i1].ampOld + m_packet[i1].dAmp); // smoothly increase amplitude from last timestep
			// update variables needed for packet display
			Vector2f posMidNew = 0.5f*(m_packet[i1].pos1 + m_packet[i1].pos2);
			Vector2f posMidOld = 0.5f*(m_packet[i1].pOld1 + m_packet[i1].pOld2);
			Vector2f dirN = (posMidNew - posMidOld).normalized();				// vector in traveling direction
			m_packet[i1].midPos = posMidNew;
			m_packet[i1].travelDir = dirN;
			m_packet[i1].bending = GetIntersectionDistance(posMidNew, dirN, m_packet[i1].pos1, m_packet[i1].dir1);
		}
	// delete all packets identified to be deleted (important: NO parallelization here!)
	for (int uP = 0; uP < m_usedPackets; uP++)
		if (m_packet[m_usedPacket[uP]].toDelete)
			DeletePacket(uP);

	// advect ghost packets
	#pragma omp parallel for
	for (int uG = 0; uG < m_usedGhosts; uG++)
	{
		int i1 = m_usedGhost[uG];
		m_ghostPacket[i1].pos += m_elapsedTime*m_ghostPacket[i1].speed*m_ghostPacket[i1].dir;
		m_ghostPacket[i1].phase += m_ghostPacket[i1].dPhase;
		m_ghostPacket[i1].ampOld = max(0.0f, m_ghostPacket[i1].ampOld - m_softDampFactor*m_ghostPacket[i1].dAmp);  // decrease amplitude to let this wave disappear (take budget-based softdamping into account)
	}
	// delete all ghost packets if they traveled long enough (important: NO parallelization here!)
	for (int uG = 0; uG < m_usedGhosts; uG++)
		if (m_ghostPacket[m_usedGhost[uG]].ampOld <= 0.0)
			DeleteGhost(uG);
}



