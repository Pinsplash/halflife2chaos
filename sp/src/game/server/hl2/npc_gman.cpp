//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The G-Man, misunderstood servant of the people
//
// $NoKeywords: $
//
//=============================================================================//


//-----------------------------------------------------------------------------
// Generic NPC - purely for scripted sequence work.
//-----------------------------------------------------------------------------
#include "cbase.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "ai_hull.h"
#include "ai_behavior_follow.h"
#include "ai_playerally.h"
#include "npc_playercompanion.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// NPC's Anim Events Go Here
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_GMan : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS(CNPC_GMan, CNPC_PlayerCompanion);
	DECLARE_DATADESC();

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	virtual Disposition_t IRelationType(CBaseEntity *pTarget);
	int		GetSoundInterests ( void );
	bool	CreateBehaviors( void );
	int		SelectSchedule( void );

private:
	//CAI_FollowBehavior	m_FollowBehavior;
};

LINK_ENTITY_TO_CLASS( npc_gman, CNPC_GMan );

BEGIN_DATADESC( CNPC_GMan )
// (auto saved by AI)
//	DEFINE_FIELD( m_FollowBehavior, FIELD_EMBEDDED ),	(auto saved by AI)
END_DATADESC()

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_GMan::Classify ( void )
{
	return CLASS_PLAYER_ALLY_VITAL;
}



//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_GMan::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_GMan::GetSoundInterests ( void )
{
	return NULL;
}


//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_GMan::Spawn()
{
	if (!m_bChaosSpawned)
		AddSpawnFlags(SF_NPC_NO_PLAYER_PUSHAWAY);
	Precache();

	BaseClass::Spawn();

	SetModel( "models/gman.mdl" );

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal(true);//in ep2, his bounding box is really weird and was causing issues. he couldn't move at the inn and floated in the outland 3 cutscene
	
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= 80;
	m_flFieldOfView		= 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState			= NPC_STATE_NONE;
	SetImpactEnergyScale( 0.0f ); // no physics damage on the gman
	
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD | bits_CAP_USE_WEAPONS | bits_CAP_WEAPON_RANGE_ATTACK1);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE | bits_CAP_SQUAD);
	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL );

	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_GMan::Precache()
{
	PrecacheModel( "models/gman.mdl" );
	
	BaseClass::Precache();
}	

//-----------------------------------------------------------------------------
// The G-Man isn't scared of anything.
//-----------------------------------------------------------------------------
Disposition_t CNPC_GMan::IRelationType(CBaseEntity *pTarget)
{
	return m_bChaosSpawned ? BaseClass::IRelationType(pTarget) : D_NU;
}


//=========================================================
// Purpose:
//=========================================================
bool CNPC_GMan::CreateBehaviors()
{
	//AddBehavior( &m_FollowBehavior );
	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_GMan::SelectSchedule( void )
{
	if ( !BehaviorSelectSchedule() )
	{
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------
