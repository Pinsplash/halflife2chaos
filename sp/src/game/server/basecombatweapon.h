//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#ifndef COMBATWEAPON_H
#define COMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#include "entityoutput.h"
#include "basecombatweapon_shared.h"
#include "func_break.h"

//-----------------------------------------------------------------------------
// Bullet types
//-----------------------------------------------------------------------------

// -----------------------------------------
//	Sounds
// -----------------------------------------

struct animevent_t;

extern void	SpawnBlood(Vector vecSpot, const Vector &vecDir, int bloodColor, float flDamage);


//-----------------------------------------------------------------------------
// Purpose: Weapons ignore other weapons when LOS tracing
//-----------------------------------------------------------------------------
class CWeaponLOSFilter : public CTraceFilterSkipTwoEntities
{
	DECLARE_CLASS(CWeaponLOSFilter, CTraceFilterSkipTwoEntities);
public:
	CWeaponLOSFilter(IHandleEntity* pHandleEntity, IHandleEntity* pHandleEntity2, int collisionGroup) :
		CTraceFilterSkipTwoEntities(pHandleEntity, pHandleEntity2, collisionGroup), m_pVehicle(NULL)
	{
		// If the tracing entity is in a vehicle, then ignore it
		if (pHandleEntity != NULL)
		{
			CBaseCombatCharacter* pBCC = ((CBaseEntity*)pHandleEntity)->MyCombatCharacterPointer();
			if (pBCC != NULL)
			{
				m_pVehicle = pBCC->GetVehicleEntity();
			}
		}
	}
	virtual bool ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask)
	{
		CBaseEntity* pEntity = (CBaseEntity*)pServerEntity;

		if (pEntity->GetCollisionGroup() == COLLISION_GROUP_WEAPON)
			return false;

		// Don't collide with the tracing entity's vehicle (if it exists)
		if (pServerEntity == m_pVehicle)
			return false;

		if (pEntity->GetHealth() > 0)
		{
			CBreakable* pBreakable = dynamic_cast<CBreakable*>(pEntity);
			if (pBreakable && pBreakable->IsBreakable() && pBreakable->GetMaterialType() == matGlass)
			{
				return false;
			}
		}

		return BaseClass::ShouldHitEntity(pServerEntity, contentsMask);
	}

private:
	CBaseEntity* m_pVehicle;
};
#endif // COMBATWEAPON_H
