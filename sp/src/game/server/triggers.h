//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TRIGGERS_H
#define TRIGGERS_H
#ifdef _WIN32
#pragma once
#endif

#include "basetoggle.h"
#include "entityoutput.h"
#include "EntityParticleTrail.h"

// ACBOB: This is global scope in a header, so should be externed...
extern bool g_bEndSolidTriggers;

//
// Spawnflags
//

enum
{
	SF_TRIGGER_ALLOW_CLIENTS				= 0x01,		// Players can fire this trigger
	SF_TRIGGER_ALLOW_NPCS					= 0x02,		// NPCS can fire this trigger
	SF_TRIGGER_ALLOW_PUSHABLES				= 0x04,		// Pushables can fire this trigger
	SF_TRIGGER_ALLOW_PHYSICS				= 0x08,		// Physics objects can fire this trigger
	SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS		= 0x10,		// *if* NPCs can fire this trigger, this flag means only player allies do so
	SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES		= 0x20,		// *if* Players can fire this trigger, this flag means only players inside vehicles can 
	SF_TRIGGER_ALLOW_ALL					= 0x40,		// Everything can fire this trigger EXCEPT DEBRIS!
	SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES	= 0x200,	// *if* Players can fire this trigger, this flag means only players outside vehicles can 
	SF_TRIG_PUSH_ONCE						= 0x80,		// trigger_push removes itself after firing once
	SF_TRIG_PUSH_AFFECT_PLAYER_ON_LADDER	= 0x100,	// if pushed object is player on a ladder, then this disengages them from the ladder (HL2only)
	SF_TRIG_TOUCH_DEBRIS 					= 0x400,	// Will touch physics debris objects
	SF_TRIGGER_ONLY_NPCS_IN_VEHICLES		= 0X800,	// *if* NPCs can fire this trigger, only NPCs in vehicles do so (respects player ally flag too)
	SF_TRIGGER_DISALLOW_BOTS                = 0x1000,   // Bots are not allowed to fire this trigger
};

// DVS TODO: get rid of CBaseToggle
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CBaseTrigger : public CBaseToggle
{
	DECLARE_CLASS( CBaseTrigger, CBaseToggle );
public:
	CBaseTrigger();
	virtual void LogicExplode();
	void Activate( void );
	virtual void PostClientActive( void );
	void InitTrigger( void );

	void Enable( void );
	void Disable( void );
	void Spawn( void );
	void UpdateOnRemove( void );
	void TouchTest(  void );

	// Input handlers
	virtual void InputEnable( inputdata_t &inputdata );
	virtual void InputDisable( inputdata_t &inputdata );
	virtual void InputToggle( inputdata_t &inputdata );
	virtual void InputTouchTest ( inputdata_t &inputdata );

	virtual void InputStartTouch( inputdata_t &inputdata );
	virtual void InputEndTouch( inputdata_t &inputdata );

	virtual bool UsesFilter(void){ return (gEntList.FindEntityByName(NULL, m_iFilterName) != NULL); }
	virtual bool PassesTriggerFilters(CBaseEntity *pOther);
	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);
	bool IsTouching( CBaseEntity *pOther );

	CBaseEntity *GetTouchedEntityOfType( const char *sClassName );

	int	 DrawDebugTextOverlays(void);

	// by default, triggers don't deal with TraceAttack
	void TraceAttack(CBaseEntity *pAttacker, float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType) {}

	bool PointIsWithin( const Vector &vecPoint );

	bool		m_bDisabled;
	string_t	m_iFilterName;
	//CHandle<class CBaseFilter>	m_hFilter;

protected:

	// Outputs
	COutputEvent m_OnStartTouch;
	COutputEvent m_OnStartTouchAll;
	COutputEvent m_OnEndTouch;
	COutputEvent m_OnEndTouchAll;
	COutputEvent m_OnTouching;
	COutputEvent m_OnNotTouching;

	// Entities currently being touched by this trigger
	CUtlVector< EHANDLE >	m_hTouchingEntities;

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------
// Purpose: Variable sized repeatable trigger.  Must be targeted at one or more entities.
//			If "delay" is set, the trigger waits some time after activating before firing.
//			"wait" : Seconds between triggerings. (.2 default/minimum)
//-----------------------------------------------------------------------------
class CTriggerMultiple : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerMultiple, CBaseTrigger );
public:
	void Spawn( void );
	void MultiTouch( CBaseEntity *pOther );
	void MultiWaitOver( void );
	void ActivateMultiTrigger(CBaseEntity *pActivator);

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnTrigger;
};

// Global list of triggers that care about weapon fire
extern CUtlVector< CHandle<CTriggerMultiple> >	g_hWeaponFireTriggers;


//------------------------------------------------------------------------------
// Base VPhysics trigger implementation
// NOTE: This uses vphysics to compute touch events.  It doesn't do a per-frame Touch call, so the 
// Entity I/O is different from a regular trigger
//------------------------------------------------------------------------------
#define SF_VPHYSICS_MOTION_MOVEABLE	0x1000

class CBaseVPhysicsTrigger : public CBaseEntity
{
	DECLARE_CLASS( CBaseVPhysicsTrigger , CBaseEntity );

public:
	DECLARE_DATADESC();
	virtual void LogicExplode();
	virtual void Spawn();
	virtual void UpdateOnRemove();
	virtual bool CreateVPhysics();
	virtual void Activate( void );
	virtual bool PassesTriggerFilters(CBaseEntity *pOther);

	// UNDONE: Pass trigger event in or change Start/EndTouch.  Add ITriggerVPhysics perhaps?
	// BUGBUG: If a player touches two of these, his movement will screw up.
	// BUGBUG: If a player uses crouch/uncrouch it will generate touch events and clear the motioncontroller flag
	virtual void StartTouch( CBaseEntity *pOther );
	virtual void EndTouch( CBaseEntity *pOther );

	void InputToggle( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputDisable(inputdata_t &inputdata);
	bool						m_bDisabled;
	

protected:
	string_t					m_iFilterName;
	CHandle<class CBaseFilter>	m_hFilter;
};
//=====================================================================================================================
//-----------------------------------------------------------------------------
// Purpose: VPhysics trigger that changes the motion of vphysics objects that touch it
//-----------------------------------------------------------------------------
class CTriggerVPhysicsMotion : public CBaseVPhysicsTrigger, public IMotionEvent
{
	DECLARE_CLASS(CTriggerVPhysicsMotion, CBaseVPhysicsTrigger);

public:
	void Spawn();
	void Precache();
	virtual void UpdateOnRemove();
	bool CreateVPhysics();
	void OnRestore();
	virtual void LogicExplode();
	// UNDONE: Pass trigger event in or change Start/EndTouch.  Add ITriggerVPhysics perhaps?
	// BUGBUG: If a player touches two of these, his movement will screw up.
	// BUGBUG: If a player uses crouch/uncrouch it will generate touch events and clear the motioncontroller flag
	void StartTouch(CBaseEntity *pOther);
	void EndTouch(CBaseEntity *pOther);

	void InputSetVelocityLimitTime(inputdata_t &inputdata);

	float LinearLimit();

	inline bool HasGravityScale() { return m_gravityScale != 1.0 ? true : false; }
	inline bool HasAirDensity() { return m_addAirDensity != 0 ? true : false; }
	inline bool HasLinearLimit() { return LinearLimit() != 0.0f; }
	inline bool HasLinearScale() { return m_linearScale != 1.0 ? true : false; }
	inline bool HasAngularLimit() { return m_angularLimit != 0 ? true : false; }
	inline bool HasAngularScale() { return m_angularScale != 1.0 ? true : false; }
	inline bool HasLinearForce() { return m_linearForce != 0.0 ? true : false; }

	DECLARE_DATADESC();

	virtual simresult_e	Simulate(IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular);

private:
	IPhysicsMotionController	*m_pController;

#ifndef _XBOX
	EntityParticleTrailInfo_t	m_ParticleTrail;
#endif //!_XBOX

	float						m_gravityScale;
	float						m_addAirDensity;
	float						m_linearLimit;
	float						m_linearLimitDelta;
	float						m_linearLimitTime;
	float						m_linearLimitStart;
	float						m_linearLimitStartTime;
	float						m_linearScale;
	float						m_angularLimit;
	float						m_angularScale;
	float						m_linearForce;
	QAngle						m_linearForceAngles;
};
// ##################################################################################
//	>> TriggerVolume
// ##################################################################################
class CTriggerVolume : public CPointEntity	// Derive from point entity so this doesn't move across levels
{
public:
	DECLARE_CLASS(CTriggerVolume, CPointEntity);

	void		Spawn(void);
};

//-----------------------------------------------------------------------------
// Purpose: Hurts anything that touches it. If the trigger has a targetname,
//			firing it will toggle state.
//-----------------------------------------------------------------------------
class CTriggerHurt : public CBaseTrigger
{
public:
	CTriggerHurt()
	{
		// This field came along after levels were built so the field defaults to 20 here in the constructor.
		m_flDamageCap = 20.0f;
	}

	DECLARE_CLASS( CTriggerHurt, CBaseTrigger );
	virtual void LogicExplode();
	void Spawn( void );
	void RadiationThink( void );
	void HurtThink( void );
	void Touch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );
	bool HurtEntity( CBaseEntity *pOther, float damage );
	int HurtAllTouchers( float dt );

	DECLARE_DATADESC();

	float	m_flOriginalDamage;	// Damage as specified by the level designer.
	float	m_flDamage;			// Damage per second.
	float	m_flDamageCap;		// Maximum damage per second.
	float	m_flLastDmgTime;	// Time that we last applied damage.
	float	m_flDmgResetTime;	// For forgiveness, the time to reset the counter that accumulates damage.
	int		m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does
	int		m_damageModel;
	bool	m_bNoDmgForce;		// Should damage from this trigger impart force on what it's hurting

	enum
	{
		DAMAGEMODEL_NORMAL = 0,
		DAMAGEMODEL_DOUBLE_FORGIVENESS,
	};

	// Outputs
	COutputEvent m_OnHurt;
	COutputEvent m_OnHurtPlayer;

	CUtlVector<EHANDLE>	m_hurtEntities;
};

#endif // TRIGGERS_H
