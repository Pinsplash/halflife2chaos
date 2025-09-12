//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "basebludgeonweapon.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "decals.h"

ConVar sk_harpoon_refire("sk_harpoon_refire", "0.8");
ConVar sk_plr_dmg_harpoon("sk_plr_dmg_harpoon", "60");
ConVar sk_npc_dmg_harpoon("sk_npc_dmg_harpoon", "100");
ConVar sk_npc_harpoon_range("sk_harpoon_range", "50");
ConVar sk_plr_harpoon_range("sk_harpoon_range", "100");
ConVar sk_harpoon_attack_delay_plr("sk_harpoon_attack_delay_plr", "0.5");
ConVar sk_harpoon_lead_time("sk_harpoon_lead_time", "0.9");
ConVar sk_harpoon_force_scale("sk_harpoon_force_scale", "0.3");
#define BLUDGEON_HULL_DIM		16
static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

//-----------------------------------------------------------------------------
// Purpose: Old Man Harpoon - Lost Coast.
//-----------------------------------------------------------------------------
class CWeaponOldManHarpoon : public CBaseHLBludgeonWeapon
{
	DECLARE_CLASS(CWeaponOldManHarpoon, CBaseHLBludgeonWeapon);
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();	
	DECLARE_ACTTABLE();
	CNetworkVar(float, m_flAttackTime);
	virtual int		CapabilitiesGet(void);
	float		GetDamageForActivity(Activity hitActivity);
	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	virtual float		GetFireRate(void)		{ return	sk_harpoon_refire.GetFloat(); }
	void		PrimaryAttack(void);
	void		SecondaryAttack(void)	{ return; }
	bool CanHolster(void);
	void ItemPostFrame(void);
	void HarpoonSwing();
	virtual int GetDamageType() { return DMG_SLASH | DMG_NEVERGIB; }
	virtual float	GetForceScale() { return sk_harpoon_force_scale.GetFloat(); }
};

IMPLEMENT_SERVERCLASS_ST( CWeaponOldManHarpoon, DT_WeaponOldManHarpoon )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponOldManHarpoon )
DEFINE_FIELD(m_flAttackTime, FIELD_TIME),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_oldmanharpoon, CWeaponOldManHarpoon );
PRECACHE_WEAPON_REGISTER( weapon_oldmanharpoon );

acttable_t	CWeaponOldManHarpoon::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_HARPOON,					false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_HARPOON,				false },
	{ ACT_WALK,						ACT_WALK_HARPOON,					false },
	{ ACT_RUN,						ACT_RUN_HARPOON,					false },
	{ ACT_MELEE_ATTACK1,			ACT_MELEE_ATTACK_SKEWER,			true },
};
IMPLEMENT_ACTTABLE( CWeaponOldManHarpoon );

int CWeaponOldManHarpoon::CapabilitiesGet()
{
	return bits_CAP_WEAPON_MELEE_ATTACK1;
}
float CWeaponOldManHarpoon::GetDamageForActivity(Activity hitActivity)
{
	return sk_plr_dmg_harpoon.GetFloat();
}
void CWeaponOldManHarpoon::ItemPostFrame(void)
{
	if (m_flAttackTime && gpGlobals->curtime > m_flAttackTime)
	{
		HarpoonSwing();
		m_flAttackTime = 0;
	}
	BaseClass::ItemPostFrame();
}

void CWeaponOldManHarpoon::PrimaryAttack(void)
{
	if (m_flAttackTime)
		return;
	SendWeaponAnim(ACT_VM_HITCENTER);
	m_flAttackTime = gpGlobals->curtime + sk_harpoon_attack_delay_plr.GetFloat();
}

bool CWeaponOldManHarpoon::CanHolster(void)
{
	if (m_flAttackTime)
		return false;

	return BaseClass::CanHolster();
}

int CWeaponOldManHarpoon::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity();

	// Project where the enemy will be in a little while
	float dt = sk_harpoon_lead_time.GetFloat();
	dt += random->RandomFloat(-0.3f, 0.2f);
	if (dt < 0.0f)
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA(pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos);

	Vector vecDelta;
	VectorSubtract(vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta);

	if (fabs(vecDelta.z) > sk_npc_harpoon_range.GetFloat())
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D();
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize(vecDelta.AsVector2D());
	if ((flDist > sk_npc_harpoon_range.GetFloat()) && (flExtrapolatedDist > sk_npc_harpoon_range.GetFloat()))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D(vecDelta.AsVector2D(), vecForward.AsVector2D());
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}
void CWeaponOldManHarpoon::HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors(GetAbsAngles(), &vecDirection);

	CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
	if (pEnemy)
	{
		Vector vecDelta;
		VectorSubtract(pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta);
		VectorNormalize(vecDelta);

		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize(vecDelta2D);
		if (DotProduct2D(vecDelta2D, vecDirection.AsVector2D()) > 0.8f)
		{
			vecDirection = vecDelta;
		}
	}
	trace_t traceHit;
	CTakeDamageInfo info(GetOwner(), GetOwner(), sk_npc_dmg_harpoon.GetFloat(), GetDamageType());
	Vector vecEnd;
	VectorMA(pOperator->Weapon_ShootPosition(), sk_npc_harpoon_range.GetFloat(), vecDirection, vecEnd);
	UTIL_TraceLine(pOperator->Weapon_ShootPosition(), vecEnd, MASK_SHOT_HULL, GetOwner(), COLLISION_GROUP_NONE, &traceHit);
	//CBaseEntity *pHurt = pOperator->CheckTraceHullAttack(pOperator->Weapon_ShootPosition(), vecEnd, Vector(-8, -8, -8), Vector(8, 8, 8), 100, iDamageType, 0.75);
	CBaseEntity *pHurt = traceHit.m_pEnt;

	// did I hit someone?
	if (pHurt)
	{
		// play sound
		WeaponSound(MELEE_HIT);
		pHurt->DispatchTraceAttack(info, vecDirection, &traceHit);
		ApplyMultiDamage();
	}
	else
	{
		WeaponSound(MELEE_MISS);
	}
}
void CWeaponOldManHarpoon::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit(pEvent, pOperator);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
void CWeaponOldManHarpoon::HarpoonSwing()
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, sk_plr_harpoon_range.GetFloat());

	Vector swingEnd = swingStart + forward * sk_plr_harpoon_range.GetFloat();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = ACT_VM_HITCENTER;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), GetDamageType());
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	m_iPrimaryAttacks++;

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * sk_plr_harpoon_range.GetFloat();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{
		Hit(traceHit, nHitActivity, false);
		if (traceHit.m_pEnt)
		{
			char cMaterial = physprops->GetSurfaceData(traceHit.surface.surfaceProps)->game.material;
			bool bFlesh = (cMaterial == CHAR_TEX_FLESH) || (cMaterial == CHAR_TEX_ANTLION) || (cMaterial == CHAR_TEX_BLOODYFLESH) || (cMaterial == CHAR_TEX_EGGSHELL) || (cMaterial == CHAR_TEX_ALIENFLESH) || (cMaterial == CHAR_TEX_SLOSH);
			if (bFlesh)
				WeaponSound(MELEE_HIT);
			else
				WeaponSound(MELEE_HIT_WORLD);
		}
	}

	// Send the anim
	//SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}
