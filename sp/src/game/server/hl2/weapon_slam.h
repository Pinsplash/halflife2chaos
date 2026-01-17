//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		SLAM 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

//i don't know why but this has to be changed to 2.
#ifndef	WEAPONSLAM2_H
#define	WEAPONSLAM2_H
#pragma once
#include "basegrenade_shared.h"
#include "basehlcombatweapon_shared.h"
class CSoundPatch;//

enum SlamState_t
{
	SLAM_TRIPMINE_READY,
	SLAM_SATCHEL_THROW,
	SLAM_SATCHEL_ATTACH,
};

class CBeam;

class CWeapon_SLAM : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeapon_SLAM, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();

	SlamState_t			m_tSlamState;
	bool				m_bDetonatorArmed;
	bool				m_bNeedDetonatorDraw;
	bool				m_bNeedDetonatorHolster;
	bool				m_bNeedReload;
	bool				m_bClearReload;
	bool				m_bThrowSatchel;
	bool				m_bAttachSatchel;
	bool				m_bAttachTripmine;
	float				m_flWallSwitchTime;

	void				Spawn(void);
	void				Precache(void);

	int					CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void				PrimaryAttack(void);
	void				SecondaryAttack(void);
	void				WeaponIdle(void);
	void				WeaponSwitch(void);
	void				SLAMThink(void);

	void				SetPickupTouch(void);
	void				SlamTouch(CBaseEntity *pOther);	// default weapon touch
	void				ItemPostFrame(void);
	bool				Reload(void);
	void				SetSlamState(SlamState_t newState);
	bool				CanAttachSLAM(void);		// In position where can attach SLAM?
	bool				AnyUndetonatedCharges(void);
	void				StartTripmineAttach(void);
	void				TripmineAttach(void);

	void				StartSatchelDetonate(void);
	void				SatchelDetonate(void);
	void				StartSatchelThrow(void);
	void				StartSatchelAttach(void);
	void				SatchelThrow(void);
	void				SatchelAttach(void);
	bool				Deploy(void);
	bool				Holster(CBaseCombatWeapon *pSwitchingTo = NULL);


	CWeapon_SLAM();

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};

class CSatchelCharge : public CBaseGrenade
{
public:
	DECLARE_CLASS(CSatchelCharge, CBaseGrenade);
	virtual void LogicExplode();
	void			Spawn(void);
	void			Precache(void);
	void			BounceSound(void);
	void			UpdateSlideSound(void);
	void			KillSlideSound(void);
	void			SatchelTouch(CBaseEntity *pOther);
	void			SatchelThink(void);
	void			SatchelUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	CSoundPatch*	m_soundSlide;
	float			m_flSlideVolume;
	float			m_flNextBounceSoundTime;
	bool			m_bInAir;
	Vector			m_vLastPosition;

public:
	CWeapon_SLAM*	m_pMyWeaponSLAM;	// Who shot me..
	bool			m_bIsAttached;
	void			Deactivate(void);

	CSatchelCharge();
	~CSatchelCharge();

	DECLARE_DATADESC();

private:
	void InitSlideSound(void);
};

class CTripmineGrenade : public CBaseGrenade
{
public:
	DECLARE_CLASS(CTripmineGrenade, CBaseGrenade);

	CTripmineGrenade();
	void Spawn(void);
	void Precache(void);

	int OnTakeDamage_Alive(const CTakeDamageInfo &info);

	void BeamBreakThink(void);
	void MineExplode(void);
	void Event_Killed(const CTakeDamageInfo &info);

	void MakeBeam(trace_t* tr);
	void KillBeam(void);

	EHANDLE		m_hOwner;

	float		m_flPowerUp;
	Vector		m_vecDir;
	Vector		m_vecEnd;

	CBeam		*m_pBeam;
	Vector		m_posOwner;
	Vector		m_angleOwner;

	DECLARE_DATADESC();
};
#endif	//WEAPONSLAM2_H
