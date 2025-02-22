//========= Copyright Valve Corporation, All rights reserved. ============//
//
// npc_blob - experimental, cpu-intensive monster made of lots of smaller elements
//
//=============================================================================//
#include "cbase.h"
#include "hl2_shareddefs.h"
//pretty sure i have to make this a constant for networking...
//default 20
#define BLOB_NUM_ELEMENTS 10
#define MASK_BLOB_SOLID (MASK_SHOT)
#ifndef CLIENT_DLL
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "vstdlib/jobthread.h"
#include "saverestore_utlvector.h"
#include "eventqueue.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern float MOVE_HEIGHT_EPSILON;

#define BLOB_MAX_AVOID_ORIGINS 3

ConVar blob_mindist( "blob_mindist", "120.0" );
#ifdef DEBUG
ConVar blob_element_speed( "blob_element_speed", "0" );
#else
ConVar blob_element_speed("blob_element_speed", "120");
#endif
ConVar npc_blob_idle_speed_factor( "npc_blob_idle_speed_factor", "0.5" );
ConVar blob_wall_climb_debug("blob_wall_climb_debug", "0");
ConVar blob_wall_climb_debug_ceiling("blob_wall_climb_debug_ceiling", "0");
//ConVar blob_numelements( "blob_numelements", "20" );
ConVar blob_batchpercent( "blob_batchpercent", "100" );

ConVar blob_stuck_time("blob_stuck_time", "2.5");
ConVar blob_stuck_moveddist("blob_stuck_moveddist", "32");
//ConVar blob_spread_radius( "blob_spread_radius", "30" );//renamed from blob_radius to avoid confusion

//ConVar blob_min_element_speed( "blob_min_element_speed", "50" );
//ConVar blob_max_element_speed( "blob_max_element_speed", "250" );

ConVar npc_blob_use_threading( "npc_blob_use_threading", "1" );

ConVar npc_blob_sin_amplitude( "npc_blob_sin_amplitude", "90.0f" );

ConVar npc_blob_show_centroid( "npc_blob_show_centroid", "0" );

ConVar npc_blob_straggler_dist( "npc_blob_straggler_dist", "20000" );

ConVar npc_blob_use_orientation( "npc_blob_use_orientation", "1" );
ConVar npc_blob_use_model( "npc_blob_use_model", "1" );

ConVar blob_velocity_show("blob_velocity_show", "0");

ConVar npc_blob_think_interval( "npc_blob_think_interval", "0.025" );
ConVar element_number_debug("element_number_debug", "-1");

#define NPC_BLOB_MODEL "models/headcrab.mdl"

//=========================================================
// Blob movement rules
//=========================================================
enum
{
	BLOB_MOVE_SWARM = 0,				// Just swarm with the rest of the group
	BLOB_MOVE_TO_TARGET_LOCATION,		// Move to a designated location
	BLOB_MOVE_TO_TARGET_ENTITY,			// Chase the designated entity
	BLOB_MOVE_DONT_MOVE,				// Sit still!!!!
};

//=========================================================
//=========================================================
class CBlobElement : public CBaseAnimating
{
public:
	void Precache();
	void Spawn();
	int	DrawDebugTextOverlays(void); 

	void	SetElementVelocity( Vector vecVelocity, bool bPlanarOnly );
	void	AddElementVelocity( Vector vecVelocityAdd, bool bPlanarOnly );
	void	ModifyVelocityForSurface( float flInterval, float flSpeed );

	void	SetSinePhase( float flPhase ) { m_flSinePhase = flPhase; }
	float	GetSinePhase() { return m_flSinePhase; }

	float	GetSineAmplitude() { return m_flSineAmplitude; }
	float	GetSineFrequency() { return m_flSineFrequency; }

	void	SetActiveMovementRule( int moveRule ) { m_iMovementRule = moveRule; }
	int		GetActiveMovementRule() { return m_iMovementRule; }

	void	MoveTowardsTargetEntity( float speed );
	void	SetTargetEntity( CBaseEntity *pEntity ) { m_hTargetEntity = pEntity; }
	CBaseEntity *GetTargetEntity() { return m_hTargetEntity.Get(); }

	void	MoveTowardsTargetLocation( float speed );
	void	SetTargetLocation( const Vector &vecLocation ) { m_vecTargetLocation = vecLocation; }

	void	ReconfigureRandomParams();
	void	EnforceSpeedLimits( float flMinSpeed, float flMaxSpeed );

	unsigned int PhysicsSolidMaskForEntity() const;
	DECLARE_DATADESC();

public:
	Vector	m_vecPrevOrigin;	// Only exists for debugging (isolating stuck elements)
	int		m_iStuckCount;
	bool	m_bOnWall;
	float	m_flDistFromCentroidSqr;
	int		m_iElementNumber;
	Vector	m_vecTargetLocation;
	float	m_flRandomEightyPercent;
	float	m_flStuckTime = 0;
	Vector	m_flWishVel;
private:
	EHANDLE	m_hTargetEntity;
	float	m_flSinePhase;
	float	m_flSineAmplitude;
	float	m_flSineFrequency;
	int		m_iMovementRule;
};
LINK_ENTITY_TO_CLASS( blob_element, CBlobElement );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CBlobElement )

DEFINE_FIELD( m_vecPrevOrigin,			FIELD_POSITION_VECTOR ),
DEFINE_FIELD( m_iStuckCount,			FIELD_INTEGER ),
DEFINE_FIELD( m_bOnWall,				FIELD_BOOLEAN ),
DEFINE_FIELD( m_flDistFromCentroidSqr,	FIELD_FLOAT ),
DEFINE_FIELD( m_iElementNumber,			FIELD_INTEGER ),
DEFINE_FIELD( m_vecTargetLocation,		FIELD_POSITION_VECTOR ),
DEFINE_FIELD( m_hTargetEntity,			FIELD_EHANDLE ),
DEFINE_FIELD( m_flSinePhase,			FIELD_FLOAT ),
DEFINE_FIELD( m_flSineAmplitude,		FIELD_FLOAT ),
DEFINE_FIELD( m_flSineFrequency,		FIELD_FLOAT ),
DEFINE_FIELD( m_iMovementRule,			FIELD_INTEGER ),
DEFINE_FIELD(m_flStuckTime, FIELD_FLOAT),
DEFINE_FIELD(m_flWishVel, FIELD_VECTOR),
END_DATADESC()


const char *pszBlobModels[] =
{
	"models/gibs/agibs.mdl",
	"models/props_junk/watermelon01.mdl",
	"models/w_squeak.mdl",
	"models/baby_headcrab.mdl"
};

const char *GetBlobModelName()
{
	int index = npc_blob_use_model.GetInt();

	return pszBlobModels[ index ];
}

unsigned int CBlobElement::PhysicsSolidMaskForEntity() const
{
	//return BaseClass::PhysicsSolidMaskForEntity() & ~CONTENTS_GRATE;
	return MASK_BLOB_SOLID;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CBlobElement::Precache()
{
	PrecacheModel( GetBlobModelName() );

	m_flRandomEightyPercent = random->RandomFloat( 0.8f, 1.0f );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CBlobElement::Spawn()
{
	Precache();
	
	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_STEP );
	AddSolidFlags( FSOLID_NOT_STANDABLE | FSOLID_NOT_SOLID );
	SetRenderMode(kRenderTransAdd);
	SetRenderColorA(0);
	SetModel( GetBlobModelName() );
	UTIL_SetSize(this, Vector(-1, -1, -1), Vector(1, 1, 1));

	QAngle angles(0,0,0);
	angles.y = random->RandomFloat( 0, 180 );
	SetAbsAngles( angles );

	AddEffects( EF_NOSHADOW );//EF_NODRAW intentionally left off so that elements will be networked... probably a better way out there
	SetCollisionGroup(HL2COLLISION_GROUP_BLOB);
	ReconfigureRandomParams();
}

//---------------------------------------------------------
//---------------------------------------------------------
int CBlobElement::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();
	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];
		Q_snprintf(tempstr,sizeof(tempstr), "Element #:%d", m_iElementNumber );
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}


//---------------------------------------------------------
// This is the official way to set velocity for an element
// Do not call SetAbsVelocity() directly, since we also
// need to record the last velocity we intended to give the
// element, so that we can detect changes after game physics
// runs.
//---------------------------------------------------------
void CBlobElement::SetElementVelocity( Vector vecVelocity, bool bPlanarOnly )
{
	if (blob_velocity_show.GetBool()) NDebugOverlay::Line(GetAbsOrigin(), GetAbsOrigin() + vecVelocity, 255, 0, 0, true, -1);
	SetAbsVelocity( vecVelocity );
}

//---------------------------------------------------------
// This is the official way to add velocity to an element. 
// See SetElementVelocity() for explanation.
//---------------------------------------------------------
void CBlobElement::AddElementVelocity( Vector vecVelocityAdd, bool bPlanarOnly )
{
	Vector vecSum = GetAbsVelocity() + vecVelocityAdd;
	SetAbsVelocity( vecSum );
}

//---------------------------------------------------------
// This function seeks to keep the blob element moving along
// multiple different types of surfaces (climbing walls, etc)
//---------------------------------------------------------
#define BLOB_TRACE_HEIGHT 8.0f
void CBlobElement::ModifyVelocityForSurface( float flInterval, float flSpeed )
{
	Assert(element_number_debug.GetInt() != m_iElementNumber);
	trace_t tr;
	Vector vecStart = GetAbsOrigin();
	Vector up = Vector( 0, 0, BLOB_TRACE_HEIGHT );

	Vector vecWishedGoal = vecStart + (m_flWishVel * flInterval);//half to climb over rail in coast 04

	UTIL_TraceHull(vecStart + Vector(0, 0, 1)/* + up*/, vecWishedGoal + up, Vector(-1, -1, -1), Vector(1, 1, 1), MASK_BLOB_SOLID, this, HL2COLLISION_GROUP_BLOB, &tr);

	//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, false, 0.1f );

	//look at ceilings
	//the ceiling above the element is not the one of the blob (target entity is always the npc_blob unless you use the chaseentity input)
	//we're probably entering a building, and if we climb, we'll just end up on the roof
	trace_t tr2;
	UTIL_TraceHull(vecStart, vecStart + Vector(0, 0, MAX_TRACE_LENGTH), Vector(-1, -1, -1), Vector(1, 1, 1), MASK_BLOB_SOLID, this, HL2COLLISION_GROUP_BLOB, &tr2);
	trace_t tr3;
	UTIL_TraceHull(tr2.endpos, GetTargetEntity()->GetAbsOrigin(), Vector(-1, -1, -1), Vector(1, 1, 1), MASK_BLOB_SOLID & ~CONTENTS_MONSTER, GetTargetEntity(), HL2COLLISION_GROUP_BLOB, &tr3);

	if (blob_wall_climb_debug_ceiling.GetBool())
	{
		NDebugOverlay::Line(vecStart, tr2.endpos, 0, 255, 255, true, -1);
		NDebugOverlay::Line(tr2.endpos, tr3.endpos, 0, 0, 255, true, -1);
	}

	if (blob_wall_climb_debug.GetBool())
	{
		Msg("vecStart %0.1f %0.1f %0.1f GetAbsVelocity() %0.1f %0.1f %0.1f flInterval %f\n", vecStart.x, vecStart.y, vecStart.z, GetAbsVelocity().x, GetAbsVelocity().y, GetAbsVelocity().z, flInterval);
		NDebugOverlay::Line(vecStart + up, tr.endpos, 0, 255, 0, true, -1);
		NDebugOverlay::Line(tr.endpos, vecWishedGoal + up, 255, 0, 0, true, -1);
	}

	m_bOnWall = false;

	if (tr.fraction == 1.0f || tr3.fraction < 0.9)
	{
		UTIL_TraceHull(vecWishedGoal + up, vecWishedGoal - (up * 2.0f), Vector(-1, -1, -1), Vector(1, 1, 1), MASK_BLOB_SOLID, this, HL2COLLISION_GROUP_BLOB, &tr);
		//NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 255, 0, false, 0.1f );
		tr.endpos.z += MOVE_HEIGHT_EPSILON;
	}
	else
	{
		m_bOnWall = true;

		if (tr.m_pEnt != NULL && !tr.m_pEnt->IsWorld())
		{
			IPhysicsObject *pPhysics = tr.m_pEnt->VPhysicsGetObject();

			if (pPhysics != NULL)
			{
				Vector vecMassCenter;
				Vector vecMassCenterWorld;

				vecMassCenter = pPhysics->GetMassCenterLocalSpace();
				pPhysics->LocalToWorld(&vecMassCenterWorld, vecMassCenter);

				if (tr.endpos.z > vecMassCenterWorld.z)
				{
					pPhysics->ApplyForceOffset((-150.0f * m_flRandomEightyPercent) * tr.plane.normal, tr.endpos);
				}
			}
		}
	}

	Vector vecDir = tr.endpos - vecStart;
	if (!m_bOnWall)
		vecDir.z = min(vecDir.z, 0);
	VectorNormalize( vecDir );
	SetElementVelocity( vecDir * flSpeed, false );
}

//---------------------------------------------------------
// Set velocity that will carry me towards a specified entity
// Most often used to move along with the npc_blob that 
// is directing me.
//---------------------------------------------------------
void CBlobElement::MoveTowardsTargetEntity( float speed )
{
	CBaseEntity *pTarget = m_hTargetEntity.Get();

	if( pTarget != NULL )
	{
		// Try to attack my target's enemy directly if I can.
		CBaseEntity *pTargetEnemy = pTarget->GetEnemy();

		if( pTargetEnemy != NULL )
		{
			//don't go to enemy if we can't see it
			trace_t trToEnemy;
			UTIL_TraceLine(GetAbsOrigin(), pTargetEnemy->GetAbsOrigin(), MASK_BLOB_SOLID, this, HL2COLLISION_GROUP_BLOB, &trToEnemy);
			if (trToEnemy.fraction == 1 || trToEnemy.m_pEnt == pTargetEnemy)
				pTarget = pTargetEnemy;
		}

		Vector vecDir = pTarget->WorldSpaceCenter() - GetAbsOrigin();
		vecDir.NormalizeInPlace();
		//Msg("target z: %0.1f my z: %0.1f going %f\n", pTarget->WorldSpaceCenter().z, GetAbsOrigin().z, vecDir.z);
		//NDebugOverlay::Line(pTarget->WorldSpaceCenter(), GetAbsOrigin(), 0, 255, 0, true, -1);
		//NDebugOverlay::Line(GetAbsOrigin(), GetAbsOrigin() + vecDir * speed, 0, 255, 0, true, -1);
		//SetElementVelocity( vecDir * speed, true );
		m_flWishVel = vecDir * speed;
	}
	else
	{
        //SetElementVelocity( vec3_origin, true );
		m_flWishVel = vec3_origin;
	}
}

//---------------------------------------------------------
// Set velocity that will take me towards a specified location.
// This is often used to send all blob elements to specific
// locations, causing the blob to appear as though it has
// formed a specific shape.
//---------------------------------------------------------
void CBlobElement::MoveTowardsTargetLocation( float speed )
{
	Vector vecDir = m_vecTargetLocation - GetAbsOrigin();
	float dist = VectorNormalize( vecDir );

	//!!!HACKHACK - how about a real way to tell if we've reached our goal?
	if( dist <= 8.0f )
	{
		SetActiveMovementRule( BLOB_MOVE_DONT_MOVE );
	}

	speed = MIN( dist, speed );

	SetElementVelocity( vecDir * speed, true );
}

//---------------------------------------------------------
// Pick new random numbers for the parameters that create
// variations in movement.
//---------------------------------------------------------
void CBlobElement::ReconfigureRandomParams()
{
	m_flSinePhase = random->RandomFloat( 0.01f, 0.5f );
	m_flSineFrequency = random->RandomFloat( 7.5f, 20.0f );
	m_flSineAmplitude = random->RandomFloat( 0.25f, 1.0f );
}

//---------------------------------------------------------
// Adjust velocity if this element is moving faster than 
// flMaxSpeed or slower than flMinSpeed
//---------------------------------------------------------
void CBlobElement::EnforceSpeedLimits( float flMinSpeed, float flMaxSpeed )
{
	Vector vecVelocity = m_flWishVel;// GetAbsVelocity();
	float flSpeed = VectorNormalize( vecVelocity );

	if( flSpeed > flMaxSpeed )
	{
		//SetElementVelocity( vecVelocity * flMaxSpeed, true );
		m_flWishVel = vecVelocity * flMaxSpeed;
	}
	else if( flSpeed < flMinSpeed )
	{
		//SetElementVelocity( vecVelocity * flMinSpeed, true );
		m_flWishVel = vecVelocity * flMinSpeed;
	}
}

//=========================================================
// Custom schedules
//=========================================================
enum
{
	SCHED_MYCUSTOMSCHEDULE = LAST_SHARED_SCHEDULE,
};

//=========================================================
// Custom tasks
//=========================================================
enum 
{
	TASK_MYCUSTOMTASK = LAST_SHARED_TASK,
};


//=========================================================
// Custom Conditions
//=========================================================
enum 
{
	COND_MYCUSTOMCONDITION = LAST_SHARED_CONDITION,
};


//=========================================================
//=========================================================
class CNPC_Blob : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_Blob, CAI_BaseNPC );

public:
	CNPC_Blob();
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
	void	RunAI();
	void	GatherConditions( void );
	int		SelectSchedule( void );
	int		GetSoundInterests( void ) { return (SOUND_BUGBAIT); }

	float	GetSequenceGroundSpeed(CStudioHdr *pStudioHdr, int iSequence);
	void	ComputeCentroid();

	void	DoBlobBatchedAI( int iStart, int iEnd );

	int		ComputeBatchSize();
	void	AdvanceBatch();
	int		GetBatchStart();
	int		GetBatchEnd();

	CBlobElement *CreateNewElement();
	void	InitializeElements();
	void	RecomputeIdealElementDist();

	void	RemoveExcessElements( int iNumElements );
	void	AddNewElements( int iNumElements );

	void	FormShapeFromPath( string_t iszPathName );
	void	SetRadius( float flRadius );
	float	m_flRadius;
	virtual Vector BodyTarget(bool bNoisy);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CNetworkVar(int, m_iNumElements);
	bool	m_bInitialized;
	int		m_iBatchStart;
	Vector	m_vecCentroid;
	float	m_flMinElementDist;

	//CUtlVector<CHandle< CBlobElement > >m_Elements;
	CNetworkArray(EHANDLE, m_Elements, BLOB_NUM_ELEMENTS);
	DEFINE_CUSTOM_AI;

public:
	void InputFormPathShape( inputdata_t &inputdata );
	void InputSetRadius( inputdata_t &inputdata );
	void InputChaseEntity( inputdata_t &inputdata );
	void InputFormHemisphere( inputdata_t &inputdata );
	void InputFormTwoSpheres( inputdata_t &inputdata );
	void OnKilledNPC(CBaseCombatCharacter *pKilled);
public:
	Vector	m_vecAvoidOrigin[ BLOB_MAX_AVOID_ORIGINS ];
	float	m_flAvoidRadiusSqr;
	virtual Disposition_t IRelationType(CBaseEntity *pTarget);
private:
	int		m_iReconfigureElement;
	int		m_iNumAvoidOrigins;

	bool	m_bEatCombineHack;
};

LINK_ENTITY_TO_CLASS( npc_blob, CNPC_Blob );
IMPLEMENT_CUSTOM_AI( npc_blob,CNPC_Blob );


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Blob )

DEFINE_FIELD( m_iNumElements, FIELD_INTEGER ),
DEFINE_FIELD( m_bInitialized, FIELD_BOOLEAN ),
DEFINE_FIELD( m_iBatchStart, FIELD_INTEGER ),
DEFINE_FIELD( m_vecCentroid, FIELD_POSITION_VECTOR ),
DEFINE_FIELD( m_flMinElementDist, FIELD_FLOAT ),
DEFINE_FIELD( m_iReconfigureElement, FIELD_INTEGER ),
DEFINE_AUTO_ARRAY( m_Elements, FIELD_EHANDLE ),
DEFINE_FIELD(m_flRadius, FIELD_FLOAT),

DEFINE_INPUTFUNC( FIELD_STRING, "FormPathShape", InputFormPathShape ),
DEFINE_INPUTFUNC( FIELD_FLOAT, "SetRadius", InputSetRadius ),
DEFINE_INPUTFUNC( FIELD_STRING, "ChaseEntity", InputChaseEntity ),
DEFINE_INPUTFUNC( FIELD_VOID, "FormHemisphere", InputFormHemisphere ),
DEFINE_INPUTFUNC( FIELD_VOID, "FormTwoSpheres", InputFormTwoSpheres ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CNPC_Blob, DT_NPC_Blob)
SendPropArray3(SENDINFO_ARRAY3(m_Elements), SendPropEHandle(SENDINFO_ARRAY(m_Elements))),
SendPropInt(SENDINFO(m_iNumElements), 3, SPROP_UNSIGNED),
END_SEND_TABLE()

//---------------------------------------------------------
//---------------------------------------------------------
CNPC_Blob::CNPC_Blob()
{
	m_iNumElements = 0;
	m_bInitialized = false;
	m_iBatchStart = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the custom schedules
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_Blob::InitCustomSchedules(void) 
{
	INIT_CUSTOM_AI(CNPC_Blob);

	ADD_CUSTOM_TASK(CNPC_Blob,		TASK_MYCUSTOMTASK);

	ADD_CUSTOM_SCHEDULE(CNPC_Blob,	SCHED_MYCUSTOMSCHEDULE);

	ADD_CUSTOM_CONDITION(CNPC_Blob,	COND_MYCUSTOMCONDITION);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_Blob::Precache( void )
{
	PrecacheModel( NPC_BLOB_MODEL );
	UTIL_PrecacheOther( "blob_element" );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_Blob::Spawn( void )
{
	Precache();

	SetModel( NPC_BLOB_MODEL );

	SetHullType(HULL_TINY);
	SetHullSizeNormal();

	SetSolid( SOLID_NONE );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= INT_MAX;
	m_flFieldOfView		= -1.0f;
	m_NPCState			= NPC_STATE_NONE;

	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND );

	//m_Elements.RemoveAll();

	NPCInit();
	m_flRadius = 30;
	AddEffects( EF_NOSHADOW );

	m_flMinElementDist = blob_mindist.GetFloat();
}

Disposition_t CNPC_Blob::IRelationType(CBaseEntity *pTarget)
{
	//we're useless against flying things
	if (pTarget->Classify() == CLASS_SCANNER || pTarget->Classify() == CLASS_MANHACK)
	{
		return D_NU;
	}
	if (pTarget->GetFlags() & FL_FLY)
	{
		return D_NU;
	}
	return BaseClass::IRelationType(pTarget);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Blob::Classify( void )
{
	return	CLASS_PLAYER_ALLY;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::RunAI()
{
	BaseClass::RunAI();

	if( !m_bInitialized )
	{
		// m_bInitialized is set to false in the constructor. So this bit of
		// code runs one time, the first time I think.
		Msg("I need to initialize\n");
		InitializeElements();
		m_bInitialized = true;
		return;
	}

	int iIdealNumElements = BLOB_NUM_ELEMENTS;
	if( iIdealNumElements != m_iNumElements )
	{
		int delta = iIdealNumElements - m_iNumElements;

		if( delta < 0 )
		{
			delta = -delta;
			delta = MIN(delta, 5 );
			RemoveExcessElements( delta );
			
			if( m_iReconfigureElement > m_iNumElements )
			{
				// Start this index over at zero, if it is past the new end of the utlvector.
				m_iReconfigureElement = 0;
			}
		}
		else
		{
			delta = MIN(delta, 5 );
			AddNewElements( delta );
		}
	
		RecomputeIdealElementDist();
	}

	ComputeCentroid();

	if( npc_blob_show_centroid.GetBool() )
	{
		NDebugOverlay::Cross3D( m_vecCentroid + Vector( 0, 0, 12 ), 32, 0, 255, 0, false, 0.025f );
	}

	if( npc_blob_use_threading.GetBool() )
	{
		IterRangeParallel( this, &CNPC_Blob::DoBlobBatchedAI, 0, m_Elements.Count() );
	}
	else
	{
		DoBlobBatchedAI( 0, m_Elements.Count() );
	}

	if( GetEnemy() != NULL )
	{
		float flEnemyDistSqr = m_vecCentroid.DistToSqr( GetEnemy()->WorldSpaceCenter() );

		if( flEnemyDistSqr <= Square( 40.0f ) )
		{
			//NDebugOverlay::Line(m_vecCentroid, GetEnemy()->WorldSpaceCenter(), 0, 255, 0, true, -1);
			if( GetEnemy()->Classify() == CLASS_COMBINE )
			{
				if( !m_bEatCombineHack )
				{
					variant_t var;

					var.SetFloat(0);
					g_EventQueue.AddEvent(GetEnemy(), "HitByBugBait", 0.0f, this, this);
					g_EventQueue.AddEvent(GetEnemy(), "SetHealth", var, 3.0f, this, this);
					g_EventQueue.AddEvent(this, "KilledNPC", var, 3.0f, this, this);
					m_bEatCombineHack = true;

					m_flRadius = 48;
					RecomputeIdealElementDist();
				}
			}
			else
			{
				CTakeDamageInfo info;

				info.SetAttacker( this );
				info.SetInflictor( this );
				info.SetDamage( 5 );
				info.SetDamageType( DMG_SLASH );
				info.SetDamageForce( Vector( 0, 0, 10 ) );

				GetEnemy()->TakeDamage( info );
			}
		}
		else
		{
			//NDebugOverlay::Line(m_vecCentroid, GetEnemy()->WorldSpaceCenter(), 255, 0, 0, true, -1);
		}
	}

	SetNextThink( gpGlobals->curtime + npc_blob_think_interval.GetFloat() );
}
void CNPC_Blob::OnKilledNPC(CBaseCombatCharacter *pKilled)
{
	if (m_bEatCombineHack)
	{
		m_bEatCombineHack = false;
		m_flRadius = 30;
		RecomputeIdealElementDist();
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::GatherConditions( void )
{
	if( m_bEatCombineHack )
	{
		// We just ate someone.
		if( !GetEnemy() || !GetEnemy()->IsAlive() )
		{
			m_bEatCombineHack = false;
			m_flRadius = 30;
			RecomputeIdealElementDist();
		}
	}

	BaseClass::GatherConditions();

}

//-----------------------------------------------------------------------------
// Either stand still or chase the enemy, for now.
//-----------------------------------------------------------------------------
int CNPC_Blob::SelectSchedule( void )
{
	if( GetEnemy() == NULL )
		return SCHED_IDLE_STAND;

	//if we've lost sight of the centroid, stop and wait for elements to catch up
	trace_t trToCentroid;
	UTIL_TraceLine(GetAbsOrigin(), m_vecCentroid, MASK_BLOB_SOLID, this, HL2COLLISION_GROUP_BLOB, &trToCentroid);
	if (trToCentroid.DidHit())
		return SCHED_IDLE_STAND;

	return SCHED_CHASE_ENEMY;
}

//-----------------------------------------------------------------------------
// Average the origin of all elements to get the centroid for the group
//-----------------------------------------------------------------------------
void CNPC_Blob::ComputeCentroid()
{
	m_vecCentroid = vec3_origin;

	for( int i = 0 ; i < m_Elements.Count() ; i++ )
	{
		if (!m_Elements[i])
			continue;
		m_vecCentroid += m_Elements[i]->GetAbsOrigin();
	}

	m_vecCentroid /= m_Elements.Count();
}

//-----------------------------------------------------------------------------
// Run all of the AI for elements within the range iStart to iEnd 
//-----------------------------------------------------------------------------
void CNPC_Blob::DoBlobBatchedAI( int iStart, int iEnd )
{
	float flInterval = gpGlobals->curtime - GetLastThink();

	// Local fields for sin-wave movement variance
	float flMySine;
	float flAmplitude = npc_blob_sin_amplitude.GetFloat();
	float flMyAmplitude;
	Vector vecRight = vec3_origin;
	Vector vecForward;

	// Local fields for attract/repel
	float minDistSqr = Square( m_flMinElementDist );
	float flBlobSpeed = blob_element_speed.GetFloat();
	float flSpeed;

	// Local fields for speed limiting
	float flMinSpeed = blob_element_speed.GetFloat() * 0.5f;
	float flMaxSpeed = blob_element_speed.GetFloat() * 1.5f;
	bool bEnforceSpeedLimit;
	bool bEnforceRelativePositions;
	bool bDoMovementVariation;
	bool bDoOrientation = npc_blob_use_orientation.GetBool();
	float flIdleSpeedFactor = npc_blob_idle_speed_factor.GetFloat();

	// Group cohesion
	float flBlobRadiusSqr = Square(m_flRadius + 48.0f); // Four feet of fudge

	// Build a right-hand vector along which we'll add some sine wave data to give each
	// element a unique insect-like undulation along an axis perpendicular to their path,
	// which makes the entire group look far less orderly
	if( GetEnemy() != NULL )
	{
		// If I have an enemy, the right-hand vector is perpendicular to a straight line 
		// from the group's centroid to the enemy's origin.
		vecForward = GetEnemy()->GetAbsOrigin() - m_vecCentroid;
		VectorNormalize( vecForward );
		vecRight.x = vecForward.y;
		vecRight.y = -vecForward.x;
	}
	else
	{
		// If there is no enemy, wobble along the axis from the centroid to me.
		vecForward = GetAbsOrigin() - m_vecCentroid;
		VectorNormalize( vecForward );
		vecRight.x = vecForward.y;
		vecRight.y = -vecForward.x;
	}

	//--
	// MAIN LOOP - Run all of the elements in the set iStart to iEnd
	//--
	for( int i = iStart ; i < iEnd ; i++ )
	{
		CBlobElement *pThisElement = (CBlobElement *)m_Elements[i].Get();
		if (!pThisElement)
			continue;

		//--
		// Initial movement
		//--
		// Start out with bEnforceSpeedLimit set to false. This is because an element
		// can't overspeed if it's moving undisturbed towards its target entity or 
		// target location. An element can only under or overspeed when it is repelled 
		// by multiple other elements in the group. See "Relative Positions" below.
		//
		// Initialize some 'defaults' that may be changed for each iteration of this loop
		bEnforceSpeedLimit = false;
		bEnforceRelativePositions = true;
		bDoMovementVariation = true;
		flSpeed = flBlobSpeed;

		switch( pThisElement->GetActiveMovementRule() )
		{
		case BLOB_MOVE_DONT_MOVE:
			{
				//pThisElement->SetElementVelocity( vec3_origin, true );
				pThisElement->m_flWishVel = vec3_origin;

				trace_t tr;
				Vector vecOrigin = pThisElement->GetAbsOrigin();

				UTIL_TraceHull(vecOrigin, vecOrigin - Vector(0, 0, 16), Vector(-1, -1, -1), Vector(1, 1, 1), MASK_BLOB_SOLID, this, HL2COLLISION_GROUP_BLOB, &tr);

				if( tr.fraction < 1.0f )
				{
					QAngle angles;

					VectorAngles( tr.plane.normal, angles );

					float flSwap = angles.x;

					angles.x = -angles.y;
					angles.y = flSwap;

					pThisElement->SetAbsAngles( angles );
				}
			}
			continue;
			break;

		case BLOB_MOVE_TO_TARGET_LOCATION:
			{
				Vector vecDiff = pThisElement->GetAbsOrigin() - pThisElement->m_vecTargetLocation;

				if( vecDiff.Length2DSqr() <= Square(80.0f) )
				{
					// Don't shove this guy around any more, let him get to his goal position.
					flSpeed *= 0.5f;
					bEnforceRelativePositions = false;
					bDoMovementVariation = false;
				}

				pThisElement->MoveTowardsTargetLocation( flSpeed );
			}
			break;

		case BLOB_MOVE_TO_TARGET_ENTITY:
			{
				if( !IsMoving() && GetEnemy() == NULL )
				{
					if( pThisElement->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) <= flBlobRadiusSqr )
					{
						flSpeed = (flSpeed * flIdleSpeedFactor) * pThisElement->m_flRandomEightyPercent;
					}
				}
				pThisElement->MoveTowardsTargetEntity( flSpeed );
			}
			break;

		default:
			Msg("ERROR: Blob Element with unspecified Movement Rule\n");
			break;
		}

		//---
		// Relative positions
		//--
		// Check this element against ALL other elements. If the two elements are closer
		// than the allowed minimum distance, repel this element away. (The other element
		// will repel when its AI runs). A single element can be repelled by many other 
		// elements. This is why bEnforceSpeedLimit is set to true if any of the repelling
		// code runs for this element. Multiple attempts to repel an element in the same
		// direction will cause overspeed. Conflicting attempts to repel an element in opposite
		// directions will cause underspeed.
		Vector vecDir = Vector( 0, 0, 0 );
		Vector vecThisElementOrigin = pThisElement->GetAbsOrigin();

		if( bEnforceRelativePositions )
		{
			for( int j = 0 ; j < m_Elements.Count() ; j++ )
			{
				// This is the innermost loop! We should optimize here, if anywhere.

				// If this element is on the wall, then don't be repelled by anyone. Repelling
				// elements that are trying to climb a wall usually make them look like they 
				// fall off the wall a few times while climbing.
				if( pThisElement->m_bOnWall )
					continue;

				CBlobElement *pThatElement = (CBlobElement *)m_Elements[j].Get();
				if (!pThatElement)
					continue;
				if( i != j )
				{
					Vector vecThatElementOrigin = pThatElement->GetAbsOrigin();
					float distSqr = vecThisElementOrigin.DistToSqr( vecThatElementOrigin );

					if( distSqr < minDistSqr )
					{
						// Too close to the other element. Move away.
						float flRepelSpeed;
						Vector vecRepelDir = ( vecThisElementOrigin - vecThatElementOrigin );

						vecRepelDir.NormalizeInPlace();
						flRepelSpeed = (flSpeed * (1.0f - (distSqr / minDistSqr))) * pThatElement->GetSinePhase();
						if (GetEnemy())
							flRepelSpeed /= 3;//spread less when chasing an enemy for visual appeal
						//pThisElement->AddElementVelocity( vecRepelDir * flRepelSpeed, true );
						pThisElement->m_flWishVel += vecRepelDir * flRepelSpeed;
						// Since we altered this element's velocity after it was initially set, there's a chance
						// that the sums of multiple vectors will cause the element to over or underspeed, so 
						// mark it for speed limit enforcement
						bEnforceSpeedLimit = true;
					}
				}
			}
		}

		//--
		// Movement variation
		//--
		if( bDoMovementVariation )
		{
			flMySine = sin( gpGlobals->curtime * pThisElement->GetSineFrequency() );
			flMyAmplitude = flAmplitude * pThisElement->GetSineAmplitude();
			//pThisElement->AddElementVelocity( vecRight * (flMySine * flMyAmplitude), true );
			pThisElement->m_flWishVel += vecRight * (flMySine * flMyAmplitude);
		}

		// Avoidance
		for( int a = 0 ; a < m_iNumAvoidOrigins ; a++ )
		{
			Vector vecAvoidDir = pThisElement->GetAbsOrigin() - m_vecAvoidOrigin[ a ];

			if( vecAvoidDir.LengthSqr() <= (m_flAvoidRadiusSqr * pThisElement->m_flRandomEightyPercent) )
			{
				VectorNormalize( vecAvoidDir );
				//pThisElement->AddElementVelocity( vecAvoidDir * (flSpeed * 2.0f), true );
				pThisElement->m_flWishVel += vecAvoidDir * (flSpeed * 2.0f);
				break;
			}
		}

		//--
		// Speed limits
		//---
		if( bEnforceSpeedLimit == true )
		{
			pThisElement->EnforceSpeedLimits( flMinSpeed, flMaxSpeed );
		}

		//--
		// Wall crawling
		//--
		pThisElement->ModifyVelocityForSurface( flInterval, flSpeed );

		// For identifying stuck elements.
		pThisElement->m_flDistFromCentroidSqr = pThisElement->GetAbsOrigin().DistToSqr(m_vecCentroid);
		//if an element is stuck for too long, let it teleport to the blob
		trace_t trToCentroid;
		UTIL_TraceLine(pThisElement->GetAbsOrigin(), m_vecCentroid, MASK_BLOB_SOLID, this, HL2COLLISION_GROUP_BLOB, &trToCentroid);
		//NDebugOverlay::Line(pThisElement->GetAbsOrigin(), m_vecCentroid, bFarAway ? 255 : 0, bBlocked ? 255 : 0, bNotMoving ? 255 : 0, true, -1);
		//an element is stuck if it's far from the centroid or it is obstructed from the centroid, and it isn't moving much.
		if ((pThisElement->m_flDistFromCentroidSqr > npc_blob_straggler_dist.GetFloat() || trToCentroid.fraction < 1 || trToCentroid.m_pEnt != GetEnemy()) && (pThisElement->m_vecPrevOrigin - pThisElement->GetAbsOrigin()).Length() < blob_stuck_moveddist.GetFloat())
		{
			pThisElement->m_flStuckTime += npc_blob_think_interval.GetFloat();
			if (pThisElement->m_flStuckTime > blob_stuck_time.GetFloat())
			{
				pThisElement->SetAbsOrigin(GetAbsOrigin() + Vector(RandomFloat(-1, 1), RandomFloat(-1, 1), 2));//offset prevents elements being stuck together forever because vecRight may come back 0
			}
		}
		else
		{
			//if (pThisElement->m_flStuckTime > 0 && (pThisElement->m_vecPrevOrigin - pThisElement->GetAbsOrigin()).Length() < 32)
			//	Msg("Element became unstuck after being stuck for %f seconds\n", pThisElement->m_flStuckTime);
			pThisElement->m_flStuckTime = 0;
			pThisElement->m_vecPrevOrigin = pThisElement->GetAbsOrigin();
		}

		// Orientation
		if( bDoOrientation )
		{
			QAngle angles;
			VectorAngles( pThisElement->GetAbsVelocity(), angles );
			pThisElement->SetAbsAngles( angles );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: The blob has too many elements. Locate good candidates and remove
// this many elements.
//-----------------------------------------------------------------------------
void CNPC_Blob::RemoveExcessElements( int iNumElements )
{
	// For now we're not assessing candidates, just blindly removing.
	int i;
	for( i = 0 ; i < iNumElements ; i++ )
	{
		int iLastElement = m_iNumElements - 1;
		
		// Nuke the associated entity
		m_Elements[ iLastElement ]->SUB_Remove();

		m_Elements.Set(iLastElement, NULL);
		m_iNumElements--;
	}
}

//-----------------------------------------------------------------------------
// Purpose: This blob has too few elements. Add this many elements by stacking
// them on top of existing elements and allowing them to disperse themselves
// into the blob.
//-----------------------------------------------------------------------------
void CNPC_Blob::AddNewElements( int iNumElements )
{
	int i;
	
	// Keep track of how many elements we had when we came into this function.
	// Since the new elements copy their origins from existing elements, we only want
	// to copy origins from elements that existed before we came into this function. 
	// Otherwise, the more elements we create while in this function, the more likely it 
	// becomes that several of them will stack on the same origin.
	int iInitialElements = m_iNumElements;

	for( i = 0 ; i < iNumElements ; i++ )
	{
		CBlobElement *pElement = CreateNewElement();

		if( pElement != NULL )
		{
			// Copy the origin of some element that is not me. This will make the expansion
			// of the group easier on the eye, since this element will spawn inside of some
			// other element, and then be pushed out by the blob's repel rules.
			int iCopyElement = random->RandomInt(0, iInitialElements - 1);
			CBlobElement *pCopyElement = (CBlobElement *)m_Elements[iCopyElement].Get();
			if (!pCopyElement)
				continue;
			if (pCopyElement->m_flStuckTime > 0)//don't copy an element that's stuck
				continue;
			pElement->SetAbsOrigin( m_Elements[iCopyElement]->GetAbsOrigin() );
			//Msg("new element at %0.1f %0.1f %0.1f\n", m_Elements[iCopyElement]->GetAbsOrigin().x, m_Elements[iCopyElement]->GetAbsOrigin().y, m_Elements[iCopyElement]->GetAbsOrigin().z);
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define BLOB_MAX_VERTS 128
void CNPC_Blob::FormShapeFromPath( string_t iszPathName )
{
	Vector vertex[ BLOB_MAX_VERTS ];

	int i;
	int iNumVerts = 0;

	for ( i = 0 ; i < BLOB_MAX_VERTS ; i++ )
	{
		if( iszPathName == NULL_STRING )
		{
			//Msg("Terminal path\n");
			break;
		}

		CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, iszPathName );

		if( pEntity != NULL )
		{
			bool bClosedPath = false;

			for( int j = 0 ; j < i ; j++ )
			{
				// Stop if we reach a vertex that's already in the array (closed path)
				if( vertex[ j ] == pEntity->GetAbsOrigin() )
				{
					//Msg("Closed path!\n");
					bClosedPath = true;
					break;
				}
			}

			vertex[ i ] = pEntity->GetAbsOrigin();
			iszPathName = pEntity->m_target;
			iNumVerts++;

			if( bClosedPath )
				break;
		}
	}

	//Msg("%d verts found in path!\n", iNumVerts);

	float flPathLength = 0.0f;
	float flDistribution;

	for( i = 0 ; i < iNumVerts - 1 ; i++ )
	{
		Vector vecDiff = vertex[ i ] - vertex[ i + 1 ];

		flPathLength += vecDiff.Length();
	}

	flDistribution = flPathLength / m_iNumElements;
	Msg("Path length is %f, distribution is %f\n", flPathLength, flDistribution );

	int element = 0;
	for( i = 0 ; i < iNumVerts - 1 ; i++ )
	{
		//NDebugOverlay::Line( vertex[ i ], vertex[ i + 1 ], 0, 255, 0, false, 10.0f );
		Vector vecDiff = vertex[ i + 1 ] - vertex[ i ];
		Vector vecStart = vertex[ i ];

		float flSegmentLength = VectorNormalize( vecDiff );

		float flStep;

		for( flStep = 0.0f ; flStep < flSegmentLength ; flStep += flDistribution )
		{
			//NDebugOverlay::Cross3D( vecStart + vecDiff * flStep, 16, 255, 255, 255, false, 10.0f );
			CBlobElement *pThisElement = (CBlobElement *)m_Elements[element].Get();
			pThisElement->SetTargetLocation(vecStart + vecDiff * flStep);
			pThisElement->SetActiveMovementRule(BLOB_MOVE_TO_TARGET_LOCATION);
			element++;

			if( element == m_iNumElements )
				return;
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::SetRadius( float flRadius )
{
	m_flRadius = flRadius;
	RecomputeIdealElementDist();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::InputFormPathShape( inputdata_t &inputdata )
{
	string_t shape = inputdata.value.StringID();

	if( shape == NULL_STRING )
		return;

	//Msg("I'm supposed to form some shape called:%s\n", shape );

	FormShapeFromPath( shape );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::InputSetRadius( inputdata_t &inputdata )
{
	float flNewRadius = inputdata.value.Float();

	SetRadius( flNewRadius );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::InputChaseEntity( inputdata_t &inputdata )
{
	CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, inputdata.value.StringID(), NULL, inputdata.pActivator, inputdata.pCaller );
	
	if ( pEntity )
	{
		for( int i = 0 ; i < m_Elements.Count() ; i++ )
		{
			CBlobElement *pElement = (CBlobElement *)m_Elements[i].Get();

			pElement->SetTargetEntity( pEntity );
			pElement->SetActiveMovementRule( BLOB_MOVE_TO_TARGET_ENTITY );
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::InputFormHemisphere( inputdata_t &inputdata )
{
	Vector center = GetAbsOrigin();
	const float flRadius = 240.0f;

	Vector vecDir;

	for( int i = 0 ; i < m_Elements.Count() ; i++ )
	{
		CBlobElement *pElement = (CBlobElement *)m_Elements[i].Get();

		// Compute a point around my center
		vecDir.x = random->RandomFloat( -1, 1 );
		vecDir.y = random->RandomFloat( -1, 1 );
		vecDir.z = random->RandomFloat( 0, 1 );

		VectorNormalize( vecDir );

		pElement->SetTargetLocation( center + vecDir * flRadius );
		pElement->SetActiveMovementRule( BLOB_MOVE_TO_TARGET_LOCATION );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::InputFormTwoSpheres( inputdata_t &inputdata )
{
	Vector center = GetAbsOrigin();
	Vector sphere1 = GetAbsOrigin() + Vector( 120.0f, 0, 120.0f );
	Vector sphere2 = GetAbsOrigin() + Vector( -120.0f, 0, 120.0f );
	const float flRadius = 100.0f;

	Vector vecDir;

	int batchSize = m_Elements.Count() / 2;

	for( int i = 0 ; i < batchSize ; i++ )
	{
		CBlobElement *pElement = (CBlobElement *)m_Elements[i].Get();

		// Compute a point around my center
		vecDir.x = random->RandomFloat( -1, 1 );
		vecDir.y = random->RandomFloat( -1, 1 );
		vecDir.z = random->RandomFloat( -1, 1 );

		VectorNormalize( vecDir );

		pElement->SetTargetLocation( sphere1 + vecDir * flRadius );
		pElement->SetActiveMovementRule( BLOB_MOVE_TO_TARGET_LOCATION );
	}

	for( int i = batchSize ; i < m_Elements.Count() ; i++ )
	{
		CBlobElement *pElement = (CBlobElement *)m_Elements[i].Get();

		// Compute a point around my center
		vecDir.x = random->RandomFloat( -1, 1 );
		vecDir.y = random->RandomFloat( -1, 1 );
		vecDir.z = random->RandomFloat( -1, 1 );

		VectorNormalize( vecDir );

		pElement->SetTargetLocation( sphere2 + vecDir * flRadius );
		pElement->SetActiveMovementRule( BLOB_MOVE_TO_TARGET_LOCATION );
	}

}

//-----------------------------------------------------------------------------
// Get the index of the element to start processing with for this batch.
//-----------------------------------------------------------------------------
int CNPC_Blob::GetBatchStart()
{
	return m_iBatchStart;
}

//-----------------------------------------------------------------------------
// Get the index of the element to stop processing with for this batch.
//-----------------------------------------------------------------------------
int CNPC_Blob::GetBatchEnd()
{
	int batchDone = m_iBatchStart + ComputeBatchSize();
	batchDone = MIN( batchDone, m_Elements.Count() );

	return batchDone;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_Blob::ComputeBatchSize()
{
	int batchSize = m_Elements.Count() / ( 100 / blob_batchpercent.GetInt() );
	return batchSize;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void CNPC_Blob::AdvanceBatch()
{
	m_iBatchStart += ComputeBatchSize();

	if( m_iBatchStart >= m_Elements.Count() )
		m_iBatchStart = 0;
}

//-----------------------------------------------------------------------------
// Creates a new blob element from scratch and adds it to the blob
//-----------------------------------------------------------------------------
CBlobElement *CNPC_Blob::CreateNewElement()
{
	CBlobElement *pElement = static_cast<CBlobElement*>(CreateEntityByName( "blob_element" ));

	if( pElement != NULL )
	{
		pElement->SetOwnerEntity( this );
		pElement->SetSinePhase( fabs( sin(((float)m_iNumElements)/10.0f) ) );
		pElement->SetActiveMovementRule( BLOB_MOVE_TO_TARGET_ENTITY );
		pElement->SetTargetEntity( this );

		pElement->m_iElementNumber = m_iNumElements;
		m_iNumElements++;
		pElement->Spawn();
		m_Elements.Set(m_iNumElements - 1, pElement);
		return pElement;
	}

	Warning("Blob could not spawn new element!\n");
	return NULL;
}

//-----------------------------------------------------------------------------
// Create, initialize, and distribute all blob elements
//-----------------------------------------------------------------------------
void CNPC_Blob::InitializeElements()
{
	// Squirt all of the elements out into a circle
	int i;
	QAngle angDistributor( 0, 0, 0 );

	int iNumElements = BLOB_NUM_ELEMENTS;

	float step = 360.0f / ((float)iNumElements);
	for( i = 0 ; i < iNumElements ; i++ )
	{
		Vector vecDir;
		Vector vecDest;
		AngleVectors( angDistributor, &vecDir, NULL, NULL );
		vecDest = WorldSpaceCenter() + vecDir * m_flRadius;

		CBlobElement *pElement = CreateNewElement();

		if( !pElement )
		{
			Msg("Blob could not create all elements!!\n");
			return;
		}

		trace_t tr;
		UTIL_TraceHull(vecDest, vecDest + Vector(0, 0, MIN_COORD_FLOAT), Vector(-1, -1, -1), Vector(1, 1, 1), MASK_BLOB_SOLID, pElement, HL2COLLISION_GROUP_BLOB, &tr);

		pElement->SetAbsOrigin( tr.endpos + Vector( 0, 0, 1 ) );

		angDistributor.y += step;
	}

	CBaseEntity *pEntity = gEntList.FindEntityByClassname( NULL, "info_target" );
	for( i = 0 ; i < BLOB_MAX_AVOID_ORIGINS ; i++ )
	{
		if( pEntity )
		{
			if( pEntity->NameMatches("avoid") )
			{
				m_vecAvoidOrigin[ i ] = pEntity->GetAbsOrigin();
				m_flAvoidRadiusSqr = Square( 120.0f );
				m_iNumAvoidOrigins++;
			}

			pEntity = gEntList.FindEntityByClassname( pEntity, "info_target" );
		}
		else
		{
			break;
		}
	}

	Msg("%d avoid origins\n", m_iNumAvoidOrigins );

	RecomputeIdealElementDist();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Blob::RecomputeIdealElementDist()
{
	float area = M_PI * Square(m_flRadius);

	//Msg("Area of blob is: %f\n", area );

	//m_flMinElementDist =  2.75f * sqrt( area / m_iNumElements );
	m_flMinElementDist =  M_PI * sqrt( area / m_iNumElements );

	//Msg("New element dist: %f\n", m_flMinElementDist );
}

Vector CNPC_Blob::BodyTarget(bool bNoisy)
{
	return m_vecCentroid;
}

float CNPC_Blob::GetSequenceGroundSpeed(CStudioHdr *pStudioHdr, int iSequence)
{
	return blob_element_speed.GetFloat();
}
#else //#ifndef CLIENT_DLL
#include "debugoverlay_shared.h"
#include "c_ai_basenpc.h"
#include "fmtstr.h"
#include "filesystem.h"
#include "toolframework/ienginetool.h"

// offsets from the minimal corner to other corners
static Vector cornerOffsets[8]
{
	Vector(0, 0, 0),
	Vector(1, 0, 0),
	Vector(1, 1, 0),
	Vector(0, 1, 0),
	Vector(0, 0, 1),
	Vector(1, 0, 1),
	Vector(1, 1, 1),
	Vector(0, 1, 1)
};

// offsets from the minimal corner to 2 ends of the edges
static Vector edgeVertexOffsets[12 * 2]
{
		Vector(0, 0, 0), Vector(1, 0, 0),//X 0
		Vector(1, 0, 0), Vector(1, 1, 0),//Y 1
		Vector(0, 1, 0), Vector(1, 1, 0),//X 2
		Vector(0, 0, 0), Vector(0, 1, 0),//Y 3
		Vector(0, 0, 1), Vector(1, 0, 1),//X 4
		Vector(1, 0, 1), Vector(1, 1, 1),//Y 5
		Vector(0, 1, 1), Vector(1, 1, 1),//X 6
		Vector(0, 0, 1), Vector(0, 1, 1),//Y 7
		Vector(0, 0, 0), Vector(0, 0, 1),//Z 8
		Vector(1, 0, 0), Vector(1, 0, 1),//Z 9
		Vector(1, 1, 0), Vector(1, 1, 1),//Z 10
		Vector(0, 1, 0), Vector(0, 1, 1) //Z 11
};

//edgeCase to interpCache
//1st int: direction we're moving back in. -1 means this edge cannot be used
//2nd int: edge to look at in cube we've moved back to, inside cache
static int ecTOic[12 * 2]
{
		2, 4,//0
		2, 5,//1
		2, 6,//2
		2, 7,//3
		1, 6,//4
		-1, -1,//5
		-1, -1,//6
		0, 5,//7
		1, 11,//8
		1, 10,//9
		-1, -1,//10
		0, 10 //11
};

//alternate table because some edges could reference 1 extra cube (and 1 more after that but there's no need)
static int ecTOicAlt[12 * 2]
{
		1, 2,//0
		-1, -1,//1
		-1, -1,//2
		0, 1,//3
		-1, -1,//4
		-1, -1,//5
		-1, -1,//6
		-1, -1,//7
		0, 9,//8
		-1, -1,//9
		-1, -1,//10
		-1, -1 //11
};

// list of triangles/vertices for every possible case
// up to 15 vertices per case and -1 indicates end of sequence
static int triangleTable[256 * 16]
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 ,
	 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 ,
	 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 ,
	 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 ,
	 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 ,
	 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 ,
	 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 ,
	 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 ,
	 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 ,
	 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 ,
	 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 ,
	 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 ,
	 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 ,
	 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 ,
	 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 ,
	 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 ,
	 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 ,
	 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 ,
	 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 ,
	 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 ,
	 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 ,
	 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 ,
	 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 ,
	 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 ,
	 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 ,
	 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 ,
	 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 ,
	 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 ,
	 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 ,
	 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 ,
	 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 ,
	 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 ,
	 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 ,
	 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 ,
	 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 ,
	 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 ,
	 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 ,
	 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 ,
	 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 ,
	 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 ,
	 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 ,
	 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 ,
	 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 ,
	 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 ,
	 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 ,
	 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 ,
	 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 ,
	 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 ,
	 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 ,
	 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 ,
	 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 ,
	 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 ,
	 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 ,
	 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 ,
	 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 ,
	 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 ,
	 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 ,
	 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 ,
	 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 ,
	 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 ,
	 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 ,
	 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 ,
	 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 ,
	 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 ,
	 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 ,
	 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 ,
	 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 ,
	 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 ,
	 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 ,
	 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 ,
	 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 ,
	 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 ,
	 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 ,
	 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 ,
	 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 ,
	 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 ,
	 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 ,
	 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 ,
	 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 ,
	 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 ,
	 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 ,
	 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 ,
	 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 ,
	 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 ,
	 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 ,
	 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 ,
	 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 ,
	 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 ,
	 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 ,
	 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 ,
	 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 ,
	 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 ,
	 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 ,
	 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 ,
	 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 ,
	 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 ,
	 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 ,
	 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 ,
	 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 ,
	 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 ,
	 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 ,
	 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 ,
	 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 ,
	 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 ,
	 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 ,
	 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 ,
	 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 ,
	 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 ,
	 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 ,
	 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 ,
	 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 ,
	 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 ,
	 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 ,
	 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 ,
	 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 ,
	 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 
};
#define MAX_SAMPLES_PER_AXIS 64
#define MAX_SAMPLES_PER_AXIS_SQUARED	MAX_SAMPLES_PER_AXIS * MAX_SAMPLES_PER_AXIS
#define MAX_SAMPLES_PER_AXIS_CUBED		MAX_SAMPLES_PER_AXIS * MAX_SAMPLES_PER_AXIS * MAX_SAMPLES_PER_AXIS
#define MIN_SAMPLE_INTERVAL 5
struct Sample
{
	float dif;
	Vector normal;
};
struct BlobVert
{
	Vector pos, normal;
};
struct CubemapSample_t
{
	Vector origin;
	CTextureReference texture;
	int resolution;

	// HACKHACK: CUtlVector is confused without this
	void operator=(const CubemapSample_t ref)
	{
		origin = ref.origin;
		texture = *const_cast<CTextureReference*>(&ref.texture);
		resolution = resolution;
	}
};
struct LightingState_t
{
	Vector         ambient[6];
	int            light_count;
	dworldlight_t* lights[4];
};
typedef ITexture* (*FuncGetLightingState)(Vector*, LightingState_t*, int*, int, bool);
FuncGetLightingState GetLightingState = nullptr;
class C_NPC_Blob : public C_AI_BaseNPC
{
	DECLARE_CLASS(C_NPC_Blob, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();
	int					m_iNumElements;
	EHANDLE				m_Elements[BLOB_NUM_ELEMENTS];
	virtual int			DrawModel(int flags);
	virtual void		GetRenderBounds( Vector& mins, Vector& maxs );
	void				SampleValue(Vector pos, bool bPrint, float* passdif, Vector* passnormal);
	void				MarchCube(Vector minCornerPos, int i, int j, int k);
	CMeshBuilder		meshBuilder;
	CUtlVector<BlobVert>	vertices;
	CUtlVector<int>		indices;
	Sample				sampleCache[MAX_SAMPLES_PER_AXIS_CUBED];
	Sample				interpCache[MAX_SAMPLES_PER_AXIS_CUBED][12];
	Vector				vecRMins;
	Vector				vecRMaxs;
	Vector				ClosestElementPos();
	void				EdgeInterp(Vector vert1, Vector vert2, bool bDebug, int i, int j, int k, int edgeCase, float* passdif, Vector* passnormal);
	float				m_flCubeWidth;
	void				Spawn();
	IMaterial*			m_pMaterial;
	//CUtlVector<CubemapSample_t> m_Cubemaps;
	//CUtlVector<int> m_SortedCubemapIndices;
	//int _cdecl			CubemapSort(const int *_a, const int *_b);
};
IMPLEMENT_CLIENTCLASS_DT(C_NPC_Blob, DT_NPC_Blob, CNPC_Blob)
RecvPropArray3(RECVINFO_ARRAY(m_Elements), RecvPropEHandle(RECVINFO(m_Elements[0]))),
RecvPropInt(RECVINFO(m_iNumElements)),
END_RECV_TABLE()
/*
int _cdecl C_NPC_Blob::CubemapSort(const int *_a, const int *_b)
{
	int
		a = *_a,
		b = *_b;
	const CubemapSample_t
		c1 = m_Cubemaps[a],
		c2 = m_Cubemaps[b];

	float distance1 = (c1.origin - GetAbsOrigin()).Length();
	float distance2 = (c2.origin - GetAbsOrigin()).Length();
	return int(distance1 - distance2);
}
*/
ConVar blob_wireframe("blob_wireframe", "0");
void C_NPC_Blob::Spawn()
{
	if (blob_wireframe.GetBool())
	{
		m_pMaterial = materials->FindMaterial("shadertest/wireframevertexcolor", TEXTURE_GROUP_OTHER);
	}
	else
	{
		m_pMaterial = materials->FindMaterial("blobs/blob_black_surf", TEXTURE_GROUP_MODEL);
	}
	/*
	dheader_t hdr;
	lump_t &cubemapLump = hdr.lumps[LUMP_CUBEMAPS];
	const char *szLevelName = modelinfo->GetModelName(modelinfo->GetModel(1));
	bool bHDR = g_pMaterialSystemHardwareConfig->GetHDREnabled();
	if (cubemapLump.filelen == 0)
	{
		// TODO: add a single default cubemap here as fallback?
	}
	else
	{
		FileHandle_t hFile = NULL;
		g_pFullFileSystem->Seek(hFile, cubemapLump.fileofs, FILESYSTEM_SEEK_HEAD);
		int count = cubemapLump.filelen / sizeof(dcubemapsample_t);
		m_Cubemaps.SetCount(count);

		dcubemapsample_t *cubemaps = static_cast<dcubemapsample_t *>(calloc(count, sizeof(dcubemapsample_t)));

		g_pFullFileSystem->Read(cubemaps, cubemapLump.filelen, hFile);

		int i = 0;
		for (auto &cubemap : m_Cubemaps)
		{
			dcubemapsample_t &in = cubemaps[i];
			cubemap.origin = Vector(in.origin[0], in.origin[1], in.origin[2]);
			if (bHDR)
				cubemap.texture.Init(materials->FindTexture(CFmtStr("%s/c%d_%d_%d.hdr", szLevelName, in.origin[0], in.origin[1], in.origin[2]), TEXTURE_GROUP_CUBE_MAP));
			else
				cubemap.texture.Init(materials->FindTexture(CFmtStr("%s/c%d_%d_%d", szLevelName, in.origin[0], in.origin[1], in.origin[2]), TEXTURE_GROUP_CUBE_MAP));
			cubemap.resolution = (in.size != 0) ? 1 << (in.size - 1) : 32;

			//if (cubemap.texture->IsError())
			//	Warning("FAILED TO LOAD CUBEMAP %s/c%d_%d_%d\n", szLevelName, in.origin[0], in.origin[1], in.origin[2]);

			m_SortedCubemapIndices.AddToTail(i);
			i++;
		}
	}
	*/
	char* GetLightingConditions = *(*(char***)enginetools + 77);
	GetLightingState = (FuncGetLightingState)((GetLightingConditions + 0x20) + *(int*)(GetLightingConditions + 0x21) + 5);
}
ConVar blob_show_rbox("blob_show_rbox", "0");
ConVar blob_debug_sample("blob_debug_sample", "-1");
int C_NPC_Blob::DrawModel(int flags)
{
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->MatrixMode(MATERIAL_MODEL);
	pRenderContext->Bind(m_pMaterial);
	//LightDesc_t spotLight(Vector(0, 0, 0), WorldGetLightForPoint(GetAbsOrigin(), false));
	//pRenderContext->SetLight(0, spotLight);
	//pRenderContext->SetAmbientLight(0.04, 0.04, 0.04);
	IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
	Vector vOrigin = GetAbsOrigin();
	if ((UTIL_PlayerByIndex(1)->GetAbsOrigin() - vOrigin).Length() > 1500)
		return 0;//far away
	if (m_iNumElements <= 0)
		return 0;
	
	m_flCubeWidth = MIN_SAMPLE_INTERVAL;
	//int iMarches = 0;
	//int iSamples = 0;
	int iXBound = vecRMaxs.x - vecRMins.x;
	int iYBound = vecRMaxs.y - vecRMins.y;
	int iZBound = vecRMaxs.z - vecRMins.z;
	int iXSamples = iXBound / m_flCubeWidth + 1;
	int iYSamples = iYBound / m_flCubeWidth + 1;
	int iZSamples = iZBound / m_flCubeWidth + 1;
	//use coarser grid if getting too big
	///*
	if (iXSamples > MAX_SAMPLES_PER_AXIS)
	{
		m_flCubeWidth = max(m_flCubeWidth, iXBound / MAX_SAMPLES_PER_AXIS);
		iXSamples = MAX_SAMPLES_PER_AXIS;
	}
	if (iYSamples > MAX_SAMPLES_PER_AXIS)
	{
		m_flCubeWidth = max(m_flCubeWidth, iYBound / MAX_SAMPLES_PER_AXIS);
		iYSamples = MAX_SAMPLES_PER_AXIS;
	}
	if (iZSamples > MAX_SAMPLES_PER_AXIS)
	{
		m_flCubeWidth = max(m_flCubeWidth, iZBound / MAX_SAMPLES_PER_AXIS);
		iZSamples = MAX_SAMPLES_PER_AXIS;
	}
	//*/
	///*
	//precompute samples so we don't have so much redundancy
	float x = vecRMins.x;
	//Vector iStart, iEnd;
	for (int i = 0; i < iXSamples; i++)
	{
		x = vecRMins.x + (m_flCubeWidth * i);
		float y = vecRMins.y;
		//Vector jStart, jEnd;
		for (int j = 0; j < iYSamples; j++)
		{
			y = vecRMins.y + (m_flCubeWidth * j);
			float z = vecRMins.z;
			//Vector kStart, kEnd;
			for (int k = 0; k < iZSamples; k++)
			{
				z = vecRMins.z + (m_flCubeWidth * k);
				Sample sample;
				float dif;
				Vector normal;
				SampleValue(vOrigin + Vector(x, y, z), /*blob_debug_sample.GetInt() == i + j * MAX_SAMPLES_PER_AXIS + k * MAX_SAMPLES_PER_AXIS_SQUARED*/ false, &dif, &normal);
				sample.dif = dif;
				sample.normal = normal;
				//iSamples++;
				//Msg("SampleValue to [%i][%i][%i], sample %f\n", i, j, k, sample);
				//Assert(sample != 1);
				sampleCache[i + j * MAX_SAMPLES_PER_AXIS + k * MAX_SAMPLES_PER_AXIS_SQUARED] = sample;
				//NDebugOverlay::Line(kStart, kEnd, 255, 255, 0, true, -1);
			}
			//NDebugOverlay::Line(jStart, jEnd, 255, 255, 0, true, -1);
		}
		//NDebugOverlay::Line(iStart, iEnd, 255, 255, 0, true, -1);
	}
	//*/
	//iterate on every voxel in our bbox/rbox
	//must find number of verts and indices before building mesh
	x = vecRMins.x;
	for (int i = 0; i < iXSamples; i++)
	{
		x = vecRMins.x + (m_flCubeWidth * i);
		float y = vecRMins.y;
		for (int j = 0; j < iYSamples; j++)
		{
			y = vecRMins.y + (m_flCubeWidth * j);
			float z = vecRMins.z;
			for (int k = 0; k < iZSamples; k++)
			{
				z = vecRMins.z + (m_flCubeWidth * k);
				MarchCube(vOrigin + Vector(x, y, z), i, j, k);
				//iMarches++;
				//Msg("MarchCube at %f %f %f\n", x, y, z);
			}
		}
	}

	//Msg("iMarches: %i iSamples: %i Verts: %i\n", iMarches, iSamples, vertices.Count());
	if (vertices.Count() <= 0)
		return 0;

	LightingState_t state;
	int dummy;
	ITexture* pCubemap = GetLightingState(&vOrigin, &state, &dummy, 7, false);
	pRenderContext->BindLocalCubemap(pCubemap);

	meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, vertices.Count(), indices.Count());
	//NDebugOverlay::Line(GetAbsOrigin(), vertices[0], 255, 255, 0, true, 0.1);
	//now build mesh
	for (int iVert = 0; iVert < vertices.Count(); iVert++)
	{
		//int iVertInTri = iVert % 3;
		/*
		if (iVertInTri == 0)
		{
			Vector vecPoint2 = vertices[iVert + 1]->pos;
			if (!vecPoint2.IsValid())
			{
				Msg("Vert %i, Vert %i was invalid, could not form a complete tri\n", iVert, iVert + 1);
				continue;
			}
			Vector vecA = vecPoint2 - vertices[iVert]->pos;

			Vector vecPoint3 = vertices[iVert + 2]->pos;
			if (!vecPoint3.IsValid())
			{
				Msg("Vert %i, Vert %i was invalid, could not form a complete tri\n", iVert, iVert + 2);
				continue;
			}
			Vector vecB = vecPoint3 - vertices[iVert]->pos;

			CrossProduct(vecA, vecB, vecNorm);
			vecNorm.NormalizeInPlace();
			vecNorm = -vecNorm;
			Assert(vecNorm.IsValid());
		}
		*/
		//DevMsg("Vert %i at %f %f %f\n", iVert, vertices[iVert].x, vertices[iVert].y, vertices[iVert].z);
		BlobVert bvert = vertices[iVert];
		meshBuilder.Position3fv(bvert.pos.Base());
		//if (iVertInTri == 0)
		//	meshBuilder.TexCoord2f(0, 0, 0);
		//else if (iVertInTri == 1)
		//	meshBuilder.TexCoord2f(0, 0, 1);
		//else
		//	meshBuilder.TexCoord2f(0, 1, 0);
		Vector vecNorm = bvert.normal;
		vecNorm = -vecNorm.Normalized();
		meshBuilder.Normal3fv(vecNorm.Base());
		if (blob_wireframe.GetBool())
			meshBuilder.Color4ub(vecNorm.x * 255, vecNorm.y * 255, vecNorm.z * 255, 100);
		else
		{
			/*
			Vector vLight;// = WorldGetLightForPoint(bvert.pos, false);
			engine->ComputeLighting(bvert.pos, &vecNorm, true, vLight, NULL);
			//if (g_pMaterialSystemHardwareConfig->GetHDREnabled())
			//vLight *= 2;
			vLight[0] = clamp(vLight[0], 0, 1);
			vLight[1] = clamp(vLight[1], 0, 1);
			vLight[2] = clamp(vLight[2], 0, 1);
			meshBuilder.Color3f(vLight[0], vLight[1], vLight[2]);
			*/
		}
		//Msg("vecNorm %f %f %f\n", vecNorm.x, vecNorm.y, vecNorm.z);
		meshBuilder.AdvanceVertex();
	}
	for (int iInd = 0; iInd < indices.Count(); iInd += 3)
	{
		meshBuilder.FastIndex(indices[iInd + 0]);
		meshBuilder.FastIndex(indices[iInd + 1]);
		meshBuilder.FastIndex(indices[iInd + 2]);
	}
	meshBuilder.End();
	pMesh->Draw();
	vertices.RemoveAll();
	indices.RemoveAll();
	//sampleCache.RemoveAll();
	///*
	for (int i = 0; i < MAX_SAMPLES_PER_AXIS; i++)
	{
		for (int j = 0; j < MAX_SAMPLES_PER_AXIS; j++)
		{
			for (int k = 0; k < MAX_SAMPLES_PER_AXIS; k++)
			{
				sampleCache[i + j * MAX_SAMPLES_PER_AXIS + k * MAX_SAMPLES_PER_AXIS_SQUARED].dif = 0;
			}
		}
	}
	//*/
	return 1;
}
ConVar blob_element_radius("blob_element_radius", "22");//radius
ConVar blob_rbox_padtoradius("blob_rbox_padtoradius", "36");//we should pretend this is the radius for the purpose of expanding the render box, because when balls overlap, they become larger
void C_NPC_Blob::GetRenderBounds(Vector& theMins, Vector& theMaxs)
{
	//render bounds should encompass all our elements
	Vector mins = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector maxs = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int i = 0; i < m_iNumElements; i++)
	{
		C_BaseEntity* pEle = m_Elements[i].Get();
		if (pEle)
		{
			Vector vecElemPos = pEle->GetAbsOrigin();
			if (mins.x > vecElemPos.x) mins.x = vecElemPos.x;
			if (mins.y > vecElemPos.y) mins.y = vecElemPos.y;
			if (mins.z > vecElemPos.z) mins.z = vecElemPos.z;
			if (maxs.x < vecElemPos.x) maxs.x = vecElemPos.x;
			if (maxs.y < vecElemPos.y) maxs.y = vecElemPos.y;
			if (maxs.z < vecElemPos.z) maxs.z = vecElemPos.z;
		}
	}
	float flPad = blob_rbox_padtoradius.GetFloat() / 2;
	theMins = vecRMins = mins - Vector(flPad, flPad, flPad) - GetAbsOrigin();
	theMaxs = vecRMaxs = maxs + Vector(flPad, flPad, flPad) - GetAbsOrigin();
	Assert(theMins.IsValid() && theMaxs.IsValid());
}
ConVar blob_metaball("blob_metaball", "1");//0 = simple ball sampling
ConVar blob_isomode("blob_isomode", "0");//1 = simple ball marching cubes
void C_NPC_Blob::SampleValue(Vector pos, bool bPrint, float *passdif, Vector *passnormal)
{
	//if (bPrint) NDebugOverlay::Line(pos, pos - Vector(1, 1, 1), 0, 255, 0, true, -1);
	float dif = 0;
	Vector normal = vec3_origin;
	if (blob_metaball.GetBool())
	{
		//metaball
		for (int i = 0; i < m_iNumElements; i++)
		{
			if (m_Elements[i].Get())
			{
				Vector vDelta = (m_Elements[i].Get()->GetAbsOrigin() - pos);
				float flDistance = vDelta.Length();
				float flRadius = blob_element_radius.GetFloat();
				if (flDistance > flRadius)//sample point was right on an element
					continue;
				//if (bPrint) NDebugOverlay::Line(pos, pEle->GetAbsOrigin(), 0, 255, 0, true, -1);
				if (flDistance == 0)
				{
					*passdif = 1;
					*passnormal = vec3_origin;
					return;
				}
				//if (bPrint) Msg("flValue (%f) += (%f - %f) / %f = %f\n", dif, flRadius, flDistance, flRadius, (flRadius - flDistance) / flRadius);
				//https://github.com/Owlrazum/HeresyPackages/blob/66557172d8e08ccb36a47c2021c68eaf8884a8f3/Project/Assets/Heresy/MarchingCubes/Source/TestMarchingCubes.cs#L219
				dif += (flRadius - flDistance) / flRadius;
				//Msg("delta %0.1f %0.1f %0.1f\n", vDelta.x, vDelta.y, vDelta.z);
				Vector vecOnSurface = vDelta.Normalized() * flRadius;
				normal += (vecOnSurface - vDelta) / flRadius;
			}
		}
		Assert(IsFinite(dif));
		Assert(normal.IsValid());
		*passdif = clamp(dif, 0, 1);
		*passnormal = normal.Normalized();
		/*
		if (bPrint)
		{
			int color = 255 * flValue;
			NDebugOverlay::Line(pos, pos - Vector(0, 0, 1), color, color, color, true, -1);
		}
		*/
		return;
	}
	else
	{
		//simple ball, inside out
		Vector vecClosest = vec3_origin;
		for (int i = 0; i < m_iNumElements; i++)
		{
			Vector vDelta = (m_Elements[i].Get()->GetAbsOrigin() - pos);
			if (i == 0)
			{
				vecClosest = vDelta;
				normal = vDelta;
			}
			else if (vDelta.Length() < vecClosest.Length())
			{
				vecClosest = vDelta;
				normal = vDelta;
			}
		}
		*passdif = vecClosest.Length() - blob_element_radius.GetFloat();
		*passnormal = normal.Normalized();
		return;
	}
}
ConVar blob_isolevel("blob_isolevel", "0.5");
ConVar blob_interpfactor("blob_interpfactor", "1");
ConVar blob_interp_debug_min("blob_interp_debug_min", "0");//when blob_case_debug, filters lines to be above this dif
ConVar blob_interp_debug_max("blob_interp_debug_max", "1");//when blob_case_debug, filters lines to be below this dif
void C_NPC_Blob::EdgeInterp(Vector vert1, Vector vert2, bool bDebug, int i, int j, int k, int edgeCase, float *passdif, Vector *passnormal)
{
	Vector vecOffset1 = edgeVertexOffsets[edgeCase * 2];
	int i1 = i + (int)vecOffset1.x;
	int j1 = (j + (int)vecOffset1.y) * MAX_SAMPLES_PER_AXIS;
	int k1 = (k + (int)vecOffset1.z) * MAX_SAMPLES_PER_AXIS_SQUARED;
	Sample s1 = sampleCache[i1 + j1 + k1];
	Vector vecOffset2 = edgeVertexOffsets[edgeCase * 2 + 1];
	int i2 = i + (int)vecOffset2.x;
	int j2 = (j + (int)vecOffset2.y) * MAX_SAMPLES_PER_AXIS;
	int k2 = (k + (int)vecOffset2.z) * MAX_SAMPLES_PER_AXIS_SQUARED;
	Sample s2 = sampleCache[i2 + j2 + k2];

	// interpolate along the edge
	//s1 = SampleValue(vert1, bDebug);
	//s2 = SampleValue(vert2, bDebug);
	float dif;
	Vector normal;
	if (blob_isomode.GetBool())
	{
		//simple balls
		dif = s1.dif - s2.dif;
		if (dif == 0.0f)
			dif = 0.5f;
		else
			dif = s1.dif / dif;
		normal = s1.normal - s2.normal;
		if (dif == 0.0f)
			normal = s1.normal;
		else
			normal = s1.normal / dif;
		//Msg("s1 %f s2 %f dif %f\n", s1, s2, dif);
	}
	else
	{
		//metaballs https://github.com/Owlrazum/HeresyPackages/blob/66557172d8e08ccb36a47c2021c68eaf8884a8f3/Project/Assets/Heresy/MarchingCubes/Source/MarchingCubes.cs#L118
		float flLevel = blob_isolevel.GetFloat();
		dif = (flLevel - s1.dif) / (s2.dif - s1.dif);
		//dif = clamp(dif * blob_interpfactor.GetFloat(), 0, 1);
		float flMinDif = min(s2.dif, s1.dif);
		float flMaxDif = max(s1.dif, s2.dif);
		normal.x = (flLevel - max(s1.normal.x, s2.normal.x)) / (flMinDif - flMaxDif);
		normal.y = (flLevel - max(s1.normal.y, s2.normal.y)) / (flMinDif - flMaxDif);
		normal.z = (flLevel - max(s1.normal.z, s2.normal.z)) / (flMinDif - flMaxDif);
	}
	if (bDebug && dif >= blob_interp_debug_min.GetFloat() && dif <= blob_interp_debug_max.GetFloat())
	{
		Vector vDelta = vert1 - vert2;
		bool bInvalid = dif == 0 || dif == 1;//1.0 is not invalid. this means that both samples are symmetric on the sphere. at the moment i'm not sure if the code handles 1.0 properly though so i'll leave this here until after i fix a bunch of other stuff
		NDebugOverlay::Line(vert1, vert2, 255, bInvalid ? 0 : 255, 0, true, -1);
		NDebugOverlay::Line(vert1, vert1 + Vector(.3, .3, .3), 255, bInvalid ? 0 : 255, 0, true, -1);
		if (bInvalid)
			Warning("s1 %f s2 %f dif %f vert1 %0.1f %0.1f %0.1f vert2 %0.1f %0.1f %0.1f delta %0.1f %0.1f %0.1f\n", s1, s2, dif, vert1.x, vert1.y, vert1.z, vert2.x, vert2.y, vert2.z, vDelta.x, vDelta.y, vDelta.z);
		else
			Msg("s1 %f s2 %f dif %f vert1 %0.1f %0.1f %0.1f vert2 %0.1f %0.1f %0.1f delta %0.1f %0.1f %0.1f\n", s1, s2, dif, vert1.x, vert1.y, vert1.z, vert2.x, vert2.y, vert2.z, vDelta.x, vDelta.y, vDelta.z);
	}
	Assert(IsFinite(dif));
	*passdif = dif;
	*passnormal = normal;
}
ConVar blob_case_debug("blob_case_debug", "-1");//show edges of a cube that are being interp'd on
ConVar blob_case_override("blob_case_override", "-1");//make all cubes display one particular case. does not work right with blob_interp 1
ConVar blob_threshold("blob_threshold", "0.5");
ConVar blob_interp("blob_interp", "1");//do interpolation
ConVar blob_spew_cases("blob_spew_cases", "0");
void C_NPC_Blob::MarchCube(Vector minCornerPos, int i, int j, int k)
{
	//https://gist.github.com/metalisai/a3cdc214023f8c92b1f0bf27e7cc08d1
	// construct case index from 8 corner samples
	int caseIndex = 0;
	int iNumCases = 0;
	for (int l = 0; l < 8; l++)
	{
		//Msg("MarchCube to [%i][%i][%i] i %i j %i k %i offset %i %i %i\n", i + (int)cornerOffsets[l].x, j + (int)cornerOffsets[l].y, k + (int)cornerOffsets[l].z, i, j, k, (int)cornerOffsets[l].x, (int)cornerOffsets[l].y, (int)cornerOffsets[l].z);
		Vector vecOffset = cornerOffsets[l];
		float sample = sampleCache[i + (int)vecOffset.x + (j + (int)vecOffset.y) * MAX_SAMPLES_PER_AXIS + (k + (int)vecOffset.z) * MAX_SAMPLES_PER_AXIS_SQUARED].dif;
		//float sample = SampleValue((minCornerPos + cornerOffsets[l] * m_flCubeWidth), false);
		if (blob_metaball.GetBool())
		{
			//metaballs
			if (sample >= blob_threshold.GetFloat())
			{
				caseIndex |= 1 << l;
				iNumCases++;
			}
			/*
			if (max(sample, blob_threshold.GetFloat()) - min(sample, blob_threshold.GetFloat()) < 0.2)//range check to avoid drawing too many lines and crashing
			{
				//int color = 255 * (sample / blob_threshold.GetFloat());
				int color = 255 * sample;
				NDebugOverlay::Line((minCornerPos + cornerOffsets[l] * m_flCubeWidth), (minCornerPos + cornerOffsets[l] * m_flCubeWidth) - Vector(0, 0, 0.3), color, color, color, true, -1);
			}
			*/
		}
		else
		{
			//simple balls
			if (sample >= 0)
			{
				caseIndex |= 1 << l;
				iNumCases++;
			}
			/*
			//int color =  255 * (sample / blob_threshold.GetFloat());
			int color = 255 * sample;
			NDebugOverlay::Line((minCornerPos + cornerOffsets[l] * m_flCubeWidth), (minCornerPos + cornerOffsets[l] * m_flCubeWidth) - Vector(0, 0, 0.3), color, color, color, true, -1);
			*/
		}
	}

	// early out if entirely inside or outside the volume
	if (caseIndex == 0 || caseIndex == 0xFF)
		return;
	if (blob_spew_cases.GetBool()) Msg("case: %i (num: %i)\n", caseIndex, iNumCases);
	bool bDebug = blob_case_debug.GetInt() == caseIndex;
	if (blob_case_override.GetInt() != -1)
	{
		caseIndex = blob_case_override.GetInt();
	}
	int caseVert = 0;
	for (int l = 0; l < 5; l++)
	{
		for (int tri = 0; tri < 3; tri++)
		{
			// get edge index
			int edgeCase = triangleTable[caseIndex * 16 + caseVert];
			if (edgeCase == -1)
				return;
			Sample sample;
			float dif;
			Vector normal;
			if (bDebug) Msg("%i + %i * 256 = %i (%i)\n", caseIndex, caseVert, caseIndex + (caseVert * 256), edgeCase);

			Vector vert1 = minCornerPos + (edgeVertexOffsets[edgeCase * 2] * m_flCubeWidth); // beginning of the edge
			Vector vert2 = minCornerPos + (edgeVertexOffsets[edgeCase * 2 + 1] * m_flCubeWidth); // end of the edge

			Vector vertPosInterpolated;
			if (blob_interp.GetBool() && blob_case_override.GetInt() == -1)
			{
				//float dif = EdgeInterp(vert1, vert2, bDebug, i, j, k, edgeCase);
				///*
				bool bGotResult = false;
				//int oldI = i;
				//int oldJ = j;
				//int oldK = k;
				int* vals[3] = { &i, &j, &k };
				if (ecTOic[edgeCase * 2] != -1)
				{
					*vals[ecTOic[edgeCase * 2]] -= 1;
					if (*vals[ecTOic[edgeCase * 2]] > -1)
					{
						sample = interpCache[i + j * MAX_SAMPLES_PER_AXIS + k * MAX_SAMPLES_PER_AXIS_SQUARED][ecTOic[edgeCase * 2 + 1]];
						bGotResult = true;
						//Msg("Copying interp value %f from cube %i %i %i edge %i to cube %i %i %i edge %i vert1 %0.1f %0.1f %0.1f vert2 %0.1f %0.1f %0.1f\n", dif, i, j, k, ecTOic[edgeCase][1], oldI, oldJ, oldK, edgeCase, vert1.x, vert1.y, vert1.z, vert2.x, vert2.y, vert2.z);
					}
					*vals[ecTOic[edgeCase * 2]] += 1;
				}

				//try other direction
				if (!bGotResult && (edgeCase == 0 || edgeCase == 3 || edgeCase == 8) && ecTOicAlt[edgeCase * 2] != -1)
				{
					*vals[ecTOicAlt[edgeCase * 2]] -= 1;
					if (*vals[ecTOicAlt[edgeCase * 2]] > -1)
					{
						sample = interpCache[i + j * MAX_SAMPLES_PER_AXIS + k * MAX_SAMPLES_PER_AXIS_SQUARED][ecTOicAlt[edgeCase * 2 + 1]];
						bGotResult = true;
						//Msg("Copying interp value %f from cube %i %i %i edge %i to cube %i %i %i edge %i vert1 %0.1f %0.1f %0.1f vert2 %0.1f %0.1f %0.1f\n", dif, i, j, k, ecTOicAlt[edgeCase][1], oldI, oldJ, oldK, edgeCase, vert1.x, vert1.y, vert1.z, vert2.x, vert2.y, vert2.z);
					}
					*vals[ecTOicAlt[edgeCase * 2]] += 1;
				}

				if (!bGotResult)
				//*/
				{
					EdgeInterp(vert1, vert2, bDebug, i, j, k, edgeCase, &dif, &normal);
					sample.dif = dif;
					sample.normal = normal;
					//Msg("New interp value %f at cube %i %i %i edge %i vert1 %0.1f %0.1f %0.1f vert2 %0.1f %0.1f %0.1f\n", dif, i, j, k, edgeCase, vert1.x, vert1.y, vert1.z, vert2.x, vert2.y, vert2.z);
				}
				interpCache[i + j * MAX_SAMPLES_PER_AXIS + k * MAX_SAMPLES_PER_AXIS_SQUARED][edgeCase] = sample;
				
				// Lerp
				//dif *= m_flCubeWidth;
				vertPosInterpolated = vert1 + ((vert2 - vert1) * sample.dif);
				Assert(vertPosInterpolated.IsValid());
			}
			else
			{
				//blocky
				vertPosInterpolated = (vert1 + vert2) / 2;
				Assert(vertPosInterpolated.IsValid());
				if (bDebug)
				{
					EdgeInterp(vert1, vert2, bDebug, i, j, k, edgeCase, &dif, &normal);//just to print debug info
				}
			}
			if (!vertPosInterpolated.IsValid())
				NDebugOverlay::Line(vert1, vert2, 255, 0, 0, true, -1);
			BlobVert* bvert = new BlobVert;
			bvert->pos = vertPosInterpolated;
			bvert->normal = sample.normal;
			vertices.AddToTail(*bvert);
			indices.AddToTail(vertices.Count() - 1);
			delete bvert;
			caseVert++;
		}
	}
}
#endif //#ifndef CLIENT_DLL