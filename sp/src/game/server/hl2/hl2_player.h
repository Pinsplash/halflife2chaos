//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2_PLAYER_H
#define HL2_PLAYER_H
#pragma once


#include "player.h"
#include "hl2_playerlocaldata.h"
#include "simtimer.h"
#include "soundenvelope.h"
//#include "chaos.h"

class CAI_Squad;
class CPropCombineBall;

extern int TrainSpeed(int iSpeed, int iMax);
extern void CopyToBodyQue( CBaseAnimating *pCorpse );

#define ARMOR_DECAY_TIME 3.5f

enum HL2PlayerPhysFlag_e
{
	// 1 -- 5 are used by enum PlayerPhysFlag_e in player.h

	PFLAG_ONBARNACLE	= ( 1<<6 )		// player is hangning from the barnalce
};

class IPhysicsPlayerController;
class CLogicPlayerProxy;

struct commandgoal_t
{
	Vector		m_vecGoalLocation;
	CBaseEntity	*m_pGoalEntity;
};

// Time between checks to determine whether NPCs are illuminated by the flashlight
#define FLASHLIGHT_NPC_CHECK_INTERVAL	0.4

//----------------------------------------------------
// Definitions for weapon slots
//----------------------------------------------------
#define	WEAPON_MELEE_SLOT			0
#define	WEAPON_SECONDARY_SLOT		1
#define	WEAPON_PRIMARY_SLOT			2
#define	WEAPON_EXPLOSIVE_SLOT		3
#define	WEAPON_TOOL_SLOT			4

//=============================================================================
//=============================================================================
class CSuitPowerDevice
{
public:
	CSuitPowerDevice( int bitsID, float flDrainRate ) { m_bitsDeviceID = bitsID; m_flDrainRate = flDrainRate; }
private:
	int		m_bitsDeviceID;	// tells what the device is. DEVICE_SPRINT, DEVICE_FLASHLIGHT, etc. BITMASK!!!!!
	float	m_flDrainRate;	// how quickly does this device deplete suit power? ( percent per second )

public:
	int		GetDeviceID( void ) const { return m_bitsDeviceID; }
	float	GetDeviceDrainRate( void ) const
	{	
		if( g_pGameRules->GetSkillLevel() == SKILL_EASY && hl2_episodic.GetBool() && !(GetDeviceID()&bits_SUIT_DEVICE_SPRINT) )
			return m_flDrainRate * 0.5f;
		else
			return m_flDrainRate; 
	}
};

//=============================================================================
// >> HL2_PLAYER
//=============================================================================
class CHL2_Player : public CBasePlayer
{
public:
	DECLARE_CLASS(CHL2_Player, CBasePlayer);
	int PickEffect(int iWeightSum);
	void StartGivenEffect(int nID);
	void StopGivenEffect(int nID);
	void MaintainEvils();
	void PopulateEffects();
	void StartGame();//called after loads of all kinds
	template<class T = CChaosEffect> void CreateEffect(int nEffect, string_t strHudName, int nContext, float flDurationMult, int iWeight);
	bool EffectOrGroupAlreadyActive(int iEffect);
	//we actually do want to remember what effects are present at the moment of a save so that we may eliminate them upon reloading if needed
	int m_iActiveEffects[64];
	//HACK: the chaos HUD does not appear on its own after loading a save. it must be done at some time after the screen has begun updating(?),
	//and Activate() is too early, but PreThink() is fine, so we will have to remember to restart the HUD.
	//I'd rather find a function that respresents precisely when it becomes ok to call the HUD functions.
	bool m_bRestartHUD;
	virtual void			Event_PreSaveGameLoaded(char const *pSaveName, bool bInGame);
	virtual void		InputInsideTransition(inputdata_t &inputdata);
	void		DoChaosHUDBar();
	void		DoChaosHUDText();
	void		ForceUnstuck();
	Vector		RotatedOffset(Vector vecOffset, bool bNoVertical);
	void		ReplaceEffects();
	void		RemoveDeadEnts();
	void		SpawnStoredEnts();
	CHL2_Player();
	~CHL2_Player( void );
	
	static CHL2_Player *CreatePlayer( const char *className, edict_t *ed )
	{
		CHL2_Player::s_PlayerEdict = ed;
		return (CHL2_Player*)CreateEntityByName( className );
	}

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void		CreateCorpse( void ) { CopyToBodyQue( this ); };

	virtual void		Precache( void );
	virtual void		Spawn(void);
	virtual void		Activate( void );
	virtual void		CheatImpulseCommands( int iImpulse );
	virtual void		PlayerRunCommand( CUserCmd *ucmd, IMoveHelper *moveHelper);
	virtual void		PlayerUse ( void );
	virtual void		SuspendUse( float flDuration ) { m_flTimeUseSuspended = gpGlobals->curtime + flDuration; }
	virtual void		UpdateClientData( void );
	virtual void		OnRestore();
	virtual void		StopLoopingSounds( void );
	virtual void		Splash( void );
	virtual void 		ModifyOrAppendPlayerCriteria( AI_CriteriaSet& set );

	void				DrawDebugGeometryOverlays(void);

	virtual Vector		EyeDirection2D( void );
	virtual Vector		EyeDirection3D( void );

	virtual void		CommanderMode();

	virtual bool		ClientCommand( const CCommand &args );

	// from cbasecombatcharacter
	void				InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity );
	WeaponProficiency_t CalcWeaponProficiency( CBaseCombatWeapon *pWeapon );

	Class_T				Classify ( void );

	// from CBasePlayer
	virtual void		SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );

	// Suit Power Interface
	void SuitPower_Update( void );
	bool SuitPower_Drain( float flPower ); // consume some of the suit's power.
	void SuitPower_Charge( float flPower ); // add suit power.
	void SuitPower_SetCharge( float flPower ) { m_HL2Local.m_flSuitPower = flPower; }
	void SuitPower_Initialize( void );
	bool SuitPower_IsDeviceActive( const CSuitPowerDevice &device );
	bool SuitPower_AddDevice( const CSuitPowerDevice &device );
	bool SuitPower_RemoveDevice( const CSuitPowerDevice &device );
	bool SuitPower_ShouldRecharge( void );
	float SuitPower_GetCurrentPercentage( void ) { return m_HL2Local.m_flSuitPower; }
	
	void SetFlashlightEnabled( bool bState );

	// Apply a battery
	bool ApplyBattery( float powerMultiplier = 1.0 );

	// Commander Mode for controller NPCs
	enum CommanderCommand_t
	{
		CC_NONE,
		CC_TOGGLE,
		CC_FOLLOW,
		CC_SEND,
	};

	void CommanderUpdate();
	void CommanderExecute( CommanderCommand_t command = CC_TOGGLE );
	bool CommanderFindGoal( commandgoal_t *pGoal );
	void NotifyFriendsOfDamage( CBaseEntity *pAttackerEntity );
	CAI_BaseNPC *GetSquadCommandRepresentative();
	int GetNumSquadCommandables();
	int GetNumSquadCommandableMedics();

	// Locator
	void UpdateLocatorPosition( const Vector &vecPosition );

	// Sprint Device
	void StartAutoSprint( void );
	void StartSprinting( void );
	void StopSprinting( void );
	void InitSprinting( void );
	bool IsSprinting( void ) { return m_fIsSprinting; }
	bool CanSprint( void );
	void EnableSprint( bool bEnable);

	bool CanZoom( CBaseEntity *pRequester );
	void ToggleZoom(void);
	void StartZooming( void );
	void StopZooming( void );
	bool IsZooming( void );
	void CheckSuitZoom(void);

	// Walking
	void StartWalking(void);
	void StopWalking(void);
	bool IsWalking(void) { return m_fIsWalking; }

	// Ducking
	void StartDucking(void);
	void StopDucking(void);
	bool IsDucking(void) { return m_fIsDucking; }

	// Aiming heuristics accessors
	virtual float		GetIdleTime( void ) const { return ( m_flIdleTime - m_flMoveTime ); }
	virtual float		GetMoveTime( void ) const { return ( m_flMoveTime - m_flIdleTime ); }
	virtual float		GetLastDamageTime( void ) const { return m_flLastDamageTime; }
	virtual bool		IsDucking( void ) const { return !!( GetFlags() & FL_DUCKING ); }

	virtual bool		PassesDamageFilter( const CTakeDamageInfo &info );
	void				InputIgnoreFallDamage( inputdata_t &inputdata );
	void				InputIgnoreFallDamageWithoutReset( inputdata_t &inputdata );
	void				InputEnableFlashlight( inputdata_t &inputdata );
	void				InputDisableFlashlight( inputdata_t &inputdata );

	const impactdamagetable_t &GetPhysicsImpactDamageTable();
	virtual int			OnTakeDamage( const CTakeDamageInfo &info );
	virtual int			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void		OnDamagedByExplosion( const CTakeDamageInfo &info );
	bool				ShouldShootMissTarget( CBaseCombatCharacter *pAttacker );

	void				CombineBallSocketed( CPropCombineBall *pCombineBall );

	virtual void		Event_KilledOther( CBaseEntity *pVictim, const CTakeDamageInfo &info );

	virtual void		GetAutoaimVector( autoaim_params_t &params );
	bool				ShouldKeepLockedAutoaimTarget( EHANDLE hLockedTarget );

	void				SetLocatorTargetEntity( CBaseEntity *pEntity ) { m_hLocatorTargetEntity.Set( pEntity ); }

	virtual int			GiveAmmo( int nCount, int nAmmoIndex, bool bSuppressSound);
	virtual bool		BumpWeapon( CBaseCombatWeapon *pWeapon );
	
	virtual bool		Weapon_CanUse( CBaseCombatWeapon *pWeapon );
	virtual void		Weapon_Equip( CBaseCombatWeapon *pWeapon );
	virtual bool		Weapon_Lower( void );
	virtual bool		Weapon_Ready( void );
	virtual bool		Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex = 0 );
	virtual bool		Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon );

	void FirePlayerProxyOutput( const char *pszOutputName, variant_t variant, CBaseEntity *pActivator, CBaseEntity *pCaller );

	CLogicPlayerProxy	*GetPlayerProxy( void );

	// Flashlight Device
	void				CheckFlashlight( void );
	int					FlashlightIsOn( void );
	void				FlashlightTurnOn( void );
	void				FlashlightTurnOff( void );
	bool				IsIlluminatedByFlashlight( CBaseEntity *pEntity, float *flReturnDot );
	void				SetFlashlightPowerDrainScale( float flScale ) { m_flFlashlightPowerDrainScale = flScale; }

	// Underwater breather device
	virtual void		SetPlayerUnderwater( bool state );
	virtual bool		CanBreatheUnderwater() const { return m_HL2Local.m_flSuitPower > 0.0f; }

	// physics interactions
	virtual void		PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize );
	virtual	bool		IsHoldingEntity( CBaseEntity *pEnt );
	virtual void		ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldindThis );
	virtual float		GetHeldObjectMass( IPhysicsObject *pHeldObject );

	virtual bool		IsFollowingPhysics( void ) { return (m_afPhysicsFlags & PFLAG_ONBARNACLE) > 0; }
	void				InputForceDropPhysObjects( inputdata_t &data );

	virtual void		Event_Killed( const CTakeDamageInfo &info );
	void				NotifyScriptsOfDeath( void );

	// override the test for getting hit
	virtual bool		TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );

	LadderMove_t		*GetLadderMove() { return &m_HL2Local.m_LadderMove; }
	virtual void		ExitLadder();
	virtual surfacedata_t *GetLadderSurface( const Vector &origin );

	virtual void EquipSuit( bool bPlayEffects = true );
	virtual void RemoveSuit( void );
	void  HandleAdmireGlovesAnimation( void );
	void  StartAdmireGlovesAnimation( void );
	
	void  HandleSpeedChanges( void );

	void SetControlClass( Class_T controlClass ) { m_nControlClass = controlClass; }
	
	void StartWaterDeathSounds( void );
	void StopWaterDeathSounds( void );

	bool IsWeaponLowered( void ) { return m_HL2Local.m_bWeaponLowered; }
	void HandleArmorReduction( void );
	void StartArmorReduction( void ) { m_flArmorReductionTime = gpGlobals->curtime + ARMOR_DECAY_TIME; 
									   m_iArmorReductionFrom = ArmorValue(); 
									 }

	void MissedAR2AltFire();

	inline void EnableCappedPhysicsDamage();
	inline void DisableCappedPhysicsDamage();

	// HUD HINTS
	void DisplayLadderHudHint();

	CSoundPatch *m_sndLeeches;
	CSoundPatch *m_sndWaterSplashes;

protected:
	virtual void		PreThink( void );
	virtual	void		PostThink( void );
	virtual bool		HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	virtual void		UpdateWeaponPosture( void );

	virtual void		ItemPostFrame();
	virtual void		PlayUseDenySound();

private:
	bool				CommanderExecuteOne( CAI_BaseNPC *pNpc, const commandgoal_t &goal, CAI_BaseNPC **Allies, int numAllies );

	void				OnSquadMemberKilled( inputdata_t &data );

	Class_T				m_nControlClass;			// Class when player is controlling another entity
	// This player's HL2 specific data that should only be replicated to 
	//  the player and not to other players.
	CNetworkVarEmbedded( CHL2PlayerLocalData, m_HL2Local );

	float				m_flTimeAllSuitDevicesOff;

	bool				m_bSprintEnabled;		// Used to disable sprint temporarily
	bool				m_bIsAutoSprinting;		// A proxy for holding down the sprint key.
	float				m_fAutoSprintMinTime;	// Minimum time to maintain autosprint regardless of player speed. 

	CNetworkVar(bool, m_fIsSprinting);
	CNetworkVarForDerived(bool, m_fIsWalking);
	CNetworkVarForDerived(bool, m_fIsDucking);

protected:	// Jeep: Portal_Player needs access to this variable to overload PlayerUse for picking up objects through portals
	bool				m_bPlayUseDenySound;		// Signaled by PlayerUse, but can be unset by HL2 ladder code...

private:

	CAI_Squad *			m_pPlayerAISquad;
	CSimpleSimTimer		m_CommanderUpdateTimer;
	float				m_RealTimeLastSquadCommand;
	CommanderCommand_t	m_QueuedCommand;

	Vector				m_vecMissPositions[16];
	int					m_nNumMissPositions;

	float				m_flTimeIgnoreFallDamage;
	bool				m_bIgnoreFallDamageResetAfterImpact;

	// Suit power fields
	float				m_flSuitPowerLoad;	// net suit power drain (total of all device's drainrates)
	float				m_flAdmireGlovesAnimTime;

	float				m_flNextFlashlightCheckTime;
	float				m_flFlashlightPowerDrainScale;

	// Aiming heuristics code
	float				m_flIdleTime;		//Amount of time we've been motionless
	float				m_flMoveTime;		//Amount of time we've been in motion
	float				m_flLastDamageTime;	//Last time we took damage
	float				m_flTargetFindTime;

	EHANDLE				m_hPlayerProxy;

	bool				m_bFlashlightDisabled;
	bool				m_bUseCappedPhysicsDamageTable;
	
	float				m_flArmorReductionTime;
	int					m_iArmorReductionFrom;

	float				m_flTimeUseSuspended;

	CSimpleSimTimer		m_LowerWeaponTimer;
	CSimpleSimTimer		m_AutoaimTimer;

	EHANDLE				m_hLockedAutoAimEntity;

	EHANDLE				m_hLocatorTargetEntity; // The entity that's being tracked by the suit locator.

	float				m_flTimeNextLadderHint;	// Next time we're eligible to display a HUD hint about a ladder.
	
	friend class CHL2GameMovement;
};


//-----------------------------------------------------------------------------
// FIXME: find a better way to do this
// Switches us to a physics damage table that caps the max damage.
//-----------------------------------------------------------------------------
void CHL2_Player::EnableCappedPhysicsDamage()
{
	m_bUseCappedPhysicsDamageTable = true;
}


void CHL2_Player::DisableCappedPhysicsDamage()
{
	m_bUseCappedPhysicsDamageTable = false;
}

static const char *g_MapNames[] =
{
	{ "d1_trainstation_01" },
	{ "d1_trainstation_02" },
	{ "d1_trainstation_03" },//|
	{ "d1_trainstation_04" },//|
	{ "d1_trainstation_05" },//|
	{ "d1_trainstation_06" },
	{ "d1_canals_01" },
	{ "d1_canals_01a" },
	{ "d1_canals_02" },//|
	{ "d1_canals_03" },//|
	{ "d1_canals_05" },
	{ "d1_canals_06" },
	{ "d1_canals_07" },
	{ "d1_canals_08" },
	{ "d1_canals_09" },
	{ "d1_canals_10" },
	{ "d1_canals_11" },
	{ "d1_canals_12" },
	{ "d1_canals_13" },
	{ "d1_eli_01" },//|
	{ "d1_eli_02" },
	{ "d1_town_01" },
	{ "d1_town_01a" },
	{ "d1_town_02" },
	{ "d1_town_03" },
	{ "d1_town_02a" },
	{ "d1_town_04" },//|
	{ "d1_town_05" },
	{ "d2_coast_01" },
	{ "d2_coast_03" },
	{ "d2_coast_04" },
	{ "d2_coast_05" },
	{ "d2_coast_07" },
	{ "d2_coast_08" },
	{ "d2_coast_09" },
	{ "d2_coast_10" },
	{ "d2_coast_11" },
	{ "d2_coast_12" },
	{ "d2_prison_01" },
	{ "d2_prison_02" },//|
	{ "d2_prison_03" },//|
	{ "d2_prison_04" },//|
	{ "d2_prison_05" },//|
	{ "d2_prison_06" },//|
	{ "d2_prison_07" },//|
	{ "d2_prison_08" },
	{ "d3_c17_01" },//|
	{ "d3_c17_02" },
	{ "d3_c17_03" },
	{ "d3_c17_04" },
	{ "d3_c17_05" },//|
	{ "d3_c17_06a" },//|
	{ "d3_c17_06b" },//|
	{ "d3_c17_07" },
	{ "d3_c17_08" },
	{ "d3_c17_09" },
	{ "d3_c17_10a" },
	{ "d3_c17_10b" },//|
	{ "d3_c17_11" },
	{ "d3_c17_12" },
	{ "d3_c17_12b" },
	{ "d3_c17_13" },
	{ "d3_citadel_01" },
	{ "d3_citadel_02" },//|
	{ "d3_citadel_03" },
	{ "d3_citadel_04" },
	{ "d3_citadel_05" },//|
	{ "d3_breen_01" },//|
	{ "" },
	{ "ep1_citadel_00" },
	{ "ep1_citadel_01" },
	{ "ep1_citadel_02" },
	{ "ep1_citadel_02b" },//|
	{ "ep1_citadel_03" },//|
	{ "ep1_citadel_04" },
	{ "ep1_c17_00" },//|
	{ "ep1_c17_00a" },//|
	{ "ep1_c17_01" },
	{ "ep1_c17_02" },
	{ "ep1_c17_02b" },//yes, a and b are switched around
	{ "ep1_c17_02a" },//|
	{ "ep1_c17_05" },
	{ "ep1_c17_06" },
	{ "" },
	{ "ep2_outland_01" },
	{ "ep2_outland_01a" },//|
	{ "ep2_outland_02" },//|
	{ "ep2_outland_03" },//|
	{ "ep2_outland_04" },//|
	{ "ep2_outland_05" },
	{ "ep2_outland_06" },
	{ "ep2_outland_06a" },
	{ "ep2_outland_07" },
	{ "ep2_outland_08" },
	{ "ep2_outland_09" },
	{ "ep2_outland_10" },
	{ "ep2_outland_10a" },
	{ "ep2_outland_11" },//|
	{ "ep2_outland_11a" },//|
	{ "ep2_outland_11b" },//|
	{ "ep2_outland_12" },
	{ "ep2_outland_12a" },//|
};

#endif	//HL2_PLAYER_H
