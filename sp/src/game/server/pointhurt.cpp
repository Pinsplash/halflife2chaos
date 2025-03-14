//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements hurting point entity
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "entitylist.h"
#include "gamerules.h"
#include "basecombatcharacter.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const int SF_PHURT_START_ON			= 1;

class CPointHurt : public CPointEntity
{
	DECLARE_CLASS( CPointHurt, CPointEntity );

public:
	void	Spawn( void );
	void	Precache( void );
	void	HurtThink( void );
	virtual void LogicExplode();
	// Input handlers
	void InputTurnOn(inputdata_t &inputdata);
	void InputTurnOff(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);
	void InputHurt(inputdata_t &inputdata);
	
	DECLARE_DATADESC();

	int			m_nDamage;
	int			m_bitsDamageType;
	float		m_flRadius;
	float		m_flDelay;
	string_t	m_strTarget;
	EHANDLE		m_pActivator;
};

BEGIN_DATADESC( CPointHurt )

	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "DamageRadius" ),
	DEFINE_KEYFIELD( m_nDamage, FIELD_INTEGER, "Damage" ),
	DEFINE_KEYFIELD( m_flDelay, FIELD_FLOAT, "DamageDelay" ),
	DEFINE_KEYFIELD( m_bitsDamageType, FIELD_INTEGER, "DamageType" ),
	DEFINE_KEYFIELD( m_strTarget, FIELD_STRING, "DamageTarget" ),
	
	// Function Pointers
	DEFINE_FUNCTION( HurtThink ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Hurt", InputHurt ),

	DEFINE_FIELD( m_pActivator, FIELD_EHANDLE ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( point_hurt, CPointHurt );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointHurt::Spawn(void)
{
	SetThink( NULL );
	SetUse( NULL );
		
	m_pActivator = NULL;

	if ( HasSpawnFlags( SF_PHURT_START_ON ) )
	{
		SetThink( &CPointHurt::HurtThink );
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
	
	if ( m_flRadius <= 0.0f )
	{
		m_flRadius = 128.0f;
	}

	if ( m_nDamage <= 0 )
	{
		m_nDamage = 2;
	}

	if ( m_flDelay <= 0 )
	{
		m_flDelay = 0.1f;
	}

	Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointHurt::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointHurt::HurtThink( void )
{
	if ( m_strTarget != NULL_STRING )
	{
		CBaseEntity	*pEnt = NULL;
			
		CTakeDamageInfo info( this, m_pActivator, m_nDamage, m_bitsDamageType );
		while ( ( pEnt = gEntList.FindEntityByName( pEnt, m_strTarget, NULL, m_pActivator ) ) != NULL )
		{
			GuessDamageForce( &info, (pEnt->GetAbsOrigin() - GetAbsOrigin()), pEnt->GetAbsOrigin() );
			pEnt->TakeDamage( info );
		}
	}
	else
	{
		RadiusDamage( CTakeDamageInfo( this, this, m_nDamage, m_bitsDamageType ), GetAbsOrigin(), m_flRadius, CLASS_NONE, NULL );
	}

	SetNextThink( gpGlobals->curtime + m_flDelay );
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for turning on the point hurt.
//-----------------------------------------------------------------------------
void CPointHurt::InputTurnOn( inputdata_t &data )
{
	SetThink( &CPointHurt::HurtThink );

	SetNextThink( gpGlobals->curtime + 0.1f );

	m_pActivator = data.pActivator;
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for turning off the point hurt.
//-----------------------------------------------------------------------------
void CPointHurt::InputTurnOff( inputdata_t &data )
{
	SetThink( NULL );

	m_pActivator = data.pActivator;
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for toggling the on/off state of the point hurt.
//-----------------------------------------------------------------------------
void CPointHurt::InputToggle( inputdata_t &data )
{
	m_pActivator = data.pActivator;

	if ( m_pfnThink == (void (CBaseEntity::*)())&CPointHurt::HurtThink )
	{
		SetThink( NULL );
	}
	else
	{
		SetThink( &CPointHurt::HurtThink );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for instantaneously hurting whatever is near us.
//-----------------------------------------------------------------------------
void CPointHurt::InputHurt( inputdata_t &data )
{
	m_pActivator = data.pActivator;

	HurtThink();
}

void CPointHurt::LogicExplode()
{
	int nRandom = RandomInt(0, 2);
	variant_t variant;
	switch (nRandom)
	{
		//skipped TurnOn and TurnOff
	case 0:
		AcceptInput("Hurt", this, this, variant, 0);
		break;
	case 1:
		AcceptInput("Toggle", this, this, variant, 0);
		break;
	case 2:
		BaseClass::LogicExplode();
		break;
	}
}