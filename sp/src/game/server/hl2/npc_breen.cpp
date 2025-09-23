//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dr. Breen, the oft maligned genius, heroically saving humanity from 
//			its own worst enemy, itself.
//=============================================================================//


//-----------------------------------------------------------------------------
// Generic NPC - purely for scripted sequence work.
//-----------------------------------------------------------------------------
#include	"cbase.h"
#include	"npcevent.h"
#include	"ai_basenpc.h"
#include	"ai_hull.h"
#include "ai_baseactor.h"
#include "npc_playercompanion.h"
#include "weapon_physcannon.h"
#include "collisionutils.h"
#include "ai_tacticalservices.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Spawnflags
#define SF_BREEN_BACKGROUND_TALK		( 1 << 16 )		// 65536 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_Breen : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS(CNPC_Breen, CNPC_PlayerCompanion);

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		GetSoundInterests ( void );
	bool	UseSemaphore( void );
	virtual int SelectSchedule(void);
	int		TranslateSchedule(int scheduleType);
	enum
	{
		SCHED_BREEN_GET_PROP_TO_LAUNCH = BaseClass::NEXT_SCHEDULE,
		SCHED_BREEN_RANGE_ATTACK1,
		SCHED_BREEN_ESTABLISH_LINE_OF_FIRE,
	};
	enum
	{
		TASK_BREEN_FIND_PROP = BaseClass::NEXT_TASK,
		TASK_BREEN_GO_TO_PROP,
		TASK_BREEN_PICK_UP_PROP,
	};
	enum
	{
		COND_BREEN_PROP_LOST = BaseClass::NEXT_CONDITION,
	};
	CBaseEntity* FindUsableProp();
	int GetAvailablePropsInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs);
	virtual void StartTask(const Task_t* pTask);
	virtual void RunTask(const Task_t* pTask);
	virtual void NPCThink(void);
	DEFINE_CUSTOM_AI;
};

LINK_ENTITY_TO_CLASS( npc_breen, CNPC_Breen );

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Breen::Classify ( void )
{
	return CLASS_NONE;
}

void CNPC_Breen::NPCThink(void)
{
	BaseClass::NPCThink();

	if (GetActiveWeapon())
	{
		CWeaponPhysCannon* pPhysCannon = dynamic_cast<CWeaponPhysCannon*>(GetActiveWeapon());
		if (pPhysCannon)
			pPhysCannon->ItemPreFrame();
	}
}

//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_Breen::HandleAnimEvent( animevent_t *pEvent )
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
int CNPC_Breen::GetSoundInterests ( void )
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Breen::Spawn()
{
	if (!m_bChaosSpawned)
		AddSpawnFlags(SF_NPC_NO_PLAYER_PUSHAWAY);
	// Breen is allowed to use multiple models, because he has a torso version for monitors.
	// He defaults to his normal model.
	char *szModel = (char *)STRING( GetModelName() );
	if (!szModel || !*szModel)
	{
		szModel = "models/breen.mdl";
		SetModelName( AllocPooledString(szModel) );
	}

	Precache();
	SetModel( szModel );

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	//SetSolid( SOLID_BBOX );
	//AddSolidFlags( FSOLID_NOT_STANDABLE );
	//SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= 80;
	m_flFieldOfView		= 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState			= NPC_STATE_NONE;
	
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD | bits_CAP_USE_WEAPONS);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE | bits_CAP_WEAPON_RANGE_ATTACK1);
	AddEFlags( EFL_NO_DISSOLVE );

	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_Breen::Precache()
{
	PrecacheModel( STRING( GetModelName() ) );
	BaseClass::Precache();
}	

bool CNPC_Breen::UseSemaphore( void )	
{ 
	if ( HasSpawnFlags( SF_BREEN_BACKGROUND_TALK ) )
		return false;

	return BaseClass::UseSemaphore();
}

int CNPC_Breen::SelectSchedule(void)
{
	if (HasCondition(COND_WEAPON_NOT_READY) && GetActiveWeapon())
	{
		CWeaponPhysCannon* pPhysCannon = dynamic_cast<CWeaponPhysCannon*>(GetActiveWeapon());
		if (pPhysCannon)
			return SCHED_BREEN_GET_PROP_TO_LAUNCH;
	}
	return BaseClass::SelectSchedule();
}

void CNPC_Breen::StartTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		CWeaponPhysCannon* pPhysCannon = dynamic_cast<CWeaponPhysCannon*>(GetActiveWeapon());
		if (pPhysCannon)
			pPhysCannon->PrimaryAttack();
		TaskComplete();
		break;
	}
	case TASK_BREEN_FIND_PROP:
	{
		CWeaponPhysCannon* pPhysCannon = dynamic_cast<CWeaponPhysCannon*>(GetActiveWeapon());
		if (pPhysCannon && !pPhysCannon->m_bActive)
		{
			SetTarget(FindUsableProp());
		}
		TaskComplete();
		break;
	}
	case TASK_BREEN_GO_TO_PROP:
	{
		CWeaponPhysCannon* pPhysCannon = dynamic_cast<CWeaponPhysCannon*>(GetActiveWeapon());
		if (pPhysCannon && !pPhysCannon->m_bActive)
		{
			CBaseEntity* pTarget = GetTarget();
			if (!pTarget || IsUnreachable(pTarget))
			{
				SetCondition(COND_BREEN_PROP_LOST);
				break;
			}
			Vector posLos;
			if (pPhysCannon && pTarget && GetTacticalServices()->FindLos(pTarget->GetAbsOrigin(), pTarget->GetAbsOrigin(), 0, pPhysCannon->TraceLength(), 1.0, FLANKTYPE_NONE, vec3_origin, 0, &posLos))
			{
				GetNavigator()->SetGoal(AI_NavGoal_t(posLos, ACT_RUN, AIN_HULL_TOLERANCE));
			}
		}
		TaskComplete();
		break;
	}
	default:
		BaseClass::StartTask(pTask);
	}
}

void CNPC_Breen::RunTask(const Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_BREEN_PICK_UP_PROP:
	{
		CWeaponPhysCannon* pPhysCannon = dynamic_cast<CWeaponPhysCannon*>(GetActiveWeapon());
		if (pPhysCannon && !pPhysCannon->m_bActive)
		{
			pPhysCannon->SecondaryAttack();
			if (pPhysCannon->m_bActive)
				TaskComplete();
			if (!GetTarget() || IsUnreachable(GetTarget()) || (GetAbsOrigin() - GetTarget()->GetAbsOrigin()).Length() > pPhysCannon->TraceLength())
			{
				SetCondition(COND_BREEN_PROP_LOST);
			}
		}
		TaskComplete();
		break;
	}
	default:
		BaseClass::RunTask(pTask);
	}
}

int CNPC_Breen::GetAvailablePropsInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs)
{
	// linear search all weapons
	int count = 0;
	CBaseEntity* pSearch = NULL;
	CWeaponPhysCannon* pPhysCannon = dynamic_cast<CWeaponPhysCannon*>(GetActiveWeapon());
	while (pPhysCannon && (pSearch = gEntList.NextEnt(pSearch)) != NULL)
	{
		// skip any held weapon
		if (pPhysCannon->CanPickupObject(pSearch))
		{
			// restrict to mins/maxs
			if (IsPointInBox(pSearch->GetAbsOrigin(), mins, maxs))
			{
				if (count < listMax)
				{
					pList[count] = pSearch;
					count++;
				}
			}
		}
	}

	return count;
}

//adapted from CBaseCombatCharacter::Weapon_FindUsable
CBaseEntity* CNPC_Breen::FindUsableProp()
{
	Vector range = Vector(540, 540, 100);

	CBaseEntity* propList[64];
	CBaseEntity* pBestProp = NULL;

	Vector mins = GetAbsOrigin() - range;
	Vector maxs = GetAbsOrigin() + range;
	int listCount = GetAvailablePropsInBox(propList, ARRAYSIZE(propList), mins, maxs);

	float fBestMass = 1e6;

	for (int i = 0; i < listCount; i++)
	{
		// Make sure not moving (ie flying through the air)
		Vector velocity;

		CBaseEntity* pProp = propList[i];
		Assert(pProp);
		pProp->GetVelocity(&velocity, NULL);

		if (IsUnreachable(pProp))
			continue;

		//TODO: Detect if prop is held by other gravity gun?
		//if (pProp->IsLocked(this))
			//continue;

		//Don't throw health or ammo
		if (pProp->ClassMatches("item*"))
			continue;

		//heuristic is now mass, not distance
		float fCurMass = pProp->VPhysicsGetObject()->GetMass();

		if (pBestProp)
		{
			if (fCurMass < fBestMass)
			{
				continue;
			}
		}

		//not sure if props will need "on ground" detection
		/*
		if (Weapon_IsOnGround(pProp))
		{
			// Weapon appears to be lying on the ground. Make sure this weapon is reachable
			// by tracing out a human sized hull just above the weapon.  If not, reject
			trace_t tr;

			Vector	vAboveWeapon = pWeapon->GetAbsOrigin();
			UTIL_TraceEntity(this, vAboveWeapon, vAboveWeapon + Vector(0, 0, 1), MASK_SOLID, pWeapon, COLLISION_GROUP_NONE, &tr);

			if (tr.startsolid || (tr.fraction < 1.0))
				continue;
		}
		*/

		fBestMass = fCurMass;
		pBestProp = pProp;
	}
	return pBestProp;
}

int CNPC_Breen::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_RANGE_ATTACK1:
		return SCHED_BREEN_RANGE_ATTACK1;
	case SCHED_ESTABLISH_LINE_OF_FIRE:
		return SCHED_BREEN_ESTABLISH_LINE_OF_FIRE;
	}
	return BaseClass::TranslateSchedule(scheduleType);
}


AI_BEGIN_CUSTOM_NPC(npc_breen, CNPC_Breen)
DECLARE_TASK(TASK_BREEN_FIND_PROP)
DECLARE_TASK(TASK_BREEN_GO_TO_PROP)
DECLARE_TASK(TASK_BREEN_PICK_UP_PROP)
DECLARE_CONDITION(COND_BREEN_PROP_LOST)
DEFINE_SCHEDULE
(
	SCHED_BREEN_GET_PROP_TO_LAUNCH,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_SET_TOLERANCE_DISTANCE		5"
	"		TASK_BREEN_FIND_PROP			0"
	"		TASK_BREEN_GO_TO_PROP			0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_STOP_MOVING				0"
	"		TASK_FACE_TARGET				0"
	"		TASK_BREEN_PICK_UP_PROP			0"
	"		TASK_WAIT				.1"
	""
	"	Interrupts"
	//"		COND_WEAPON_CHANGED"
	"		COND_BREEN_PROP_LOST"
)
DEFINE_SCHEDULE
(
	SCHED_BREEN_RANGE_ATTACK1,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_FACE_ENEMY			0"
	"		TASK_WAIT				.2"
	"		TASK_RANGE_ATTACK1		0"
	""
	"	Interrupts"
	"		COND_ENEMY_WENT_NULL"
	"		COND_HEAVY_DAMAGE"
	"		COND_ENEMY_OCCLUDED"
	"		COND_NO_PRIMARY_AMMO"
	"		COND_HEAR_DANGER"
	"		COND_WEAPON_BLOCKED_BY_FRIEND"
	"		COND_WEAPON_SIGHT_OCCLUDED"
)
DEFINE_SCHEDULE
(
	SCHED_BREEN_ESTABLISH_LINE_OF_FIRE,

	"	Tasks "
	//"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK"
	"		TASK_GET_PATH_TO_ENEMY_LOS		0"
	"		TASK_SPEAK_SENTENCE				1"
	"		TASK_RUN_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
	""
	"	Interrupts "
	"		COND_NEW_ENEMY"
	"		COND_ENEMY_DEAD"
	"		COND_LOST_ENEMY"
	"		COND_CAN_RANGE_ATTACK1"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_CAN_RANGE_ATTACK2"
	"		COND_CAN_MELEE_ATTACK2"
	"		COND_HEAR_DANGER"
);
AI_END_CUSTOM_NPC()