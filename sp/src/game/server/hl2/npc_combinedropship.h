//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: dropship

//i don't know why but this has to be changed to 2.
#ifndef	DROPSHIP_H
#define	DROPSHIP_H
#pragma once
#include "cbasehelicopter.h"

class CCombineDropshipContainer : public CPhysicsProp
{
	DECLARE_CLASS(CCombineDropshipContainer, CPhysicsProp);
	DECLARE_DATADESC();

public:
	void Precache();
	virtual void Spawn();
	virtual bool OverridePropdata(void);
	virtual int OnTakeDamage(const CTakeDamageInfo& info);
	virtual void Event_Killed(const CTakeDamageInfo& info);

private:
	enum
	{
		MAX_SMOKE_TRAILS = 4,
		MAX_EXPLOSIONS = 4,
	};

	// Should we trigger a damage effect?
	bool ShouldTriggerDamageEffect(int nPrevHealth, int nEffectCount) const;

	// Add a smoke trail since we've taken more damage
	void AddSmokeTrail(const Vector& vecPos);

	// Pow!
	void ThrowFlamingGib();

	// Create a corpse
	void CreateCorpse();

private:
	int m_nSmokeTrailCount;
	EHANDLE m_hLastInflictor;
	float m_flLastHitTime;
};

enum LandingState_t
{
	LANDING_NO = 0,

	// Dropoff
	LANDING_LEVEL_OUT,		// Heading to a point above the dropoff point
	LANDING_DESCEND,		// Descending from to the dropoff point
	LANDING_TOUCHDOWN,
	LANDING_UNLOADING,
	LANDING_UNLOADED,
	LANDING_LIFTOFF,

	// Pickup
	LANDING_SWOOPING,		// Swooping down to the target

	// Hovering, which we're saying is a type of landing since there's so much landing code to leverage
	LANDING_START_HOVER,
	LANDING_HOVER_LEVEL_OUT,
	LANDING_HOVER_DESCEND,
	LANDING_HOVER_TOUCHDOWN,
	LANDING_END_HOVER,
};
#define DROPSHIP_MAX_SOLDIERS			6
//=============================================================================
// The combine dropship
//=============================================================================
class CNPC_CombineDropship : public CBaseHelicopter
{
	DECLARE_CLASS(CNPC_CombineDropship, CBaseHelicopter);

public:
	~CNPC_CombineDropship();
	virtual void LogicExplode();
	// Setup
	void	Spawn(void);
	void	Precache(void);

	void	Activate(void);

	// Thinking/init
	void	InitializeRotorSound(void);
	void	StopLoopingSounds();
	void	PrescheduleThink(void);
	void	ChaosDeploy();

	// Flight/sound
	void	Hunt(void);
	void	Flight(void);
	float	GetAltitude(void);
	void	DoRotorWash(void);
	void	UpdateRotorSoundPitch(int iPitch);
	void	UpdatePickupNavigation(void);
	void	UpdateLandTargetNavigation(void);
	void	CalculateSoldierCount(int iSoldiers);

	// Updates the facing direction
	virtual void UpdateFacingDirection();

	// Combat
	void	GatherEnemyConditions(CBaseEntity* pEnemy);
	void	DoCombatStuff(void);
	void	SpawnTroop(void);
	void	DropMine(void);
	void	UpdateContainerGunFacing(Vector& vecMuzzle, Vector& vecToTarget, Vector& vecAimDir, float* flTargetRange);
	bool	FireCannonRound(void);
	void	DoImpactEffect(trace_t& tr, int nDamageType);
	void	StartCannon(void);
	void	StopCannon(void);
	void	MakeTracer(const Vector& vecTracerSrc, const trace_t& tr, int iTracerType);
	int		OnTakeDamage_Alive(const CTakeDamageInfo& inputInfo);

	// Input handlers.
	void	InputLandLeave(inputdata_t& inputdata);
	void	InputLandTake(inputdata_t& inputdata);
	void	InputSetLandTarget(inputdata_t& inputdata);
	void	InputDropMines(inputdata_t& inputdata);
	void	InputDropStrider(inputdata_t& inputdata);
	void	InputDropAPC(inputdata_t& inputdata);

	void	InputPickup(inputdata_t& inputdata);
	void	InputSetGunRange(inputdata_t& inputdata);
	void	InputNPCFinishDustoff(inputdata_t& inputdata);
	void	InputStopWaitingForDropoff(inputdata_t& inputdata);

	void	InputHover(inputdata_t& inputdata);

	// From AI_TrackPather
	virtual void InputFlyToPathTrack(inputdata_t& inputdata);

	Vector	GetDropoffFinishPosition(Vector vecOrigin, CAI_BaseNPC* pNPC, Vector vecMins, Vector vecMaxs);
	void	LandCommon(bool bHover = false);

	Class_T Classify(void) { return CLASS_COMBINE_GUNSHIP; }

	// Drop the soldier container
	void	DropSoldierContainer();

	// Sounds
	virtual void UpdateRotorWashVolume();

	void SetLandingState(LandingState_t landingState);
	LandingState_t GetLandingState() const { return (LandingState_t)m_iLandState; }
	bool IsHovering();
	void UpdateGroundRotorWashSound(float flAltitude);
	void UpdateRotorWashVolume(CSoundPatch* pRotorSound, float flVolume, float flDeltaTime);

	// Timers
	float	m_flTimeTakeOff;
	float	m_flNextTroopSpawnAttempt;
	float	m_flDropDelay;			// delta between each mine
	float	m_flTimeNextAttack;
	float	m_flLastTime;

	// States and counters
	int		m_iMineCount;		// index for current mine # being deployed
	int		m_totalMinesToDrop;	// total # of mines to drop as a group (based upon triggered input)
	int		m_soldiersToDrop;
	int		m_iDropState;
	int		m_iLandState;
	float	m_engineThrust;		// for tracking sound volume/pitch
	float	m_existPitch;
	float	m_existRoll;
	bool	m_bDropMines;		// signal to drop mines
	bool	m_bIsFiring;
	int		m_iBurstRounds;
	bool	m_leaveCrate;
	bool	m_bHasDroppedOff;
	int		m_iCrateType;
	float	m_flLandingSpeed;
	float	m_flGunRange;
	bool	m_bInvulnerable;

	QAngle	m_vecAngAcceleration;

	// Misc Vars
	CHandle<CBaseAnimating>	m_hContainer;
	EHANDLE		m_hPickupTarget;
	int			m_iContainerMoveType;
	bool		m_bWaitForDropoffInput;

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	EHANDLE		m_hLandTarget;
	string_t	m_iszLandTarget;

	string_t	m_iszAPCVehicleName;

	// Templates for soldier's dropped off
	string_t	m_sNPCTemplate[DROPSHIP_MAX_SOLDIERS];
	string_t	m_sNPCTemplateData[DROPSHIP_MAX_SOLDIERS];
	string_t	m_sDustoffPoints[DROPSHIP_MAX_SOLDIERS];
	int			m_iCurrentTroopExiting;
	EHANDLE		m_hLastTroopToLeave;

	// Template for rollermines dropped by this dropship
	string_t	m_sRollermineTemplate;
	string_t	m_sRollermineTemplateData;

	// Cached attachment points
	int			m_iMuzzleAttachment;
	int			m_iMachineGunBaseAttachment;
	int			m_iMachineGunRefAttachment;
	int			m_iAttachmentTroopDeploy;
	int			m_iAttachmentDeployStart;

	// Sounds
	CSoundPatch* m_pCannonSound;
	CSoundPatch* m_pRotorOnGroundSound;
	CSoundPatch* m_pDescendingWarningSound;
	CSoundPatch* m_pNearRotorSound;

	// Outputs
	COutputEvent	m_OnFinishedDropoff;
	COutputEvent	m_OnFinishedPickup;

	COutputFloat	m_OnContainerShotDownBeforeDropoff;
	COutputEvent	m_OnContainerShotDownAfterDropoff;

protected:
	// Because the combine dropship is a leaf class, we can use
	// static variables to store this information, and save some memory.
	// Should the dropship end up having inheritors, their activate may
	// stomp these numbers, in which case you should make these ordinary members
	// again.
	static int m_poseBody_Accel, m_poseBody_Sway, m_poseCargo_Body_Accel, m_poseCargo_Body_Sway,
		m_poseWeapon_Pitch, m_poseWeapon_Yaw;
	static bool m_sbStaticPoseParamsLoaded;
	virtual void	PopulatePoseParameters(void);
};

#endif	//DROPSHIP_H
