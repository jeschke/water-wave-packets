#pragma once

#include "GlobalDefs.h"

#include <iostream>
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;


// simulation parameters
#define PACKET_SPLIT_ANGLE 0.95105f			// direction angle variation threshold: 0.95105=18 degree
#define PACKET_SPLIT_DISPERSION 0.3f		// if the fastest wave in a packet traveled PACKET_SPLIT_DISPERSION*Envelopesize ahead, or the slowest by the same amount behind, subdivide this packet into two wavelength intervals
#define PACKET_KILL_AMPLITUDE_DERIV 0.0001f	// waves below this maximum amplitude derivative gets killed
#define PACKET_BLEND_TRAVEL_FACTOR 1.0f		// in order to be fully blended (appear or disappear), any wave must travel PACKET_BLEND_TRAVEL_FACTOR times "envelope size" in space (1.0 is standard)
#define PACKET_ENVELOPE_SIZE_FACTOR 3.0f	// size of the envelope relative to wavelength (determines how many "bumps" appear)
#define PACKET_ENVELOPE_MINSIZE 0.02f		// minimum envelope size in meters (smallest expected feature)
#define PACKET_ENVELOPE_MAXSIZE 10.0f		// maximum envelope size in meters (largest expected feature)
#define PACKET_BOUNCE_FREQSPLIT true 		// (boolean) should a wave packet produce smaller waves at a bounce/reflection (->widen the wavelength interval of this packet)?
#define PACKET_BOUNCE_FREQSPLIT_K 31.4f		// if k_L is smaller than this value (lambda = 20cm), the wave is (potentially) split after a bounce
#define MAX_SPEEDNESS 0.07f					// all wave amplitudes a are limited to a <= MAX_SPEEDNESS*2.0*M_PI/k

// physical parameters
#define SIGMA 0.074f						// surface tension N/m at 20 grad celsius
#define GRAVITY 9.81f						// GRAVITY m/s^2
#define DENSITY 998.2071f					// water density at 20 degree celsius
#define KINEMATIC_VISCOSITY 0.0000089f		// kinematic viscosity
#define PACKET_SLOWAVE_K 143.1405792f		// k of the slowest possible wave packet
#define PACKET_SLOWAVE_W0 40.2646141f		// w_0 of the slowest possible wave packet

// memory management
#define PACKET_BUFFER_DELTA 500000			// initial number of vertices, packet memory will be increased on demand by this stepsize




struct WAVE_PACKET
{
	// positions, directions, speed of the tracked vertices
	Vector2f	pos1,pos2,pos3;				// 2D position
	Vector2f	dir1,dir2,dir3;				// current movement direction
	float		speed1,speed2,speed3;		// speed of the particle
    Vector2f	pOld1,pOld2,pOld3;			// position in last timestep (needed to handle bouncing)
	Vector2f	dOld1,dOld2,dOld3;			// direction in last timestep (needed to handle bouncing)
	float		sOld1,sOld2,sOld3;			// speed in last timestep (needed to handle bouncing)
	Vector2f	midPos;						// middle position (tracked each timestep, used for rendering)
	Vector2f	travelDir;					// travel direction (tracked each timestep, used for rendering)
	float		bending;					// point used for circular arc bending of the wave function inside envelope

											// bouncing and sliding
	bool		bounced1, bounced2, bounced3;	// indicates if this vertex bounced in this timestep
	bool		sliding3;					// indicates if the 3rd vertex is "sliding" (used for diffraction)
	bool		use3rd;						// indicates if the third vertex is present (it marks a (potential) sliding point)
	// wave function related
	float		phase;						// phase of the representative wave inside the envelope, phase speed vs. group speed
	float		phOld;						// old phase
	float		E;							// wave energy flux for this packet (determines amplitude)
	float		envelope;					// envelope size for this packet
	float		k,w0;						// w0 = angular frequency, k = current wavenumber
	float		k_L,w0_L,k_H,w0_H;			// w0 = angular frequency, k = current wavenumber,  L/H are for lower/upper boundary
	float		d_L,d_H;					// d = travel distance to reference wave (gets accumulated over time),  L/H are for lower/upper boundary
	float		ampOld;						// amplitude from last timestep, will be smoothly adjusted in each timestep to meet current desired amplitude
	float		dAmp;						// amplitude change in each timestep (depends on desired waveheight so all waves (dis)appear with same speed)
	// serial deletion step variable
	bool		toDelete;					// used internally for parallel deletion criterion computation
public:
EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



struct GHOST_PACKET
{
    Vector2f	pos;					// 2D position
	Vector2f	dir;					// current movement direction
	float		speed;					// speed of the packet
	float		envelope;				// envelope size for this packet
	float		bending;				// point used for circular arc bending of the wave function inside envelope
	float		k;						// k = current (representative) wavenumber(s)
	float		phase;					// phase of the representative wave inside the envelope
	float		dPhase;					// phase speed relative to group speed inside the envelope
	float		ampOld;					// amplitude from last timestep, will be smoothly adjusted in each timestep to meet current desired amplitude
	float		dAmp;					// change in amplitude in each timestep (waves travel PACKET_BLEND_TRAVEL_FACTOR*envelopesize in space until they disappear)
public:
EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


class Packets
{
public:
	// scene
	int			m_groundSizeX, m_groundSizeY;	// pixel size of the ground texture
	float		*m_ground;						// texture containing the water depth and land (0.95)
	float		*m_distMap;						// distance map of the boundary map
	Vector2f	*m_gndDeriv;
	Vector2f	*m_bndDeriv;

	// packet managing
	WAVE_PACKET	*m_packet;						// wave packet data 
	GHOST_PACKET*m_ghostPacket;					// ghost packet data 
	int			m_packetBudget;					// this can be changed any time (soft budget)
	int			m_packetNum;					// current size of the buffer used for packets / ghosts
	float		m_softDampFactor;
	int			*m_usedPacket;
	int			m_usedPackets;
	int			*m_freePacket;
	int			m_freePackets;
	int			*m_usedGhost;
	int			m_usedGhosts;
	int			*m_freeGhost;
	int			m_freeGhosts;

	// simulation
	float		m_time;
	float		m_oldTime;
	float		m_elapsedTime;

	public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Packets(int packetBudget);
	~Packets(void);
	void Reset();
	float GetBoundaryDist(Vector2f &p);
	Vector2f GetBoundaryNormal(Vector2f &p);
	float GetGroundVal(Vector2f &p);
	Vector2f GetGroundNormal(Vector2f &p);
	float GetWaterDepth(Vector2f &p);
	void UpdateTime(float dTime);
	void ExpandWavePacketMemory(int targetNum);
	int GetFreePackedID();
	void DeletePacket(int id);
	int GetFreeGhostID();
	void DeleteGhost(int id);
	void CreatePacket(float pos1x, float pos1y, float pos2x, float pos2y, float dir1x, float dir1y, float dir2x, float dir2y, float k_L, float k_H, float E);
	void CreateLinearWavefront(float xPos, float yPos, float dirx, float diry, float crestlength, float lambda_L, float lambda_H, float E);
	void CreateSpreadingPacket(float xPos, float yPos, float dirx, float diry, float spreadFactor, float crestlength, float lambda_L, float lambda_H, float E);
	void CreateCircularWavefront(float xPos, float yPos, float radius, float lambda_L, float lambda_H, float E);
	void GetWaveParameters(float waterDepth, float w0, float kIn, float &k_out, float &speed_out);
	float GetPhaseSpeed(float w_0, float kIn);
	float GetWaveAmplitude(float area, float E, float k);
	float GetIntersectionDistance(Vector2f pos1, Vector2f dir1, Vector2f pos2, Vector2f dir2);
	bool AdvectPacketVertex(float elapsedTime, Vector2f &posIn, Vector2f &dirIn, float w0, float &kIn, float &speedIn, Vector2f &posOut, Vector2f &dirOut, float &speedOut);
	void AdvectWavePackets(float dTime);
};
