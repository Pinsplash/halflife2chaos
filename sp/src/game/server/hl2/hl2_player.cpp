//========= Copyright Valve Corporation, All rights reserved. ============//
//
//Purpose: Player for HL2.
//
//========================================================================//

#include "cbase.h"
// do not include <string> !!! it breaks compilation with latest gcc
// from stdstring.h: GCC 4.2.2 headers screw up our min/max defs.
#include "stdstring.h"
#include <cmath>
#include "hl2_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "trains.h"
#include "basehlcombatweapon_shared.h"
#include "vcollide_parse.h"
#include "in_buttons.h"
#include "ai_interactions.h"
#include "ai_squad.h"
#include "igamemovement.h"
#include "ai_hull.h"
#include "hl2_shareddefs.h"
#include "info_camera_link.h"
#include "point_camera.h"
#include "engine/IEngineSound.h"
#include "ndebugoverlay.h"
#include "iservervehicle.h"
#include "IVehicle.h"
#include "globals.h"
#include "collisionutils.h"
#include "coordsize.h"
#include "effect_color_tables.h"
#include "vphysics/player_controller.h"
#include "player_pickup.h"
#include "weapon_physcannon.h"
#include "script_intro.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h" 
#include "ai_basenpc.h"
#include "AI_Criteria.h"
#include "npc_barnacle.h"
#include "entitylist.h"
#include "env_zoom.h"
#include "hl2_gamerules.h"
#include "prop_combine_ball.h"
#include "datacache/imdlcache.h"
#include "eventqueue.h"
#include "gamestats.h"
#include "filters.h"
#include "tier0/icommandline.h"
#include "saverestore_utlvector.h"
#include "movevars_shared.h"
#include "physics.h"
#include "physobj.h"
#include "ai_network.h"
#include "ai_node.h"
#include "saverestoretypes.h"
#include "saverestore.h"
#include "chaos.h"
#include "world.h"
#include "triggers.h"
#include "func_ladder.h"
#include "vehicle_base.h"
#include "fourwheelvehiclephysics.h"
#include "vehicle_jeep.h"
#include "model_types.h"
#include "npc_headcrab.h"
#include "npc_antlion.h"
#include "ai_playerally.h"
#include "hl_gamemovement.h"
#include "npc_PoisonZombie.h"
#include "physics_saverestore.h"
#include "npc_playercompanion.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "istudiorender.h"
#include "vgui/ILocalize.h"
#include "tier3/tier3.h"
#include "ai_tacticalservices.h"

#ifdef HL2_EPISODIC
#include "npc_alyx_episodic.h"
#endif

//memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar weapon_showproficiency;
extern ConVar autoaim_max_dist;
extern ConVar player_use_dist;
extern ConVar player_throwforce;
extern ConVar chaos_replace_bullets_with_crossbow;
extern ConVar chaos_replace_bullets_with_grenades;
extern ConVar chaos_explode_on_death;
extern ConVar chaos_bullet_teleport;
extern ConVar chaos_barrel_shotgun;
extern ConVar chaos_cant_leave_map;
extern ConVar r_handbrake_allowed;
extern ConVar r_vehicleBrakeRate;
extern ConVar steepness_limit;
extern ConVar chaos_no_reload;
extern ConVar chaos_npc_teleport;
extern ConVar chaos_disable_ladders;
extern ConVar ai_block_damage;
extern ConVar chaos_steal_health;
extern ConVar completed_town03;
extern ConVar completed_coast08;
//Do not touch with without seeing me, please! (sjb)
//For consistency's sake, enemy gunfire is traced against a scaled down
//version of the player's hull, not the hitboxes for the player's model
//because the player isn't aware of his model, and can't do anything about
//preventing headshots and other such things. Also, game difficulty will
//not change if the model changes. This is the value by which to scale
//the X/Y of the player's hull to get the volume to trace bullets against.
#define PLAYER_HULL_REDUCTION	0.70

//This switches between the single primary weapon, and multiple weapons with buckets approach (jdw)
#define	HL2_SINGLE_PRIMARY_WEAPON_MODE	0

#define TIME_IGNORE_FALL_DAMAGE 10.0

extern int gEvilImpulse101;
class CChaosEffect;
//extern CUtlVector<CChaosEffect>	g_ActiveEffects;
extern CUtlVector<CChaosEffect*>	g_ChaosEffects;
extern int						g_iChaosSpawnCount;
//CUtlVector<int>			g_iTerminated;

ConVar sv_autojump("sv_autojump", "0");

ConVar hl2_walkspeed("hl2_walkspeed", "150");
ConVar hl2_normspeed("hl2_normspeed", "190");
ConVar hl2_sprintspeed("hl2_sprintspeed", "320");
ConVar hl2_duckspeed("hl2_duckspeed", "64");//goodbye sanity

ConVar hl2_darkness_flashlight_factor("hl2_darkness_flashlight_factor", "1");

#define	HL2_WALK_SPEED hl2_walkspeed.GetFloat()
#define	HL2_NORM_SPEED hl2_normspeed.GetFloat()
#define	HL2_SPRINT_SPEED hl2_sprintspeed.GetFloat()
#define	HL2_DUCK_SPEED hl2_duckspeed.GetFloat()

ConVar player_showpredictedposition("player_showpredictedposition", "0");
ConVar player_showpredictedposition_timestep("player_showpredictedposition_timestep", "1.0");

ConVar player_squad_transient_commands("player_squad_transient_commands", "1", FCVAR_REPLICATED);
ConVar player_squad_double_tap_time("player_squad_double_tap_time", "0.25");

ConVar sv_infinite_aux_power("sv_infinite_aux_power", "0", FCVAR_NONE);

ConVar autoaim_unlock_target("autoaim_unlock_target", "0.8666");

ConVar sv_stickysprint("sv_stickysprint", "0", FCVAR_ARCHIVE | FCVAR_ARCHIVE_XBOX);

#define	FLASH_DRAIN_TIME	 1.1111	//100 units / 90 secs
#define	FLASH_CHARGE_TIME	 50.0f	//100 units / 2 secs

ConVar chaos("chaos", "0", FCVAR_NONE);
ConVar chaos_effect_interval("chaos_effect_interval", "30", FCVAR_NONE, "Time between each effect.");
ConVar chaos_effect_time("chaos_effect_time", "90", FCVAR_NONE, "Standard effect length.");
ConVar chaos_instant_off("chaos_instant_off", "0", FCVAR_NONE);
ConVar chaos_rng1("chaos_rng1", "-1", FCVAR_NONE, "Controls the random number generated by effects that use RNG");
ConVar chaos_pushpull_strength("chaos_pushpull_strength", "0.15", FCVAR_NONE, "Controls strength of repulsive and black hole");
ConVar chaos_beer_size_limit("chaos_beer_size_limit", "48", FCVAR_NONE, "Maximum size of beer bottle.");
ConVar chaos_strike_max("chaos_strike_max", "5", FCVAR_NONE, "Max number of times to allow an effect to kill player before ending it immediately");
ConVar chaos_ignore_activeness("chaos_ignore_activeness", "0");
ConVar chaos_ignore_group("chaos_ignore_group", "0");
ConVar chaos_ignore_context("chaos_ignore_context", "0");
ConVar chaos_print_rng("chaos_print_rng", "0");
ConVar chaos_vote_enable("chaos_vote_enable", "0");
ConVar chaos_unstuck_neweffect("chaos_unstuck_neweffect", "1", FCVAR_NONE, "Get the player unstuck every time a new effect starts. may not be wanted by some technical players.");
ConVar chaos_shuffle_mode("chaos_shuffle_mode", "0");
ConVar chaos_shuffle_debug("chaos_shuffle_debug", "0");

CON_COMMAND(logex, "logic explosion")
{
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		pEnt->LogicExplode();
		pEnt = gEntList.NextEnt(pEnt);
	}
}

void RandomizeReadiness(CBaseEntity* pNPC)
{
	variant_t emptyVariant;
	float nRandom = RandomInt(0, 3);
	if (nRandom == 0)
		pNPC->AcceptInput("SetReadinessLow", pNPC, pNPC, emptyVariant, 0);
	if (nRandom == 1)
		pNPC->AcceptInput("SetReadinessMedium", pNPC, pNPC, emptyVariant, 0);
	if (nRandom == 2)
		pNPC->AcceptInput("SetReadinessHigh", pNPC, pNPC, emptyVariant, 0);
	if (nRandom == 3)
		pNPC->AcceptInput("SetReadinessPanic", pNPC, pNPC, emptyVariant, 0);
}
void ClearPersistEnts()
{
	g_PersistEnts.RemoveAll();
}
void ClearShuffleData()
{
	if (chaos_shuffle_debug.GetBool()) Msg("clearing shuffle list\n");
	for (int k = 0; k < NUM_EFFECTS; k++)
		g_iShufflePicked[k] = NULL;
}
void ClearChaosData()
{
	ClearPersistEnts();
	for (int i = 0; g_ChaosEffects.Size() >= i + 1; i++)
	{
		CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
		CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
		pHL2Player->StopGivenEffect(g_ChaosEffects[i]->m_nID);
	}
	//Invert gravity messes this up
	sv_gravity.SetValue(600);
	//g_ActiveEffects.RemoveAll();
	g_iChaosSpawnCount = 0;
	g_flEffectThinkRem = 0;
	g_flNextEffectRem = -1;
	//g_iActiveEffects.RemoveAll();
	for (int j = 0; j < MAX_ACTIVE_EFFECTS; j++)
	{
		g_iActiveEffects[j] = NULL;
	}

	ClearShuffleData();

	g_iTerminated.RemoveAll();
	g_PersistEnts.RemoveAll();
}

CChaosStoredEnt* StoreEnt(CBaseEntity* pEnt)
{
	CChaosStoredEnt* pStoredEnt = new CChaosStoredEnt;
	const char* constcharClassName = pEnt->GetClassname();
	int len = Q_strlen(constcharClassName) + 1;
	char* charCname = new char[len];
	Q_strncpy(charCname, constcharClassName, len);
	//pStoredEnt->classname = AllocPooledString(pEnt->GetClassname());
	pStoredEnt->strClassname = charCname;
	pStoredEnt->targetname = pEnt->GetEntityName();
	pStoredEnt->chaosid = pEnt->m_iChaosID;
	pStoredEnt->origin = pEnt->GetAbsOrigin();
	pStoredEnt->angle = pEnt->GetAbsAngles();
	pStoredEnt->health = pEnt->GetHealth();
	pStoredEnt->max_health = pEnt->GetMaxHealth();
	pStoredEnt->spawnflags = pEnt->GetSpawnFlags();
	pStoredEnt->model = pEnt->GetModelName();
	pStoredEnt->effects = pEnt->GetEffects();
	pStoredEnt->rendermode = pEnt->GetRenderMode();
	pStoredEnt->renderamt = pEnt->GetRenderColor().a;
	pStoredEnt->rendercolor = Color(pEnt->GetRenderColor().r, pEnt->GetRenderColor().g, pEnt->GetRenderColor().b);
	pStoredEnt->renderfx = pEnt->m_nRenderFX;
	pStoredEnt->modelindex = pEnt->GetModelIndex();
	pStoredEnt->speed = pEnt->m_flSpeed;
	pStoredEnt->solid = pEnt->CollisionProp()->GetSolid();

	//pStoredEnt.touchStamp = pEnt->touchStamp;//this was a speculative fix for some kind of touchlink related crash. if that crash comes back, put this back in.

	CBaseAnimating* pAnimating = dynamic_cast<CBaseAnimating*>(pEnt);
	if (pAnimating)
	{
		pStoredEnt->animating = true;
		pStoredEnt->skin = pAnimating->m_nSkin;
		pStoredEnt->body = pAnimating->m_nBody;
		CBaseCombatCharacter* pCombatCharacter = dynamic_cast<CBaseCombatCharacter*>(pAnimating);
		if (pCombatCharacter)
		{
			pStoredEnt->combatcharacter = true;
			CAI_BaseNPC* pNPC = dynamic_cast<CAI_BaseNPC*>(pCombatCharacter);
			if (pNPC)
			{
				pStoredEnt->npc = true;
				pStoredEnt->evil = pNPC->m_bEvil;
				pStoredEnt->hasweapon = (pNPC->GetActiveWeapon() != NULL);
				if (pNPC->GetActiveWeapon())
				{
					pStoredEnt->additionalequipment = MAKE_STRING(pNPC->GetActiveWeapon()->GetClassname());
				}
				CBaseHeadcrab* pHeadcrab = dynamic_cast<CBaseHeadcrab*>(pNPC);
				if (pHeadcrab)
				{
					pStoredEnt->headcrab = true;
					pStoredEnt->burrowed = pHeadcrab->m_bBurrowed;
					pStoredEnt->hiding = pHeadcrab->m_bHidden;
					pStoredEnt->ceiling = pHeadcrab->IsHangingFromCeiling();
				}
				CNPC_PoisonZombie* pPZombie = dynamic_cast<CNPC_PoisonZombie*>(pNPC);
				if (pPZombie)
				{
					pStoredEnt->poisonzombie = true;
					pStoredEnt->crabcount = 0;
					pStoredEnt->crabs[0] = pPZombie->m_bCrabs[0];
					pStoredEnt->crabs[1] = pPZombie->m_bCrabs[1];
					pStoredEnt->crabs[2] = pPZombie->m_bCrabs[2];
					if (pStoredEnt->crabs[0])
					{
						pStoredEnt->crabcount++;
					}
					if (pStoredEnt->crabs[1])
					{
						pStoredEnt->crabcount++;
					}
					if (pStoredEnt->crabs[2])
					{
						pStoredEnt->crabcount++;
					}
				}
				CNPC_Antlion* pAntlion = dynamic_cast<CNPC_Antlion*>(pNPC);
				if (pAntlion)
				{
					pStoredEnt->antlion = true;
					pStoredEnt->burrowed = pAntlion->m_bStartBurrowed;//this gets set to false once unburrowed, don't worry
				}
				if (pNPC->ClassMatches("npc_antliong*"))
				{
					pStoredEnt->antlionguard = true;
					pStoredEnt->burrowed = pNPC->m_takedamage == DAMAGE_NO;
					pStoredEnt->cavernbreed = pAnimating->m_nSkin == 1;
				}
			}
		}
	}
	return pStoredEnt;
}

CBaseEntity* RetrieveStoredEnt(CChaosStoredEnt* pStoredEnt, bool bPersist)
{
	CBaseEntity* pEnt = CreateEntityByName(pStoredEnt->strClassname);
	//Msg("Spawning persist ent %i\n", pStoredEnt->chaosid);
	//if we don't find a duplicate, that's also okay.
	pEnt->KeyValue("targetname", STRING(pStoredEnt->targetname));
	pEnt->m_iChaosID = pStoredEnt->chaosid;

	pEnt->SetAbsOrigin(pStoredEnt->origin);

	pEnt->SetAbsAngles(pStoredEnt->angle);

	pEnt->SetHealth(pStoredEnt->health);
	pEnt->SetMaxHealth(pStoredEnt->max_health);
	pEnt->AddSpawnFlags(pStoredEnt->spawnflags);
	pEnt->KeyValue("model", STRING(pStoredEnt->model));
	pEnt->AddEffects(pStoredEnt->effects);
	pEnt->SetRenderMode((RenderMode_t)pStoredEnt->rendermode);
	pEnt->SetRenderColorA(pStoredEnt->renderamt);
	pEnt->SetRenderColor(pStoredEnt->rendercolor.r(), pStoredEnt->rendercolor.g(), pStoredEnt->rendercolor.b());
	pEnt->m_nRenderFX = pStoredEnt->renderfx;
	pEnt->SetModelIndex(pStoredEnt->modelindex);
	pEnt->m_flSpeed = pStoredEnt->speed;
	pEnt->CollisionProp()->SetSolid((SolidType_t)pStoredEnt->solid);
	//pEnt->touchStamp = pKey->GetInt("touchStamp");//this was a speculative fix for some kind of touchlink related crash. if that crash comes back, put this back in.

	pEnt->m_bChaosSpawned = true;
	pEnt->m_bChaosPersist = bPersist;

	if (pStoredEnt->animating)//CBaseAnimating
	{
		pEnt->KeyValue("skin", pStoredEnt->skin);
		pEnt->KeyValue("body", pStoredEnt->body);
		if (pStoredEnt->combatcharacter)//CBaseCombatCharacter
		{
			if (pStoredEnt->npc)//CAI_BaseNPC
			{
				CAI_BaseNPC* pNPC = static_cast<CAI_BaseNPC*>(pEnt);
				pNPC->m_bEvil = pStoredEnt->evil;
				if (pStoredEnt->hasweapon)
				{
					pEnt->KeyValue("additionalequipment", STRING(pStoredEnt->additionalequipment));
				}
				if (pStoredEnt->headcrab)
				{
					CBaseHeadcrab* pHeadcrab = dynamic_cast<CBaseHeadcrab*>(pNPC);
					pHeadcrab->SetBurrowed(pStoredEnt->burrowed);
					pHeadcrab->m_bHidden = pStoredEnt->hiding;
					pHeadcrab->m_bHangingFromCeiling = pStoredEnt->ceiling;
				}
				if (pStoredEnt->poisonzombie)
				{
					CNPC_PoisonZombie* pPZombie = dynamic_cast<CNPC_PoisonZombie*>(pNPC);
					pPZombie->m_nCrabCount = pStoredEnt->crabcount;
				}
				if (pStoredEnt->antlion)
				{
					CNPC_Antlion* pAntlion = dynamic_cast<CNPC_Antlion*>(pNPC);
					pAntlion->m_bStartBurrowed = pStoredEnt->burrowed;
				}
				if (pStoredEnt->antlionguard)
				{
					pEnt->KeyValue("startburrowed", pStoredEnt->burrowed);
				}
			}
		}
	}
	//PIN: force EF_NODRAW off. we've found some zombies in ep1_c17_01 that have the flag on for no apparent reason
	pEnt->RemoveEffects(EF_NODRAW);
	return pEnt;
}

//
//save chaos-persistent entities. for instance, a griefer npc should keep its health to whatever it ends up becoming to lessen softlock potential
//
void SavePersistEnts()
{
	int iEnts = 0;
	ClearPersistEnts();
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		if (pEnt->m_bChaosPersist && (!pEnt->IsNPC() || pEnt->IsAlive()))
		{
			iEnts++;
			g_PersistEnts.AddToTail(StoreEnt(pEnt));
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
}
CON_COMMAND(chaos_print, "print all effects for debugging")
{
	for (int i = 0; i < NUM_EFFECTS; i++)
	{
		if (g_ChaosEffects.Size() < i + 1)
			break;
		Msg("%i: %s %s\n", i, STRING(g_ChaosEffects[i]->m_strGeneralName), g_ChaosEffects[i]->m_bActive ? "ACTIVE" : "");
	}
}
#ifdef DEBUG
CON_COMMAND(cte, "turn on a specific effect")
#else
CON_COMMAND(chaos_test_effect, "turn on a specific effect")
#endif // DEBUG
{
	//
	if (args.ArgC() > 1 && atoi(args[1]) < NUM_EFFECTS)
	{
		CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

		if (pPlayer)
		{
			CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);

			if (pHL2Player)
			{
				pHL2Player->StartGivenEffect(atoi(args[1]));
			}
		}
	}
}
CON_COMMAND(chaos_vote_internal_poll, "used by an external client. returns vote number and possible choices for this vote")
{
	if (!g_ChaosEffects.Size())
		return;

	char szLocalized0[512];
	wchar_t* pwcText0 = g_pVGuiLocalize->Find(g_ChaosEffects[g_arriVoteEffects[0]]->m_strGeneralName.ToCStr());
	if (pwcText0) g_pVGuiLocalize->ConvertUnicodeToANSI(pwcText0, szLocalized0, sizeof(szLocalized0));
	else Msg("null string - chaos vote internal poll\n");

	char szLocalized1[512];
	wchar_t* pwcText1 = g_pVGuiLocalize->Find(g_ChaosEffects[g_arriVoteEffects[1]]->m_strGeneralName.ToCStr());
	if (pwcText1) g_pVGuiLocalize->ConvertUnicodeToANSI(pwcText1, szLocalized1, sizeof(szLocalized1));
	else Msg("null string - chaos vote internal poll\n");

	char szLocalized2[512];
	wchar_t* pwcText2 = g_pVGuiLocalize->Find(g_ChaosEffects[g_arriVoteEffects[2]]->m_strGeneralName.ToCStr());
	if (pwcText2) g_pVGuiLocalize->ConvertUnicodeToANSI(pwcText2, szLocalized2, sizeof(szLocalized2));
	else Msg("null string - chaos vote internal poll\n");

	char szLocalized3[512];
	wchar_t* pwcText3 = g_pVGuiLocalize->Find(g_ChaosEffects[g_arriVoteEffects[3]]->m_strGeneralName.ToCStr());
	if (pwcText3) g_pVGuiLocalize->ConvertUnicodeToANSI(pwcText3, szLocalized3, sizeof(szLocalized3));
	else Msg("null string - chaos vote internal poll\n");

	ConMsg("%d;%s;%s;%s;%s",
		g_iVoteNumber,
		(szLocalized0),
		(szLocalized1),
		(szLocalized2),
		(szLocalized3)
	);

}
CON_COMMAND(chaos_vote_internal_set, "used by an external client. sets current votes")
{
	// TODO: assumes vote number and 4 choices.
	if (args.ArgC() < 6)
		return;

	int givenVoteNumber = atoi(args[1]);

	if (givenVoteNumber != g_iVoteNumber)
		return;

	for (int i = 0; i < 4; ++i)
	{
		g_arriVotes[i] = atoi(args[i + 2]);
	}
}
CON_COMMAND(chaos_vote_debug, "prints info about the votes")
{
	ConMsg("vote#: %d, votes: %d;%d;%d;%d, effects: %s;%s;%s;%s\n",
		g_iVoteNumber,
		g_arriVotes[0],
		g_arriVotes[1],
		g_arriVotes[2],
		g_arriVotes[3],
		STRING(g_ChaosEffects[g_arriVoteEffects[0]]->m_strGeneralName),
		STRING(g_ChaosEffects[g_arriVoteEffects[1]]->m_strGeneralName),
		STRING(g_ChaosEffects[g_arriVoteEffects[2]]->m_strGeneralName),
		STRING(g_ChaosEffects[g_arriVoteEffects[3]]->m_strGeneralName)
	);
}
void DisplayGeneratedCC()
{
	CBaseEntity* pCC = (CBaseEntity*)CreateEntityByName("color_correction");
	if (pCC)
	{
		pCC->SetAbsOrigin(vec3_origin);
		pCC->KeyValue("targetname", "chaos_cc");
		pCC->KeyValue("minfalloff", "-1");
		pCC->KeyValue("maxfalloff", "-1");
		pCC->KeyValue("maxweight", "1");
		pCC->KeyValue("fadeInDuration", "0");
		pCC->KeyValue("fadeOutDuration", "0");
		pCC->KeyValue("filename", "materials/chaos_random_cc.raw");
		pCC->KeyValue("StartDisabled", "0");
		pCC->Spawn();
		pCC->Activate();
	}
}
CON_COMMAND(cc_generate, "")
{
	FileHandle_t file_handle = filesystem->Open("materials/chaos_random_cc.raw", "wb");
	int rOut = RandomInt(0, 255);
	int gOut = RandomInt(0, 255);
	int bOut = RandomInt(0, 255);
	float flFrequency = 1;
	int r = 0;
	int g = 0;
	int b = 0;
	for (r = 0; r < 32; r++)
	{
		//change colors at a controlled rate
		if (flFrequency >= 6)
		{
			rOut = (RandomInt(0, 255) + (r * 10) + (rOut * 4)) / 4;
			gOut = (RandomInt(0, 255) + (g * 10) + (rOut * 4)) / 4;
			bOut = (RandomInt(0, 255) + (b * 10) + (rOut * 4)) / 4;
			flFrequency = 1;
		}
		flFrequency += 1;
		for (g = 0; g < 32; g++)
		{
			for (b = 0; b < 32; b++)
			{
				color24 outColor;
				outColor.r = rOut;
				outColor.g = gOut;
				outColor.b = bOut;
				filesystem->Write(&outColor, sizeof(color24), file_handle);
			}
		}
	}
	filesystem->Close(file_handle);
	DisplayGeneratedCC();
}
CON_COMMAND(chaos_vote_reset, "choses new effects and resets votes")
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	if (!pPlayer)
		return;

	CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	// TODO: do we really need this null check? i feel like the cast above is supposed to be dynamic_cast
	if (!pHL2Player)
		return;
	int iWeightSum = pHL2Player->FindWeightSum();
	pHL2Player->ResetVotes(iWeightSum);
	ConMsg("%d;%s;%s;%s;%s\n",
		g_iVoteNumber,
		STRING(g_ChaosEffects[g_arriVoteEffects[0]]->m_strHudName),
		STRING(g_ChaosEffects[g_arriVoteEffects[1]]->m_strHudName),
		STRING(g_ChaosEffects[g_arriVoteEffects[2]]->m_strHudName),
		STRING(g_ChaosEffects[g_arriVoteEffects[3]]->m_strHudName)
	);
}
CON_COMMAND(chaos_reset, "resets stuff like sv_gravity. executes chaos_restart.cfg.")
{
	//Reset anything that persists forever
	ClearChaosData();
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "exec chaos_restart\n");
	sv_gravity.SetValue(600);
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
	g_bAvoidExtreme = true;
}
CON_COMMAND(chaos_restart, "restarts map and resets stuff like sv_gravity. executes chaos_restart.cfg.")
{
	//Reset anything that persists forever
	ClearChaosData();
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "restart;exec chaos_restart\n");
	g_ChaosEffects.RemoveAll();
	g_bAvoidExtreme = true;
}
//chaos_ignore_activeness
//chaos_ignore_group
//chaos_ignore_context
//chaos_print_rng
CON_COMMAND(chaos_test_rng, "test if RNG works well monte-carlo style. arg 1 is how many effects to pick.")
{
	if (args.ArgC() > 1)
	{
		CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
		if (pPlayer)
		{
			CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
			if (pHL2Player)
			{
				int iWeightSum = 0;
				int iPicks[NUM_EFFECTS];
				iPicks[0] = 0;
				for (int i = 1; i < NUM_EFFECTS; i++)
				{
					if (chaos_print_rng.GetBool()) Msg("i %i, %s %i += %i\n", i, STRING(g_ChaosEffects[i]->m_strGeneralName), iWeightSum, g_ChaosEffects[i]->m_iCurrentWeight);
					iWeightSum += g_ChaosEffects[i]->m_iCurrentWeight;
					iPicks[i] = 0;
				}
				pHL2Player->ClearEffectContextCache();
				for (int j = 0; j < atoi(args[1]); j++)
				{
					int nID = pHL2Player->PickEffect(iWeightSum, true);
					iPicks[nID]++;
				}
				for (int k = 0; k < NUM_EFFECTS; k++)
				{
					Msg("%i: %s picked %i times\n", k, STRING(g_ChaosEffects[k]->m_strGeneralName), iPicks[k]);
				}
			}
		}
	}
	else
	{
		Msg("Specify number of times to run RNG\n");
	}
}
CON_COMMAND(chaos_test_rng_uniform, "return result of every possible number. good for testing if picking logic is faulty.")
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
		if (pHL2Player)
		{
			int iWeightSum = 0;
			int iPicks[NUM_EFFECTS];
			iPicks[0] = 0;
			for (int i = 1; i < NUM_EFFECTS; i++)
			{
				if (chaos_print_rng.GetBool()) Msg("i %i, %s %i += %i\n", i, STRING(g_ChaosEffects[i]->m_strGeneralName), iWeightSum, g_ChaosEffects[i]->m_iCurrentWeight);
				iWeightSum += g_ChaosEffects[i]->m_iCurrentWeight;
				iPicks[i] = 0;
			}
			pHL2Player->ClearEffectContextCache();
			for (int j = 0; j < iWeightSum; j++)
			{
				int nID = pHL2Player->PickEffect(iWeightSum, true, j);
				if (nID >= 0)
					iPicks[nID]++;
			}
			for (int k = 0; k < NUM_EFFECTS; k++)
			{
				Msg("%i: %s picked %i times\n", k, STRING(g_ChaosEffects[k]->m_strGeneralName), iPicks[k]);
			}
		}
	}
}

//commands stored in groups.cfg, which is execed in CHL2_Player::Spawn()
CON_COMMAND_F(chaos_group, "Creates a chaos group.", FCVAR_SERVER_CAN_EXECUTE)
{
	if (args.ArgC() > 1)
	{
		/*
		bool bFullOnGroups = true;
		for (int i = 0; i < MAX_EFFECTS_IN_GROUP; i++)
		{
			if (g_iGroups[i][0] == 0)
			{
				Msg("Making group %i\n", i);
				if (atoi(args[MAX_EFFECTS_IN_GROUP + 1]))
					Msg("Desired group exceeds group size limit\n");
				bFullOnGroups = false;
				for (int j = 1; j < MAX_EFFECTS_IN_GROUP + 1; j++)
				{
					if (!atoi(args[j]))
						break;
					Msg("Adding effect %i to group %i\n", , i);
					g_iGroups[i][j - 1] = atoi(args[j]);
				}
				break;
			}
		}
		if (bFullOnGroups)
			Msg("Could not create group, no more groups can be made\n");
		*/
		for (int i = 1; atoi(args[i]); i++)
		{
			for (int j = 1; atoi(args[j]); j++)
			{
				int iAddMe = atoi(args[j]);
				int iToMe = atoi(args[i]);
				if (iAddMe == iToMe)//dont add an effect to its own exclude list because why
					continue;
				Msg("Added %s to effect %s's exclusion list in slot %i\n", STRING(g_ChaosEffects[iAddMe]->m_strGeneralName), STRING(g_ChaosEffects[iToMe]->m_strGeneralName), g_ChaosEffects[iToMe]->m_iExcludeCount);
				g_ChaosEffects[iToMe]->m_iExclude[g_ChaosEffects[iToMe]->m_iExcludeCount] = iAddMe;
				g_ChaosEffects[iToMe]->m_iExcludeCount++;
			}
		}
	}
	else
	{
		Msg("Specify numbers to assign to group\n");
	}
}
CON_COMMAND(getunstuck, "try to get unstuck right now")
{
	UTIL_GetLocalPlayer()->GetUnstuck(200);
}
CON_COMMAND(putatnearestnode, "failsafe part of unstuck test")
{
	UTIL_GetLocalPlayer()->PutAtNearestNode(200, false);
}
Vector CHL2_Player::RotatedOffset(Vector vecOffset, bool bNoVertical)
{
	QAngle angEye = GetAbsAngles();
	if (bNoVertical)
		angEye.x = 0;//forget about pitch
	//else
	//	vecOffset.z -= abs(angEye.x);
	Vector vecBarrelPos;
	VectorRotate(vecOffset, angEye, vecBarrelPos);
	return vecBarrelPos + GetAbsOrigin();

	//CBaseEntity *pBarrel = (CBaseEntity *)CreateEntityByName("prop_physics");
	//pBarrel->SetModel("models/props_c17/oildrum001_explosive.mdl");

	/*pBarrel->SetParent(pPlayer);
	pBarrel->SetLocalOrigin(Vector(64, 64, 64));
	pBarrel->SetParent(NULL);
	Vector vecForward;
	pPlayer->EyeVectors(&vecForward);
	Vector vecVelocity = (vecForward * 1200) + pPlayer->GetAbsVelocity();
	pBarrel->SetAbsVelocity(vecVelocity);*/

	//DispatchSpawn(pBarrel);
	//pBarrel->Activate();
	//pBarrel->Teleport(&vecBarrelPos, &angEye, NULL);
}
//==============================================================================================
//CAPPED PLAYER PHYSICS DAMAGE TABLE
//==============================================================================================
static impactentry_t cappedPlayerLinearTable[] =
{
	{ 150 * 150, 5 },
	{ 250 * 250, 10 },
	{ 450 * 450, 20 },
	{ 550 * 550, 30 },
	//{ 700*700, 100 },
	//{ 1000*1000, 500 },
};

static impactentry_t cappedPlayerAngularTable[] =
{
	{ 100 * 100, 10 },
	{ 150 * 150, 20 },
	{ 200 * 200, 30 },
	//{ 300*300, 500 },
};

static impactdamagetable_t gCappedPlayerImpactDamageTable =
{
	cappedPlayerLinearTable,
	cappedPlayerAngularTable,

	ARRAYSIZE(cappedPlayerLinearTable),
	ARRAYSIZE(cappedPlayerAngularTable),

	24 * 24.0f,	//minimum linear speed
	360 * 360.0f,	//minimum angular speed
	2.0f,		//can't take damage from anything under 2kg

	5.0f,		//anything less than 5kg is "small"
	5.0f,		//never take more than 5 pts of damage from anything under 5kg
	36 * 36.0f,	//<5kg objects must go faster than 36 in/s to do damage

	0.0f,		//large mass in kg (no large mass effects)
	1.0f,		//large mass scale
	2.0f,		//large mass falling scale
	320.0f,		//min velocity for player speed to cause damage

};

//Flashlight utility
bool g_bCacheLegacyFlashlightStatus = true;
bool g_bUseLegacyFlashlight;
bool Flashlight_UseLegacyVersion(void)
{
	//If this is the first run through, cache off what the answer should be (cannot change during a session)
	if (g_bCacheLegacyFlashlightStatus)
	{
		char modDir[MAX_PATH];
		if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
			return false;

		g_bUseLegacyFlashlight = (!Q_strcmp(modDir, "hl2chaos") ||
			!Q_strcmp(modDir, "ep1chaos") ||
			!Q_strcmp(modDir, "lostcoast") || !Q_strcmp(modDir, "hl1"));

		g_bCacheLegacyFlashlightStatus = false;
	}

	//Return the results
	return g_bUseLegacyFlashlight;
}

//-----------------------------------------------------------------------------
//Purpose: Used to relay outputs/inputs from the player to the world and viceversa
//-----------------------------------------------------------------------------
class CLogicPlayerProxy : public CLogicalEntity
{
	DECLARE_CLASS(CLogicPlayerProxy, CLogicalEntity);

private:

	DECLARE_DATADESC();

public:
	virtual void LogicExplode();
	COutputEvent m_OnFlashlightOn;
	COutputEvent m_OnFlashlightOff;
	COutputEvent m_PlayerHasAmmo;
	COutputEvent m_PlayerHasNoAmmo;
	COutputEvent m_PlayerDied;
	COutputEvent m_PlayerMissedAR2AltFire; //Player fired a combine ball which did not dissolve any enemies. 

	COutputInt m_RequestedPlayerHealth;

	void InputRequestPlayerHealth(inputdata_t& inputdata);
	void InputSetFlashlightSlowDrain(inputdata_t& inputdata);
	void InputSetFlashlightNormalDrain(inputdata_t& inputdata);
	void InputSetPlayerHealth(inputdata_t& inputdata);
	void InputRequestAmmoState(inputdata_t& inputdata);
	void InputLowerWeapon(inputdata_t& inputdata);
	void InputEnableCappedPhysicsDamage(inputdata_t& inputdata);
	void InputDisableCappedPhysicsDamage(inputdata_t& inputdata);
	void InputSetLocatorTargetEntity(inputdata_t& inputdata);

	void Activate(void);

	bool PassesDamageFilter(const CTakeDamageInfo& info);

	EHANDLE m_hPlayer;
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CC_ToggleZoom(void)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();

	if (pPlayer)
	{
		CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);

		if (pHL2Player && pHL2Player->IsSuitEquipped())
		{
			pHL2Player->ToggleZoom();
		}
	}
}

static ConCommand toggle_zoom("toggle_zoom", CC_ToggleZoom, "Toggles zoom display");

//ConVar cl_forwardspeed( "cl_forwardspeed", "400", FCVAR_NONE ); //Links us to the client's version
ConVar xc_crouch_range("xc_crouch_range", "0.85", FCVAR_ARCHIVE, "Percentarge [1..0] of joystick range to allow ducking within");	//Only 1/2 of the range is used
ConVar xc_use_crouch_limiter("xc_use_crouch_limiter", "0", FCVAR_ARCHIVE, "Use the crouch limiting logic on the controller");

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CC_ToggleDuck(void)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	if (pPlayer == NULL)
		return;

	//Cannot be frozen
	if (pPlayer->GetFlags() & FL_FROZEN)
		return;

	static bool		bChecked = false;
	static ConVar* pCVcl_forwardspeed = NULL;
	if (!bChecked)
	{
		bChecked = true;
		pCVcl_forwardspeed = (ConVar*)cvar->FindVar("cl_forwardspeed");
	}


	//If we're not ducked, do extra checking
	if (xc_use_crouch_limiter.GetBool())
	{
		if (pPlayer->GetToggledDuckState() == false)
		{
			float flForwardSpeed = 400.0f;
			if (pCVcl_forwardspeed)
			{
				flForwardSpeed = pCVcl_forwardspeed->GetFloat();
			}

			flForwardSpeed = MAX(1.0f, flForwardSpeed);

			//Make sure we're not in the blindspot on the crouch detection
			float flStickDistPerc = (pPlayer->GetStickDist() / flForwardSpeed); //Speed is the magnitude
			if (flStickDistPerc > xc_crouch_range.GetFloat())
				return;
		}
	}

	//Toggle the duck
	pPlayer->ToggleDuck();
}

static ConCommand toggle_duck("toggle_duck", CC_ToggleDuck, "Toggles duck");

#ifndef HL2MP
#ifndef PORTAL
LINK_ENTITY_TO_CLASS(player, CHL2_Player);
#endif
#endif

PRECACHE_REGISTER(player);

CBaseEntity* FindEntityForward(CBasePlayer* pMe, bool fHull);

BEGIN_SIMPLE_DATADESC(LadderMove_t)
DEFINE_FIELD(m_bForceLadderMove, FIELD_BOOLEAN),
DEFINE_FIELD(m_bForceMount, FIELD_BOOLEAN),
DEFINE_FIELD(m_flStartTime, FIELD_TIME),
DEFINE_FIELD(m_flArrivalTime, FIELD_TIME),
DEFINE_FIELD(m_vecGoalPosition, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_vecStartPosition, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_hForceLadder, FIELD_EHANDLE),
DEFINE_FIELD(m_hReservedSpot, FIELD_EHANDLE),
END_DATADESC()

//Global Savedata for HL2 player
BEGIN_DATADESC(CHL2_Player)

DEFINE_FIELD(m_nControlClass, FIELD_INTEGER),
DEFINE_EMBEDDED(m_HL2Local),

DEFINE_FIELD(m_bSprintEnabled, FIELD_BOOLEAN),
DEFINE_FIELD(m_flTimeAllSuitDevicesOff, FIELD_TIME),
DEFINE_FIELD(m_fIsSprinting, FIELD_BOOLEAN),
DEFINE_FIELD(m_fIsWalking, FIELD_BOOLEAN),

/*
//These are initialized every time the player calls Activate()
DEFINE_FIELD( m_bIsAutoSprinting, FIELD_BOOLEAN ),
DEFINE_FIELD( m_fAutoSprintMinTime, FIELD_TIME ),
*/

//	Field is used within a single tick, no need to save restore
//DEFINE_FIELD( m_bPlayUseDenySound, FIELD_BOOLEAN ),  
//							m_pPlayerAISquad reacquired on load

DEFINE_AUTO_ARRAY(m_vecMissPositions, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_nNumMissPositions, FIELD_INTEGER),

//					m_pPlayerAISquad
DEFINE_EMBEDDED(m_CommanderUpdateTimer),
//					m_RealTimeLastSquadCommand
DEFINE_FIELD(m_QueuedCommand, FIELD_INTEGER),

DEFINE_FIELD(m_flTimeIgnoreFallDamage, FIELD_TIME),
DEFINE_FIELD(m_bIgnoreFallDamageResetAfterImpact, FIELD_BOOLEAN),

//Suit power fields
DEFINE_FIELD(m_flSuitPowerLoad, FIELD_FLOAT),

DEFINE_FIELD(m_flIdleTime, FIELD_TIME),
DEFINE_FIELD(m_flMoveTime, FIELD_TIME),
DEFINE_FIELD(m_flLastDamageTime, FIELD_TIME),
DEFINE_FIELD(m_flTargetFindTime, FIELD_TIME),

DEFINE_FIELD(m_flAdmireGlovesAnimTime, FIELD_TIME),
DEFINE_FIELD(m_flNextFlashlightCheckTime, FIELD_TIME),
DEFINE_FIELD(m_flFlashlightPowerDrainScale, FIELD_FLOAT),
DEFINE_FIELD(m_bFlashlightDisabled, FIELD_BOOLEAN),

DEFINE_FIELD(m_bUseCappedPhysicsDamageTable, FIELD_BOOLEAN),

DEFINE_FIELD(m_hLockedAutoAimEntity, FIELD_EHANDLE),

DEFINE_EMBEDDED(m_LowerWeaponTimer),
DEFINE_EMBEDDED(m_AutoaimTimer),

DEFINE_INPUTFUNC(FIELD_FLOAT, "IgnoreFallDamage", InputIgnoreFallDamage),
DEFINE_INPUTFUNC(FIELD_FLOAT, "IgnoreFallDamageWithoutReset", InputIgnoreFallDamageWithoutReset),
DEFINE_INPUTFUNC(FIELD_VOID, "OnSquadMemberKilled", OnSquadMemberKilled),
DEFINE_INPUTFUNC(FIELD_VOID, "DisableFlashlight", InputDisableFlashlight),
DEFINE_INPUTFUNC(FIELD_VOID, "EnableFlashlight", InputEnableFlashlight),
DEFINE_INPUTFUNC(FIELD_VOID, "ForceDropPhysObjects", InputForceDropPhysObjects),

DEFINE_SOUNDPATCH(m_sndLeeches),
DEFINE_SOUNDPATCH(m_sndWaterSplashes),

DEFINE_FIELD(m_flArmorReductionTime, FIELD_TIME),
DEFINE_FIELD(m_iArmorReductionFrom, FIELD_INTEGER),

DEFINE_FIELD(m_flTimeUseSuspended, FIELD_TIME),

DEFINE_FIELD(m_hLocatorTargetEntity, FIELD_EHANDLE),

DEFINE_FIELD(m_flTimeNextLadderHint, FIELD_TIME),

//DEFINE_FIELD( m_hPlayerProxy, FIELD_EHANDLE ), //Shut up class check!
//DEFINE_GLOBAL_FIELD(m_bFirstEffect, FIELD_BOOLEAN),
//DEFINE_FIELD(m_flNextEffectTime, FIELD_FLOAT),
//DEFINE_FIELD(m_flEffectThinkRem, FIELD_FLOAT),
//DEFINE_FIELD(m_flNextEffectRem, FIELD_FLOAT),
//DEFINE_FIELD(m_flFirstEffectRem, FIELD_FLOAT),
//DEFINE_FIELD(m_flRemAtLoad, FIELD_FLOAT),
DEFINE_FIELD(m_bRestartHUD, FIELD_BOOLEAN),
//DEFINE_FIELD(m_iChaosSpawnCount, FIELD_INTEGER),

//DEFINE_UTLVECTOR(m_iActiveEffects, FIELD_INTEGER),
DEFINE_AUTO_ARRAY(m_iActiveEffects, FIELD_INTEGER),
//DEFINE_UTLVECTOR(m_iTerminated, FIELD_INTEGER),
DEFINE_INPUTFUNC(FIELD_VOID, "InsideTransition", InputInsideTransition),
//DEFINE_PHYSPTR(m_pController),
END_DATADESC()


CHL2_Player::CHL2_Player()
{
	m_nNumMissPositions = 0;
	m_pPlayerAISquad = 0;
	m_bSprintEnabled = true;
	m_flArmorReductionTime = 0.0f;
	m_iArmorReductionFrom = 0;
}

//
//SUIT POWER DEVICES
//
#define SUITPOWER_CHARGE_RATE	12.5											//100 units in 8 seconds

#ifdef HL2MP
CSuitPowerDevice SuitDeviceSprint(bits_SUIT_DEVICE_SPRINT, 25.0f);				//100 units in 4 seconds
#else
CSuitPowerDevice SuitDeviceSprint(bits_SUIT_DEVICE_SPRINT, 12.5f);				//100 units in 8 seconds
#endif

#ifdef HL2_EPISODIC
CSuitPowerDevice SuitDeviceFlashlight(bits_SUIT_DEVICE_FLASHLIGHT, 1.111);	//100 units in 90 second
#else
CSuitPowerDevice SuitDeviceFlashlight(bits_SUIT_DEVICE_FLASHLIGHT, 2.222);	//100 units in 45 second
#endif
CSuitPowerDevice SuitDeviceBreather(bits_SUIT_DEVICE_BREATHER, 6.7f);		//100 units in 15 seconds (plus three padded seconds)


IMPLEMENT_SERVERCLASS_ST(CHL2_Player, DT_HL2_Player)
SendPropDataTable(SENDINFO_DT(m_HL2Local), &REFERENCE_SEND_TABLE(DT_HL2Local), SendProxy_SendLocalDataTable),
SendPropBool(SENDINFO(m_fIsSprinting)),
END_SEND_TABLE()


void CHL2_Player::Precache(void)
{
	BaseClass::Precache();
	PrecacheModel("models/props_junk/garbage_glassbottle003a.mdl");
	PrecacheModel("models/props_junk/garbage_glassbottle003a_chunk01.mdl");
	PrecacheModel("models/props_junk/garbage_glassbottle003a_chunk02.mdl");
	PrecacheModel("models/props_junk/garbage_glassbottle003a_chunk03.mdl");
	PrecacheModel("models/props_foliage/tree_pine04.mdl");
	PrecacheModel("models/props_c17/oildrum001.mdl");
	UTIL_PrecacheOther("func_tank");

	PrecacheScriptSound("HL2Player.SprintNoPower");
	PrecacheScriptSound("HL2Player.SprintStart");
	PrecacheScriptSound("HL2Player.UseDeny");
	PrecacheScriptSound("HL2Player.FlashLightOn");
	PrecacheScriptSound("HL2Player.FlashLightOff");
	PrecacheScriptSound("HL2Player.PickupWeapon");
	PrecacheScriptSound("HL2Player.TrainUse");
	PrecacheScriptSound("HL2Player.Use");
	PrecacheScriptSound("HL2Player.BurnPain");
	PrecacheScriptSound("*#music/hl2_song3.mp3");
	PrecacheScriptSound("*#music/hl1_song25_remix3.mp3");
}

//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::CheckSuitZoom(void)
{
	//#ifndef _XBOX 
		//Adrian - No zooming without a suit!
	if (IsSuitEquipped())
	{
		if (m_afButtonReleased & IN_ZOOM)
		{
			StopZooming();
		}
		else if (m_afButtonPressed & IN_ZOOM)
		{
			StartZooming();
		}
	}
	//#endif//_XBOX
}

void CHL2_Player::EquipSuit(bool bPlayEffects)
{
	MDLCACHE_CRITICAL_SECTION();
	BaseClass::EquipSuit();

	m_HL2Local.m_bDisplayReticle = true;

	if (bPlayEffects == true)
	{
		StartAdmireGlovesAnimation();
	}
}

void CHL2_Player::RemoveSuit(void)
{
	BaseClass::RemoveSuit();

	m_HL2Local.m_bDisplayReticle = false;
}

void CHL2_Player::HandleSpeedChanges(void)
{
	int buttonsChanged = m_afButtonPressed | m_afButtonReleased;

	bool bCanSprint = CanSprint();
	bool bIsSprinting = IsSprinting();
	bool bWantSprint = (bCanSprint && IsSuitEquipped() && (m_nButtons & IN_SPEED));
	if (bIsSprinting != bWantSprint && (buttonsChanged & IN_SPEED))
	{
		//If someone wants to sprint, make sure they've pressed the button to do so. We want to prevent the
		//case where a player can hold down the sprint key and burn tiny bursts of sprint as the suit recharges
		//We want a full debounce of the key to resume sprinting after the suit is completely drained
		if (bWantSprint)
		{
			if (sv_stickysprint.GetBool())
			{
				StartAutoSprint();
			}
			else
			{
				StartSprinting();
			}
		}
		else
		{
			if (!sv_stickysprint.GetBool())
			{
				StopSprinting();
			}
			//Reset key, so it will be activated post whatever is suppressing it.
			m_nButtons &= ~IN_SPEED;
		}
	}

	bool bIsWalking = IsWalking();
	//have suit, pressing button, not sprinting or ducking
	bool bWantWalking;

	//if( IsSuitEquipped() )
	//{
	bWantWalking = (m_nButtons & IN_WALK) && !IsSprinting() && !(m_nButtons & IN_DUCK);
	//}
	//else
	//{
	//	bWantWalking = true;
	//}

	if (bIsWalking != bWantWalking)
	{
		if (IsSuitEquipped())
		{
			if (bWantWalking)
			{
				StartWalking();
			}
			else
			{
				StopWalking();
			}
		}
		else
		{
			StartWalking();
		}
	}
	bool bIsDucking = IsDucking();
	bool bWantDucking = (m_Local.m_bDucked) && GetGroundEntity() != NULL;
	//if (!bIsSprinting && !bIsWalking)
	//{
	if (bWantDucking)
	{
		StartDucking();
	}
	else if (bIsDucking)
	{
		StopDucking();
	}
	//}
}

//-----------------------------------------------------------------------------
//This happens when we powerdown from the mega physcannon to the regular one
//-----------------------------------------------------------------------------
void CHL2_Player::HandleArmorReduction(void)
{
	if (m_flArmorReductionTime < gpGlobals->curtime)
		return;

	if (ArmorValue() <= 0)
		return;

	float flPercent = 1.0f - ((m_flArmorReductionTime - gpGlobals->curtime) / ARMOR_DECAY_TIME);

	int iArmor = Lerp(flPercent, m_iArmorReductionFrom, 0);

	SetArmorValue(iArmor);
}

//
//about to load a save, but we can still read/write the current world state
//
void CHL2_Player::Event_PreSaveGameLoaded(char const* pSaveName, bool bInGame)
{
	Msg("CHL2_Player::Event_PreSaveGameLoaded [%s] %s\n", pSaveName, bInGame ? "in-game" : "at console");
	//write out to a text file.
	SavePersistEnts();
	//might end up not being able to see anything
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_lockpvs 0; exec portalsopenall\n");
}
int CHL2_Player::FindWeightSum()
{
	int iWeightSum = 0;
	CChaosEffect* pEffect;
	for (int i = 1; i < NUM_EFFECTS; i++)
	{
		pEffect = g_ChaosEffects[i];
		if (g_ChaosEffects[i]->WasShufflePicked())
			continue;
		if (chaos_print_rng.GetBool()) Msg("i %i, %s %i += %i\n", i, STRING(pEffect->m_strGeneralName), iWeightSum, pEffect->m_iCurrentWeight);
		iWeightSum += pEffect->m_iCurrentWeight;
		//recover weight for recent effects
		//add a fraction of the maximum weight on every interval
		//if (!EffectOrGroupAlreadyActive(i) && pEffect->m_iCurrentWeight < pEffect->m_iMaxWeight)
		//	pEffect->m_iCurrentWeight = min(pEffect->m_iCurrentWeight + pEffect->m_iMaxWeight * 0.2, pEffect->m_iMaxWeight);
	}
	return iWeightSum;
}
void CHL2_Player::ResetVotes(int iWeightSum)
{
	g_iVoteNumber++;
	ClearEffectContextCache();
	//choose effects to nominate
	for (int i = 0; i < 4; i++)
	{
		g_arriVoteEffects[i] = PickEffect(iWeightSum);
		Assert(g_arriVoteEffects[i] != EFFECT_ERROR);
		g_arriVotes[i] = 0;
	}
}

int GetVoteWinnerEffect()
{
	int bestVotes = -1;
	int bestEffect = 0;
	for (int i = 0; i < 4; i++)
	{
		if (g_arriVotes[i] > bestVotes)
		{
			bestVotes = g_arriVotes[i];
			bestEffect = g_arriVoteEffects[i];
		}
		g_ChaosEffects[g_arriVoteEffects[i]]->m_bInVoteList = false;
	}
	return bestEffect;
}
ConVar dvdcross_forcetocorner("dvdcross_forcetocorner", "0");
ConVar dvdcross("dvdcross", "0");
void CHL2_Player::CrosshairBounceMotion()
{
	//DVD Crosshair
	if (dvdcross_forcetocorner.GetBool())
	{
		m_CrosshairOffset.x = -1;
		m_CrosshairOffset.y = -0.75;
	}
	else if (dvdcross.GetBool())
	{
		if (m_CrosshairOffset.x < -1)
			m_bCrosshairMoveX = false;
		if (m_CrosshairOffset.x > 1)
			m_bCrosshairMoveX = true;
		if (m_CrosshairOffset.y < -0.75)
			m_bCrosshairMoveY = false;
		if (m_CrosshairOffset.y > 0.75)
			m_bCrosshairMoveY = true;

		if (m_bCrosshairMoveX)
			m_CrosshairOffset.x -= .01;
		else
			m_CrosshairOffset.x += .01;

		if (m_bCrosshairMoveY)
			m_CrosshairOffset.y -= .01;
		else
			m_CrosshairOffset.y += .01;
	}
	if (dvdcross.GetBool())
	{
		Vector forward, right, up, vecpitchbound, vecyawbound;
		AngleVectors(EyeAngles() + m_Local.m_vecPunchAngle, &forward);
		QAngle angpitchbound = QAngle((float)(m_iDefaultFOV * 0.75f), 0, 0);
		AngleVectors(angpitchbound, &vecpitchbound);
		QAngle angyawbound = QAngle(0, (float)(m_iDefaultFOV), 0);
		AngleVectors(angyawbound, &vecyawbound);
		VectorVectors(forward, right, up);
		//Msg("offset %0.1f %0.1f vecpitchbound %f %f %f L %f vecyawbound %f %f %f L %f\n", m_CrosshairOffset.x, m_CrosshairOffset.y, vecpitchbound.x, vecpitchbound.y, vecpitchbound.z, vecpitchbound.Length(), vecyawbound.x, vecyawbound.y, vecyawbound.z, vecyawbound.Length());
		//i don't know what i'm doing in here, but a cubic function fixes a problem where the offset doesn't scale correctly to the FOV. works best between 80 and 135 fov
		float flScale = 0.00000799327 * (float)m_iDefaultFOV * (float)m_iDefaultFOV * (float)m_iDefaultFOV - 0.00213912 * (float)m_iDefaultFOV * (float)m_iDefaultFOV + 0.205545 * (float)m_iDefaultFOV - 5.9739;
		Vector term1 = (m_CrosshairOffset.x * vecyawbound.Length2D() * right);
		term1 *= flScale;
		Vector term2 = (m_CrosshairOffset.y * vecpitchbound.z * up);
		term2 *= flScale;
		Vector vCrosshairDir = forward + term1 + term2;
		QAngle ang;
		VectorAngles(vCrosshairDir, ang);
		if (ang.y > 180)
			ang.y -= 360;
		if (ang.y < -180)
			ang.y += 360;
		m_vOffsetedCrosshairDir = ang - (EyeAngles() + m_Local.m_vecPunchAngle);
		//Msg("TERM 1: %f * %f * (%f %f %f) = (%f %f %f)\n", m_CrosshairOffset.x, vecyawbound.Length2D(), right.x, right.y, right.z, term1.x, term1.y, term1.z);
		//Msg("(%f %f %f) + (%f %f %f) + (%f %f %f) = (%f %f %f)\n", forward.x, forward.y, forward.z, term1.x, term1.y, term1.z, term2.x, term2.y, term2.z, offsetCrosshairDirection.x, offsetCrosshairDirection.y, offsetCrosshairDirection.z);
		//NDebugOverlay::Cross3D(EyePosition() + vCrosshairDir * 60, 1, 255, 255, 255, true, 1);
		//NDebugOverlay::Line(EyePosition(), EyePosition() + vCrosshairDir * 60, 255, 255, 255, true, 1);
	}
	else
	{
		m_vOffsetedCrosshairDir = vec3_angle;
		m_CrosshairOffset = vec3_origin;
	}
}
//-----------------------------------------------------------------------------
//Purpose: Allow pre-frame adjustments on the player
//-----------------------------------------------------------------------------
void CHL2_Player::PreThink(void)
{
	if (g_arriVoteEffects[0] == 0 && chaos_vote_enable.GetBool())
	{
		// if haven't been set yet...
		int iWeightSum = FindWeightSum();
		ResetVotes(iWeightSum);
	}
	if (gpGlobals->frametime != 0)//don't do anything if game is paused
	{
		if (chaos.GetBool())
		{
			if (m_bRestartHUD)
			{
#ifdef DEBUG
				UTIL_CenterPrintAll("DEBUG BUILD. If you're seeing this and don't know why, please tell Pinsplash.\n");
#endif
				m_bRestartHUD = false;
				g_flEffectThinkRem = 0;
			}
			bool bNoEffectsOn = true;
			for (int i = 0; g_ChaosEffects.Size() >= i + 1; i++)
			{
				if (g_ChaosEffects[i]->m_bActive)
					bNoEffectsOn = false;
			}
			if (bNoEffectsOn && g_flNextEffectRem <= -1)
			{
				g_flEffectThinkRem = 0;
				//m_bFirstEffect = true;
				g_flNextEffectRem = chaos_effect_interval.GetFloat();
				//m_flFirstEffectRem = 0;
				m_bRestartHUD = true;//Fixes bar not showing when map is loaded if chaos was enabled when no map was "on" at the time. Stupid but works.
			}
			if (g_flNextEffectRem <= 0 && !pl.deadflag)//don't start new effects when dead
			{
				engine->ClientCommand(engine->PEntityOfEntIndex(1), "sv_cheats 1");//always force cheats on to ensure everything works
				int nID = 0;
				int iWeightSum = FindWeightSum();
				if (!chaos_vote_enable.GetBool())
				{
					ClearEffectContextCache();
					nID = PickEffect(iWeightSum);
					Assert(nID != EFFECT_ERROR);
				}
				else
				{
					nID = GetVoteWinnerEffect();
					Assert(nID != EFFECT_ERROR);
					//TODO: ResetVotes should probably be in here
				}
				g_flEffectThinkRem = 0;
				//start effect
				StartGivenEffect(nID);
				if (chaos_vote_enable.GetBool())
					ResetVotes(iWeightSum);
				if (GetMoveType() != MOVETYPE_NOCLIP && chaos_unstuck_neweffect.GetBool())
					GetUnstuck(500);
			}
		}
		else if (chaos_instant_off.GetBool())
		{
			//chaos was turned off
			for (int i = 0; g_ChaosEffects.Size() >= i + 1; i++)
			{
				g_ChaosEffects[i]->AbortEffect();
				//g_ActiveEffects.Remove(i);
			}
			//m_iActiveEffects.RemoveAll();
			//g_iActiveEffects.RemoveAll();
			for (int k = 0; k < MAX_ACTIVE_EFFECTS; k++)
			{
				m_iActiveEffects[k] = NULL;
				g_iActiveEffects[k] = NULL;
			}
			//some transient effects can technically last indefinitely (like spawning an npc)
			//some such effects (like spawning a weapon) are not worth the hassle of accurately tracking and disposing of. this code is more about function than form.
			CBaseEntity* pEnt = gEntList.FirstEnt();
			while (pEnt)
			{
				if (pEnt->m_bChaosSpawned || pEnt->m_bChaosPersist)
					pEnt->SUB_Remove();
				pEnt = gEntList.NextEnt(pEnt);
			}
		}
		CrosshairBounceMotion();
		float flTimeScale = cvar->FindVar("host_timescale")->GetFloat();
		if (!pl.deadflag)
		{
			//you may think this is dumb, but the alternative is a bunch of arithmetic involving the effect durations, start times, and the curtime both before and after loading new states
			//and you must consider dying, miscellaneous fails, manual reloads, and level transitions, including transitions to prior levels.
			g_flEffectThinkRem -= gpGlobals->interval_per_tick / flTimeScale;
			g_flNextEffectRem -= gpGlobals->interval_per_tick / flTimeScale;
			if (chaos.GetBool())
				DoChaosHUDBar();
			DoChaosHUDText();
		}
		bool bResetMaintainTimer = false;
		for (int i = 0; g_ChaosEffects.Size() >= i + 1; i++)
		{
			if (!g_ChaosEffects[i]->m_bActive)
				continue;
			//CChaosEffect pEffect = g_ActiveEffects[i];
			if (!pl.deadflag)
				g_ChaosEffects[i]->m_flTimeRem -= gpGlobals->interval_per_tick / flTimeScale;//don't progress timer when dead to avoid confusion
			g_ChaosEffects[i]->FastThink();
			if (g_flEffectThinkRem <= 0)
			{
				g_ChaosEffects[i]->MaintainEffect();
				bResetMaintainTimer = true;
			}
			if (g_ChaosEffects[i]->m_flTimeRem <= 0 && !pl.deadflag)//stop effects that are expiring, unless dead cause that's cheating
			{
				StopGivenEffect(g_ChaosEffects[i]->m_nID);
			}
		}
		if (bResetMaintainTimer)
		{
			MaintainEvils();
			g_flEffectThinkRem = 1;
		}
	}
	//if we're in a vehicle, we can't be drowning/breathing underwater no matter what
	//this makes swim in air more manageable
	//we don't keep this in CESwimInAir::FastThink because that only runs when the effect is active, so if the effect was on during a save, but off when loading, the game will still act like you're underwater
	if (IsInAVehicle())
	{
		SetWaterLevel(WL_NotInWater);
		SetPlayerUnderwater(false);
		RemoveFlag(FL_INWATER);
		RemoveFlag(FL_SWIM);
	}
	if (player_showpredictedposition.GetBool())
	{
		Vector	predPos;

		UTIL_PredictedPosition(this, player_showpredictedposition_timestep.GetFloat(), &predPos);

		NDebugOverlay::Box(predPos, NAI_Hull::Mins(GetHullType()), NAI_Hull::Maxs(GetHullType()), 0, 255, 0, 0, 0.01f);
		NDebugOverlay::Line(GetAbsOrigin(), predPos, 0, 255, 0, 0, 0.01f);
	}

	if (m_hLocatorTargetEntity != NULL)
	{
		//Keep track of the entity here, the client will pick up the rest of the work
		m_HL2Local.m_vecLocatorOrigin = m_hLocatorTargetEntity->WorldSpaceCenter();
	}
	else
	{
		m_HL2Local.m_vecLocatorOrigin = vec3_invalid; //This tells the client we have no locator target.
	}

	//Riding a vehicle?
	if (IsInAVehicle())
	{
		VPROF("CHL2_Player::PreThink-Vehicle");
		//make sure we update the client, check for timed damage and update suit even if we are in a vehicle
		UpdateClientData();
		CheckTimeBasedDamage();

		//Allow the suit to recharge when in the vehicle.
		SuitPower_Update();
		CheckSuitUpdate();
		CheckSuitZoom();

		WaterMove();
		return;
	}

	//This is an experiment of mine- autojumping! 
	//only affects you if sv_autojump is nonzero.
	if ((GetFlags() & FL_ONGROUND) && sv_autojump.GetFloat() != 0)
	{
		VPROF("CHL2_Player::PreThink-Autojump");
		//check autojump
		Vector vecCheckDir;

		vecCheckDir = GetAbsVelocity();

		float flVelocity = VectorNormalize(vecCheckDir);

		if (flVelocity > 200)
		{
			//Going fast enough to autojump
			vecCheckDir = WorldSpaceCenter() + vecCheckDir * 34 - Vector(0, 0, 16);

			trace_t tr;

			UTIL_TraceHull(WorldSpaceCenter() - Vector(0, 0, 16), vecCheckDir, NAI_Hull::Mins(HULL_TINY_CENTERED), NAI_Hull::Maxs(HULL_TINY_CENTERED), MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr);

			//NDebugOverlay::Line( tr.startpos, tr.endpos, 0,255,0, true, 10 );

			if (tr.fraction == 1.0 && !tr.startsolid)
			{
				//Now trace down!
				UTIL_TraceLine(vecCheckDir, vecCheckDir - Vector(0, 0, 64), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr);

				//NDebugOverlay::Line( tr.startpos, tr.endpos, 0,255,0, true, 10 );

				if (tr.fraction == 1.0 && !tr.startsolid)
				{
					//!!!HACKHACK
					//I KNOW, I KNOW, this is definitely not the right way to do this,
					//but I'm prototyping! (sjb)
					Vector vecNewVelocity = GetAbsVelocity();
					vecNewVelocity.z += 250;
					SetAbsVelocity(vecNewVelocity);
				}
			}
		}
	}

	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-Speed");
	HandleSpeedChanges();
#ifdef HL2_EPISODIC
	HandleArmorReduction();
#endif

	if (sv_stickysprint.GetBool() && m_bIsAutoSprinting)
	{
		//If we're ducked and not in the air
		if (IsDucked() && GetGroundEntity() != NULL)
		{
			StopSprinting();
		}
		//Stop sprinting if the player lets off the stick for a moment.
		else if (GetStickDist() == 0.0f)
		{
			if (gpGlobals->curtime > m_fAutoSprintMinTime)
			{
				StopSprinting();
			}
		}
		else
		{
			//Stop sprinting one half second after the player stops inputting with the move stick.
			m_fAutoSprintMinTime = gpGlobals->curtime + 0.5f;
		}
	}
	else if (IsSprinting())
	{
		//Disable sprint while ducked unless we're in the air (jumping)
		if (IsDucked() && (GetGroundEntity() != NULL))
		{
			StopSprinting();
		}
	}

	VPROF_SCOPE_END();

	if (g_fGameOver || IsPlayerLockedInPlace())
		return;         //finale

	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-ItemPreFrame");
	ItemPreFrame();
	VPROF_SCOPE_END();

	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-WaterMove");
	WaterMove();
	VPROF_SCOPE_END();

	if (g_pGameRules && g_pGameRules->FAllowFlashlight())
		m_Local.m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_Local.m_iHideHUD |= HIDEHUD_FLASHLIGHT;


	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-CommanderUpdate");
	CommanderUpdate();
	VPROF_SCOPE_END();

	//Operate suit accessories and manage power consumption/charge
	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-SuitPower_Update");
	SuitPower_Update();
	VPROF_SCOPE_END();

	//checks if new client data (for HUD and view control) needs to be sent to the client
	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-UpdateClientData");
	UpdateClientData();
	VPROF_SCOPE_END();

	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-CheckTimeBasedDamage");
	CheckTimeBasedDamage();
	VPROF_SCOPE_END();

	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-CheckSuitUpdate");
	CheckSuitUpdate();
	VPROF_SCOPE_END();

	VPROF_SCOPE_BEGIN("CHL2_Player::PreThink-CheckSuitZoom");
	CheckSuitZoom();
	VPROF_SCOPE_END();

	if (m_lifeState >= LIFE_DYING)
	{
		PlayerDeathThink();
		return;
	}

#ifdef HL2_EPISODIC
	CheckFlashlight();
#endif	//HL2_EPISODIC

	//So the correct flags get sent to client asap.
	//
	if (m_afPhysicsFlags & PFLAG_DIROVERRIDE)
		AddFlag(FL_ONTRAIN);
	else
		RemoveFlag(FL_ONTRAIN);

	//Train speed control
	if (m_afPhysicsFlags & PFLAG_DIROVERRIDE)
	{
		CBaseEntity* pTrain = GetGroundEntity();
		float vel;

		if (pTrain)
		{
			if (!(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE))
				pTrain = NULL;
		}

		if (!pTrain)
		{
			if (GetActiveWeapon() && (GetActiveWeapon()->ObjectCaps() & FCAP_DIRECTIONAL_USE))
			{
				m_iTrain = TRAIN_ACTIVE | TRAIN_NEW;

				if (m_nButtons & IN_FORWARD)
				{
					m_iTrain |= TRAIN_FAST;
				}
				else if (m_nButtons & IN_BACK)
				{
					m_iTrain |= TRAIN_BACK;
				}
				else
				{
					m_iTrain |= TRAIN_NEUTRAL;
				}
				return;
			}
			else
			{
				trace_t trainTrace;
				//Maybe this is on the other side of a level transition
				UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -38),
					MASK_PLAYERSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trainTrace);

				if (trainTrace.fraction != 1.0 && trainTrace.m_pEnt)
					pTrain = trainTrace.m_pEnt;


				if (!pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(this))
				{
					//					Warning( "In train mode with no train!\n" );
					m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
					m_iTrain = TRAIN_NEW | TRAIN_OFF;
					return;
				}
			}
		}
		else if (!(GetFlags() & FL_ONGROUND) || pTrain->HasSpawnFlags(SF_TRACKTRAIN_NOCONTROL) || (m_nButtons & (IN_MOVELEFT | IN_MOVERIGHT)))
		{
			//Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
			m_iTrain = TRAIN_NEW | TRAIN_OFF;
			return;
		}

		SetAbsVelocity(vec3_origin);
		vel = 0;
		if (m_afButtonPressed & IN_FORWARD)
		{
			vel = 1;
			pTrain->Use(this, this, USE_SET, (float)vel);
		}
		else if (m_afButtonPressed & IN_BACK)
		{
			vel = -1;
			pTrain->Use(this, this, USE_SET, (float)vel);
		}

		if (vel)
		{
			m_iTrain = TrainSpeed(pTrain->m_flSpeed, ((CFuncTrackTrain*)pTrain)->GetMaxSpeed());
			m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;
		}
	}
	else if (m_iTrain & TRAIN_ACTIVE)
	{
		m_iTrain = TRAIN_NEW; //turn off train
	}


	//
	//If we're not on the ground, we're falling. Update our falling velocity.
	//
	if (!(GetFlags() & FL_ONGROUND))
	{
		m_Local.m_flFallVelocity = -GetAbsVelocity().z;
	}

	if (m_afPhysicsFlags & PFLAG_ONBARNACLE)
	{
		bool bOnBarnacle = false;
		CNPC_Barnacle* pBarnacle = NULL;
		do
		{
			//FIXME: Not a good or fast solution, but maybe it will catch the bug!
			pBarnacle = (CNPC_Barnacle*)gEntList.FindEntityByClassname(pBarnacle, "npc_barnacle");
			if (pBarnacle)
			{
				if (pBarnacle->GetEnemy() == this)
				{
					bOnBarnacle = true;
				}
			}
		} while (pBarnacle);

		if (!bOnBarnacle)
		{
			Warning("Attached to barnacle?\n");
			Assert(0);
			m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
		}
		else
		{
			SetAbsVelocity(vec3_origin);
		}
	}
	//StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	//Update weapon's ready status
	UpdateWeaponPosture();

	//Disallow shooting while zooming
	if (IsX360())
	{
		if (IsZooming())
		{
			if (GetActiveWeapon() && !GetActiveWeapon()->IsWeaponZoomed())
			{
				//If not zoomed because of the weapon itself, do not attack.
				m_nButtons &= ~(IN_ATTACK | IN_ATTACK2);
			}
		}
	}
	else
	{
		if (m_nButtons & IN_ZOOM)
		{
			//FIXME: Held weapons like the grenade get sad when this happens
#ifdef HL2_EPISODIC
		//Episodic allows players to zoom while using a func_tank
			CBaseCombatWeapon* pWep = GetActiveWeapon();
			if (!m_hUseEntity || (pWep && pWep->IsWeaponVisible()))
#endif
				m_nButtons &= ~(IN_ATTACK | IN_ATTACK2);
		}
	}
}

void CHL2_Player::PostThink(void)
{
	BaseClass::PostThink();

	if (!g_fGameOver && !IsPlayerLockedInPlace() && IsAlive())
	{
		HandleAdmireGlovesAnimation();
	}
}

void CHL2_Player::StartAdmireGlovesAnimation(void)
{
	MDLCACHE_CRITICAL_SECTION();
	CBaseViewModel* vm = GetViewModel(0);

	if (vm && !GetActiveWeapon())
	{
		vm->SetWeaponModel("models/weapons/v_hands.mdl", NULL);
		ShowViewModel(true);

		int	idealSequence = vm->SelectWeightedSequence(ACT_VM_IDLE);

		if (idealSequence >= 0)
		{
			vm->SendViewModelMatchingSequence(idealSequence);
			m_flAdmireGlovesAnimTime = gpGlobals->curtime + vm->SequenceDuration(idealSequence);
		}
	}
}

void CHL2_Player::HandleAdmireGlovesAnimation(void)
{
	CBaseViewModel* pVM = GetViewModel();

	if (pVM && pVM->GetOwningWeapon() == NULL)
	{
		if (m_flAdmireGlovesAnimTime != 0.0)
		{
			if (m_flAdmireGlovesAnimTime > gpGlobals->curtime)
			{
				pVM->m_flPlaybackRate = 1.0f;
				pVM->StudioFrameAdvance();
			}
			else if (m_flAdmireGlovesAnimTime < gpGlobals->curtime)
			{
				m_flAdmireGlovesAnimTime = 0.0f;
				pVM->SetWeaponModel(NULL, NULL);
			}
		}
	}
	else
		m_flAdmireGlovesAnimTime = 0.0f;
}

#define HL2PLAYER_RELOADGAME_ATTACK_DELAY 1.0f

/*float CHL2_Player::GetElapsedEffectTime()
{
	float flLowestElapsed = 1000;
	for (int i = 0; g_ActiveEffects.Size() >= i + 1; i++)
	{
		float flElapsedTime = m_flTimeAtLoad - g_ActiveEffects[i].m_flStartTime;
		if (flElapsedTime < flLowestElapsed)
			flLowestElapsed = flElapsedTime;
	}
	return flLowestElapsed;
}*/

/*void CHL2_Player::RewindEffectTimes(float flExtraTime)
{
	Msg("CHL2_Player::RewindEffectTimes\n");
	m_flNextEffectTime = chaos_effect_interval.GetFloat() - GetElapsedEffectTime() + flExtraTime;
	for (int i = 0; g_ActiveEffects.Size() >= i + 1; i++)
	{
		float flEffectBulkTime = (g_ActiveEffects.Size() - 1 - i) * chaos_effect_interval.GetFloat();//extra time to add on to effect duration based on how many turns it is away from expiring
		//using chaos_effect_interval here is okay even for non-standard lengths as their position in the list still indicates their age. a 15-second effect could only be on the tail if interval is 30 for instance (assuming no manual adds)

		//0 * 30 = 0
		//1 * 30 = 30
		//2 * 30 = 60

		//m_ActiveEffects[m_ActiveEffects.Size() - 1 - i].m_flDuration = chaos_effect_interval.GetFloat() - GetElapsedEffectTime() + flEffectBulkTime;//hit elements in reverse order
		//Msg("%s duration changed to %0.1f\n", STRING(m_ActiveEffects[m_ActiveEffects.Size() - 1 - i].m_strHudName), m_ActiveEffects[m_ActiveEffects.Size() - 1 - i].m_flDuration);

		g_ActiveEffects[i].m_flStartTime = gpGlobals->curtime - (GetElapsedEffectTime() + flEffectBulkTime);
		Msg("%s start time changed to %0.1f - (%0.1f + %0.1f) = %0.1f\n", STRING(g_ActiveEffects[i].m_strHudName), gpGlobals->curtime, GetElapsedEffectTime(), flEffectBulkTime, g_ActiveEffects[i].m_flStartTime);
	}
	m_flFirstEffectTime = m_flNextEffectTime - chaos_effect_interval.GetFloat();
}*/

void CHL2_Player::InputInsideTransition(inputdata_t& inputdata)
{
	//level transitions put the server timer (gpGlobals->curtime) back to 0, so we must account for that here
	//RewindEffectTimes(0);
	//m_flRemAtLoad = gpGlobals->curtime;
}

void CHL2_Player::Activate(void)
{
	StartGame();
	m_bRestartHUD = true;
	//STOP TRYING TO DO MAPLOAD_NEWGAME STUFF HERE DUMMY. GO TO SPAWN()
	if (gpGlobals->eLoadType == MapLoad_LoadGame)
	{
		//chaos_strike_max strikes, yer out
		for (int i = 0; g_ChaosEffects.Size() >= i + 1; i++)
		{
			CChaosEffect* pEffect = g_ChaosEffects[i];
			if (pEffect->m_iStrikes >= chaos_strike_max.GetInt())
			{
				Warning("Effect %s reached strike %i and was aborted\n", STRING(pEffect->m_strGeneralName), pEffect->m_iStrikes);
				//pEffect->StopEffect();//can't use, we need to tell all the other code that the effect is over. i feel like this used to be StopGivenEffect() originally but it was changed for some unknown reason
				StopGivenEffect(i);
			}
		}
		//TODO: test again, death water kills instantly on relaod because water world was on at the time of save, even though it's off during load
		//
		//SetWaterLevel(WL_NotInWater);
		//SetPlayerUnderwater(false);
		//RemoveFlag(FL_INWATER);
		//RemoveFlag(FL_SWIM);

		ReplaceEffects();
		RemoveDeadEnts();
		SpawnStoredEnts();
	}
	//Tell effects that we're on a new map so they can rebuild themselves or whatever
	if (gpGlobals->eLoadType == MapLoad_Transition)
	{
		for (int i = 0; g_ChaosEffects.Size() >= i + 1; i++)
		{
			if (!g_ChaosEffects[i]->m_bActive)
				continue;
			g_flEffectThinkRem = 0;
			CChaosEffect* pEffect = g_ChaosEffects[i];
			pEffect->TransitionEffect();
		}
		g_bAvoidExtreme = false;
	}
	BaseClass::Activate();
	InitSprinting();

#ifdef HL2_EPISODIC

	//Delay attacks by 1 second after loading a game.
	if (GetActiveWeapon())
	{
		float flRemaining = GetActiveWeapon()->m_flNextPrimaryAttack - gpGlobals->curtime;

		if (flRemaining < HL2PLAYER_RELOADGAME_ATTACK_DELAY)
		{
			GetActiveWeapon()->m_flNextPrimaryAttack = gpGlobals->curtime + HL2PLAYER_RELOADGAME_ATTACK_DELAY;
		}

		flRemaining = GetActiveWeapon()->m_flNextSecondaryAttack - gpGlobals->curtime;

		if (flRemaining < HL2PLAYER_RELOADGAME_ATTACK_DELAY)
		{
			GetActiveWeapon()->m_flNextSecondaryAttack = gpGlobals->curtime + HL2PLAYER_RELOADGAME_ATTACK_DELAY;
		}
	}

#endif

	GetPlayerProxy();
}
void CHL2_Player::ReplaceEffects()
{
	//m_iActiveEffects: effects that were active at the time of saving
	//g_iActiveEffects: effects at time of loading
	//these two must ALWAYS be identical in all places except this.

	//copy load list into temp list because it will get trampled on by AbortEffect
	int iEffects[MAX_ACTIVE_EFFECTS];
	for (int n = 0; n < MAX_ACTIVE_EFFECTS; n++)
	{
		iEffects[n] = g_iActiveEffects[n];
	}
	//get rid of effects that existed at the time of save that are no longer active
	//for (int l = 0; m_iActiveEffects.Size() >= l + 1; l++)
	for (int l = 0; l < MAX_ACTIVE_EFFECTS; l++)
	{
		if (!m_iActiveEffects[l])
			continue;
		int nID = m_iActiveEffects[l];
		CChaosEffect* pEffect = g_ChaosEffects[nID];

		//UNDONE: unless that certain effect happens to be active from the abandoned game state as well!
		//it's not enough to abort only the now-inactives. if an effect expired after saving but before loading, this check will let the effect linger.
		//if (!pEffect->m_bActive)
		{
			if (pEffect->DoRestorationAbort())
			{
				Msg("Killing effect %s\n", STRING(pEffect->m_strHudName));
				pEffect->StopEffect();
			}
		}
		m_iActiveEffects[l] = NULL;
	}
	//now we have to restore the effects that were there at the time of load
	//for (int m = 0; g_iActiveEffects.Size() >= m + 1; m++)
	for (int m = 0; m < MAX_ACTIVE_EFFECTS; m++)
	{
		if (!iEffects[m])
			continue;
		CChaosEffect* pEffect = g_ChaosEffects[iEffects[m]];

		//yes this is necessary. g_ChaosEffects is the final authority on activeness
		if (!pEffect->m_bActive)
			continue;

		if (pEffect->DoRestorationAbort())
		{
			Msg("Restoring effect %s\n", STRING(pEffect->m_strHudName));
			pEffect->RestoreEffect();
		}
		//make two lists equal again
		m_iActiveEffects[m] = iEffects[m];
		g_iActiveEffects[m] = iEffects[m];
	}
	g_bGoBackLevel = false;
	//make two lists equal again
	//m_iActiveEffects = g_iActiveEffects;
}

//
//the save may contain certain persist entities that were killed in the now-destroyed timeline that gordon created.
//we would like for those entities to not reappear.
//
void CHL2_Player::RemoveDeadEnts()
{
	//find entity with matching int and replace it with our entity in the global
	CBaseEntity* pDeadEnt = gEntList.FirstEnt();
	while (pDeadEnt)
	{
		if (pDeadEnt->m_bChaosPersist)
		{
			for (int kID = 0; g_iTerminated.Size() >= kID + 1; kID++)
			{
				//Warning("Does id %i (name '%s') match dead id? %i\n", pDeadEnt->m_iChaosID, STRING(pDeadEnt->GetEntityName()), g_iTerminated[kID]);
				if (g_iTerminated[kID] == pDeadEnt->m_iChaosID)
				{
					//Msg("Removing dead persist entity %i\n", g_iTerminated[kID]);
					variant_t value;
					g_EventQueue.AddEvent(pDeadEnt, "Kill", value, 0.01f, this, this);
					break;
				}
			}
		}
		else
		{
			//Msg("Entity '%s' was not chaos-persist\n", STRING(pDeadEnt->GetEntityName()));
		}
		pDeadEnt = gEntList.NextEnt(pDeadEnt);
	}
}
//
//spawn certain entities that transcend time
//
void CHL2_Player::SpawnStoredEnts()
{
	for (int j = 0; g_PersistEnts.Size() >= j + 1; j++)
	{
		//Msg("Entity block\n");
		CBaseEntity* pDupeEnt = gEntList.FirstEnt();
		while (pDupeEnt)
		{
			if (pDupeEnt->m_bChaosPersist)
			{
				//Msg("Is chaosid %i (name '%s') what we have on file? %i\n", pDupeEnt->m_iChaosID, STRING(pDupeEnt->GetEntityName()), g_PersistEnts[j]->chaosid);
				if (pDupeEnt->m_iChaosID == g_PersistEnts[j]->chaosid)
				{
					//Msg("Found duplicate entity %s with chaosid %i, replacing with saved version\n", STRING(pDupeEnt->GetEntityName()), pDupeEnt->m_iChaosID);
					pDupeEnt->SUB_Remove();
					break;
				}
			}
			else
			{
				//Msg("Entity '%s' was not chaos-persist\n", STRING(pDupeEnt->GetEntityName()));
			}
			pDupeEnt = gEntList.NextEnt(pDupeEnt);
		}
		CChaosStoredEnt* pStored = g_PersistEnts[j];
		CBaseEntity* pEnt = RetrieveStoredEnt(pStored, true);
		if (pEnt)
		{
			DispatchSpawn(pEnt);
			pEnt->Activate();//according to some assert, we don't need this
			//pEnt->Teleport(&vecOrigin, &vecAngle, NULL);
			//this has to be done post spawn... ick
			if (pStored->poisonzombie)
			{
				CNPC_PoisonZombie* pPZombie = dynamic_cast<CNPC_PoisonZombie*>(pEnt);
				pPZombie->EnableCrab(0, pStored->crabs[0]);
				pPZombie->EnableCrab(1, pStored->crabs[1]);
				pPZombie->EnableCrab(2, pStored->crabs[2]);
			}
		}
	}
}
//------------------------------------------------------------------------------
//Purpose :
//Input   :
//Output  :
//------------------------------------------------------------------------------
Class_T  CHL2_Player::Classify(void)
{
	//If player controlling another entity?  If so, return this class
	if (m_nControlClass != CLASS_NONE)
	{
		return m_nControlClass;
	}
	else
	{
		if (IsInAVehicle())
		{
			IServerVehicle* pVehicle = GetVehicle();
			return pVehicle->ClassifyPassenger(this, CLASS_PLAYER);
		}
		else
		{
			return CLASS_PLAYER;
		}
	}
}

//-----------------------------------------------------------------------------
//Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
//Input  :  Constant for the type of interaction
//Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CHL2_Player::HandleInteraction(int interactionType, void* data, CBaseCombatCharacter* sourceEnt)
{
	if (interactionType == g_interactionBarnacleVictimDangle)
		return false;

	if (interactionType == g_interactionBarnacleVictimReleased)
	{
		m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
		SetMoveType(MOVETYPE_WALK);
		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimGrab)
	{
#ifdef HL2_EPISODIC
		CNPC_Alyx* pAlyx = CNPC_Alyx::GetAlyx();
		if (pAlyx)
		{
			//Make Alyx totally hate this barnacle so that she saves the player.
			int priority;

			priority = pAlyx->IRelationPriority(sourceEnt);
			pAlyx->AddEntityRelationship(sourceEnt, D_HT, priority + 5);
		}
#endif//HL2_EPISODIC

		m_afPhysicsFlags |= PFLAG_ONBARNACLE;
		ClearUseEntity();
		return true;
	}
	return false;
}


void CHL2_Player::PlayerRunCommand(CUserCmd* ucmd, IMoveHelper* moveHelper)
{
	//Handle FL_FROZEN.
	if (m_afPhysicsFlags & PFLAG_ONBARNACLE)
	{
		ucmd->forwardmove = 0;
		ucmd->sidemove = 0;
		ucmd->upmove = 0;
		ucmd->buttons &= ~IN_USE;
	}

	//Can't use stuff while dead
	if (IsDead())
	{
		ucmd->buttons &= ~IN_USE;
	}

	//Update our movement information
	if ((ucmd->forwardmove != 0) || (ucmd->sidemove != 0) || (ucmd->upmove != 0))
	{
		m_flIdleTime -= TICK_INTERVAL * 2.0f;

		if (m_flIdleTime < 0.0f)
		{
			m_flIdleTime = 0.0f;
		}

		m_flMoveTime += TICK_INTERVAL;

		if (m_flMoveTime > 4.0f)
		{
			m_flMoveTime = 4.0f;
		}
	}
	else
	{
		m_flIdleTime += TICK_INTERVAL;

		if (m_flIdleTime > 4.0f)
		{
			m_flIdleTime = 4.0f;
		}

		m_flMoveTime -= TICK_INTERVAL * 2.0f;

		if (m_flMoveTime < 0.0f)
		{
			m_flMoveTime = 0.0f;
		}
	}

	//Msg("Player time: [ACTIVE: %f]\t[IDLE: %f]\n", m_flMoveTime, m_flIdleTime );

	BaseClass::PlayerRunCommand(ucmd, moveHelper);
}
void CHL2_Player::StartGame()
{
	if (gpGlobals->eLoadType == MapLoad_NewGame || gpGlobals->eLoadType == MapLoad_Background)
		PopulateEffects();
	gEntList.AddListenerEntity(this);
	const char* pMapName = STRING(gpGlobals->mapname);
	//scripting of canals 11 requires an airboat to be present, give player a new one if they came here without one
	if (!Q_strcmp(pMapName, "d1_canals_11"))
	{
		if (!IsInAVehicle() && gpGlobals->eLoadType != MapLoad_NewGame)
		{
			g_ChaosEffects[EFFECT_SPAWN_VEHICLE]->ChaosSpawnVehicle("prop_vehicle_airboat", MAKE_STRING("#hl2c_spawn_airboat"), SPAWNTYPE_VEHICLE, "models/airboat.mdl", "airboat", "scripts/vehicles/airboat.txt");
		}
		return;
	}
	//start halfway through when it's from Restart Level and we already went through the detour map
	if (g_bGoBackLevel && completed_town03.GetBool() && !Q_strcmp(pMapName, "d1_town_02"))
	{
		Vector vecOrigin = Vector(-3648, 0, -3419);
		static const char* strWeapons[] = {
			"item_suit",
			"weapon_crowbar",
			"weapon_physcannon",
			"weapon_pistol",
			"weapon_pistol",
			"weapon_357",
			"weapon_smg1",
			"weapon_smg1",
			"weapon_shotgun",
			"weapon_frag",
			"weapon_frag",
			"weapon_frag"
		};
		for (int i = 0; i < 12; i++)
		{
			CBaseEntity* pItem = (CBaseEntity*)CreateEntityByName(strWeapons[i]);
			if (pItem)
			{
				pItem->SetAbsOrigin(vecOrigin);
				pItem->Spawn();
				pItem->Activate();
			}
		}
	}
	//remove the fucky-wucky triggers so that invert gravity doesn't softlock you
	else if (!Q_strcmp(pMapName, "d2_coast_04"))
	{
		variant_t emptyVariant;
		CBaseEntity* pFallTrigger = gEntList.FindEntityByName(NULL, "fall_trigger");
		while (pFallTrigger)
		{
			g_EventQueue.AddEvent(pFallTrigger, "Kill", emptyVariant, 0.1f, this, this);
			pFallTrigger = gEntList.FindEntityByName(pFallTrigger, "fall_trigger");
		}
		return;
	}
	//start halfway through when it's from Restart Level and we already went through the detour map
	if (g_bGoBackLevel && completed_coast08.GetBool() && !Q_strcmp(pMapName, "d2_coast_07"))
	{
		Vector vecOrigin = Vector(3008, 5184, 1572);
		CBaseEntity* pItem = (CBaseEntity*)CreateEntityByName("item_suit");
		if (pItem)
		{
			pItem->SetAbsOrigin(vecOrigin);
			pItem->Spawn();
			pItem->Activate();
		}
		CBaseEntity* pTrigger = gEntList.FindEntityByName(NULL, "gunship_trigger_10");
		if (pTrigger)
		{
			CTriggerMultiple* pTriggerMultiple = static_cast<CTriggerMultiple*>(pTrigger);
			pTriggerMultiple->m_OnTrigger.FireOutput(this, this);
		}
		CBaseEntity* pDropship = gEntList.FindEntityByName(NULL, "dropship");
		if (pDropship)
		{
			g_EventQueue.AddEvent(pDropship, "Activate", 0, this, this);
		}
		CBaseEntity* pTemplate = gEntList.FindEntityByName(NULL, "player_spawn_items_maker");
		if (pTemplate)
		{
			variant_t emptyVariant;
			pTemplate->AcceptInput("ForceSpawn", this, this, emptyVariant, 0);
		}
		CBaseEntity* pJeep = gEntList.FindEntityByClassname(NULL, "prop_vehicle_jeep");
		if (pJeep)
		{
			Vector vecOrigin = Vector(2496, 5568, 1600);
			pJeep->Teleport(&vecOrigin, NULL, NULL);
		}
	}
	//this map has a filter that's meant to only allow the suppression device to pass, but it does this by allowing the projectile, not the tank entity.
	//the entity that's called in the relevant code is the attacker, not the inflictor, so this should always fail.
	//why am i just now observing this? i don't know.
	//so we're just going to set this filter to the new correct class.
	else if (!Q_strcmp(pMapName, "d3_c17_10a"))
	{
		CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "mortar_targets_damagefilter");
		pEnt->KeyValue("filterclass", "func_tankmortar");
	}
	//weapon destruction part requires a gravity gun in order to finish, but what if we lost it?
	else if (!Q_strcmp(pMapName, "d3_citadel_03"))
	{
		CBaseEntity* pGravGun = gEntList.FindEntityByClassname(NULL, "weapon_physcannon");
		if (!pGravGun && gpGlobals->eLoadType != MapLoad_NewGame)
		{
			g_ChaosEffects[EFFECT_GIVE_WEAPON]->ChaosSpawnWeapon("weapon_physcannon", MAKE_STRING("#hl2c_give_gravgun"));
		}
		return;
	}
	//move camera to better suit ep2 advisor model
	//and replace advisor model because apparently the cable movement doesn't appear on monitors!
	else if (!Q_strcmp(pMapName, "d3_breen_01"))
	{
		CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "cam_advisor_1");
		pEnt->SetAbsOrigin(Vector(-1338, -1107, 1114));
		pEnt->SetAbsAngles(QAngle(-10.74, 90, 0));
		CBaseEntity* pCycle = gEntList.FindEntityByClassname(NULL, "cycler");
		if (pCycle)
		{
			CBaseEntity* pAdv = (CBaseEntity*)CreateEntityByName("npc_advisor");
			if (pAdv)
			{
				pAdv->SetAbsOrigin(pCycle->GetAbsOrigin());
				pAdv->SetAbsAngles(pCycle->GetAbsAngles());
				pAdv->KeyValue("model", "models/advisor_nocables.mdl");
				pAdv->Spawn();
				pAdv->Activate();
				UTIL_Remove(pCycle);
			}
		}
	}
	//advisor model, same as breen 01
	else if (!Q_strcmp(pMapName, "ep1_citadel_01"))
	{
		CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "camera_advisor");
		pEnt->SetAbsOrigin(Vector(-96, -103, 96));
		pEnt->SetAbsAngles(QAngle(-10.74, 90, 0));
		CBaseEntity* pCycle = gEntList.FindEntityByClassname(NULL, "cycler");
		if (pCycle)
		{
			CBaseEntity* pAdv = (CBaseEntity*)CreateEntityByName("npc_advisor");
			if (pAdv)
			{
				pAdv->SetAbsOrigin(pCycle->GetAbsOrigin());
				pAdv->SetAbsAngles(pCycle->GetAbsAngles());
				pAdv->KeyValue("model", "models/advisor_nocables.mdl");
				pAdv->Spawn();
				pAdv->Activate();
				UTIL_Remove(pCycle);
			}
		}
	}
	//advisor model, same as breen 01
	else if (!Q_strcmp(pMapName, "ep1_c17_02"))
	{
		CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "cam_advisor_1");
		pEnt->SetAbsOrigin(Vector(2304, 693, -396));
		pEnt->SetAbsAngles(QAngle(-10.74, 90, 0));
		CBaseEntity* pCycle = gEntList.FindEntityByClassname(NULL, "cycler");
		if (pCycle)
		{
			CBaseEntity* pAdv = (CBaseEntity*)CreateEntityByName("npc_advisor");
			if (pAdv)
			{
				pAdv->SetAbsOrigin(pCycle->GetAbsOrigin());
				pAdv->SetAbsAngles(pCycle->GetAbsAngles());
				pAdv->KeyValue("model", "models/advisor_nocables.mdl");
				pAdv->Spawn();
				pAdv->Activate();
				UTIL_Remove(pCycle);
			}
		}
	}
	//advisor model, same as breen 01
	else if (!Q_strcmp(pMapName, "ep1_c17_05"))
	{
		CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "cam_advisor");
		pEnt->SetAbsOrigin(Vector(10212, 10489, -1368));
		pEnt->SetAbsAngles(QAngle(-10.74, 90, 0));
		CBaseEntity* pCycle = gEntList.FindEntityByClassname(NULL, "cycler");
		if (pCycle)
		{
			CBaseEntity* pAdv = (CBaseEntity*)CreateEntityByName("npc_advisor");
			if (pAdv)
			{
				pAdv->SetAbsOrigin(pCycle->GetAbsOrigin());
				pAdv->SetAbsAngles(pCycle->GetAbsAngles());
				pAdv->KeyValue("model", "models/advisor_nocables.mdl");
				pAdv->Spawn();
				pAdv->Activate();
				UTIL_Remove(pCycle);
			}
		}
	}
}
//-----------------------------------------------------------------------------
//Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2_Player::Spawn(void)
{
	//m_pController = physenv->CreateMotionController(this);
	StartGame();
	if (gpGlobals->eLoadType == MapLoad_NewGame)
	{
		//copy any IDs in global active list to memory active list right now we can restore/abort as needed
		for (int i = 0; i < MAX_ACTIVE_EFFECTS; i++)
		{
			m_iActiveEffects[i] = NULL;
			if (!g_iActiveEffects[i])//prefer to check with m list but in this case we have to use g list
				continue;
			m_iActiveEffects[i] = g_iActiveEffects[i];
		}
		m_bRestartHUD = true;
		if (g_bGoBackLevel)
			ReplaceEffects();
		//for HL2 Chaos, you always want to have a save to load
		CBaseEntity* pAutosave = CBaseEntity::Create("logic_autosave", vec3_origin, vec3_angle, NULL);
		if (pAutosave)
		{
			g_EventQueue.AddEvent(pAutosave, "Save", 1.0, NULL, NULL);
			g_EventQueue.AddEvent(pAutosave, "Kill", 1.1, NULL, NULL);
		}
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "exec groups\n");
	}
#ifndef HL2MP
#ifndef PORTAL
	SetModel("models/player.mdl");
#endif
#endif

	BaseClass::Spawn();

	//
	//Our player movement speed is set once here. This will override the cl_xxxx
	//cvars unless they are set to be lower than this.
	//
	//m_flMaxspeed = 320;

	if (!IsSuitEquipped())
		StartWalking();

	SuitPower_SetCharge(100);

	m_Local.m_iHideHUD |= HIDEHUD_CHAT;

	m_pPlayerAISquad = g_AI_SquadManager.FindCreateSquad(AllocPooledString(PLAYER_SQUADNAME));

	InitSprinting();

	//Setup our flashlight values
#ifdef HL2_EPISODIC
	m_HL2Local.m_flFlashBattery = 100.0f;
#endif 

	GetPlayerProxy();

	SetFlashlightPowerDrainScale(1.0f);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::UpdateLocatorPosition(const Vector& vecPosition)
{
#ifdef HL2_EPISODIC
	m_HL2Local.m_vecLocatorOrigin = vecPosition;
#endif//HL2_EPISODIC 
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::InitSprinting(void)
{
	StopSprinting();
}


//-----------------------------------------------------------------------------
//Purpose: Returns whether or not we are allowed to sprint now.
//-----------------------------------------------------------------------------
bool CHL2_Player::CanSprint()
{
	return (m_bSprintEnabled &&										//Only if sprint is enabled 
		!IsWalking() &&												//Not if we're walking
		!(m_Local.m_bDucked && !m_Local.m_bDucking) &&			//Nor if we're ducking
		(GetWaterLevel() != 3) &&									//Certainly not underwater
		(GlobalEntity_GetState("suit_no_sprint") != GLOBAL_ON));	//Out of the question without the sprint module
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StartAutoSprint()
{
	if (IsSprinting())
	{
		StopSprinting();
	}
	else
	{
		StartSprinting();
		m_bIsAutoSprinting = true;
		m_fAutoSprintMinTime = gpGlobals->curtime + 1.5f;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StartSprinting(void)
{
	if (m_HL2Local.m_flSuitPower < 10)
	{
		//Don't sprint unless there's a reasonable
		//amount of suit power.

		//debounce the button for sound playing
		if (m_afButtonPressed & IN_SPEED)
		{
			CPASAttenuationFilter filter(this);
			filter.UsePredictionRules();
			EmitSound(filter, entindex(), "HL2Player.SprintNoPower");
		}
		return;
	}

	if (!SuitPower_AddDevice(SuitDeviceSprint))
		return;

	CPASAttenuationFilter filter(this);
	filter.UsePredictionRules();
	EmitSound(filter, entindex(), "HL2Player.SprintStart");

	SetMaxSpeed(HL2_SPRINT_SPEED);
	m_fIsSprinting = true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StopSprinting(void)
{
	if (m_HL2Local.m_bitsActiveDevices & SuitDeviceSprint.GetDeviceID())
	{
		SuitPower_RemoveDevice(SuitDeviceSprint);
	}

	if (IsSuitEquipped())
	{
		SetMaxSpeed(HL2_NORM_SPEED);
	}
	else
	{
		SetMaxSpeed(HL2_WALK_SPEED);
	}

	m_fIsSprinting = false;

	if (sv_stickysprint.GetBool())
	{
		m_bIsAutoSprinting = false;
		m_fAutoSprintMinTime = 0.0f;
	}
}


//-----------------------------------------------------------------------------
//Purpose: Called to disable and enable sprint due to temporary circumstances:
//			- Carrying a heavy object with the physcannon
//-----------------------------------------------------------------------------
void CHL2_Player::EnableSprint(bool bEnable)
{
	if (!bEnable && IsSprinting())
	{
		StopSprinting();
	}

	m_bSprintEnabled = bEnable;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StartWalking(void)
{
	SetMaxSpeed(HL2_WALK_SPEED);
	m_fIsWalking = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StopWalking(void)
{
	SetMaxSpeed(HL2_NORM_SPEED);
	m_fIsWalking = false;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StartDucking(void)
{
	SetMaxSpeed(HL2_DUCK_SPEED);
	m_fIsDucking = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::StopDucking(void)
{
	SetMaxSpeed(HL2_NORM_SPEED);
	m_fIsDucking = false;
}

//-----------------------------------------------------------------------------
//Purpose: 
//Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::CanZoom(CBaseEntity* pRequester)
{
	if (IsZooming())
		return false;

	//Check our weapon

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::ToggleZoom(void)
{
	if (IsZooming())
	{
		StopZooming();
	}
	else
	{
		StartZooming();
	}
}

//-----------------------------------------------------------------------------
//Purpose: +zoom suit zoom
//-----------------------------------------------------------------------------
void CHL2_Player::StartZooming(void)
{
	int iFOV = 25;
	if (SetFOV(this, iFOV, 0.4f))
	{
		m_HL2Local.m_bZooming = true;
	}
}

//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::StopZooming(void)
{
	int iFOV = GetZoomOwnerDesiredFOV(m_hZoomOwner);

	if (SetFOV(this, iFOV, 0.2f))
	{
		m_HL2Local.m_bZooming = false;
	}
}

//-----------------------------------------------------------------------------
//Purpose: 
//Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::IsZooming(void)
{
	if (m_hZoomOwner != NULL)
		return true;

	return false;
}

class CPhysicsPlayerCallback : public IPhysicsPlayerControllerEvent
{
public:
	int ShouldMoveTo(IPhysicsObject* pObject, const Vector& position)
	{
		CHL2_Player* pPlayer = (CHL2_Player*)pObject->GetGameData();
		if (pPlayer)
		{
			if (pPlayer->TouchedPhysics())
			{
				return 0;
			}
		}
		return 1;
	}
};

static CPhysicsPlayerCallback playerCallback;

//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::InitVCollision(const Vector& vecAbsOrigin, const Vector& vecAbsVelocity)
{
	BaseClass::InitVCollision(vecAbsOrigin, vecAbsVelocity);

	//Setup the HL2 specific callback.
	IPhysicsPlayerController* pPlayerController = GetPhysicsController();
	if (pPlayerController)
	{
		pPlayerController->SetEventHandler(&playerCallback);
	}
}


CHL2_Player::~CHL2_Player(void)
{
	gEntList.RemoveListenerEntity(this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CHL2_Player::CommanderFindGoal(commandgoal_t* pGoal)
{
	CAI_BaseNPC* pAllyNpc;
	trace_t	tr;
	Vector	vecTarget;
	Vector	forward;

	EyeVectors(&forward);

	//---------------------------------
	//MASK_SHOT on purpose! So that you don't hit the invisible hulls of the NPCs.
	CTraceFilterSkipTwoEntities filter(this, PhysCannonGetHeldEntity(GetActiveWeapon()), COLLISION_GROUP_INTERACTIVE_DEBRIS);

	UTIL_TraceLine(EyePosition(), EyePosition() + forward * MAX_COORD_RANGE, MASK_SHOT, &filter, &tr);

	if (!tr.DidHitWorld())
	{
		CUtlVector<CAI_BaseNPC*> Allies;
		AISquadIter_t iter;
		for (pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter))
		{
			if (pAllyNpc->IsCommandable())
				Allies.AddToTail(pAllyNpc);
		}

		for (int i = 0; i < Allies.Count(); i++)
		{
			if (Allies[i]->IsValidCommandTarget(tr.m_pEnt))
			{
				pGoal->m_pGoalEntity = tr.m_pEnt;
				return true;
			}
		}
	}

	if (tr.fraction == 1.0 || (tr.surface.flags & SURF_SKY))
	{
		//Move commands invalid against skybox.
		pGoal->m_vecGoalLocation = tr.endpos;
		return false;
	}

	if (tr.m_pEnt->IsNPC() && ((CAI_BaseNPC*)(tr.m_pEnt))->IsCommandable())
	{
		pGoal->m_vecGoalLocation = tr.m_pEnt->GetAbsOrigin();
	}
	else
	{
		vecTarget = tr.endpos;

		Vector mins(-16, -16, 0);
		Vector maxs(16, 16, 0);

		//Back up from whatever we hit so that there's enough space at the 
		//target location for a bounding box.
		//Now trace down. 
		//UTIL_TraceLine( vecTarget, vecTarget - Vector( 0, 0, 8192 ), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		UTIL_TraceHull(vecTarget + tr.plane.normal * 24,
			vecTarget - Vector(0, 0, 8192),
			mins,
			maxs,
			MASK_SOLID_BRUSHONLY,
			this,
			COLLISION_GROUP_NONE,
			&tr);


		if (!tr.startsolid)
			pGoal->m_vecGoalLocation = tr.endpos;
		else
			pGoal->m_vecGoalLocation = vecTarget;
	}

	pAllyNpc = GetSquadCommandRepresentative();
	if (!pAllyNpc)
		return false;

	vecTarget = pGoal->m_vecGoalLocation;
	if (!pAllyNpc->FindNearestValidGoalPos(vecTarget, &pGoal->m_vecGoalLocation))
		return false;

	return ((vecTarget - pGoal->m_vecGoalLocation).LengthSqr() < Square(15 * 12));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CAI_BaseNPC* CHL2_Player::GetSquadCommandRepresentative()
{
	if (m_pPlayerAISquad != NULL)
	{
		CAI_BaseNPC* pAllyNpc = m_pPlayerAISquad->GetFirstMember();

		if (pAllyNpc)
		{
			return pAllyNpc->GetSquadCommandRepresentative();
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2_Player::GetNumSquadCommandables()
{
	AISquadIter_t iter;
	int c = 0;
	for (CAI_BaseNPC* pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter))
	{
		if (pAllyNpc->IsCommandable())
			c++;
	}
	return c;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2_Player::GetNumSquadCommandableMedics()
{
	AISquadIter_t iter;
	int c = 0;
	for (CAI_BaseNPC* pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter))
	{
		if (pAllyNpc->IsCommandable() && pAllyNpc->IsMedic())
			c++;
	}
	return c;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::CommanderUpdate()
{
	CAI_BaseNPC* pCommandRepresentative = GetSquadCommandRepresentative();
	bool bFollowMode = false;
	if (pCommandRepresentative)
	{
		bFollowMode = (pCommandRepresentative->GetCommandGoal() == vec3_invalid);

		//set the variables for network transmission (to show on the hud)
		m_HL2Local.m_iSquadMemberCount = GetNumSquadCommandables();
		m_HL2Local.m_iSquadMedicCount = GetNumSquadCommandableMedics();
		m_HL2Local.m_fSquadInFollowMode = bFollowMode;

		//debugging code for displaying extra squad indicators
		/*
		char *pszMoving = "";
		AISquadIter_t iter;
		for ( CAI_BaseNPC *pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter) )
		{
			if ( pAllyNpc->IsCommandMoving() )
			{
				pszMoving = "<-";
				break;
			}
		}

		NDebugOverlay::ScreenText(
			0.932, 0.919,
			CFmtStr( "%d|%c%s", GetNumSquadCommandables(), ( bFollowMode ) ? 'F' : 'S', pszMoving ),
			255, 128, 0, 128,
			0 );
		*/

	}
	else
	{
		m_HL2Local.m_iSquadMemberCount = 0;
		m_HL2Local.m_iSquadMedicCount = 0;
		m_HL2Local.m_fSquadInFollowMode = true;
	}

	if (m_QueuedCommand != CC_NONE && (m_QueuedCommand == CC_FOLLOW || gpGlobals->realtime - m_RealTimeLastSquadCommand >= player_squad_double_tap_time.GetFloat()))
	{
		CommanderExecute(m_QueuedCommand);
		m_QueuedCommand = CC_NONE;
	}
	else if (!bFollowMode && pCommandRepresentative && m_CommanderUpdateTimer.Expired() && player_squad_transient_commands.GetBool())
	{
		m_CommanderUpdateTimer.Set(2.5);

		if (pCommandRepresentative->ShouldAutoSummon())
			CommanderExecute(CC_FOLLOW);
	}
}

//-----------------------------------------------------------------------------
//Purpose: 
//
//bHandled - indicates whether to continue delivering this order to
//all allies. Allows us to stop delivering certain types of orders once we find
//a suitable candidate. (like picking up a single weapon. We don't wish for
//all allies to respond and try to pick up one weapon).
//----------------------------------------------------------------------------- 
bool CHL2_Player::CommanderExecuteOne(CAI_BaseNPC* pNpc, const commandgoal_t& goal, CAI_BaseNPC** Allies, int numAllies)
{
	if (goal.m_pGoalEntity)
	{
		return pNpc->TargetOrder(goal.m_pGoalEntity, Allies, numAllies);
	}
	else if (pNpc->IsInPlayerSquad())
	{
		pNpc->MoveOrder(goal.m_vecGoalLocation, Allies, numAllies);
	}

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CHL2_Player::CommanderExecute(CommanderCommand_t command)
{
	CAI_BaseNPC* pPlayerSquadLeader = GetSquadCommandRepresentative();

	if (!pPlayerSquadLeader)
	{
		EmitSound("HL2Player.UseDeny");
		return;
	}

	int i;
	CUtlVector<CAI_BaseNPC*> Allies;
	commandgoal_t goal;

	if (command == CC_TOGGLE)
	{
		if (pPlayerSquadLeader->GetCommandGoal() != vec3_invalid)
			command = CC_FOLLOW;
		else
			command = CC_SEND;
	}
	else
	{
		if (command == CC_FOLLOW && pPlayerSquadLeader->GetCommandGoal() == vec3_invalid)
			return;
	}

	if (command == CC_FOLLOW)
	{
		goal.m_pGoalEntity = this;
		goal.m_vecGoalLocation = vec3_invalid;
	}
	else
	{
		goal.m_pGoalEntity = NULL;
		goal.m_vecGoalLocation = vec3_invalid;

		//Find a goal for ourselves.
		if (!CommanderFindGoal(&goal))
		{
			EmitSound("HL2Player.UseDeny");
			return; //just keep following
		}
	}

#ifdef _DEBUG
	if (goal.m_pGoalEntity == NULL && goal.m_vecGoalLocation == vec3_invalid)
	{
		DevMsg(1, "**ERROR: Someone sent an invalid goal to CommanderExecute!\n");
	}
#endif //_DEBUG

	AISquadIter_t iter;
	for (CAI_BaseNPC* pAllyNpc = m_pPlayerAISquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pPlayerAISquad->GetNextMember(&iter))
	{
		if (pAllyNpc->IsCommandable())
			Allies.AddToTail(pAllyNpc);
	}

	//---------------------------------
	//If the trace hits an NPC, send all ally NPCs a "target" order. Always
	//goes to targeted one first
#ifdef DBGFLAG_ASSERT
	int nAIs = g_AI_Manager.NumAIs();
#endif
	CAI_BaseNPC* pTargetNpc = (goal.m_pGoalEntity) ? goal.m_pGoalEntity->MyNPCPointer() : NULL;

	bool bHandled = false;
	if (pTargetNpc)
	{
		bHandled = !CommanderExecuteOne(pTargetNpc, goal, Allies.Base(), Allies.Count());
	}

	for (i = 0; !bHandled && i < Allies.Count(); i++)
	{
		if (Allies[i] != pTargetNpc && Allies[i]->IsPlayerAlly())
		{
			bHandled = !CommanderExecuteOne(Allies[i], goal, Allies.Base(), Allies.Count());
		}
		Assert(nAIs == g_AI_Manager.NumAIs()); //not coded to support mutating set of NPCs
	}
}

//-----------------------------------------------------------------------------
//Enter/exit commander mode, manage ally selection.
//-----------------------------------------------------------------------------
void CHL2_Player::CommanderMode()
{
	float commandInterval = gpGlobals->realtime - m_RealTimeLastSquadCommand;
	m_RealTimeLastSquadCommand = gpGlobals->realtime;
	if (commandInterval < player_squad_double_tap_time.GetFloat())
	{
		m_QueuedCommand = CC_FOLLOW;
	}
	else
	{
		m_QueuedCommand = (player_squad_transient_commands.GetBool()) ? CC_SEND : CC_TOGGLE;
	}
}

//-----------------------------------------------------------------------------
//Purpose: 
//Input  : iImpulse - 
//-----------------------------------------------------------------------------
void CHL2_Player::CheatImpulseCommands(int iImpulse)
{
	switch (iImpulse)
	{
	case 50:
	{
		CommanderMode();
		break;
	}

	case 51:
	{
		//Cheat to create a dynamic resupply item
		Vector vecForward;
		AngleVectors(EyeAngles(), &vecForward);
		CBaseEntity* pItem = (CBaseEntity*)CreateEntityByName("item_dynamic_resupply");
		if (pItem)
		{
			Vector vecOrigin = GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 64);
			QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
			pItem->SetAbsOrigin(vecOrigin);
			pItem->SetAbsAngles(vecAngles);
			pItem->KeyValue("targetname", "resupply");
			pItem->Spawn();
			pItem->Activate();
		}
		break;
	}

	case 52:
	{
		//Rangefinder
		trace_t tr;
		UTIL_TraceLine(EyePosition(), EyePosition() + EyeDirection3D() * MAX_COORD_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction != 1.0)
		{
			float flDist = (tr.startpos - tr.endpos).Length();
			float flDist2D = (tr.startpos - tr.endpos).Length2D();
			DevMsg(1, "\nStartPos: %.4f %.4f %.4f --- EndPos: %.4f %.4f %.4f\n", tr.startpos.x, tr.startpos.y, tr.startpos.z, tr.endpos.x, tr.endpos.y, tr.endpos.z);
			DevMsg(1, "3D Distance: %.4f units  (%.2f feet) --- 2D Distance: %.4f units  (%.2f feet)\n", flDist, flDist / 12.0, flDist2D, flDist2D / 12.0);
		}

		break;
	}

	default:
		BaseClass::CheatImpulseCommands(iImpulse);
	}
}

//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::SetupVisibility(CBaseEntity* pViewEntity, unsigned char* pvs, int pvssize)
{
	BaseClass::SetupVisibility(pViewEntity, pvs, pvssize);

	int area = pViewEntity ? pViewEntity->NetworkProp()->AreaNum() : NetworkProp()->AreaNum();
	PointCameraSetupVisibility(this, area, pvs, pvssize);

	//If the intro script is playing, we want to get it's visibility points
	if (g_hIntroScript)
	{
		Vector vecOrigin;
		CBaseEntity* pCamera;
		if (g_hIntroScript->GetIncludedPVSOrigin(&vecOrigin, &pCamera))
		{
			//If it's a point camera, turn it on
			CPointCamera* pPointCamera = dynamic_cast<CPointCamera*>(pCamera);
			if (pPointCamera)
			{
				pPointCamera->SetActive(true);
			}
			engine->AddOriginToPVS(vecOrigin);
		}
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::SuitPower_Update(void)
{
	if (SuitPower_ShouldRecharge())
	{
		SuitPower_Charge(SUITPOWER_CHARGE_RATE * gpGlobals->frametime);
	}
	else if (m_HL2Local.m_bitsActiveDevices)
	{
		float flPowerLoad = m_flSuitPowerLoad;

		//Since stickysprint quickly shuts off sprint if it isn't being used, this isn't an issue.
		if (!sv_stickysprint.GetBool())
		{
			if (SuitPower_IsDeviceActive(SuitDeviceSprint))
			{
				if (!fabs(GetAbsVelocity().x) && !fabs(GetAbsVelocity().y))
				{
					//If player's not moving, don't drain sprint juice.
					flPowerLoad -= SuitDeviceSprint.GetDeviceDrainRate();
				}
			}
		}

		if (SuitPower_IsDeviceActive(SuitDeviceFlashlight))
		{
			float factor;

			factor = 1.0f / m_flFlashlightPowerDrainScale;

			flPowerLoad -= (SuitDeviceFlashlight.GetDeviceDrainRate() * (1.0f - factor));
		}

		if (!SuitPower_Drain(flPowerLoad * gpGlobals->frametime))
		{
			//TURN OFF ALL DEVICES!!
			if (IsSprinting())
			{
				StopSprinting();
			}

			if (Flashlight_UseLegacyVersion())
			{
				if (FlashlightIsOn())
				{
#ifndef HL2MP
					FlashlightTurnOff();
#endif
				}
			}
		}

		if (Flashlight_UseLegacyVersion())
		{
			//turn off flashlight a little bit after it hits below one aux power notch (5%)
			if (m_HL2Local.m_flSuitPower < 4.8f && FlashlightIsOn())
			{
#ifndef HL2MP
				FlashlightTurnOff();
#endif
			}
		}
	}
}


//-----------------------------------------------------------------------------
//Charge battery fully, turn off all devices.
//-----------------------------------------------------------------------------
void CHL2_Player::SuitPower_Initialize(void)
{
	m_HL2Local.m_bitsActiveDevices = 0x00000000;
	m_HL2Local.m_flSuitPower = 100.0;
	m_flSuitPowerLoad = 0.0;
}


//-----------------------------------------------------------------------------
//Purpose: Interface to drain power from the suit's power supply.
//Input:	Amount of charge to remove (expressed as percentage of full charge)
//Output:	Returns TRUE if successful, FALSE if not enough power available.
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_Drain(float flPower)
{
	//Suitpower cheat on?
	if (sv_infinite_aux_power.GetBool())
		return true;

	m_HL2Local.m_flSuitPower -= flPower;

	if (m_HL2Local.m_flSuitPower < 0.0)
	{
		//Power is depleted!
		//Clamp and fail
		m_HL2Local.m_flSuitPower = 0.0;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//Purpose: Interface to add power to the suit's power supply
//Input:	Amount of charge to add
//-----------------------------------------------------------------------------
void CHL2_Player::SuitPower_Charge(float flPower)
{
	m_HL2Local.m_flSuitPower += flPower;

	if (m_HL2Local.m_flSuitPower > 100.0)
	{
		//Full charge, clamp.
		m_HL2Local.m_flSuitPower = 100.0;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_IsDeviceActive(const CSuitPowerDevice& device)
{
	return (m_HL2Local.m_bitsActiveDevices & device.GetDeviceID()) != 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_AddDevice(const CSuitPowerDevice& device)
{
	//Make sure this device is NOT active!!
	if (m_HL2Local.m_bitsActiveDevices & device.GetDeviceID())
		return false;

	if (!IsSuitEquipped())
		return false;

	m_HL2Local.m_bitsActiveDevices |= device.GetDeviceID();
	m_flSuitPowerLoad += device.GetDeviceDrainRate();
	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::SuitPower_RemoveDevice(const CSuitPowerDevice& device)
{
	//Make sure this device is active!!
	if (!(m_HL2Local.m_bitsActiveDevices & device.GetDeviceID()))
		return false;

	if (!IsSuitEquipped())
		return false;

	//Take a little bit of suit power when you disable a device. If the device is shutting off
	//because the battery is drained, no harm done, the battery charge cannot go below 0. 
	//This code in combination with the delay before the suit can start recharging are a defense
	//against exploits where the player could rapidly tap sprint and never run out of power.
	SuitPower_Drain(device.GetDeviceDrainRate() * 0.1f);

	m_HL2Local.m_bitsActiveDevices &= ~device.GetDeviceID();
	m_flSuitPowerLoad -= device.GetDeviceDrainRate();

	if (m_HL2Local.m_bitsActiveDevices == 0x00000000)
	{
		//With this device turned off, we can set this timer which tells us when the
		//suit power system entered a no-load state.
		m_flTimeAllSuitDevicesOff = gpGlobals->curtime;
	}

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define SUITPOWER_BEGIN_RECHARGE_DELAY	0.5f
bool CHL2_Player::SuitPower_ShouldRecharge(void)
{
	//Make sure all devices are off.
	if (m_HL2Local.m_bitsActiveDevices != 0x00000000)
		return false;

	//Is the system fully charged?
	if (m_HL2Local.m_flSuitPower >= 100.0f)
		return false;

	//Has the system been in a no-load state for long enough
	//to begin recharging?
	if (gpGlobals->curtime < m_flTimeAllSuitDevicesOff + SUITPOWER_BEGIN_RECHARGE_DELAY)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
ConVar	sk_battery("sk_battery", "0");

bool CHL2_Player::ApplyBattery(float powerMultiplier)
{
	const float MAX_NORMAL_BATTERY = 100;
	if ((ArmorValue() < MAX_NORMAL_BATTERY) && IsSuitEquipped())
	{
		int pct;
		char szcharge[64];

		IncrementArmorValue(sk_battery.GetFloat() * powerMultiplier, MAX_NORMAL_BATTERY);

		CPASAttenuationFilter filter(this, "ItemBattery.Touch");
		EmitSound(filter, entindex(), "ItemBattery.Touch");

		CSingleUserRecipientFilter user(this);
		user.MakeReliable();

		UserMessageBegin(user, "ItemPickup");
		WRITE_STRING("item_battery");
		MessageEnd();


		//Suit reports new power level
		//For some reason this wasn't working in release build -- round it.
		pct = (int)((float)(ArmorValue() * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
		pct = (pct / 5);
		if (pct > 0)
			pct--;

		Q_snprintf(szcharge, sizeof(szcharge), "!HEV_%1dP", pct);

		//UTIL_EmitSoundSuit(edict(), szcharge);
		//SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2_Player::FlashlightIsOn(void)
{
	return IsEffectActive(EF_DIMLIGHT);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::FlashlightTurnOn(void)
{
	if (m_bFlashlightDisabled)
		return;

	if (Flashlight_UseLegacyVersion())
	{
		if (!SuitPower_AddDevice(SuitDeviceFlashlight))
			return;
	}
#ifdef HL2_DLL
	if (!IsSuitEquipped())
		return;
#endif

	AddEffects(EF_DIMLIGHT);
	EmitSound("HL2Player.FlashLightOn");

	variant_t flashlighton;
	flashlighton.SetFloat(m_HL2Local.m_flSuitPower / 100.0f);
	FirePlayerProxyOutput("OnFlashlightOn", flashlighton, this, this);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::FlashlightTurnOff(void)
{
	if (Flashlight_UseLegacyVersion())
	{
		if (!SuitPower_RemoveDevice(SuitDeviceFlashlight))
			return;
	}

	RemoveEffects(EF_DIMLIGHT);
	EmitSound("HL2Player.FlashLightOff");

	variant_t flashlightoff;
	flashlightoff.SetFloat(m_HL2Local.m_flSuitPower / 100.0f);
	FirePlayerProxyOutput("OnFlashlightOff", flashlightoff, this, this);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define FLASHLIGHT_RANGE	Square(600)
bool CHL2_Player::IsIlluminatedByFlashlight(CBaseEntity* pEntity, float* flReturnDot)
{
	if (!FlashlightIsOn())
		return false;

	if (pEntity->Classify() == CLASS_BARNACLE && pEntity->GetEnemy() == this)
	{
		//As long as my flashlight is on, the barnacle that's pulling me in is considered illuminated.
		//This is because players often shine their flashlights at Alyx when they are in a barnacle's 
		//grasp, and wonder why Alyx isn't helping. Alyx isn't helping because the light isn't pointed
		//at the barnacle. This will allow Alyx to see the barnacle no matter which way the light is pointed.
		return true;
	}

	//Within 50 feet?
	float flDistSqr = GetAbsOrigin().DistToSqr(pEntity->GetAbsOrigin());
	if (flDistSqr > FLASHLIGHT_RANGE)
		return false;

	//Within 45 degrees?
	Vector vecSpot = pEntity->WorldSpaceCenter();
	Vector los;

	//If the eyeposition is too close, move it back. Solves problems
	//caused by the player being too close the target.
	if (flDistSqr < (128 * 128))
	{
		Vector vecForward;
		EyeVectors(&vecForward);
		Vector vecMovedEyePos = EyePosition() - (vecForward * 128);
		los = (vecSpot - vecMovedEyePos);
	}
	else
	{
		los = (vecSpot - EyePosition());
	}

	VectorNormalize(los);
	Vector facingDir = EyeDirection3D();
	float flDot = DotProduct(los, facingDir);

	if (flReturnDot)
	{
		*flReturnDot = flDot;
	}

	if (flDot < 0.92387f)
		return false;

	if (!FVisible(pEntity))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
//Purpose: Let NPCs know when the flashlight is trained on them
//-----------------------------------------------------------------------------
void CHL2_Player::CheckFlashlight(void)
{
	if (!FlashlightIsOn())
		return;

	if (m_flNextFlashlightCheckTime > gpGlobals->curtime)
		return;
	m_flNextFlashlightCheckTime = gpGlobals->curtime + FLASHLIGHT_NPC_CHECK_INTERVAL;

	//Loop through NPCs looking for illuminated ones
	for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
	{
		CAI_BaseNPC* pNPC = g_AI_Manager.AccessAIs()[i];

		float flDot;

		if (IsIlluminatedByFlashlight(pNPC, &flDot))
		{
			pNPC->PlayerHasIlluminatedNPC(this, flDot);
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::SetPlayerUnderwater(bool state)
{
	if (state)
	{
		SuitPower_AddDevice(SuitDeviceBreather);
	}
	else
	{
		SuitPower_RemoveDevice(SuitDeviceBreather);
	}

	BaseClass::SetPlayerUnderwater(state);
}

//-----------------------------------------------------------------------------
bool CHL2_Player::PassesDamageFilter(const CTakeDamageInfo& info)
{
	CBaseEntity* pAttacker = info.GetAttacker();
	if (pAttacker && pAttacker->MyNPCPointer() && pAttacker->MyNPCPointer()->IsPlayerAlly())
	{
		return false;
	}

	if (m_hPlayerProxy && !m_hPlayerProxy->PassesDamageFilter(info))
	{
		return false;
	}

	return BaseClass::PassesDamageFilter(info);
}

//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::SetFlashlightEnabled(bool bState)
{
	m_bFlashlightDisabled = !bState;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::InputDisableFlashlight(inputdata_t& inputdata)
{
	if (FlashlightIsOn())
		FlashlightTurnOff();

	SetFlashlightEnabled(false);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::InputEnableFlashlight(inputdata_t& inputdata)
{
	SetFlashlightEnabled(true);
}


//-----------------------------------------------------------------------------
//Purpose: Prevent the player from taking fall damage for [n] seconds, but
//reset back to taking fall damage after the first impact (so players will be
//hurt if they bounce off what they hit). This is the original behavior.
//-----------------------------------------------------------------------------
void CHL2_Player::InputIgnoreFallDamage(inputdata_t& inputdata)
{
	float timeToIgnore = inputdata.value.Float();

	if (timeToIgnore <= 0.0)
		timeToIgnore = TIME_IGNORE_FALL_DAMAGE;

	m_flTimeIgnoreFallDamage = gpGlobals->curtime + timeToIgnore;
	m_bIgnoreFallDamageResetAfterImpact = true;
}


//-----------------------------------------------------------------------------
//Purpose: Absolutely prevent the player from taking fall damage for [n] seconds. 
//-----------------------------------------------------------------------------
void CHL2_Player::InputIgnoreFallDamageWithoutReset(inputdata_t& inputdata)
{
	float timeToIgnore = inputdata.value.Float();

	if (timeToIgnore <= 0.0)
		timeToIgnore = TIME_IGNORE_FALL_DAMAGE;

	m_flTimeIgnoreFallDamage = gpGlobals->curtime + timeToIgnore;
	m_bIgnoreFallDamageResetAfterImpact = false;
}

//-----------------------------------------------------------------------------
//Purpose: Notification of a player's npc ally in the players squad being killed
//-----------------------------------------------------------------------------
void CHL2_Player::OnSquadMemberKilled(inputdata_t& data)
{
	//send a message to the client, to notify the hud of the loss
	CSingleUserRecipientFilter user(this);
	user.MakeReliable();
	UserMessageBegin(user, "SquadMemberDied");
	MessageEnd();
}

//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::NotifyFriendsOfDamage(CBaseEntity* pAttackerEntity)
{
	CAI_BaseNPC* pAttacker = pAttackerEntity->MyNPCPointer();
	if (pAttacker)
	{
		const Vector& origin = GetAbsOrigin();
		for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
		{
			const float NEAR_Z = 12 * 12;
			const float NEAR_XY_SQ = Square(50 * 12);
			CAI_BaseNPC* pNpc = g_AI_Manager.AccessAIs()[i];
			if (pNpc->IsPlayerAlly())
			{
				const Vector& originNpc = pNpc->GetAbsOrigin();
				if (fabsf(originNpc.z - origin.z) < NEAR_Z)
				{
					if ((originNpc.AsVector2D() - origin.AsVector2D()).LengthSqr() < NEAR_XY_SQ)
					{
						pNpc->OnFriendDamaged(this, pAttacker);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
ConVar test_massive_dmg("test_massive_dmg", "30");
ConVar test_massive_dmg_clip("test_massive_dmg_clip", "0.5");
int	CHL2_Player::OnTakeDamage(const CTakeDamageInfo& info)
{
	if (GlobalEntity_GetState("gordon_invulnerable") == GLOBAL_ON)
		return 0;

	//ignore fall damage if instructed to do so by input
	if ((info.GetDamageType() & DMG_FALL) && m_flTimeIgnoreFallDamage > gpGlobals->curtime)
	{
		//usually, we will reset the input flag after the first impact. However there is another input that
		//prevents this behavior.
		if (m_bIgnoreFallDamageResetAfterImpact)
		{
			m_flTimeIgnoreFallDamage = 0;
		}
		return 0;
	}

	if (info.GetDamageType() & DMG_BLAST_SURFACE)
	{
		if (GetWaterLevel() > 2)
		{
			//Don't take blast damage from anything above the surface.
			if (info.GetInflictor()->GetWaterLevel() == 0)
			{
				return 0;
			}
		}
	}

	if (info.GetDamage() > 0.0f)
	{
		m_flLastDamageTime = gpGlobals->curtime;

		if (info.GetAttacker())
			NotifyFriendsOfDamage(info.GetAttacker());
	}

	//Modify the amount of damage the player takes, based on skill.
	CTakeDamageInfo playerDamage = info;

	//Should we run this damage through the skill level adjustment?
	bool bAdjustForSkillLevel = true;

	if (info.GetDamageType() == DMG_GENERIC && info.GetAttacker() == this && info.GetInflictor() == this)
	{
		//Only do a skill level adjustment if the player isn't his own attacker AND inflictor.
		//This prevents damage from SetHealth() inputs from being adjusted for skill level.
		bAdjustForSkillLevel = false;
	}

	if (GetVehicleEntity() != NULL && GlobalEntity_GetState("gordon_protect_driver") == GLOBAL_ON)
	{
		if (playerDamage.GetDamage() > test_massive_dmg.GetFloat() && playerDamage.GetInflictor() == GetVehicleEntity() && (playerDamage.GetDamageType() & DMG_CRUSH))
		{
			playerDamage.ScaleDamage(test_massive_dmg_clip.GetFloat() / playerDamage.GetDamage());
		}
	}

	if (bAdjustForSkillLevel)
	{
		playerDamage.AdjustPlayerDamageTakenForSkillLevel();
	}

	gamestats->Event_PlayerDamage(this, info);

	return BaseClass::OnTakeDamage(playerDamage);
}

//-----------------------------------------------------------------------------
//Purpose: 
//Input  : &info - 
//-----------------------------------------------------------------------------
int CHL2_Player::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{
	//Drown
	if (info.GetDamageType() & DMG_DROWN)
	{
		if (m_idrowndmg == m_idrownrestored)
		{
			EmitSound("Player.DrownStart");
		}
		else
		{
			EmitSound("Player.DrownContinue");
		}
	}

	//Burnt
	if (info.GetDamageType() & DMG_BURN)
	{
		EmitSound("HL2Player.BurnPain");
	}


	if ((info.GetDamageType() & DMG_SLASH) && hl2_episodic.GetBool())
	{
		if (m_afPhysicsFlags & PFLAG_USING)
		{
			//Stop the player using a rotating button for a short time if hit by a creature's melee attack.
			//This is for the antlion burrow-corking training in EP1 (sjb).
			SuspendUse(0.5f);
		}
	}


	//Call the base class implementation
	return BaseClass::OnTakeDamage_Alive(info);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::OnDamagedByExplosion(const CTakeDamageInfo& info)
{
	if (info.GetInflictor() && info.GetInflictor()->ClassMatches("mortarshell"))
	{
		//No ear ringing for mortar
		UTIL_ScreenShake(info.GetInflictor()->GetAbsOrigin(), 4.0, 1.0, 0.5, 1000, SHAKE_START, false);
		return;
	}
	BaseClass::OnDamagedByExplosion(info);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::ShouldShootMissTarget(CBaseCombatCharacter* pAttacker)
{
	if (gpGlobals->curtime > m_flTargetFindTime)
	{
		//Put this off into the future again.
		m_flTargetFindTime = gpGlobals->curtime + RandomFloat(3, 5);
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//Purpose: Notifies Alyx that player has put a combine ball into a socket so she can comment on it.
//Input  : pCombineBall - ball the was socketed
//-----------------------------------------------------------------------------
void CHL2_Player::CombineBallSocketed(CPropCombineBall* pCombineBall)
{
#ifdef HL2_EPISODIC
	CNPC_Alyx* pAlyx = CNPC_Alyx::GetAlyx();
	if (pAlyx)
	{
		pAlyx->CombineBallSocketed(pCombineBall->NumBounces());
	}
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::Event_KilledOther(CBaseEntity* pVictim, const CTakeDamageInfo& info)
{
	BaseClass::Event_KilledOther(pVictim, info);

#ifdef HL2_EPISODIC

	CAI_BaseNPC** ppAIs = g_AI_Manager.AccessAIs();

	for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
	{
		if (ppAIs[i] && ppAIs[i]->IRelationType(this) == D_LI)
		{
			ppAIs[i]->OnPlayerKilledOther(pVictim, info);
		}
	}

#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::Event_Killed(const CTakeDamageInfo& info)
{
	BaseClass::Event_Killed(info);

	//freeze HUD text
	DoChaosHUDText();
	FirePlayerProxyOutput("PlayerDied", variant_t(), this, this);
	NotifyScriptsOfDeath();

	//assign strikes to effects if possible
	//for (int i = 0; m_iActiveEffects.Size() >= i + 1; i++)
	for (int i = 0; i < MAX_ACTIVE_EFFECTS; i++)
	{
		if (!m_iActiveEffects[i])
			continue;
		CChaosEffect* pEffect = g_ChaosEffects[m_iActiveEffects[i]];
		if (pEffect->CheckStrike(info))
			pEffect->m_iStrikes++;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::NotifyScriptsOfDeath(void)
{
	CBaseEntity* pEnt = gEntList.FindEntityByClassname(NULL, "scripted_sequence");

	while (pEnt)
	{
		variant_t emptyVariant;
		pEnt->AcceptInput("ScriptPlayerDeath", NULL, NULL, emptyVariant, 0);

		pEnt = gEntList.FindEntityByClassname(pEnt, "scripted_sequence");
	}

	pEnt = gEntList.FindEntityByClassname(NULL, "logic_choreographed_scene");

	while (pEnt)
	{
		variant_t emptyVariant;
		pEnt->AcceptInput("ScriptPlayerDeath", NULL, NULL, emptyVariant, 0);

		pEnt = gEntList.FindEntityByClassname(pEnt, "logic_choreographed_scene");
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2_Player::GetAutoaimVector(autoaim_params_t& params)
{
	BaseClass::GetAutoaimVector(params);

	if (IsX360())
	{
		if (IsInAVehicle())
		{
			if (m_hLockedAutoAimEntity && m_hLockedAutoAimEntity->IsAlive() && ShouldKeepLockedAutoaimTarget(m_hLockedAutoAimEntity))
			{
				if (params.m_hAutoAimEntity && params.m_hAutoAimEntity != m_hLockedAutoAimEntity)
				{
					//Autoaim has picked a new target. Switch.
					m_hLockedAutoAimEntity = params.m_hAutoAimEntity;
				}

				//Ignore autoaim and just keep aiming at this target.
				params.m_hAutoAimEntity = m_hLockedAutoAimEntity;
				Vector vecTarget = m_hLockedAutoAimEntity->BodyTarget(false);
				Vector vecDir = vecTarget - EyePosition();
				VectorNormalize(vecDir);

				params.m_vecAutoAimDir = vecDir;
				params.m_vecAutoAimPoint = vecTarget;
				return;
			}
			else
			{
				m_hLockedAutoAimEntity = NULL;
			}
		}

		//If the player manually gets his crosshair onto a target, make that target sticky
		if (params.m_fScale != AUTOAIM_SCALE_DIRECT_ONLY)
		{
			//Only affect this for 'real' queries
			//if( params.m_hAutoAimEntity && params.m_bOnTargetNatural )
			if (params.m_hAutoAimEntity)
			{
				//Turn on sticky.
				m_HL2Local.m_bStickyAutoAim = true;

				if (IsInAVehicle())
				{
					m_hLockedAutoAimEntity = params.m_hAutoAimEntity;
				}
			}
			else if (!params.m_hAutoAimEntity)
			{
				//Turn off sticky only if there's no target at all.
				m_HL2Local.m_bStickyAutoAim = false;

				m_hLockedAutoAimEntity = NULL;
			}
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::ShouldKeepLockedAutoaimTarget(EHANDLE hLockedTarget)
{
	Vector vecLooking;
	Vector vecToTarget;

	vecToTarget = hLockedTarget->WorldSpaceCenter() - EyePosition();
	float flDist = vecToTarget.Length2D();
	VectorNormalize(vecToTarget);

	if (flDist > autoaim_max_dist.GetFloat())
		return false;

	float flDot;

	vecLooking = EyeDirection3D();
	flDot = DotProduct(vecLooking, vecToTarget);

	if (flDot < autoaim_unlock_target.GetFloat())
		return false;

	return true;
}

//-----------------------------------------------------------------------------
//Purpose: 
//Input  : iCount - 
//			iAmmoIndex - 
//			bSuppressSound - 
//Output : int
//-----------------------------------------------------------------------------
int CHL2_Player::GiveAmmo(int nCount, int nAmmoIndex, bool bSuppressSound)
{
	//Don't try to give the player invalid ammo indices.
	if (nAmmoIndex < 0)
		return 0;

	bool bCheckAutoSwitch = false;
	if (!HasAnyAmmoOfType(nAmmoIndex))
	{
		bCheckAutoSwitch = true;
	}

	int nAdd = BaseClass::GiveAmmo(nCount, nAmmoIndex, bSuppressSound);

	if (nCount > 0 && nAdd == 0)
	{
		//we've been denied the pickup, display a hud icon to show that
		CSingleUserRecipientFilter user(this);
		user.MakeReliable();
		UserMessageBegin(user, "AmmoDenied");
		WRITE_SHORT(nAmmoIndex);
		MessageEnd();
	}

	//
	//If I was dry on ammo for my best weapon and justed picked up ammo for it,
	//autoswitch to my best weapon now.
	//
	if (bCheckAutoSwitch)
	{
		CBaseCombatWeapon* pWeapon = g_pGameRules->GetNextBestWeapon(this, GetActiveWeapon());

		if (pWeapon && pWeapon->GetPrimaryAmmoType() == nAmmoIndex)
		{
			SwitchToNextBestWeapon(GetActiveWeapon());
		}
	}

	return nAdd;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_CanUse(CBaseCombatWeapon* pWeapon)
{
	if (pWeapon->ClassMatches("weapon_stunstick") && !pWeapon->m_bChaosSpawned)
	{
		if (ApplyBattery(0.5))
			UTIL_Remove(pWeapon);
		return false;
	}

	return BaseClass::Weapon_CanUse(pWeapon);
}

//-----------------------------------------------------------------------------
//Purpose: 
//Input  : *pWeapon - 
//-----------------------------------------------------------------------------
void CHL2_Player::Weapon_Equip(CBaseCombatWeapon* pWeapon)
{
#if	HL2_SINGLE_PRIMARY_WEAPON_MODE

	if (pWeapon->GetSlot() == WEAPON_PRIMARY_SLOT)
	{
		Weapon_DropSlot(WEAPON_PRIMARY_SLOT);
	}

#endif

	if (GetActiveWeapon() == NULL)
	{
		m_HL2Local.m_bWeaponLowered = false;
	}

	BaseClass::Weapon_Equip(pWeapon);
}

//-----------------------------------------------------------------------------
//Purpose: Player reacts to bumping a weapon. 
//Input  : pWeapon - the weapon that the player bumped into.
//Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2_Player::BumpWeapon(CBaseCombatWeapon* pWeapon)
{

#if	HL2_SINGLE_PRIMARY_WEAPON_MODE

	CBaseCombatCharacter* pOwner = pWeapon->GetOwner();

	//Can I have this weapon type?
	if (pOwner || !Weapon_CanUse(pWeapon) || !g_pGameRules->CanHavePlayerItem(this, pWeapon))
	{
		if (gEvilImpulse101)
		{
			UTIL_Remove(pWeapon);
		}
		return false;
	}

	//----------------------------------------
	//If I already have it just take the ammo
	//----------------------------------------
	if (Weapon_OwnsThisType(pWeapon->GetClassname(), pWeapon->GetSubType()))
	{
		//Only remove the weapon if we attained ammo from it
		if (Weapon_EquipAmmoOnly(pWeapon) == false)
			return false;

		//Only remove me if I have no ammo left
		//Can't just check HasAnyAmmo because if I don't use clips, I want to be removed, 
		if (pWeapon->UsesClipsForAmmo1() && pWeapon->HasPrimaryAmmo())
			return false;

		UTIL_Remove(pWeapon);
		return false;
	}
	//-------------------------
	//Otherwise take the weapon
	//-------------------------
	else
	{
		//Make sure we're not trying to take a new weapon type we already have
		if (Weapon_SlotOccupied(pWeapon))
		{
			CBaseCombatWeapon* pActiveWeapon = Weapon_GetSlot(WEAPON_PRIMARY_SLOT);

			if (pActiveWeapon != NULL && pActiveWeapon->HasAnyAmmo() == false && Weapon_CanSwitchTo(pWeapon))
			{
				Weapon_Equip(pWeapon);
				return true;
			}

			//Attempt to take ammo if this is the gun we're holding already
			if (Weapon_OwnsThisType(pWeapon->GetClassname(), pWeapon->GetSubType()))
			{
				Weapon_EquipAmmoOnly(pWeapon);
			}

			return false;
		}

		pWeapon->CheckRespawn();

		pWeapon->AddSolidFlags(FSOLID_NOT_SOLID);
		pWeapon->AddEffects(EF_NODRAW);

		Weapon_Equip(pWeapon);

		EmitSound("HL2Player.PickupWeapon");

		return true;
	}
#else

	return BaseClass::BumpWeapon(pWeapon);

#endif

}

//-----------------------------------------------------------------------------
//Purpose: 
//Input  : *cmd - 
//Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::ClientCommand(const CCommand& args)
{
#if	HL2_SINGLE_PRIMARY_WEAPON_MODE

	//Drop primary weapon
	if (!Q_stricmp(args[0], "DropPrimary"))
	{
		Weapon_DropSlot(WEAPON_PRIMARY_SLOT);
		return true;
	}

#endif

	if (!Q_stricmp(args[0], "emit"))
	{
		CSingleUserRecipientFilter filter(this);
		if (args.ArgC() > 1)
		{
			EmitSound(filter, entindex(), args[1]);
		}
		else
		{
			EmitSound(filter, entindex(), "Test.Sound");
		}
		return true;
	}

	return BaseClass::ClientCommand(args);
}

//-----------------------------------------------------------------------------
//Purpose: 
//Output : void CBasePlayer::PlayerUse
//-----------------------------------------------------------------------------
void CHL2_Player::PlayerUse(void)
{
	//Was use pressed or released?
	if (!((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE))
		return;

	if (m_afButtonPressed & IN_USE)
	{
		//Currently using a latched entity?
		if (ClearUseEntity())
		{
			return;
		}
		else
		{
			if (m_afPhysicsFlags & PFLAG_DIROVERRIDE)
			{
				m_afPhysicsFlags &= ~PFLAG_DIROVERRIDE;
				m_iTrain = TRAIN_NEW | TRAIN_OFF;
				return;
			}
			else
			{	//Start controlling the train!
				CBaseEntity* pTrain = GetGroundEntity();
				if (pTrain && !(m_nButtons & IN_JUMP) && (GetFlags() & FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(this))
				{
					m_afPhysicsFlags |= PFLAG_DIROVERRIDE;
					m_iTrain = TrainSpeed(pTrain->m_flSpeed, ((CFuncTrackTrain*)pTrain)->GetMaxSpeed());
					m_iTrain |= TRAIN_NEW;
					EmitSound("HL2Player.TrainUse");
					return;
				}
			}
		}

		//Tracker 3926:  We can't +USE something if we're climbing a ladder
		if (GetMoveType() == MOVETYPE_LADDER)
		{
			return;
		}
	}

	if (m_flTimeUseSuspended > gpGlobals->curtime)
	{
		//Something has temporarily stopped us being able to USE things.
		//Obviously, this should be used very carefully.(sjb)
		return;
	}

	CBaseEntity* pUseEntity = FindUseEntity();

	bool usedSomething = false;

	//Found an object
	if (pUseEntity && gEntList.FindEntityByClassname(NULL, "player_pickup") == NULL)
	{
		//!!!UNDONE: traceline here to prevent +USEing buttons through walls			
		int caps = pUseEntity->ObjectCaps();
		variant_t emptyVariant;

		if (m_afButtonPressed & IN_USE)
		{
			//Robin: Don't play sounds for NPCs, because NPCs will allow respond with speech.
			if (!pUseEntity->MyNPCPointer())
			{
				EmitSound("HL2Player.Use");
			}
		}

		if (((m_nButtons & IN_USE) && ((caps & FCAP_CONTINUOUS_USE))) ||
			((m_afButtonPressed & IN_USE) && ((caps & (FCAP_IMPULSE_USE | FCAP_ONOFF_USE)))))
		{
			if (caps & FCAP_CONTINUOUS_USE)
				m_afPhysicsFlags |= PFLAG_USING;

			pUseEntity->AcceptInput("Use", this, this, emptyVariant, USE_TOGGLE);

			usedSomething = true;
		}
		//UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
		else if ((m_afButtonReleased & IN_USE) && (pUseEntity->ObjectCaps() & FCAP_ONOFF_USE))	//BUGBUG This is an "off" use
		{
			pUseEntity->AcceptInput("Use", this, this, emptyVariant, USE_TOGGLE);

			usedSomething = true;
		}

#if	HL2_SINGLE_PRIMARY_WEAPON_MODE

		//Check for weapon pick-up
		if (m_afButtonPressed & IN_USE)
		{
			CBaseCombatWeapon* pWeapon = dynamic_cast<CBaseCombatWeapon*>(pUseEntity);

			if ((pWeapon != NULL) && (Weapon_CanSwitchTo(pWeapon)))
			{
				//Try to take ammo or swap the weapon
				if (Weapon_OwnsThisType(pWeapon->GetClassname(), pWeapon->GetSubType()))
				{
					Weapon_EquipAmmoOnly(pWeapon);
				}
				else
				{
					Weapon_DropSlot(pWeapon->GetSlot());
					Weapon_Equip(pWeapon);
				}

				usedSomething = true;
			}
		}
#endif
	}
	else if (m_afButtonPressed & IN_USE)
	{
		//Signal that we want to play the deny sound, unless the user is +USEing on a ladder!
		//The sound is emitted in ItemPostFrame, since that occurs after GameMovement::ProcessMove which
		//lets the ladder code unset this flag.
		m_bPlayUseDenySound = true;
	}

	//Debounce the use key
	if (usedSomething && pUseEntity)
	{
		m_Local.m_nOldButtons |= IN_USE;
		m_afButtonPressed &= ~IN_USE;
	}
}

ConVar	sv_show_crosshair_target("sv_show_crosshair_target", "0");

//-----------------------------------------------------------------------------
//Purpose: Updates the posture of the weapon from lowered to ready
//-----------------------------------------------------------------------------
void CHL2_Player::UpdateWeaponPosture(void)
{
	CBaseCombatWeapon* pWeapon = dynamic_cast<CBaseCombatWeapon*>(GetActiveWeapon());

	if (pWeapon && m_LowerWeaponTimer.Expired() && pWeapon->CanLower())
	{
		m_LowerWeaponTimer.Set(.3);
		VPROF("CHL2_Player::UpdateWeaponPosture-CheckLower");
		Vector vecAim = BaseClass::GetAutoaimVector(AUTOAIM_SCALE_DIRECT_ONLY);

		const float CHECK_FRIENDLY_RANGE = 50 * 12;
		trace_t	tr;
		UTIL_TraceLine(EyePosition(), EyePosition() + vecAim * CHECK_FRIENDLY_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		CBaseEntity* aimTarget = tr.m_pEnt;

		//If we're over something
		if (aimTarget && !tr.DidHitWorld())
		{
			if (!aimTarget->IsNPC() || aimTarget->MyNPCPointer()->GetState() != NPC_STATE_COMBAT)
			{
				Disposition_t dis = IRelationType(aimTarget);

				//Debug info for seeing what an object "cons" as
				if (sv_show_crosshair_target.GetBool())
				{
					int text_offset = BaseClass::DrawDebugTextOverlays();

					char tempstr[255];

					switch (dis)
					{
					case D_LI:
						Q_snprintf(tempstr, sizeof(tempstr), "Disposition: Like");
						break;

					case D_HT:
						Q_snprintf(tempstr, sizeof(tempstr), "Disposition: Hate");
						break;

					case D_FR:
						Q_snprintf(tempstr, sizeof(tempstr), "Disposition: Fear");
						break;

					case D_NU:
						Q_snprintf(tempstr, sizeof(tempstr), "Disposition: Neutral");
						break;

					default:
					case D_ER:
						Q_snprintf(tempstr, sizeof(tempstr), "Disposition: !!!ERROR!!!");
						break;
					}

					//Draw the text
					NDebugOverlay::EntityText(aimTarget->entindex(), text_offset, tempstr, 0);
				}

				//See if we hates it
				if (dis == D_LI)
				{
					//We're over a friendly, drop our weapon
					if (Weapon_Lower() == false)
					{
						//FIXME: We couldn't lower our weapon!
					}

					return;
				}
			}
		}

		if (Weapon_Ready() == false)
		{
			//FIXME: We couldn't raise our weapon!
		}
	}

	if (g_pGameRules->GetAutoAimMode() != AUTOAIM_NONE)
	{
		if (!pWeapon)
		{
			//This tells the client to draw no crosshair
			m_HL2Local.m_bWeaponLowered = true;
			return;
		}
		else
		{
			if (!pWeapon->CanLower() && m_HL2Local.m_bWeaponLowered)
				m_HL2Local.m_bWeaponLowered = false;
		}

		if (!m_AutoaimTimer.Expired())
			return;

		m_AutoaimTimer.Set(.1);

		VPROF("hl2_x360_aiming");

		//Call the autoaim code to update the local player data, which allows the client to update.
		autoaim_params_t params;
		params.m_vecAutoAimPoint.Init();
		params.m_vecAutoAimDir.Init();
		params.m_fScale = AUTOAIM_SCALE_DEFAULT;
		params.m_fMaxDist = autoaim_max_dist.GetFloat();
		GetAutoaimVector(params);
		m_HL2Local.m_hAutoAimTarget.Set(params.m_hAutoAimEntity);
		m_HL2Local.m_vecAutoAimPoint.Set(params.m_vecAutoAimPoint);
		m_HL2Local.m_bAutoAimTarget = (params.m_bAutoAimAssisting || params.m_bOnTargetNatural);
		return;
	}
	else
	{
		//Make sure there's no residual autoaim target if the user changes the xbox_aiming convar on the fly.
		m_HL2Local.m_hAutoAimTarget.Set(NULL);
	}
}

//-----------------------------------------------------------------------------
//Purpose: Lowers the weapon posture (for hovering over friendlies)
//Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_Lower(void)
{
	VPROF("CHL2_Player::Weapon_Lower");
	//Already lowered?
	if (m_HL2Local.m_bWeaponLowered)
		return true;

	m_HL2Local.m_bWeaponLowered = true;

	CBaseCombatWeapon* pWeapon = dynamic_cast<CBaseCombatWeapon*>(GetActiveWeapon());

	if (pWeapon == NULL)
		return false;

	return pWeapon->Lower();
}

//-----------------------------------------------------------------------------
//Purpose: Returns the weapon posture to normal
//Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_Ready(void)
{
	VPROF("CHL2_Player::Weapon_Ready");

	//Already ready?
	if (m_HL2Local.m_bWeaponLowered == false)
		return true;

	m_HL2Local.m_bWeaponLowered = false;

	CBaseCombatWeapon* pWeapon = dynamic_cast<CBaseCombatWeapon*>(GetActiveWeapon());

	if (pWeapon == NULL)
		return false;

	return pWeapon->Ready();
}

//-----------------------------------------------------------------------------
//Purpose: Returns whether or not we can switch to the given weapon.
//Input  : pWeapon - 
//-----------------------------------------------------------------------------
bool CHL2_Player::Weapon_CanSwitchTo(CBaseCombatWeapon* pWeapon)
{
	CBasePlayer* pPlayer = (CBasePlayer*)this;
#if !defined( CLIENT_DLL )
	IServerVehicle* pVehicle = pPlayer->GetVehicle();
#else
	IClientVehicle* pVehicle = pPlayer->GetVehicle();
#endif
	if (pVehicle && !pPlayer->UsingStandardWeaponsInVehicle())
		return false;

	if (!pWeapon->HasAnyAmmo() && !GetAmmoCount(pWeapon->m_iPrimaryAmmoType))
		return false;

	if (!pWeapon->CanDeploy())
		return false;

	if (GetActiveWeapon())
	{
		if (PhysCannonGetHeldEntity(GetActiveWeapon()) == pWeapon &&
			Weapon_OwnsThisType(pWeapon->GetClassname(), pWeapon->GetSubType()))
		{
			return true;
		}

		if (!GetActiveWeapon()->CanHolster())
			return false;
	}

	return true;
}

void CHL2_Player::PickupObject(CBaseEntity* pObject, bool bLimitMassAndSize)
{
	//can't pick up what you're standing on
	if (GetGroundEntity() == pObject)
		return;

	if (bLimitMassAndSize == true && !m_bSuperGrab)
	{
		if ((CBasePlayer::CanPickupObject(pObject, 35, 128, m_bSuperGrab)) == false)
			return;
	}

	//Can't be picked up if NPCs are on me
	if (pObject->HasNPCsOnIt())
		return;

	PlayerPickupObject(this, pObject);
}

//-----------------------------------------------------------------------------
//Purpose: 
//Output : CBaseEntity
//-----------------------------------------------------------------------------
bool CHL2_Player::IsHoldingEntity(CBaseEntity* pEnt)
{
	return PlayerPickupControllerIsHoldingEntity(m_hUseEntity, pEnt);
}

float CHL2_Player::GetHeldObjectMass(IPhysicsObject* pHeldObject)
{
	float mass = PlayerPickupGetHeldObjectMass(m_hUseEntity, pHeldObject);
	if (mass == 0.0f)
	{
		mass = PhysCannonGetHeldObjectMass(GetActiveWeapon(), pHeldObject);
	}
	return mass;
}

//-----------------------------------------------------------------------------
//Purpose: Force the player to drop any physics objects he's carrying
//-----------------------------------------------------------------------------
void CHL2_Player::ForceDropOfCarriedPhysObjects(CBaseEntity* pOnlyIfHoldingThis)
{
	if (PhysIsInCallback())
	{
		variant_t value;
		g_EventQueue.AddEvent(this, "ForceDropPhysObjects", value, 0.01f, pOnlyIfHoldingThis, this);
		return;
	}

#ifdef HL2_EPISODIC
	if (hl2_episodic.GetBool())
	{
		CBaseEntity* pHeldEntity = PhysCannonGetHeldEntity(GetActiveWeapon());
		if (pHeldEntity && pHeldEntity->ClassMatches("grenade_helicopter"))
		{
			return;
		}
	}
#endif

	//Drop any objects being handheld.
	ClearUseEntity();

	//Then force the physcannon to drop anything it's holding, if it's our active weapon
	PhysCannonForceDrop(GetActiveWeapon(), NULL);
}

void CHL2_Player::InputForceDropPhysObjects(inputdata_t& data)
{
	ForceDropOfCarriedPhysObjects(data.pActivator);
}


//-----------------------------------------------------------------------------
//Purpose: 
//-----------------------------------------------------------------------------
void CHL2_Player::UpdateClientData(void)
{
	if (m_DmgTake || m_DmgSave || m_bitsHUDDamage != m_bitsDamageType)
	{
		//Comes from inside me if not set
		Vector damageOrigin = GetLocalOrigin();
		//send "damage" message
		//causes screen to flash, and pain compass to show direction of damage
		damageOrigin = m_DmgOrigin;

		//only send down damage type that have hud art
		int iShowHudDamage = g_pGameRules->Damage_GetShowOnHud();
		int visibleDamageBits = m_bitsDamageType & iShowHudDamage;

		m_DmgTake = clamp(m_DmgTake, 0, 255);
		m_DmgSave = clamp(m_DmgSave, 0, 255);

		//If we're poisoned, but it wasn't this frame, don't send the indicator
		//Without this check, any damage that occured to the player while they were
		//recovering from a poison bite would register as poisonous as well and flash
		//the whole screen! -- jdw
		if (visibleDamageBits & DMG_POISON)
		{
			float flLastPoisonedDelta = gpGlobals->curtime - m_tbdPrev;
			if (flLastPoisonedDelta > 0.1f)
			{
				visibleDamageBits &= ~DMG_POISON;
			}
		}

		CSingleUserRecipientFilter user(this);
		user.MakeReliable();
		UserMessageBegin(user, "Damage");
		WRITE_BYTE(m_DmgSave);
		WRITE_BYTE(m_DmgTake);
		WRITE_LONG(visibleDamageBits);
		WRITE_FLOAT(damageOrigin.x);	//BUG: Should be fixed point (to hud) not floats
		WRITE_FLOAT(damageOrigin.y);	//BUG: However, the HUD does _not_ implement bitfield messages (yet)
		WRITE_FLOAT(damageOrigin.z);	//BUG: We use WRITE_VEC3COORD for everything else
		MessageEnd();

		m_DmgTake = 0;
		m_DmgSave = 0;
		m_bitsHUDDamage = m_bitsDamageType;

		//Clear off non-time-based damage indicators
		int iTimeBasedDamage = g_pGameRules->Damage_GetTimeBased();
		m_bitsDamageType &= iTimeBasedDamage;
	}

	//Update Flashlight
#ifdef HL2_EPISODIC
	if (Flashlight_UseLegacyVersion() == false)
	{
		if (FlashlightIsOn() && sv_infinite_aux_power.GetBool() == false)
		{
			m_HL2Local.m_flFlashBattery -= FLASH_DRAIN_TIME * gpGlobals->frametime;
			if (m_HL2Local.m_flFlashBattery < 0.0f)
			{
				FlashlightTurnOff();
				m_HL2Local.m_flFlashBattery = 0.0f;
			}
		}
		else
		{
			m_HL2Local.m_flFlashBattery += FLASH_CHARGE_TIME * gpGlobals->frametime;
			if (m_HL2Local.m_flFlashBattery > 100.0f)
			{
				m_HL2Local.m_flFlashBattery = 100.0f;
			}
		}
	}
	else
	{
		m_HL2Local.m_flFlashBattery = -1.0f;
	}
#endif //HL2_EPISODIC

	BaseClass::UpdateClientData();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CHL2_Player::OnRestore()
{
	BaseClass::OnRestore();
	/*
	if (m_pController)
	{
		m_pController->SetEventHandler(this);
	}
	*/
	m_pPlayerAISquad = g_AI_SquadManager.FindCreateSquad(AllocPooledString(PLAYER_SQUADNAME));
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CHL2_Player::EyeDirection2D(void)
{
	Vector vecReturn = EyeDirection3D();
	vecReturn.z = 0;
	vecReturn.AsVector2D().NormalizeInPlace();

	return vecReturn;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CHL2_Player::EyeDirection3D(void)
{
	Vector vecForward;

	//Return the vehicle angles if we request them
	if (GetVehicle() != NULL)
	{
		CacheVehicleView();
		EyeVectors(&vecForward);
		return vecForward;
	}

	AngleVectors(EyeAngles(), &vecForward);
	return vecForward;
}


//---------------------------------------------------------
//---------------------------------------------------------
bool CHL2_Player::Weapon_Switch(CBaseCombatWeapon* pWeapon, int viewmodelindex)
{
	MDLCACHE_CRITICAL_SECTION();

	//Recalculate proficiency!
	SetCurrentWeaponProficiency(CalcWeaponProficiency(pWeapon));

	//Come out of suit zoom mode
	if (IsZooming())
	{
		StopZooming();
	}

	return BaseClass::Weapon_Switch(pWeapon, viewmodelindex);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
WeaponProficiency_t CHL2_Player::CalcWeaponProficiency(CBaseCombatWeapon* pWeapon)
{
	WeaponProficiency_t proficiency;

	proficiency = WEAPON_PROFICIENCY_PERFECT;

	if (weapon_showproficiency.GetBool() != 0)
	{
		Msg("Player switched to %s, proficiency is %s\n", pWeapon->GetClassname(), GetWeaponProficiencyName(proficiency));
	}

	return proficiency;
}

//-----------------------------------------------------------------------------
//Purpose: override how single player rays hit the player
//-----------------------------------------------------------------------------

bool LineCircleIntersection(
	const Vector2D& center,
	const float radius,
	const Vector2D& vLinePt,
	const Vector2D& vLineDir,
	float* fIntersection1,
	float* fIntersection2)
{
	//Line = P + Vt
	//Sphere = r (assume we've translated to origin)
	//(P + Vt)^2 = r^2
	//VVt^2 + 2PVt + (PP - r^2)
	//Solve as quadratic:  (-b  +/-  sqrt(b^2 - 4ac)) / 2a
	//If (b^2 - 4ac) is < 0 there is no solution.
	//If (b^2 - 4ac) is = 0 there is one solution (a case this function doesn't support).
	//If (b^2 - 4ac) is > 0 there are two solutions.
	Vector2D P;
	float a, b, c, sqr, insideSqr;


	//Translate circle to origin.
	P[0] = vLinePt[0] - center[0];
	P[1] = vLinePt[1] - center[1];

	a = vLineDir.Dot(vLineDir);
	b = 2.0f * P.Dot(vLineDir);
	c = P.Dot(P) - (radius * radius);

	insideSqr = b * b - 4 * a * c;
	if (insideSqr <= 0.000001f)
		return false;

	//Ok, two solutions.
	sqr = (float)FastSqrt(insideSqr);

	float denom = 1.0 / (2.0f * a);

	*fIntersection1 = (-b - sqr) * denom;
	*fIntersection2 = (-b + sqr) * denom;

	return true;
}

static void Collision_ClearTrace(const Vector& vecRayStart, const Vector& vecRayDelta, CBaseTrace* pTrace)
{
	pTrace->startpos = vecRayStart;
	pTrace->endpos = vecRayStart;
	pTrace->endpos += vecRayDelta;
	pTrace->startsolid = false;
	pTrace->allsolid = false;
	pTrace->fraction = 1.0f;
	pTrace->contents = 0;
}


bool IntersectRayWithAACylinder(const Ray_t& ray,
	const Vector& center, float radius, float height, CBaseTrace* pTrace)
{
	Assert(ray.m_IsRay);
	Collision_ClearTrace(ray.m_Start, ray.m_Delta, pTrace);

	//First intersect the ray with the top + bottom planes
	float halfHeight = height * 0.5;

	//Handle parallel case
	Vector vStart = ray.m_Start - center;
	Vector vEnd = vStart + ray.m_Delta;

	float flEnterFrac, flLeaveFrac;
	if (FloatMakePositive(ray.m_Delta.z) < 1e-8)
	{
		if ((vStart.z < -halfHeight) || (vStart.z > halfHeight))
		{
			return false; //no hit
		}
		flEnterFrac = 0.0f; flLeaveFrac = 1.0f;
	}
	else
	{
		//Clip the ray to the top and bottom of box
		flEnterFrac = IntersectRayWithAAPlane(vStart, vEnd, 2, 1, halfHeight);
		flLeaveFrac = IntersectRayWithAAPlane(vStart, vEnd, 2, 1, -halfHeight);

		if (flLeaveFrac < flEnterFrac)
		{
			float temp = flLeaveFrac;
			flLeaveFrac = flEnterFrac;
			flEnterFrac = temp;
		}

		if (flLeaveFrac < 0 || flEnterFrac > 1)
		{
			return false;
		}
	}

	//Intersect with circle
	float flCircleEnterFrac, flCircleLeaveFrac;
	if (!LineCircleIntersection(vec3_origin.AsVector2D(), radius,
		vStart.AsVector2D(), ray.m_Delta.AsVector2D(), &flCircleEnterFrac, &flCircleLeaveFrac))
	{
		return false; //no hit
	}

	Assert(flCircleEnterFrac <= flCircleLeaveFrac);
	if (flCircleLeaveFrac < 0 || flCircleEnterFrac > 1)
	{
		return false;
	}

	if (flEnterFrac < flCircleEnterFrac)
		flEnterFrac = flCircleEnterFrac;
	if (flLeaveFrac > flCircleLeaveFrac)
		flLeaveFrac = flCircleLeaveFrac;

	if (flLeaveFrac < flEnterFrac)
		return false;

	VectorMA(ray.m_Start, flEnterFrac, ray.m_Delta, pTrace->endpos);
	pTrace->fraction = flEnterFrac;
	pTrace->contents = CONTENTS_SOLID;

	//Calculate the point on our center line where we're nearest the intersection point
	Vector collisionCenter;
	CalcClosestPointOnLineSegment(pTrace->endpos, center + Vector(0, 0, halfHeight), center - Vector(0, 0, halfHeight), collisionCenter);

	//Our normal is the direction from that center point to the intersection point
	pTrace->plane.normal = pTrace->endpos - collisionCenter;
	VectorNormalize(pTrace->plane.normal);

	return true;
}


bool CHL2_Player::TestHitboxes(const Ray_t& ray, unsigned int fContentsMask, trace_t& tr)
{
	if (g_pGameRules->IsMultiplayer())
	{
		return BaseClass::TestHitboxes(ray, fContentsMask, tr);
	}
	else
	{
		Assert(ray.m_IsRay);

		Vector mins, maxs;

		mins = WorldAlignMins();
		maxs = WorldAlignMaxs();

		if (IntersectRayWithAACylinder(ray, WorldSpaceCenter(), maxs.x * PLAYER_HULL_REDUCTION, maxs.z - mins.z, &tr))
		{
			tr.hitbox = 0;
			CStudioHdr* pStudioHdr = GetModelPtr();
			if (!pStudioHdr)
				return false;

			mstudiohitboxset_t* set = pStudioHdr->pHitboxSet(m_nHitboxSet);
			if (!set || !set->numhitboxes)
				return false;

			mstudiobbox_t* pbox = set->pHitbox(tr.hitbox);
			mstudiobone_t* pBone = pStudioHdr->pBone(pbox->bone);
			tr.surface.name = "**studio**";
			tr.surface.flags = SURF_HITBOX;
			tr.surface.surfaceProps = physprops->GetSurfaceIndex(pBone->pszSurfaceProp());
		}

		return true;
	}
}

//---------------------------------------------------------
//Show the player's scaled down bbox that we use for
//bullet impacts.
//---------------------------------------------------------
void CHL2_Player::DrawDebugGeometryOverlays(void)
{
	BaseClass::DrawDebugGeometryOverlays();

	if (m_debugOverlays & OVERLAY_BBOX_BIT)
	{
		Vector mins, maxs;

		mins = WorldAlignMins();
		maxs = WorldAlignMaxs();

		mins.x *= PLAYER_HULL_REDUCTION;
		mins.y *= PLAYER_HULL_REDUCTION;

		maxs.x *= PLAYER_HULL_REDUCTION;
		maxs.y *= PLAYER_HULL_REDUCTION;

		NDebugOverlay::Box(GetAbsOrigin(), mins, maxs, 255, 0, 0, 100, 0);
	}
}

//-----------------------------------------------------------------------------
//Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void CHL2_Player::ExitLadder()
{
	if (MOVETYPE_LADDER != GetMoveType())
		return;

	SetMoveType(MOVETYPE_WALK);
	SetMoveCollide(MOVECOLLIDE_DEFAULT);
	//Remove from ladder
	m_HL2Local.m_hLadder.Set(NULL);
}


surfacedata_t* CHL2_Player::GetLadderSurface(const Vector& origin)
{
	extern const char* FuncLadder_GetSurfaceprops(CBaseEntity * pLadderEntity);

	CBaseEntity* pLadder = m_HL2Local.m_hLadder.Get();
	if (pLadder)
	{
		const char* pSurfaceprops = FuncLadder_GetSurfaceprops(pLadder);
		//get ladder material from func_ladder
		return physprops->GetSurfaceData(physprops->GetSurfaceIndex(pSurfaceprops));

	}
	return BaseClass::GetLadderSurface(origin);
}

//-----------------------------------------------------------------------------
//Purpose: Queues up a use deny sound, played in ItemPostFrame.
//-----------------------------------------------------------------------------
void CHL2_Player::PlayUseDenySound()
{
	m_bPlayUseDenySound = true;
}


void CHL2_Player::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if (m_bPlayUseDenySound)
	{
		m_bPlayUseDenySound = false;
		EmitSound("HL2Player.UseDeny");
	}
}


void CHL2_Player::StartWaterDeathSounds(void)
{
	CPASAttenuationFilter filter(this);

	if (m_sndLeeches == NULL)
	{
		m_sndLeeches = (CSoundEnvelopeController::GetController()).SoundCreate(filter, entindex(), CHAN_STATIC, "coast.leech_bites_loop", ATTN_NORM);
	}

	if (m_sndLeeches)
	{
		(CSoundEnvelopeController::GetController()).Play(m_sndLeeches, 1.0f, 100);
	}

	if (m_sndWaterSplashes == NULL)
	{
		m_sndWaterSplashes = (CSoundEnvelopeController::GetController()).SoundCreate(filter, entindex(), CHAN_STATIC, "coast.leech_water_churn_loop", ATTN_NORM);
	}

	if (m_sndWaterSplashes)
	{
		(CSoundEnvelopeController::GetController()).Play(m_sndWaterSplashes, 1.0f, 100);
	}
}

void CHL2_Player::StopWaterDeathSounds(void)
{
	if (m_sndLeeches)
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut(m_sndLeeches, 0.5f, true);
		m_sndLeeches = NULL;
	}

	if (m_sndWaterSplashes)
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut(m_sndWaterSplashes, 0.5f, true);
		m_sndWaterSplashes = NULL;
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CHL2_Player::MissedAR2AltFire()
{
	if (GetPlayerProxy() != NULL)
	{
		GetPlayerProxy()->m_PlayerMissedAR2AltFire.FireOutput(this, this);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CHL2_Player::DisplayLadderHudHint()
{
#if !defined( CLIENT_DLL )
	if (gpGlobals->curtime > m_flTimeNextLadderHint)
	{
		m_flTimeNextLadderHint = gpGlobals->curtime + 60.0f;

		CFmtStr hint;
		hint.sprintf("#Valve_Hint_Ladder");
		UTIL_HudHintText(this, hint.Access());
	}
#endif//CLIENT_DLL
}

//-----------------------------------------------------------------------------
//Shuts down sounds
//-----------------------------------------------------------------------------
void CHL2_Player::StopLoopingSounds(void)
{
	if (m_sndLeeches != NULL)
	{
		(CSoundEnvelopeController::GetController()).SoundDestroy(m_sndLeeches);
		m_sndLeeches = NULL;
	}

	if (m_sndWaterSplashes != NULL)
	{
		(CSoundEnvelopeController::GetController()).SoundDestroy(m_sndWaterSplashes);
		m_sndWaterSplashes = NULL;
	}

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
void CHL2_Player::ModifyOrAppendPlayerCriteria(AI_CriteriaSet& set)
{
	BaseClass::ModifyOrAppendPlayerCriteria(set);

	if (GlobalEntity_GetIndex("gordon_precriminal") == -1)
	{
		set.AppendCriteria("gordon_precriminal", "0");
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const impactdamagetable_t& CHL2_Player::GetPhysicsImpactDamageTable()
{
	if (m_bUseCappedPhysicsDamageTable)
		return gCappedPlayerImpactDamageTable;

	return BaseClass::GetPhysicsImpactDamageTable();
}


//-----------------------------------------------------------------------------
//Purpose: Makes a splash when the player transitions between water states
//-----------------------------------------------------------------------------
void CHL2_Player::Splash(void)
{
	CEffectData data;
	data.m_fFlags = 0;
	data.m_vOrigin = GetAbsOrigin();
	data.m_vNormal = Vector(0, 0, 1);
	data.m_vAngles = QAngle(0, 0, 0);

	if (GetWaterType() & CONTENTS_SLIME)
	{
		data.m_fFlags |= FX_WATER_IN_SLIME;
	}

	float flSpeed = GetAbsVelocity().Length();
	if (flSpeed < 300)
	{
		data.m_flScale = RandomFloat(10, 12);
		DispatchEffect("waterripple", data);
	}
	else
	{
		data.m_flScale = RandomFloat(6, 8);
		DispatchEffect("watersplash", data);
	}
}

CLogicPlayerProxy* CHL2_Player::GetPlayerProxy(void)
{
	CLogicPlayerProxy* pProxy = dynamic_cast<CLogicPlayerProxy*> (m_hPlayerProxy.Get());

	if (pProxy == NULL)
	{
		pProxy = (CLogicPlayerProxy*)gEntList.FindEntityByClassname(NULL, "logic_playerproxy");

		if (pProxy == NULL)
			return NULL;

		pProxy->m_hPlayer = this;
		m_hPlayerProxy = pProxy;
	}

	return pProxy;
}

void CHL2_Player::FirePlayerProxyOutput(const char* pszOutputName, variant_t variant, CBaseEntity* pActivator, CBaseEntity* pCaller)
{
	if (GetPlayerProxy() == NULL)
		return;

	GetPlayerProxy()->FireNamedOutput(pszOutputName, variant, pActivator, pCaller);
}

void CHL2_Player::OnEntitySpawned(CBaseEntity* pEntity)
{
	for (int i = 1; i < g_ChaosEffects.Size(); i++)
	{
		CChaosEffect* pEffect = g_ChaosEffects[i];
		if (pEffect->m_bActive)
			pEffect->OnEntitySpawned(pEntity);
	}
}

LINK_ENTITY_TO_CLASS(logic_playerproxy, CLogicPlayerProxy);

BEGIN_DATADESC(CLogicPlayerProxy)
DEFINE_OUTPUT(m_OnFlashlightOn, "OnFlashlightOn"),
DEFINE_OUTPUT(m_OnFlashlightOff, "OnFlashlightOff"),
DEFINE_OUTPUT(m_RequestedPlayerHealth, "PlayerHealth"),
DEFINE_OUTPUT(m_PlayerHasAmmo, "PlayerHasAmmo"),
DEFINE_OUTPUT(m_PlayerHasNoAmmo, "PlayerHasNoAmmo"),
DEFINE_OUTPUT(m_PlayerDied, "PlayerDied"),
DEFINE_OUTPUT(m_PlayerMissedAR2AltFire, "PlayerMissedAR2AltFire"),
DEFINE_INPUTFUNC(FIELD_VOID, "RequestPlayerHealth", InputRequestPlayerHealth),
DEFINE_INPUTFUNC(FIELD_VOID, "SetFlashlightSlowDrain", InputSetFlashlightSlowDrain),
DEFINE_INPUTFUNC(FIELD_VOID, "SetFlashlightNormalDrain", InputSetFlashlightNormalDrain),
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetPlayerHealth", InputSetPlayerHealth),
DEFINE_INPUTFUNC(FIELD_VOID, "RequestAmmoState", InputRequestAmmoState),
DEFINE_INPUTFUNC(FIELD_VOID, "LowerWeapon", InputLowerWeapon),
DEFINE_INPUTFUNC(FIELD_VOID, "EnableCappedPhysicsDamage", InputEnableCappedPhysicsDamage),
DEFINE_INPUTFUNC(FIELD_VOID, "DisableCappedPhysicsDamage", InputDisableCappedPhysicsDamage),
DEFINE_INPUTFUNC(FIELD_STRING, "SetLocatorTargetEntity", InputSetLocatorTargetEntity),
DEFINE_FIELD(m_hPlayer, FIELD_EHANDLE),
END_DATADESC()

void CLogicPlayerProxy::Activate(void)
{
	BaseClass::Activate();

	if (m_hPlayer == NULL)
	{
		m_hPlayer = AI_GetSinglePlayer();
	}
}

bool CLogicPlayerProxy::PassesDamageFilter(const CTakeDamageInfo& info)
{
	if (m_hDamageFilter)
	{
		CBaseFilter* pFilter = (CBaseFilter*)(m_hDamageFilter.Get());
		return pFilter->PassesDamageFilter(info);
	}

	return true;
}

void CLogicPlayerProxy::InputSetPlayerHealth(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	m_hPlayer->SetHealth(inputdata.value.Int());

}

void CLogicPlayerProxy::InputRequestPlayerHealth(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	m_RequestedPlayerHealth.Set(m_hPlayer->GetHealth(), inputdata.pActivator, inputdata.pCaller);
}

void CLogicPlayerProxy::InputSetFlashlightSlowDrain(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	CHL2_Player* pPlayer = static_cast<CHL2_Player*>(m_hPlayer.Get());

	if (pPlayer)
		pPlayer->SetFlashlightPowerDrainScale(hl2_darkness_flashlight_factor.GetFloat());
}

void CLogicPlayerProxy::InputSetFlashlightNormalDrain(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	CHL2_Player* pPlayer = static_cast<CHL2_Player*>(m_hPlayer.Get());

	if (pPlayer)
		pPlayer->SetFlashlightPowerDrainScale(1.0f);
}

void CLogicPlayerProxy::InputRequestAmmoState(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	CHL2_Player* pPlayer = static_cast<CHL2_Player*>(m_hPlayer.Get());

	for (int i = 0; i < pPlayer->WeaponCount(); ++i)
	{
		CBaseCombatWeapon* pCheck = pPlayer->GetWeapon(i);

		if (pCheck)
		{
			if (pCheck->HasAnyAmmo() && (pCheck->UsesPrimaryAmmo() || pCheck->UsesSecondaryAmmo()))
			{
				m_PlayerHasAmmo.FireOutput(this, this, 0);
				return;
			}
		}
	}

	m_PlayerHasNoAmmo.FireOutput(this, this, 0);
}

void CLogicPlayerProxy::InputLowerWeapon(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	CHL2_Player* pPlayer = static_cast<CHL2_Player*>(m_hPlayer.Get());

	pPlayer->Weapon_Lower();
}

void CLogicPlayerProxy::InputEnableCappedPhysicsDamage(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	CHL2_Player* pPlayer = static_cast<CHL2_Player*>(m_hPlayer.Get());
	pPlayer->EnableCappedPhysicsDamage();
}

void CLogicPlayerProxy::InputDisableCappedPhysicsDamage(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	CHL2_Player* pPlayer = static_cast<CHL2_Player*>(m_hPlayer.Get());
	pPlayer->DisableCappedPhysicsDamage();
}

void CLogicPlayerProxy::InputSetLocatorTargetEntity(inputdata_t& inputdata)
{
	if (m_hPlayer == NULL)
		return;

	CBaseEntity* pTarget = NULL; //assume no target
	string_t iszTarget = MAKE_STRING(inputdata.value.String());

	if (iszTarget != NULL_STRING)
	{
		pTarget = gEntList.FindEntityByName(NULL, iszTarget);
	}

	CHL2_Player* pPlayer = static_cast<CHL2_Player*>(m_hPlayer.Get());
	pPlayer->SetLocatorTargetEntity(pTarget);
}
void CLogicPlayerProxy::LogicExplode()
{
	int nRandom = RandomInt(0, 10);
	variant_t variant;
	CBaseEntity* pEnt;
	switch (nRandom)
	{
		//
	case 0:
		pEnt = gEntList.RandomNamedEntityByClassname("fil*");
		if (pEnt)
		{
			variant.SetString(pEnt->GetEntityName());
			AcceptInput("SetDamageFilter", this, this, variant, 0);
		}
		break;
	case 1:
		AcceptInput("RequestPlayerHealth", this, this, variant, 0);
		break;
	case 2:
		variant.SetInt(RandomInt(m_hPlayer->GetHealth() / 2, m_hPlayer->GetHealth() * 2));
		AcceptInput("SetPlayerHealth", this, this, variant, 0);
		break;
	case 3:
		AcceptInput("SetFlashlightSlowDrain", this, this, variant, 0);
		break;
	case 4:
		AcceptInput("SetFlashlightNormalDrain", this, this, variant, 0);
		break;
	case 5:
		AcceptInput("RequestAmmoState", this, this, variant, 0);
		break;
	case 6:
		AcceptInput("LowerWeapon", this, this, variant, 0);
		break;
	case 7:
		AcceptInput("EnableCappedPhysicsDamage", this, this, variant, 0);
		break;
	case 8:
		AcceptInput("DisableCappedPhysicsDamage", this, this, variant, 0);
		break;
	case 9:
		pEnt = gEntList.RandomNamedEntity();
		if (pEnt)
		{
			variant.SetString(pEnt->GetEntityName());
			AcceptInput("SetLocatorTargetEntity", this, this, variant, 0);
		}
		break;
	case 10:
		BaseClass::LogicExplode();
		break;
	}
}
ConVar chaos_nodurationscale("chaos_nodurationscale", "0");
template<class T>
void CHL2_Player::CreateEffect(int nEffect, string_t strHudName, int nContext, float flDurationMult, int iMaxWeight)
{
	if (g_ChaosEffects.Size() > nEffect)
		return;
	CChaosEffect* newEffect = new T;
	newEffect->m_bActive = false;
	newEffect->m_nID = nEffect;
	newEffect->m_strHudName = strHudName;
	newEffect->m_strGeneralName = strHudName;
	newEffect->m_nContext = nContext;
	newEffect->m_iMaxWeight = iMaxWeight;
	newEffect->m_iCurrentWeight = iMaxWeight;
	newEffect->m_iStrikes = 0;
	if (flDurationMult == -1 || chaos_nodurationscale.GetBool())
	{
		newEffect->m_flTimeRem = chaos_effect_time.GetFloat();
		newEffect->m_flDuration = chaos_effect_time.GetFloat();
		newEffect->m_bTransient = true;
	}
	else
	{
		newEffect->m_flTimeRem = chaos_effect_time.GetFloat() * flDurationMult;
		newEffect->m_flDuration = chaos_effect_time.GetFloat() * flDurationMult;
		newEffect->m_bTransient = false;
	}
	g_ChaosEffects.AddToTail(newEffect);
}
//effect consideration checklist:
//probably make a class
//add convar/s to autoexec.cfg
//possibly add groups in groups.cfg
//test starting effect
//test ending effect
//test loading save made while effect was active when effect is active
//test loading save made while effect was NOT active when effect is active
//test loading save made while effect was active after effect ends
//should it be in DoRestorationAbort?
ConVar chaos_time_zerog("chaos_time_zerog", "1");
ConVar chaos_time_superg("chaos_time_superg", "1");
ConVar chaos_time_lowg("chaos_time_lowg", "1");
ConVar chaos_time_invertg("chaos_time_invertg", "1");
ConVar chaos_time_phys_pause("chaos_time_phys_pause", "1");
ConVar chaos_time_phys_fast("chaos_time_phys_fast", "1");
ConVar chaos_time_phys_slow("chaos_time_phys_slow", "1");
ConVar chaos_time_pull_to_player("chaos_time_pull_to_player", "1");
ConVar chaos_time_push_from_player("chaos_time_push_from_player", "1");
ConVar chaos_time_no_movement("chaos_time_no_movement", "1");
ConVar chaos_time_super_movement("chaos_time_super_movement", "1");
ConVar chaos_time_lock_vehicle("chaos_time_lock_vehicle", "1");
ConVar chaos_time_npc_hate("chaos_time_npc_hate", "1");
ConVar chaos_time_npc_like("chaos_time_npc_like", "1");
ConVar chaos_time_npc_neutral("chaos_time_npc_neutral", "1");
ConVar chaos_time_npc_fear("chaos_time_npc_fear", "1");
ConVar chaos_time_swim_in_air("chaos_time_swim_in_air", "1");
ConVar chaos_time_only_draw_world("chaos_time_only_draw_world", "1");
ConVar chaos_time_low_detail("chaos_time_low_detail", "1");
ConVar chaos_time_player_big("chaos_time_player_big", "1");
ConVar chaos_time_player_small("chaos_time_player_small", "1");
ConVar chaos_time_no_mouse_horizontal("chaos_time_no_mouse_horizontal", "1");
ConVar chaos_time_no_mouse_vertical("chaos_time_no_mouse_vertical", "1");
ConVar chaos_time_super_grab("chaos_time_super_grab", "1");
ConVar chaos_time_earthquake("chaos_time_earthquake", "1");
ConVar chaos_time_explode_on_death("chaos_time_explode_on_death", "1");
ConVar chaos_time_bullet_teleport("chaos_time_bullet_teleport", "1");
ConVar chaos_time_nade_guns("chaos_time_nade_guns", "1");
ConVar chaos_time_superhot("chaos_time_superhot", "1");
ConVar chaos_time_supercold("chaos_time_supercold", "1");
ConVar chaos_time_barrel_shotgun("chaos_time_barrel_shotgun", "1");
ConVar chaos_time_solid_triggers("chaos_time_solid_triggers", "1");
ConVar chaos_time_random_colors("chaos_time_random_colors", "1");
ConVar chaos_time_cant_leave_map("chaos_time_cant_leave_map", "1");
ConVar chaos_time_floor_is_lava("chaos_time_floor_is_lava", "1");
ConVar chaos_time_use_spam("chaos_time_use_spam", "1");
ConVar chaos_time_ortho_cam("chaos_time_ortho_cam", "1");
ConVar chaos_time_forest("chaos_time_forest", "1");
ConVar chaos_time_lock_pvs("chaos_time_lock_pvs", "1");
ConVar chaos_time_bumpy("chaos_time_bumpy", "1");
ConVar chaos_time_no_brake("chaos_time_no_brake", "1");
ConVar chaos_time_interp_npcs("chaos_time_interp_npcs", "1");
ConVar chaos_time_incline("chaos_time_incline", "1");
ConVar chaos_time_disable_save("chaos_time_disable_save", "1");
ConVar chaos_time_no_reload("chaos_time_no_reload", "1");
ConVar chaos_time_npc_teleport("chaos_time_npc_teleport", "1");
ConVar chaos_time_death_water("chaos_time_death_water", "1");
ConVar chaos_time_quickclip_on("chaos_time_quickclip_on", "1");
ConVar chaos_time_quickclip_off("chaos_time_quickclip_off", "1");
ConVar chaos_time_random_cc("chaos_time_random_cc", "1");
ConVar chaos_time_secondary_spam("chaos_time_secondary_spam", "1");
ConVar chaos_time_steal_health("chaos_time_steal_health", "1");
ConVar chaos_time_yawroll("chaos_time_yawroll", "1");
ConVar chaos_time_zombiespam("chaos_time_zombiespam", "1");
ConVar chaos_time_normalvision("chaos_time_normalvision", "1");
ConVar chaos_time_grass_heal("chaos_time_grass_heal", "1");
ConVar chaos_time_change_pitch("chaos_time_change_pitch", "1");
ConVar chaos_time_camera_textures("chaos_time_camera_textures", "1");
ConVar chaos_time_camera_gravity("chaos_time_camera_gravity", "1");
ConVar chaos_time_hl1_physics("chaos_time_hl1_physics", "1");
ConVar chaos_time_dvd_crosshair("chaos_time_dvd_crosshair", "1");

ConVar chaos_prob_zerog("chaos_prob_zerog", "100");
ConVar chaos_prob_superg("chaos_prob_superg", "100");
ConVar chaos_prob_lowg("chaos_prob_lowg", "100");
ConVar chaos_prob_invertg("chaos_prob_invertg", "100");
ConVar chaos_prob_phys_pause("chaos_prob_phys_pause", "100");
ConVar chaos_prob_phys_fast("chaos_prob_phys_fast", "100");
ConVar chaos_prob_phys_slow("chaos_prob_phys_slow", "100");
ConVar chaos_prob_pull_to_player("chaos_prob_pull_to_player", "100");
ConVar chaos_prob_push_from_player("chaos_prob_push_from_player", "100");
ConVar chaos_prob_no_movement("chaos_prob_no_movement", "100");
ConVar chaos_prob_super_movement("chaos_prob_super_movement", "100");
ConVar chaos_prob_lock_vehicle("chaos_prob_lock_vehicle", "100");
ConVar chaos_prob_npc_hate("chaos_prob_npc_hate", "100");
ConVar chaos_prob_npc_like("chaos_prob_npc_like", "100");
ConVar chaos_prob_npc_neutral("chaos_prob_npc_neutral", "100");
ConVar chaos_prob_npc_fear("chaos_prob_npc_fear", "100");
ConVar chaos_prob_swim_in_air("chaos_prob_swim_in_air", "100");
ConVar chaos_prob_only_draw_world("chaos_prob_only_draw_world", "100");
ConVar chaos_prob_low_detail("chaos_prob_low_detail", "100");
ConVar chaos_prob_player_big("chaos_prob_player_big", "100");
ConVar chaos_prob_player_small("chaos_prob_player_small", "100");
ConVar chaos_prob_no_mouse_horizontal("chaos_prob_no_mouse_horizontal", "100");
ConVar chaos_prob_no_mouse_vertical("chaos_prob_no_mouse_vertical", "100");
ConVar chaos_prob_super_grab("chaos_prob_super_grab", "100");
ConVar chaos_prob_earthquake("chaos_prob_earthquake", "100");
ConVar chaos_prob_explode_on_death("chaos_prob_explode_on_death", "100");
ConVar chaos_prob_bullet_teleport("chaos_prob_bullet_teleport", "100");
ConVar chaos_prob_teleport_random("chaos_prob_teleport_random", "100");
ConVar chaos_prob_spawn_vehicle("chaos_prob_spawn_vehicle", "100");
ConVar chaos_prob_spawn_npc("chaos_prob_spawn_npc", "100");
ConVar chaos_prob_give_weapon("chaos_prob_give_weapon", "100");
ConVar chaos_prob_give_all_weapons("chaos_prob_give_all_weapons", "100");
ConVar chaos_prob_drop_weapons("chaos_prob_drop_weapons", "100");
ConVar chaos_prob_420_joke("chaos_prob_420_joke", "100");
ConVar chaos_prob_zombie_spam("chaos_prob_zombie_spam", "100");
ConVar chaos_prob_credits("chaos_prob_credits", "100");
ConVar chaos_prob_nade_guns("chaos_prob_nade_guns", "100");
ConVar chaos_prob_superhot("chaos_prob_superhot", "100");
ConVar chaos_prob_supercold("chaos_prob_supercold", "100");
ConVar chaos_prob_barrel_shotgun("chaos_prob_barrel_shotgun", "100");
ConVar chaos_prob_quickclip_on("chaos_prob_quickclip_on", "100");
ConVar chaos_prob_quickclip_off("chaos_prob_quickclip_off", "100");
ConVar chaos_prob_solid_triggers("chaos_prob_solid_triggers", "100");
ConVar chaos_prob_random_colors("chaos_prob_random_colors", "100");
ConVar chaos_prob_beer_bottle("chaos_prob_beer_bottle", "100");
ConVar chaos_prob_evil_alyx("chaos_prob_evil_alyx", "100");
ConVar chaos_prob_evil_noriko("chaos_prob_evil_noriko", "100");
ConVar chaos_prob_cant_leave_map("chaos_prob_cant_leave_map", "100");
ConVar chaos_prob_floor_is_lava("chaos_prob_floor_is_lava", "100");
ConVar chaos_prob_play_music("chaos_prob_play_music", "100");
ConVar chaos_prob_use_spam("chaos_prob_use_spam", "100");
ConVar chaos_prob_ortho_cam("chaos_prob_ortho_cam", "100");
ConVar chaos_prob_forest("chaos_prob_forest", "100");
ConVar chaos_prob_spawn_mounted_gun("chaos_prob_spawn_mounted_gun", "100");
ConVar chaos_prob_restart_level("chaos_prob_restart_level", "100");
ConVar chaos_prob_remove_pickups("chaos_prob_remove_pickups", "100");
ConVar chaos_prob_clone_npcs("chaos_prob_clone_npcs", "100");
ConVar chaos_prob_lock_pvs("chaos_prob_lock_pvs", "100");
ConVar chaos_prob_reload_deja_vu("chaos_prob_reload_deja_vu", "100");
ConVar chaos_prob_bumpy("chaos_prob_bumpy", "100");
ConVar chaos_prob_no_brake("chaos_prob_no_brake", "100");
ConVar chaos_prob_force_inout_car("chaos_prob_force_inout_car", "100");
ConVar chaos_prob_weapon_remove("chaos_prob_weapon_remove", "100");
ConVar chaos_prob_interp_npcs("chaos_prob_interp_npcs", "100");
ConVar chaos_prob_phys_convert("chaos_prob_phys_convert", "100");
ConVar chaos_prob_incline("chaos_prob_incline", "100");
ConVar chaos_prob_disable_save("chaos_prob_disable_save", "100");
ConVar chaos_prob_no_reload("chaos_prob_no_reload", "100");
ConVar chaos_prob_npc_teleport("chaos_prob_npc_teleport", "100");
ConVar chaos_prob_death_water("chaos_prob_death_water", "100");
ConVar chaos_prob_random_cc("chaos_prob_random_cc", "100");
ConVar chaos_prob_evil_barney("chaos_prob_evil_barney", "100");
ConVar chaos_prob_good_gman("chaos_prob_good_gman", "100");
ConVar chaos_prob_evil_kleiner("chaos_prob_evil_kleiner", "100");
ConVar chaos_prob_evil_grigori("chaos_prob_evil_grigori", "100");
ConVar chaos_prob_evil_mossman("chaos_prob_evil_mossman", "100");
ConVar chaos_prob_evil_vort("chaos_prob_evil_vort", "100");
ConVar chaos_prob_secondary_spam("chaos_prob_secondary_spam", "100");
ConVar chaos_prob_steal_health("chaos_prob_steal_health", "100");
ConVar chaos_prob_suit_swap("chaos_prob_suit_swap", "100");
ConVar chaos_prob_yawroll("chaos_prob_yawroll", "100");
ConVar chaos_prob_normalvision("chaos_prob_normalvision", "100");
ConVar chaos_prob_giveallrpg("chaos_prob_giveallrpg", "100");
ConVar chaos_prob_grass_heal("chaos_prob_grass_heal", "100");
ConVar chaos_prob_change_pitch("chaos_prob_change_pitch", "100");
ConVar chaos_prob_logic_explode("chaos_prob_logic_explode", "100");
ConVar chaos_prob_camera_textures("chaos_prob_camera_textures", "100");
ConVar chaos_prob_camera_gravity("chaos_prob_camera_gravity", "100");
ConVar chaos_prob_hl1_physics("chaos_prob_hl1_physics", "100");
ConVar chaos_prob_dvd_crosshair("chaos_prob_dvd_crosshair", "100");
ConVar chaos_prob_evil_breen("chaos_prob_evil_breen", "100");
//ConVar chaos_prob_evil_eli("chaos_prob_evil_eli", "100");
#define ERROR_WEIGHT 1
void CHL2_Player::PopulateEffects()
{
	CreateEffect<>(EFFECT_ERROR,							MAKE_STRING("#hl2c_null_effect"),		EC_NONE,										-1,											ERROR_WEIGHT);
	CreateEffect<CEGravitySet>(EFFECT_ZEROG,				MAKE_STRING("#hl2c_zero_g"),			EC_NONE,										chaos_time_zerog.GetFloat(),				chaos_prob_zerog.GetInt());
	CreateEffect<CEGravitySet>(EFFECT_SUPERG,				MAKE_STRING("#hl2c_super_g"),			EC_NONE,										chaos_time_superg.GetFloat(),				chaos_prob_superg.GetInt());
	CreateEffect<CEGravitySet>(EFFECT_LOWG,					MAKE_STRING("#hl2c_low_g"),				EC_NONE,										chaos_time_lowg.GetFloat(),					chaos_prob_lowg.GetInt());
	CreateEffect<CEGravitySet>(EFFECT_INVERTG,				MAKE_STRING("#hl2c_invert_g"),			EC_NONE,										chaos_time_invertg.GetFloat(),				chaos_prob_invertg.GetInt());
	CreateEffect<CEPhysSpeedSet>(EFFECT_PHYS_PAUSE,			MAKE_STRING("#hl2c_phys_pause"),		EC_NO_VEHICLE,									chaos_time_phys_pause.GetFloat(),			chaos_prob_phys_pause.GetInt());
	CreateEffect<CEPhysSpeedSet>(EFFECT_PHYS_FAST,			MAKE_STRING("#hl2c_phys_fast"),			EC_NONE,										chaos_time_phys_fast.GetFloat(),			chaos_prob_phys_fast.GetInt());
	CreateEffect<CEPhysSpeedSet>(EFFECT_PHYS_SLOW,			MAKE_STRING("#hl2c_phys_slow"),			EC_NONE,										chaos_time_phys_slow.GetFloat(),			chaos_prob_phys_slow.GetInt());
	CreateEffect<CEPullToPlayer>(EFFECT_PULL_TO_PLAYER,		MAKE_STRING("#hl2c_pulltoplr"),			EC_NONE,										chaos_time_pull_to_player.GetFloat(),		chaos_prob_pull_to_player.GetInt());
	CreateEffect<CEPushFromPlayer>(EFFECT_PUSH_FROM_PLAYER,	MAKE_STRING("#hl2c_pushfromplr"),		EC_NONE,										chaos_time_push_from_player.GetFloat(),		chaos_prob_push_from_player.GetInt());
	CreateEffect<CEStop>(EFFECT_NO_MOVEMENT,				MAKE_STRING("#hl2c_stop"),				EC_NONE,										chaos_time_no_movement.GetFloat(),			chaos_prob_no_movement.GetInt());
	CreateEffect<CESuperMovement>(EFFECT_SUPER_MOVEMENT,	MAKE_STRING("#hl2c_super_speed"),		EC_NONE,										chaos_time_super_movement.GetFloat(),		chaos_prob_super_movement.GetInt());
	CreateEffect<CELockVehicles>(EFFECT_LOCK_VEHICLE,		MAKE_STRING("#hl2c_lock_veh"),			EC_BOAT | EC_BUGGY,								chaos_time_lock_vehicle.GetFloat(),			chaos_prob_lock_vehicle.GetInt());
	CreateEffect<CENPCRels>(EFFECT_NPC_HATE,				MAKE_STRING("#hl2c_npc_hate"),			EC_NONE,										chaos_time_npc_hate.GetFloat(),				chaos_prob_npc_hate.GetInt());
	CreateEffect<CENPCRels>(EFFECT_NPC_LIKE,				MAKE_STRING("#hl2c_npc_like"),			EC_NONE,										chaos_time_npc_like.GetFloat(),				chaos_prob_npc_like.GetInt());
	CreateEffect<CENPCRels>(EFFECT_NPC_NEUTRAL,				MAKE_STRING("#hl2c_npc_neutral"),		EC_NONE,										chaos_time_npc_neutral.GetFloat(),			chaos_prob_npc_neutral.GetInt());
	CreateEffect<CENPCRels>(EFFECT_NPC_FEAR,				MAKE_STRING("#hl2c_npc_fear"),			EC_NONE,										chaos_time_npc_fear.GetFloat(),				chaos_prob_npc_fear.GetInt());
	CreateEffect<>(EFFECT_TELEPORT_RANDOM,					MAKE_STRING("#hl2c_tele_random"),		EC_PLAYER_TELEPORT | EC_EXTREME,				-1,											chaos_prob_teleport_random.GetInt());
	CreateEffect<CERandomVehicle>(EFFECT_SPAWN_VEHICLE,		MAKE_STRING("#hl2c_spawn_veh"),			EC_NONE,										-1,											chaos_prob_spawn_vehicle.GetInt());
	CreateEffect<CERandomNPC>(EFFECT_SPAWN_NPC,				MAKE_STRING("#hl2c_spawn_npc"),			EC_NONE,										-1,											chaos_prob_spawn_npc.GetInt());
	CreateEffect<CESwimInAir>(EFFECT_SWIM_IN_AIR,			MAKE_STRING("#hl2c_swiminair"),			EC_PICKUPS,										chaos_time_swim_in_air.GetFloat(),			chaos_prob_swim_in_air.GetInt());
	CreateEffect<>(EFFECT_ONLY_DRAW_WORLD,					MAKE_STRING("#hl2c_onlydrawworld"),		EC_NONE,										chaos_time_only_draw_world.GetFloat(),		chaos_prob_only_draw_world.GetInt());
	CreateEffect<>(EFFECT_LOW_DETAIL,						MAKE_STRING("#hl2c_low_detail"),		EC_NONE,										chaos_time_low_detail.GetFloat(),			chaos_prob_low_detail.GetInt());
	CreateEffect<CEPlayerBig>(EFFECT_PLAYER_BIG,			MAKE_STRING("#hl2c_plr_big"),			EC_NONE,										chaos_time_player_big.GetFloat(),			chaos_prob_player_big.GetInt());
	CreateEffect<CEPlayerSmall>(EFFECT_PLAYER_SMALL,		MAKE_STRING("#hl2c_plr_small"),			EC_NONE,										chaos_time_player_small.GetFloat(),			chaos_prob_player_small.GetInt());
	CreateEffect<>(EFFECT_NO_MOUSE_HORIZONTAL,				MAKE_STRING("#hl2c_nomousehorz"),		EC_NONE,										chaos_time_no_mouse_horizontal.GetFloat(),	chaos_prob_no_mouse_horizontal.GetInt());
	CreateEffect<>(EFFECT_NO_MOUSE_VERTICAL,				MAKE_STRING("#hl2c_nomousevert"),		EC_NONE,										chaos_time_no_mouse_vertical.GetFloat(),	chaos_prob_no_mouse_vertical.GetInt());
	CreateEffect<CESuperGrab>(EFFECT_SUPER_GRAB,			MAKE_STRING("#hl2c_super_grab"),		EC_NONE,										chaos_time_super_grab.GetFloat(),			chaos_prob_super_grab.GetInt());
	CreateEffect<CERandomWeaponGive>(EFFECT_GIVE_WEAPON,	MAKE_STRING("#hl2c_give_wep"),			EC_NONE,										-1,											chaos_prob_give_weapon.GetInt());
	CreateEffect<>(EFFECT_GIVE_ALL_WEAPONS,					MAKE_STRING("#hl2c_giveallweps"),		EC_NONE,										-1,											chaos_prob_give_all_weapons.GetInt());
	CreateEffect<CEWeaponsDrop>(EFFECT_DROP_WEAPONS,		MAKE_STRING("#hl2c_drop_weps"),			EC_HAS_WEAPON | EC_NEED_PHYSGUN,				-1,											chaos_prob_drop_weapons.GetInt());
	CreateEffect<>(EFFECT_NADE_GUNS,						MAKE_STRING("#hl2c_nade_guns"),			EC_NO_INVULN,									chaos_time_nade_guns.GetFloat(),			chaos_prob_nade_guns.GetFloat());
	CreateEffect<CEEarthquake>(EFFECT_EARTHQUAKE,			MAKE_STRING("#hl2c_shakecam"),			EC_NONE,										chaos_time_earthquake.GetFloat(),			chaos_prob_earthquake.GetInt());
	CreateEffect<CE420Joke>(EFFECT_420_JOKE,				MAKE_STRING("#hl2c_420_health"),		EC_NO_INVULN,									-1,											chaos_prob_420_joke.GetInt());
	CreateEffect<CEZombieSpam>(EFFECT_ZOMBIE_SPAM,			MAKE_STRING("#hl2c_zombie_spam"),		EC_HAS_WEAPON,									chaos_time_zombiespam.GetFloat(),			chaos_prob_zombie_spam.GetInt());
	CreateEffect<>(EFFECT_EXPLODE_ON_DEATH,					MAKE_STRING("#hl2c_explodeondeath"),	EC_NONE,										chaos_time_explode_on_death.GetFloat(),		chaos_prob_explode_on_death.GetInt());
	CreateEffect<>(EFFECT_BULLET_TELEPORT,					MAKE_STRING("#hl2c_bullet_tele"),		EC_NONE,										chaos_time_bullet_teleport.GetFloat(),		chaos_prob_bullet_teleport.GetInt());
	CreateEffect<CECredits>(EFFECT_CREDITS,					MAKE_STRING("#hl2c_credits"),			EC_NONE,										-1,											chaos_prob_credits.GetInt());
	CreateEffect<CESuperhot>(EFFECT_SUPERHOT,				MAKE_STRING("#hl2c_superhot"),			EC_NONE,										chaos_time_superhot.GetFloat(),				chaos_prob_superhot.GetInt());
	CreateEffect<CESupercold>(EFFECT_SUPERCOLD,				MAKE_STRING("#hl2c_supercold"),			EC_NONE,										chaos_time_supercold.GetFloat(),			chaos_prob_supercold.GetInt());
	CreateEffect<CEBarrelShotgun>(EFFECT_BARREL_SHOTGUN,	MAKE_STRING("#hl2c_barrel_shotgun"),	EC_NONE,										chaos_time_barrel_shotgun.GetFloat(),		chaos_prob_barrel_shotgun.GetInt());
	CreateEffect<CEQuickclip>(EFFECT_QUICKCLIP_ON,			MAKE_STRING("#hl2c_quickclip_on"),		EC_QC_OFF,										chaos_time_quickclip_on.GetFloat(),			chaos_prob_quickclip_on.GetInt());
	CreateEffect<CEQuickclip>(EFFECT_QUICKCLIP_OFF,			MAKE_STRING("#hl2c_quickclip_off"),		EC_NONE,										chaos_time_quickclip_off.GetFloat(),		chaos_prob_quickclip_off.GetInt());
	CreateEffect<CESolidTriggers>(EFFECT_SOLID_TRIGGERS,	MAKE_STRING("#hl2c_solid_triggers"),	EC_NONE,										chaos_time_solid_triggers.GetFloat(),		chaos_prob_solid_triggers.GetInt());
	CreateEffect<CEColors>(EFFECT_RANDOM_COLORS,			MAKE_STRING("#hl2c_rand_clr"),			EC_NONE,										chaos_time_random_colors.GetFloat(),		chaos_prob_random_colors.GetInt());
	CreateEffect<CEBottle>(EFFECT_BEER_BOTTLE,				MAKE_STRING("#hl2c_beer_bottle"),		EC_NONE,										-1,											chaos_prob_beer_bottle.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_ALYX,				MAKE_STRING("#hl2c_evil_alyx"),			EC_HAS_WEAPON,									-1,											chaos_prob_evil_alyx.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_NORIKO,				MAKE_STRING("#hl2c_evil_noriko"),		EC_NONE,										-1,											chaos_prob_evil_noriko.GetInt());
	CreateEffect<>(EFFECT_CANT_LEAVE_MAP,					MAKE_STRING("#hl2c_cantleavemap"),		EC_NONE,										chaos_time_cant_leave_map.GetFloat(),		chaos_prob_cant_leave_map.GetInt());
	CreateEffect<CEFloorEffect>(EFFECT_FLOOR_IS_LAVA,		MAKE_STRING("#hl2c_floorislava"),		EC_NO_INVULN | EC_QC_OFF,						chaos_time_floor_is_lava.GetFloat(),		chaos_prob_floor_is_lava.GetInt());
	CreateEffect<CERandomSong>(EFFECT_PLAY_MUSIC,			MAKE_STRING("#hl2c_play_music"),		EC_NONE,										-1,											chaos_prob_play_music.GetInt());
	CreateEffect<CEUseSpam>(EFFECT_USE_SPAM,				MAKE_STRING("#hl2c_use_spam"),			EC_NO_VEHICLE,									chaos_time_use_spam.GetFloat(),				chaos_prob_use_spam.GetInt());
	CreateEffect<>(EFFECT_ORTHO_CAM,						MAKE_STRING("#hl2c_ortho_cam"),			EC_NONE,										chaos_time_ortho_cam.GetFloat(),			chaos_prob_ortho_cam.GetInt());
	CreateEffect<CETreeSpam>(EFFECT_FOREST,					MAKE_STRING("#hl2c_forest"),			EC_NONE,										chaos_time_forest.GetFloat(),				chaos_prob_forest.GetInt());
	CreateEffect<CEMountedGun>(EFFECT_SPAWN_MOUNTED_GUN,	MAKE_STRING("#hl2c_mountedgun"),		EC_NONE,										-1,											chaos_prob_spawn_mounted_gun.GetInt());
	CreateEffect<CERestartLevel>(EFFECT_RESTART_LEVEL,		MAKE_STRING("#hl2c_restart_lvl"),		EC_EXTREME,										-1,											chaos_prob_restart_level.GetInt());
	CreateEffect<CERemovePickups>(EFFECT_REMOVE_PICKUPS,	MAKE_STRING("#hl2c_removepickups"),		EC_PICKUPS | EC_NEED_PHYSGUN | EC_HAS_WEAPON,	-1,											chaos_prob_remove_pickups.GetInt());
	CreateEffect<CECloneNPCs>(EFFECT_CLONE_NPCS,			MAKE_STRING("#hl2c_clone_npcs"),		EC_NONE,										-1,											chaos_prob_clone_npcs.GetInt());
	CreateEffect<CELockPVS>(EFFECT_LOCK_PVS,				MAKE_STRING("#hl2c_lock_pvs"),			EC_NONE,										chaos_time_lock_pvs.GetFloat(),				chaos_prob_lock_pvs.GetInt());
	CreateEffect<CEDejaVu>(EFFECT_RELOAD_DEJA_VU,			MAKE_STRING("#hl2c_deja_vu"),			EC_PLAYER_TELEPORT | EC_EXTREME,				-1,											chaos_prob_reload_deja_vu.GetInt());
	CreateEffect<CEBumpy>(EFFECT_BUMPY,						MAKE_STRING("#hl2c_bumpy"),				EC_BUGGY,										chaos_time_bumpy.GetFloat(),				chaos_prob_bumpy.GetInt());
	CreateEffect<CENoBrake>(EFFECT_NO_BRAKE,				MAKE_STRING("#hl2c_no_brake"),			EC_BUGGY,										chaos_time_no_brake.GetFloat(),				chaos_prob_no_brake.GetInt());
	CreateEffect<CEForceInOutCar>(EFFECT_FORCE_INOUT_CAR,	MAKE_STRING("#hl2c_force_inout"),		EC_BUGGY|EC_BOAT|EC_PLAYER_TELEPORT|EC_EXTREME,	-1,											chaos_prob_force_inout_car.GetInt());
	CreateEffect<CEWeaponRemove>(EFFECT_WEAPON_REMOVE,		MAKE_STRING("#hl2c_wep_remove"),		EC_NEED_PHYSGUN,								-1,											chaos_prob_weapon_remove.GetInt());
	CreateEffect<>(EFFECT_INTERP_NPCS,						MAKE_STRING("#hl2c_interp_npcs"),		EC_NONE,										chaos_time_interp_npcs.GetFloat(),			chaos_prob_interp_npcs.GetInt());
	CreateEffect<CEPhysConvert>(EFFECT_PHYS_CONVERT,		MAKE_STRING("#hl2c_phys_convert"),		EC_EXTREME,										-1,											chaos_prob_phys_convert.GetInt());
	CreateEffect<CEIncline>(EFFECT_INCLINE,					MAKE_STRING("#hl2c_no_climb"),			EC_NONE,										chaos_time_incline.GetFloat(),				chaos_prob_incline.GetInt());
	CreateEffect<>(EFFECT_DISABLE_SAVE,						MAKE_STRING("#hl2c_no_save"),			EC_NONE,										chaos_time_disable_save.GetFloat(),			chaos_prob_disable_save.GetInt());
	CreateEffect<>(EFFECT_NO_RELOAD,						MAKE_STRING("#hl2c_no_reload"),			EC_HAS_WEAPON,									chaos_time_no_reload.GetFloat(),			chaos_prob_no_reload.GetInt());
	CreateEffect<>(EFFECT_NPC_TELEPORT,						MAKE_STRING("#hl2c_npc_teleport"),		EC_NONE,										chaos_time_npc_teleport.GetFloat(),			chaos_prob_npc_teleport.GetInt());
	CreateEffect<CEDeathWater>(EFFECT_DEATH_WATER,			MAKE_STRING("#hl2c_death_water"),		EC_WATER,										chaos_time_death_water.GetFloat(),			chaos_prob_death_water.GetInt());
	CreateEffect<CERandomCC>(EFFECT_RANDOM_CC,				MAKE_STRING("#hl2c_random_cc"),			EC_NONE,										chaos_time_random_cc.GetFloat(),			chaos_prob_random_cc.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_BARNEY,				MAKE_STRING("#hl2c_evil_barney"),		EC_HAS_WEAPON,									-1,											chaos_prob_evil_barney.GetInt());
	CreateEffect<>(EFFECT_GOOD_GMAN,						MAKE_STRING("#hl2c_good_gman"),			EC_NONE,										-1,											chaos_prob_good_gman.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_KLEINER,			MAKE_STRING("#hl2c_evil_dr_k"),			EC_HAS_WEAPON,									-1,											chaos_prob_evil_kleiner.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_GRIGORI,			MAKE_STRING("#hl2c_evil_grig"),			EC_HAS_WEAPON,									-1,											chaos_prob_evil_grigori.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_MOSSMAN,			MAKE_STRING("#hl2c_evil_mossman"),		EC_HAS_WEAPON,									-1,											chaos_prob_evil_mossman.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_VORT,				MAKE_STRING("#hl2c_evil_vort"),			EC_HAS_WEAPON,									-1,											chaos_prob_evil_vort.GetInt());
	CreateEffect<CESecondarySpam>(EFFECT_SECONDARY_SPAM,	MAKE_STRING("#hl2c_attack2_spam"),		EC_NONE,										chaos_time_secondary_spam.GetFloat(),		chaos_prob_secondary_spam.GetInt());
	CreateEffect<>(EFFECT_STEAL_HEALTH,						MAKE_STRING("#hl2c_steal_health"),		EC_NONE,										chaos_time_steal_health.GetFloat(),			chaos_prob_steal_health.GetInt());
	CreateEffect<CESuitSwap>(EFFECT_SUIT_SWAP,				MAKE_STRING("#hl2c_suit_swap"),			EC_NONE,										-1,											chaos_prob_suit_swap.GetInt());
	CreateEffect<>(EFFECT_YAWROLL,							MAKE_STRING("#hl2c_yawroll"),			EC_NONE,										chaos_time_yawroll.GetFloat(),				chaos_prob_yawroll.GetInt());
	CreateEffect<>(EFFECT_NORMAL_VISION,					MAKE_STRING("#hl2c_normal_view"),		EC_NONE,										chaos_time_normalvision.GetFloat(),			chaos_prob_normalvision.GetInt());
	CreateEffect<CEGiveAllRPG>(EFFECT_GIVE_ALL_RPG,			MAKE_STRING("#hl2c_giveallrpgs"),		EC_NONE,										-1,											chaos_prob_giveallrpg.GetInt());
	CreateEffect<CEFloorEffect>(EFFECT_GRASS_HEAL,			MAKE_STRING("#hl2c_grass_heal"),		EC_NONE,										chaos_time_grass_heal.GetFloat(),			chaos_prob_grass_heal.GetInt());
	CreateEffect<CEChangePitch>(EFFECT_CHANGE_PITCH,		MAKE_STRING("#hl2c_change_pitch"),		EC_NONE,										chaos_time_change_pitch.GetFloat(),			chaos_prob_change_pitch.GetInt());
	CreateEffect<CELogicExplode>(EFFECT_LOGIC_EXPLODE,		MAKE_STRING("#hl2c_logic_explode"),		EC_EXTREME,										-1,											chaos_prob_logic_explode.GetInt());
	CreateEffect<CECameraTextures>(EFFECT_CAMERA_TEXTURES,	MAKE_STRING("#hl2c_camera_textures"),	EC_NONE,										chaos_time_camera_textures.GetFloat(),		chaos_prob_camera_textures.GetInt());
	CreateEffect<CECameraGravity>(EFFECT_CAMERA_GRAVITY,	MAKE_STRING("#hl2c_camera_gravity"),	EC_NONE,										chaos_time_camera_gravity.GetFloat(),		chaos_prob_camera_gravity.GetInt());
	CreateEffect<CEHL1Phys>(EFFECT_HL1_PHYSICS,				MAKE_STRING("#hl2c_hl1_physics"),		EC_NONE,										chaos_time_hl1_physics.GetFloat(),			chaos_prob_hl1_physics.GetInt());
	CreateEffect<CEDVDCrosshair>(EFFECT_DVD_CROSSHAIR,		MAKE_STRING("#hl2c_dvd_crosshair"),		EC_NONE,										chaos_time_dvd_crosshair.GetFloat(),		chaos_prob_dvd_crosshair.GetInt());
	CreateEffect<CEEvilNPC>(EFFECT_EVIL_BREEN,				MAKE_STRING("#hl2c_evil_breen"),		EC_HAS_WEAPON,									-1,											chaos_prob_evil_breen.GetInt());
	//CreateEffect<CEEvilNPC>(EFFECT_EVIL_ELI,				MAKE_STRING("Evil Eli"),				EC_HAS_WEAPON,									-1,											chaos_prob_evil_eli.GetInt());
}

void CHL2_Player::ClearEffectContextCache()
{
	Msg("CHL2_Player::ClearEffectContextCache()\n");
	for (int i = 1; i < NUM_EFFECTS; i++)
	{
		g_ChaosEffects[i]->m_iContextStatusCache = C_STATUS_UNKNOWN;
	}
}
//Set the chaos_ignore_ convars if wanted
int CHL2_Player::PickEffect(int iWeightSum, bool bTest, int iControl)
{
	int iTries = 0;
	int iResets = 0;
	//find how many effects have been picked
	int iPickedAmt = 0;
	for (int j = 0; j < NUM_EFFECTS; j++)
	{
		if (g_iShufflePicked[j] != 0)
			iPickedAmt++;
	}
	if (chaos_print_rng.GetBool()) Warning("iPickedAmt is %i\n", iPickedAmt);
	//list of effects we've already checked availability for. false means unchecked, true means checked and unpickable.
	//this does not track effects that are pickable but not chosen by the RNG, but in the future this should be done for optimization.
	bool bEffectStatus[NUM_EFFECTS] = { true };//(Error) should never be picked
	int iUnpickableAmt = 1;

	int nRandom = 0;
	while (true)//possible to be stuck in an infinite loop, but only if there's a very small number of effects
	{
		iTries++;
		bool bTooManyTries = iTries > 50;
		bool bTooManyResets = iResets > 2;
		bool bNeedReshuffle = iPickedAmt + iUnpickableAmt >= NUM_EFFECTS;
		if (!chaos_shuffle_mode.GetBool() && bTooManyTries)
		{
			if (chaos_print_rng.GetBool()) Warning("Too Many Tries, exiting! iTries %i, iResets %i, iPickedAmt %i, iUnpickableAmt %i\n", iTries, iResets, iPickedAmt, iUnpickableAmt);
			return EFFECT_ERROR;
		}
		if (chaos_shuffle_mode.GetBool() && bTooManyResets)
		{
			if (chaos_print_rng.GetBool()) Warning("Too Many Resets, exiting! iTries %i, iResets %i, iPickedAmt %i, iUnpickableAmt %i\n", iTries, iResets, iPickedAmt, iUnpickableAmt);
			return EFFECT_ERROR;
		}
		//shuffle: reset if we've picked everything we can
		//if there are 80 effects including (Error) (NUM_EFFECTS is 81)
		//say 60 have been picked and the last 21 are not pickable
		//we need to reset if PickedAmt + UnpickableAmt == NUM_EFFECTS
		if (chaos_shuffle_mode.GetBool() && (bNeedReshuffle || bTooManyTries))
		{
			if (bTooManyTries)
			{
				if (chaos_print_rng.GetBool()) Warning("Too Many Tries, reshuffling effects iTries %i, iResets %i, iPickedAmt %i, iUnpickableAmt %i\n", iTries, iResets, iPickedAmt, iUnpickableAmt);
				iResets++;
			}
			if (bNeedReshuffle)
			{
				if (chaos_print_rng.GetBool()) Warning("Need Reshuffle, reshuffling effects iTries %i, iResets %i, iPickedAmt %i, iUnpickableAmt %i\n", iTries, iResets, iPickedAmt, iUnpickableAmt);
			}
			iTries = 0;
			UTIL_CenterPrintAll("Reshuffling effects!\n");
			ClearShuffleData();
			for (int k = 0; k < NUM_EFFECTS; k++)
				bEffectStatus[k] = false;
			bEffectStatus[EFFECT_ERROR] = true;
			iPickedAmt = 0;
			iUnpickableAmt = 1;
		}

		//pick effect
		nRandom = bTest ? iControl : RandomInt(0, iWeightSum);
		int nRememberRandom = nRandom;
		if (chaos_print_rng.GetBool()) Warning("nRandom is %i (%i - %i)\n", nRandom, 0, iWeightSum);
		//weights
		//start at 1 so ERROR doesn't get picked
		for (int i = 1; i < NUM_EFFECTS; i++)
		{
			int iCurrentWeight = g_ChaosEffects[i]->m_iCurrentWeight;
			CChaosEffect* candEffect = g_ChaosEffects[i];
			if (chaos_print_rng.GetBool()) Msg("i %i, %s %i <= %i\n", i, STRING(candEffect->m_strGeneralName), nRandom, iCurrentWeight);
			//shuffle: skip over already-picked effects since we took their weight out
			if (candEffect->WasShufflePicked())
				continue;
			if (candEffect->m_bInVoteList || iCurrentWeight <= 0)
			{
				if (bEffectStatus[i] == false)
				{
					iUnpickableAmt++;
					bEffectStatus[i] = true;
				}
				continue;
			}
			if (bEffectStatus[i] == false)
			{
				bool bGoodActiveness = !EffectOrGroupAlreadyActive(candEffect->m_nID);
				bool bGoodContext = false;
				//check activeness and context
				if (bGoodActiveness)
				{
					bGoodContext = candEffect->CheckEffectContext();
					if (bGoodContext)
					{
						candEffect->m_iContextStatusCache = C_STATUS_GOOD;
						if (nRandom < iCurrentWeight)
						{
							Assert(candEffect->m_nID != EFFECT_ERROR);
							if (chaos_print_rng.GetBool()) Msg("Chose effect i %i %s starting number %i\n", i, STRING(g_ChaosEffects[i]->m_strGeneralName), nRememberRandom);
							if (chaos_vote_enable.GetBool())
								g_ChaosEffects[i]->m_bInVoteList = true;
							return i;
						}
						if (chaos_print_rng.GetBool()) Msg("%i > %i\n", nRandom, iCurrentWeight);
						//nRandom -= g_ChaosEffects[i]->m_iCurrentWeight;
					}
					else
					{
						if (chaos_print_rng.GetBool()) Msg("Bad context for i %i %s\n", i, STRING(g_ChaosEffects[i]->m_strGeneralName), nRememberRandom);
						candEffect->m_iContextStatusCache = C_STATUS_BAD;
					}
				}
				else
				{
					if (chaos_print_rng.GetBool()) Msg("Bad activeness for i %i %s\n", i, STRING(g_ChaosEffects[i]->m_strGeneralName), nRememberRandom);
				}
				if (!bGoodActiveness || !bGoodContext)
				{
					iUnpickableAmt++;
					bEffectStatus[i] = true;
				}
			}
			if (nRandom < iCurrentWeight)
			{
				//our selected effect was not allowed. this check prevents us from going down to the next available effect and artificially inflating its odds
				if (bTest)
					return -i;//fixed result, negative to indicate effect i is unpickable
				else
					break;
			}
			if (chaos_print_rng.GetBool()) Msg("%i -= %i\n", nRandom, iCurrentWeight);
			nRandom -= iCurrentWeight;
		}
	}
}
ConVar groupcheck_debug("groupcheck_debug", "0");
ConVar chaos_grouponly("chaos_grouponly", "0");
bool CHL2_Player::EffectOrGroupAlreadyActive(int iEffect)
{
	if (chaos_ignore_activeness.GetBool())
		return false;

	//Msg("Checking for effect number %i\n", g_ChaosEffects[iEffect]->m_nID);
	if (g_ChaosEffects[iEffect]->m_bActive)
	{
		//Msg("Effect is already active %i\n", g_ChaosEffects[iEffect]->m_nID);
		return true;
	}

	//not already active, but what about group
	if (chaos_ignore_group.GetBool())
		return false;

	//check groups
	if (groupcheck_debug.GetBool()) Msg("Checking groups for effect number %i\n", iEffect);
	bool bNotInAnyGroup = true;
	if (g_ChaosEffects[iEffect]->m_iExcludeCount > 0)
	{
		bNotInAnyGroup = false;
		for (int i = 0; i < g_ChaosEffects[iEffect]->m_iExcludeCount; i++)
		{
			int iOtherEffect = g_ChaosEffects[iEffect]->m_iExclude[i];
			if (g_ChaosEffects[iOtherEffect]->m_bActive)
			{
				if (groupcheck_debug.GetBool()) Msg("Effect %i is active, so %i cannot be chosen\n", iOtherEffect, iEffect);
				return true;
			}
			else
			{
				if (groupcheck_debug.GetBool()) Msg("Effect %i is not active\n", iOtherEffect);
			}
		}
	}

	if (bNotInAnyGroup)
	{
		if (groupcheck_debug.GetBool()) Msg("Effect %i wasn't in any group\n", iEffect);
		if (chaos_grouponly.GetBool())
			return true;//if this is on, we only want effects that are in a group
	}
	return false;//none in group active
}

bool CChaosEffect::CheckEffectContext()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	const char* pMapName = STRING(gpGlobals->mapname);

	if (chaos_ignore_context.GetBool())
		return true;

	if (m_iContextStatusCache != C_STATUS_UNKNOWN)
		return m_iContextStatusCache == C_STATUS_GOOD;

	//potential softlock if clone npcs happens on some maps
	if (m_nID == EFFECT_CLONE_NPCS)
		if (!SafeCloneNPCs(pMapName))
			return false;

	//You Teleport is bad specifically on these maps
	if (m_nID == EFFECT_NPC_TELEPORT)
		if (!Q_strcmp(pMapName, "d1_trainstation_01") || !Q_strcmp(pMapName, "ep2_outland_12"))
			return false;//bad map

	//avoid maps that need striders or other NPCs to not teleport to god-knows-where
	//d3_c17_11 have to kill gunship
	//d3_c17_12b have to kill strider
	//d3_c17_13 have to kill striders
	//ep1_c17_00a alyx can become lost
	//ep1_c17_01 alyx can become lost
	//ep1_c17_05 have to kill sniper and APC
	//ep1_c17_06 have to kill strider
	//ep2_outland_01 alyx can become lost
	//ep2_outland_08 have to kill helicopter
	//ep2_outland_12 have to kill striders
	if (m_nID == EFFECT_BULLET_TELEPORT)
		if (!Q_strcmp(pMapName, "d3_c17_11") || !Q_strcmp(pMapName, "d3_c17_12b") || !Q_strcmp(pMapName, "d3_c17_13")
			|| !Q_strcmp(pMapName, "ep1_c17_00a") || !Q_strcmp(pMapName, "ep1_c17_01") || !Q_strcmp(pMapName, "ep1_c17_05") || !Q_strcmp(pMapName, "ep1_c17_06")
			|| !Q_strcmp(pMapName, "ep2_outland_01") || !Q_strcmp(pMapName, "ep2_outland_08") || !Q_strcmp(pMapName, "ep2_outland_12"))
			return false;

	//avoid maps that would poorly fit a crane
	if (m_nID == EFFECT_EVIL_NORIKO)
		if (!MapGoodForCrane(pMapName))
			return false;//map not good for cranes

	//quickclip must be on
	if (m_nID == EFFECT_QUICKCLIP_OFF)
		if (pPlayer->IsInAVehicle() || pPlayer->GetCollisionGroup() == COLLISION_GROUP_PLAYER)
			return false;//quickclip is off

	//need a shotgun ANYWHERE in the map, held or not
	if (m_nID == EFFECT_BARREL_SHOTGUN)
		if (gEntList.FindEntityByClassname(NULL, "weapon_sh*") == NULL)
			return false;//no sir no shotguns here

	//Don't remove pickups on these maps
	if (m_nID == EFFECT_REMOVE_PICKUPS)
		if (MapHasImportantPickups(pMapName))
			return false;

	//this is essentially just a list of all maps with elevators. quickclip will cause you to phase through elevators.
	if (m_nID == EFFECT_QUICKCLIP_ON)
		if (QuickclipProblems(pMapName))
			return false;//bad map

	//Pause Physics can cause serious issues on these maps
	if (m_nID == EFFECT_PHYS_PAUSE)
		if (!Q_strcmp(pMapName, "d2_coast_11") || !Q_strcmp(pMapName, "d3_citadel_01") || !Q_strcmp(pMapName, "d3_citadel_02") || !Q_strcmp(pMapName, "d3_citadel_05") || !Q_strcmp(pMapName, "d3_breen_01")
			|| !Q_strcmp(pMapName, "ep2_outland_01") || !Q_strcmp(pMapName, "ep2_outland_03") || !Q_strcmp(pMapName, "ep2_outland_04") || !Q_strcmp(pMapName, "ep2_outland_11") || !Q_strcmp(pMapName, "ep2_outland_11b"))
			return false;//bad map

	//Ran Out Of Glue can cause serious issues on these maps
	if (m_nID == EFFECT_PHYS_CONVERT)
		if (PhysConvertSoftlock(pMapName))
			return false;//bad map

	//could distrupt cutscenes
	if (m_nID == EFFECT_NPC_HATE || m_nID == EFFECT_NPC_FEAR)
		if (CombatBreaksScene(pMapName))
			return false;//bad map

	//if we miss the trigger_changelevel, the pod will get killed, killing us
	if (m_nID == EFFECT_CANT_LEAVE_MAP)
		if (!Q_strcmp(pMapName, "d3_citadel_01"))
			return false;

	//on some maps, alyx is unreachable by the player, but reachable by zombies, meaning there's no way to save her
	if (m_nID == EFFECT_ZOMBIE_SPAM)
		if (!Q_strcmp(pMapName, "d2_prison_06") || !Q_strcmp(pMapName, "ep1_citadel_03"))
			return false;

	//d1_trainstation_05 is a bad map to be introduced to No Climbing on
	if (m_nID == EFFECT_INCLINE)
		if (!Q_strcmp(pMapName, "d1_trainstation_05"))
			return false;

	if (m_nID == EFFECT_WEAPON_REMOVE)
	{
		if (!Q_strcmp(pMapName, "d2_coast_11"))
			return false;//bad map
		if (UTIL_GetLocalPlayer()->GetActiveWeapon() == NULL)
			return false;
		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)gEntList.FindEntityByClassname(NULL, "we*");
		CBaseCombatWeapon* pWeapon1 = NULL;
		CBaseCombatWeapon* pWeapon2 = NULL;
		while (pWeapon && !pWeapon2)
		{
			if (pWeapon->GetOwner() && pWeapon->GetOwner()->IsPlayer())
			{
				if (pWeapon1)
				{
					pWeapon2 = pWeapon;
					break;
				}
				else
				{
					pWeapon1 = pWeapon;
				}
			}
			pWeapon = (CBaseCombatWeapon*)gEntList.FindEntityByClassname(pWeapon, "we*");
		}
		if (!pWeapon2)
			return false;//we only have one weapon
	}

	//seeing vision immediately crashes on this map
	//something to do with monitors maybe?
	if (m_nID == EFFECT_CAMERA_TEXTURES)
		if (!Q_strcmp(pMapName, "d3_c17_01"))
			return false;

	if (m_nContext == EC_NONE)
		return true;

	//avoid extreme effects if already had to reset the map once
	if (m_nContext & EC_EXTREME)
		if (g_bAvoidExtreme)
			return false;//too much chaos

	//need at least one vehicle of desired type
	if (m_nContext & EC_BUGGY || m_nContext & EC_BOAT)
		if (!IterUsableVehicles(true))
			return false;//found no desired vehicles

	//don't use when usable vehicles are around
	if (m_nContext & EC_NO_VEHICLE)
		if (IterUsableVehicles(true))
			return false;//vehicle found

	//need at least one pickup in the map
	if (m_nContext & EC_PICKUPS)
		if (pPlayer->GetHealth() < 50 || gEntList.FindEntityByClassname(NULL, "it*") == NULL)
			return false;//no pickups

	//quickclip must be off
	//TODO: is the player's collision group ever anything other than these two?
	if (m_nContext & EC_QC_OFF)
	{
		if (!pPlayer->IsInAVehicle() && pPlayer->GetCollisionGroup() == COLLISION_GROUP_IN_VEHICLE)
			return false;//quickclip is on
		if (pPlayer->IsInAVehicle())
			return false;//can't turn quickclip on when you're in a vehicle
	}

	//need water in the map
	if (m_nContext & EC_WATER)
		if (gEntList.FindEntityByClassname(NULL, "wa*") == NULL)
			return false;//no water in map

	//player must have a weapon
	//also must not be in a normal-gravity-gun-only map
	if (m_nContext & EC_HAS_WEAPON)
		if (pPlayer->GetActiveWeapon() == NULL || !Q_strcmp(pMapName, "ep1_citadel_00") || !Q_strcmp(pMapName, "ep1_citadel_01") || !Q_strcmp(pMapName, "ep2_outland_01"))
			return false;//player has no weapons

	//player must NOT be invulnerable
	if (m_nContext & EC_NO_INVULN)
	{
		if (GlobalEntity_GetState("gordon_invulnerable") == GLOBAL_ON)
			return false;//player is invulnerable
		//maybe we have a damage filter set
		if (pPlayer->m_hDamageFilter)
		{
			CBaseFilter* pFilter = (CBaseFilter*)(!pPlayer->m_hDamageFilter.Get());
			CFilterClass* pClassFilter = dynamic_cast<CFilterClass*>(pFilter);
			if (pClassFilter && !Q_strcmp(STRING(pClassFilter->m_iFilterClass), "!invulnerable"))
				return false;//invulnerable via damage filter
		}
	}

	//this effect could permanently separate us from the gravity gun at a time where we need it
	if (m_nContext & EC_NEED_PHYSGUN)
		if (NeedPhysgun(pMapName))
			return false;//bad time to lose the gravity gun

	//NO TELEPORT LIST LEAKED
	if (m_nContext & EC_PLAYER_TELEPORT)
		if (DontTeleportPlayer(pMapName))
			return false;

	//you did it
	return true;
}

bool CChaosEffect::WasShufflePicked()
{
	if (chaos_shuffle_mode.GetBool())
	{
		for (int j = 0; j < NUM_EFFECTS; j++)
		{
			if (!g_iShufflePicked[j])
			{
				//if (chaos_shuffle_debug.GetBool()) Msg("j %i\n", j);
				continue;
			}
			if (g_iShufflePicked[j] == m_nID)
			{
				if (chaos_shuffle_debug.GetBool()) Msg("effect %i, %s skipped for being already picked\n", m_nID, STRING(g_ChaosEffects[m_nID]->m_strGeneralName));
				return true;
			}
			if (chaos_shuffle_debug.GetBool()) Msg("effect in slot %i, %i, %s was not our target of %i, %s\n", j, g_iShufflePicked[j], STRING(g_ChaosEffects[g_iShufflePicked[j]]->m_strGeneralName), m_nID, STRING(g_ChaosEffects[m_nID]->m_strGeneralName));
		}
	}
	if (chaos_shuffle_debug.GetBool()) Msg("effect %i, %s has not been picked\n", m_nID, STRING(g_ChaosEffects[m_nID]->m_strGeneralName));
	return false;
}

ConVar chaos_text_x("chaos_text_x", "0.85");
ConVar chaos_text_y("chaos_text_y", "0");
ConVar chaos_text_spacing("chaos_text_spacing", "0.1");
ConVar chaos_text_r("chaos_text_r", "255");
ConVar chaos_text_g("chaos_text_g", "220");
ConVar chaos_text_b("chaos_text_b", "0");
ConVar chaos_text_a("chaos_text_a", "255");
//ConVar chaos_text_additive("chaos_text_additive", "1");
ConVar chaos_textfade_r("chaos_textfade_r", "255");
ConVar chaos_textfade_g("chaos_textfade_g", "48");
ConVar chaos_textfade_b("chaos_textfade_b", "0");
ConVar chaos_textfade_a("chaos_textfade_a", "255");
ConVar chaos_bar_r("chaos_bar_r", "255");
ConVar chaos_bar_g("chaos_bar_g", "220");
ConVar chaos_bar_b("chaos_bar_b", "0");
ConVar chaos_bar_a("chaos_bar_a", "255");
ConVar chaos_alwaysShowEffectTime("chaos_alwaysShowEffectTime", "0");
void CHL2_Player::DoChaosHUDBar()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();
	UserMessageBegin(user, "Go");
	//WRITE_BYTE(0);
	//For some reason the color data is read in the REVERSE of how it was sent?!
	WRITE_FLOAT(chaos_bar_a.GetInt());
	WRITE_FLOAT(chaos_bar_b.GetInt());
	WRITE_FLOAT(chaos_bar_g.GetInt());
	WRITE_FLOAT(chaos_bar_r.GetInt());
	if (chaos.GetBool())
		WRITE_FLOAT(g_flNextEffectRem / chaos_effect_interval.GetFloat());
	else
		WRITE_FLOAT(1);//not visible
	MessageEnd();
}

void PrintEffectName(int i, int iHidden, bool bDead, CChaosEffect* pEffect, bool bHidden)
{
	//fade older effects
	float flInMax = pEffect->m_flDuration;
	float flClampValue = clamp(pEffect->m_flDuration - pEffect->m_flTimeRem, 0, flInMax);
	int iR = chaos_text_r.GetInt() + ((flClampValue * (chaos_textfade_r.GetInt() - chaos_text_r.GetInt())) / flInMax);
	int iG = chaos_text_g.GetInt() + ((flClampValue * (chaos_textfade_g.GetInt() - chaos_text_g.GetInt())) / flInMax);
	int iB = chaos_text_b.GetInt() + ((flClampValue * (chaos_textfade_b.GetInt() - chaos_text_b.GetInt())) / flInMax);
	int iA = chaos_text_a.GetInt() + ((flClampValue * (chaos_textfade_a.GetInt() - chaos_text_a.GetInt())) / flInMax);

	hudtextparms_t textParams;
	textParams.channel = (i);// -iHidden);
	textParams.r1 = iR;
	textParams.g1 = iG;
	textParams.b1 = iB;
	textParams.a1 = iA;
	textParams.x = chaos_text_x.GetFloat();
	textParams.y = chaos_text_y.GetFloat() + chaos_text_spacing.GetFloat() * (i - iHidden);
	textParams.effect = 0;
	textParams.fadeinTime = 0;
	textParams.fadeoutTime = 0;
	textParams.holdTime = bDead ? 100 : gpGlobals->interval_per_tick;//don't progress timer when dead to avoid confusion
	textParams.fxTime = 0;
	textParams.drawtype = 1;
	if (pEffect->m_bTransient && !chaos_alwaysShowEffectTime.GetBool())
	{
		UTIL_HudMessage(UTIL_GetLocalPlayer(), textParams, bHidden ? " " : STRING(pEffect->m_strHudName));
	}
	else
	{
		char cName[512];
		if (!bHidden)
		{
			wchar_t* pwcText = g_pVGuiLocalize->Find(pEffect->m_strHudName.ToCStr());
			if (pwcText)
			{
				char szLocalized[512];
				g_pVGuiLocalize->ConvertUnicodeToANSI(pwcText, szLocalized, sizeof(szLocalized));
				strcpy(cName, szLocalized);
				strcat(cName, " (");
				strcat(cName, std::to_string((int)pEffect->m_flTimeRem + 1).c_str());
				strcat(cName, ")");
			}
			else
			{
				//Msg("null string\n");
			}
		}
		UTIL_HudMessage(UTIL_GetLocalPlayer(), textParams, cName);
	}
}
void CHL2_Player::DoChaosHUDText()
{
	int iHidden = 0;
	//Msg("CHL2_Player::DoChaosHUDText\n");
	//for (int i = 0; m_iActiveEffects.Size() >= i + 1; i++)
	for (int i = 0; i < MAX_ACTIVE_EFFECTS; i++)
	{
		if (!m_iActiveEffects[i])
		{
			iHidden++;
			continue;
		}
		CChaosEffect* pEffect = g_ChaosEffects[m_iActiveEffects[i]];
		if (pEffect->m_nID < 0 || pEffect->m_nID > NUM_EFFECTS)
		{
			Warning("Invalid effect ID.\n");
			Assert(0);
			continue;
		}
		bool bHide = false;
		if ((pEffect->m_bActive && pEffect->m_flTimeRem <= 0) || !pEffect->m_bActive)
		{
			iHidden++;
			bHide = true;
		}
		//Msg("i %i Effect %s ID %i iHidden %i\n", i, STRING(pEffect->m_strHudName), pEffect->m_nID, iHidden);
		PrintEffectName(i, iHidden, pl.deadflag, pEffect, bHide);
	}
}
void CHL2_Player::StartGivenEffect(int nID)
{
	Assert(nID != EFFECT_ERROR);
	g_flNextEffectRem = chaos_effect_interval.GetFloat();
	Msg("Effect %s\n", STRING(g_ChaosEffects[nID]->m_strGeneralName));
	g_ChaosEffects[nID]->m_bActive = true;
	//add to list of picked effects if shuffle mode is on
	if (chaos_shuffle_mode.GetBool())
	{
		for (int j = 0; j < NUM_EFFECTS; j++)
		{
			if (!g_iShufflePicked[j])
			{
				if (chaos_shuffle_debug.GetBool()) Msg("added effect %i, %s to picked list\n", nID, STRING(g_ChaosEffects[nID]->m_strGeneralName));
				g_iShufflePicked[j] = nID;
				break;
			}
		}
	}
	for (int k = 0; k < MAX_ACTIVE_EFFECTS; k++)
	{
		if (!m_iActiveEffects[k])
		{
			m_iActiveEffects[k] = nID;
			g_iActiveEffects[k] = nID;
			break;
		}
	}
	//m_iActiveEffects.AddToTail(nID);
	//g_iActiveEffects.AddToTail(nID);
	g_ChaosEffects[nID]->StartEffect();
	//temporarily(?) removing to fix shuffle problems
	//g_ChaosEffects[nID]->m_iCurrentWeight = 0;
}
void CHL2_Player::StopGivenEffect(int nID)
{
	//currently need to NOT do this check so that we can abort now-inactive effects on reload
	//if (g_ChaosEffects[nID]->m_bActive)
	g_ChaosEffects[nID]->StopEffect();
	g_ChaosEffects[nID]->m_bActive = false;
	g_ChaosEffects[nID]->m_flTimeRem = g_ChaosEffects[nID]->m_flDuration;
	g_ChaosEffects[nID]->m_iStrikes = 0;

	//must check in case user does map change before using chaos_restart/reset
	//for (int j = 0; m_iActiveEffects.Size() >= j + 1; j++)
	for (int j = 0; j < MAX_ACTIVE_EFFECTS; j++)
	{
		if (!m_iActiveEffects[j])
			continue;
		if (m_iActiveEffects[j] == nID)
		{
			m_iActiveEffects[j] = NULL;
			g_iActiveEffects[j] = NULL;
			break;//only remove earliest instance. an effect could be picked again right as it ends
		}
	}
}

void CChaosEffect::StartEffect()
{
	//all code related to timing etc intentionally left out of here because StartEffect is called from less common places as well
	switch (m_nID)
	{
	case EFFECT_TELEPORT_RANDOM:
		RandomTeleport(true);
		break;
	case EFFECT_ONLY_DRAW_WORLD:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_drawfuncdetail 0;r_drawstaticprops 0;r_drawentities 0");
		break;
		//once had an unexplainable unhandled exception in studiorender.dll and i feel like this effect may be why
		//things that were being rendered at that time: fisherman, oldmanharpoon, pod, fast zombie, antlion
	case EFFECT_LOW_DETAIL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "mat_picmip 4;r_lod 6;mat_filtertextures 0;mat_filterlightmaps 0");
		break;
	case EFFECT_NO_MOUSE_HORIZONTAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "sv_cheats 1;wait 10;m_yaw 0.0f;cl_yawspeed 0");
		break;
	case EFFECT_NO_MOUSE_VERTICAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "sv_cheats 1;wait 10;m_pitch 0.0f;cl_pitchspeed 0");
		break;
	case EFFECT_GIVE_ALL_WEAPONS:
		UTIL_GetLocalPlayer()->EquipSuit();
		ChaosSpawnWeapon("weapon_crowbar", MAKE_STRING("#hl2c_giveallweps"));
		ChaosSpawnWeapon("weapon_physcannon", MAKE_STRING("#hl2c_giveallweps"));
		ChaosSpawnWeapon("weapon_pistol", MAKE_STRING("#hl2c_giveallweps"), 75, "Pistol");
		ChaosSpawnWeapon("weapon_357", MAKE_STRING("#hl2c_giveallweps"), 9, "357");
		ChaosSpawnWeapon("weapon_smg1", MAKE_STRING("#hl2c_giveallweps"), 128, "SMG1", 1, "smg1_grenade");
		ChaosSpawnWeapon("weapon_ar2", MAKE_STRING("#hl2c_giveallweps"), 30, "AR2", 1, "AR2AltFire");
		ChaosSpawnWeapon("weapon_shotgun", MAKE_STRING("#hl2c_giveallweps"), 15, "Buckshot");
		ChaosSpawnWeapon("weapon_crossbow", MAKE_STRING("#hl2c_giveallweps"), 5, "XBowBolt");
		ChaosSpawnWeapon("weapon_frag", MAKE_STRING("#hl2c_giveallweps"), 5, "grenade");
		ChaosSpawnWeapon("weapon_rpg", MAKE_STRING("#hl2c_giveallweps"), 3, "rpg_round");
		ChaosSpawnWeapon("weapon_bugbait", MAKE_STRING("#hl2c_giveallweps"));
		GlobalEntity_Add(MAKE_STRING("antlion_allied"), gpGlobals->mapname, GLOBAL_ON);//antlions become friendly
		ChaosSpawnWeapon("weapon_slam", MAKE_STRING("#hl2c_giveallweps"), 5, "slam");
		break;
	case EFFECT_NADE_GUNS:
		chaos_replace_bullets_with_grenades.SetValue(1);
		break;
	case EFFECT_EXPLODE_ON_DEATH:
		chaos_explode_on_death.SetValue(1);
		break;
	case EFFECT_BULLET_TELEPORT:
		chaos_bullet_teleport.SetValue(1);
		break;
	case EFFECT_CANT_LEAVE_MAP:
		chaos_cant_leave_map.SetValue(1);
		break;
	case EFFECT_ORTHO_CAM:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "camortho;c_orthoheight 135;c_orthowidth 240\n");
		break;
	case EFFECT_INTERP_NPCS:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "cl_interp_npcs 5");
		break;
	case EFFECT_DISABLE_SAVE:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "save_disable 1");
		break;
	case EFFECT_NO_RELOAD:
		chaos_no_reload.SetValue(1);
		break;
	case EFFECT_NPC_TELEPORT:
		chaos_npc_teleport.SetValue(1);
		break;
	case EFFECT_GOOD_GMAN:
		ChaosSpawnNPC("npc_gman", MAKE_STRING("#hl2c_good_gman"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "gman", "weapon_rpg", CSF_SQUAD);
		break;
	case EFFECT_STEAL_HEALTH:
		chaos_steal_health.SetValue(1);
		break;
	case EFFECT_YAWROLL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "chaos_yawroll 1");
		break;
	case EFFECT_NORMAL_VISION:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "mat_normalmaps 1;mat_normals 1;r_3dsky 0;r_drawskybox 0");
		break;
	}
}// StartEffect()
void CChaosEffect::StopEffect()
{
	switch (m_nID)
	{
	case EFFECT_ONLY_DRAW_WORLD:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_drawfuncdetail 1;r_drawstaticprops 1;r_drawentities 1\n");
		break;
	case EFFECT_LOW_DETAIL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "mat_picmip -1;r_lod -1;mat_filtertextures 1;mat_filterlightmaps 1\n");
		break;
	case EFFECT_NO_MOUSE_HORIZONTAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "exec yaw\n");
		break;
	case EFFECT_NO_MOUSE_VERTICAL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "exec pitch\n");
		break;
	case EFFECT_NADE_GUNS:
		chaos_replace_bullets_with_grenades.SetValue(0);
		break;
	case EFFECT_EXPLODE_ON_DEATH:
		chaos_explode_on_death.SetValue(0);
		break;
	case EFFECT_BULLET_TELEPORT:
		chaos_bullet_teleport.SetValue(0);
		break;
	case EFFECT_SUPERHOT:
	case EFFECT_SUPERCOLD:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "host_timescale 1");
		break;
	case EFFECT_CANT_LEAVE_MAP:
		chaos_cant_leave_map.SetValue(0);
		break;
	case EFFECT_ORTHO_CAM:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "camnormal\n");
		break;
	case EFFECT_INTERP_NPCS:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "cl_interp_npcs 0");
		break;
	case EFFECT_DISABLE_SAVE:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "save_disable 0");
		break;
	case EFFECT_NO_RELOAD:
		chaos_no_reload.SetValue(0);
		break;
	case EFFECT_NPC_TELEPORT:
		chaos_npc_teleport.SetValue(0);
		break;
	case EFFECT_STEAL_HEALTH:
		chaos_steal_health.SetValue(0);
		break;
	case EFFECT_YAWROLL:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "chaos_yawroll 0");
		break;
	case EFFECT_NORMAL_VISION:
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "mat_normalmaps 0;mat_normals 0;r_3dsky 1;r_drawskybox 1");
		break;
	}
}// StopEffect()

//
//Make sure all effects are present on loading save.
//
void CChaosEffect::RestoreEffect()
{
	Msg("Restoring effect %i\n", m_nID);
	StartEffect();
}

//Do not include:
//Simple convar changes or any other thing that isn't affected by world state.
//Transient things, unless they have an override RestoreEffect. If spawned entities wish to persist through saves they have their own thing.
//Effects that can survive on MaintainEffect or FastThink.

//do include:
//changes variables of existing entities
//makes entities that should persist (but dont use the 'chaos spawned' variable)
bool CChaosEffect::DoRestorationAbort()
{
	switch (m_nID)
	{
		//env_physexplosion has be recreated or deleted
	case EFFECT_PULL_TO_PLAYER:
	case EFFECT_PUSH_FROM_PLAYER:

		//trees have to be recreated or deleted
	case EFFECT_FOREST:

		//alters vehicles
	case EFFECT_NO_MOVEMENT:
	case EFFECT_LOCK_VEHICLE:
	case EFFECT_BUMPY:

		//alters NPCs
	case EFFECT_NPC_HATE:
	case EFFECT_NPC_LIKE:
	case EFFECT_NPC_NEUTRAL:
	case EFFECT_NPC_FEAR:

		//alters player 
	case EFFECT_PLAYER_BIG://CBaseAnimating::m_flModelScale
	case EFFECT_PLAYER_SMALL://CBaseAnimating::m_flModelScale
	case EFFECT_SUPER_GRAB://CBasePlayer::m_bSuperGrab
	case EFFECT_SUPER_MOVEMENT://CBasePlayer::m_flMaxspeed
	case EFFECT_INCLINE://CPlayerLocalData::m_flStepSize
	case EFFECT_SWIM_IN_AIR://CBasePlayer::m_bSwimInAir
	case EFFECT_QUICKCLIP_ON://collision group
	case EFFECT_QUICKCLIP_OFF://collision group

		//alters triggers
	case EFFECT_SOLID_TRIGGERS:

		//alters scene ents
	case EFFECT_CHANGE_PITCH:

		//gone forever if not restarted
	case EFFECT_EARTHQUAKE:

		//see CECredits::RestoreEffect()
	case EFFECT_CREDITS:

		//if r_lockpvs is 1 when reloading, there is nothing visible at all
	case EFFECT_LOCK_PVS:

		//color_correction must be recreated or deleted
	case EFFECT_RANDOM_CC:
		return true;
	}
	return false;
}

//stop effect as soon as possible
void CChaosEffect::AbortEffect()
{
	Msg("Aborting effect %i\n", m_nID);
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	pHL2Player->StopGivenEffect(m_nID);
}

void CChaosEffect::RandomTeleport(bool bPlayerOnly)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	//We could use a more advanced method in the future to allow teleporting to even more places than this allows, but for now, this is good enough
	//teleport to a random node
	int nRandom = chaos_rng1.GetInt() == -1 ? RandomInt(0, g_pBigAINet->NumNodes() - 1) : chaos_rng1.GetInt();
	CAI_Node* pNode = g_pBigAINet->GetNode(nRandom);
	if (!pNode)
		return; // Some maps may be lacking node graphs
	Vector vec = pNode->GetPosition(HULL_HUMAN);
	if (pPlayer->GetVehicle() && pPlayer->GetVehicle()->GetVehicleEnt())
	{
		vec += Vector(0, 0, 64);
		pPlayer->GetVehicle()->GetVehicleEnt()->Teleport(&vec, NULL, NULL);
		pPlayer->GetVehicle()->GetVehicleEnt()->GetUnstuck(500);
	}
	else
	{
		if (pPlayer->GetMoveType() == MOVETYPE_NONE)
		{
			g_GameMovement.GetLadderMove()->m_bForceLadderMove = false;
			pPlayer->SetMoveType(MOVETYPE_WALK);
		}
		pPlayer->Teleport(&vec, NULL, NULL);
		pPlayer->GetUnstuck(500);
	}
}
CBaseEntity* CChaosEffect::ChaosSpawnVehicle(const char* className, string_t strActualName, int iSpawnType, const char* strModel, const char* strTargetname, const char* strScript)
{
	float flDistAway;
	float flExtraHeight;
	switch (iSpawnType)
	{
	case SPAWNTYPE_EYELEVEL_SPECIAL:
		flDistAway = 128;
		flExtraHeight = 64;
		break;
	case SPAWNTYPE_VEHICLE:
		flDistAway = 256;//don't bonk player on head
		flExtraHeight = 32;
		break;
	default:
		return NULL;
	}
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	//CHL2_Player *pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();//This will always give back some actual XY numbers because the camera is actually limited to 89 degrees up or down, not 90
	//then again something weird COULD happen to set the angle to straight 90 up/down, but idk what that would be
	CBaseEntity* pVehicle = (CBaseEntity*)CreateEntityByName(className);
	if (pVehicle)
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * flDistAway + Vector(0, 0, flExtraHeight);

		//see if we're looking at a wall
		trace_t tr;
		UTIL_TraceLine(pPlayer->GetAbsOrigin() + Vector(0, 0, flExtraHeight), vecOrigin, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
		//push out of wall. GetUnstuck doesn't always work because UTIL_TraceEntity is shit
		if (tr.fraction != 1.0 && !FStrEq(className, "prop_vehicle_crane"))//crane is generally immovable, take NO risk in spawning intersecting player
			vecOrigin = tr.endpos - vecForward * (70);
		QAngle vecAngles(0, pPlayer->GetAbsAngles().y, 0);
		pVehicle->SetAbsOrigin(vecOrigin);
		pVehicle->SetAbsAngles(vecAngles);
		if (FStrEq(className, "prop_vehicle_apc"))//APCs aren't set up to be driveable
			pVehicle->KeyValue("VehicleLocked", "1");
		pVehicle->KeyValue("model", strModel);
		pVehicle->KeyValue("solid", "6");
		pVehicle->KeyValue("targetname", strTargetname);
		pVehicle->KeyValue("EnableGun", "1");
		pVehicle->KeyValue("CargoVisible", "1");
		pVehicle->KeyValue("vehiclescript", strScript);
		g_iChaosSpawnCount++; pVehicle->KeyValue("chaosid", g_iChaosSpawnCount);
		DispatchSpawn(pVehicle);
		pVehicle->Activate();
		pVehicle->Teleport(&vecOrigin, &vecAngles, NULL);
		m_strHudName = strActualName;
		trace_t	trace;
		UTIL_TraceEntity(pVehicle, vecOrigin, vecOrigin, MASK_SOLID, &trace);
		if (trace.startsolid)
			pVehicle->GetUnstuck(500);
	}
	return pVehicle;
}
bool CChaosEffect::ChaosSpawnWeapon(const char* className, string_t strActualName, int iCount, const char* strAmmoType, int iCount2, const char* strAmmoType2)
{
	bool bGaveWeapon = UTIL_GetLocalPlayer()->GiveNamedItem(className, true) != NULL;
	if (!bGaveWeapon)
		return false;
	m_strHudName = strActualName;
	if (iCount)
		UTIL_GetLocalPlayer()->GiveAmmo(iCount, strAmmoType);
	if (iCount2)
		UTIL_GetLocalPlayer()->GiveAmmo(iCount2, strAmmoType2);
	return bGaveWeapon;
}
ConVar getnearbynodes_debug("getnearbynodes_debug", "0");
CNodeList* CChaosEffect::GetNearbyNodes(int iNodes)
{
	CAI_Node* pNode;
	float flClosest = FLT_MAX;
	bool full = false;
	CNodeList* result = new CNodeList;
	result->SetLessFunc(CNodeList::RevIsLowerPriority);//this impacts sorting, MUST be kept
	for (int node = 0; node < g_pBigAINet->NumNodes(); node++)
	{
		pNode = g_pBigAINet->GetNode(node);
		if (pNode->GetType() != NODE_GROUND)
		{
			if (getnearbynodes_debug.GetBool()) Msg("Rejected node %i for not being a ground node\n", pNode->GetId());
			continue;
		}
		float flDist = (UTIL_GetLocalPlayer()->GetAbsOrigin() - pNode->GetPosition(HULL_HUMAN)).Length();
		if (flDist < flClosest)
		{
			if (getnearbynodes_debug.GetBool()) Msg("node %i is closer (%0.1f) than previous closest (%0.1f)\n", pNode->GetId(), flDist, flClosest);
			flClosest = flDist;
		}
		if (!full || (flDist < result->ElementAtHead().dist))
		{
			if (getnearbynodes_debug.GetBool()) Msg("Adding node %i to list. full is %s, %0.1f < %0.1f\n", pNode->GetId(), full ? "TRUE" : "FALSE", flDist, result->Count() > 0 ? result->ElementAtHead().dist : 1234);
			if (full)
			{
				if (getnearbynodes_debug.GetBool()) Msg("List full, removing node %i to add node %i\n", result->ElementAtHead().nodeIndex, pNode->GetId());
				result->RemoveAtHead();
			}
			result->Insert(AI_NearNode_t(node, flDist));
			full = (result->Count() == iNodes);
		}
		else if (flDist >= result->ElementAtHead().dist)
		{
			if (getnearbynodes_debug.GetBool()) Warning("Not adding  %i to list. full is %s, %0.1f < %0.1f\n", pNode->GetId(), full ? "TRUE" : "FALSE", flDist, result->Count() > 0 ? result->ElementAtHead().dist : 1234);
		}
	}
	Msg("list has %i nodes\n", result->Count());
	return result;
}
CAI_Node* CChaosEffect::NearestNodeToPoint(const Vector& vPosition, bool bCheckVisibility)
{
	return g_pBigAINet->GetNode(g_pBigAINet->NearestNodeToPoint(vPosition, bCheckVisibility), false);
}
bool CChaosEffect::IterUsableVehicles(bool bFindOnly)
{
	bool bFoundSomething = false;
	bool bFindBoat = m_nContext & EC_BOAT || m_nContext & EC_NO_VEHICLE || m_nContext == EC_NONE;
	bool bFindBuggy = m_nContext & EC_BUGGY || m_nContext & EC_NO_VEHICLE || m_nContext == EC_NONE;
	//there may be more than one useable car in a map cause chaos is chaotic
	//iterate on boats then cars
	CPropVehicleDriveable* pVehicle = NULL;
	if (bFindBoat)
		pVehicle = (CPropVehicleDriveable*)gEntList.FindEntityByClassname(NULL, "prop_vehicle_airboat");
	if (!pVehicle && bFindBuggy)
	{
		pVehicle = (CPropVehicleDriveable*)gEntList.FindEntityByClassname(NULL, "prop_vehicle_jeep");
		bFindBoat = false;
	}
	while (pVehicle)
	{
		if (!enginetrace->PointOutsideWorld(pVehicle->GetAbsOrigin()))
		{
			//if iterating on cars, check model because some APCs use prop_vehicle_jeep
			if (!bFindBoat && bFindBuggy)
			{
				CPropJeep* pJeep = static_cast<CPropJeep*>(pVehicle);
				if (pJeep->m_bJeep || pJeep->m_bJalopy)
				{
					if (!bFindOnly)
						DoOnVehicles(pVehicle);
					bFoundSomething = true;
					if (bFindOnly)
						return true;
				}
			}
			else
			{
				if (!bFindOnly)
					DoOnVehicles(pVehicle);
				bFoundSomething = true;
				if (bFindOnly)
					return true;
			}
		}
		//iterate on boats then cars
		if (bFindBoat)
		{
			pVehicle = (CPropVehicleDriveable*)gEntList.FindEntityByClassname(pVehicle, "prop_vehicle_airboat");
			if (!pVehicle)
				bFindBoat = false;
		}
		//in the case that we just finished iterating on the last boat, pVehicle is NULL so it should be fine to pass into FindEntityByClassname
		if (!bFindBoat)
			pVehicle = (CPropVehicleDriveable*)gEntList.FindEntityByClassname(pVehicle, "prop_vehicle_jeep");
	}
	return bFoundSomething;
}
CBaseEntity* CChaosEffect::GetEntityWithID(int iChaosID)
{
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		if (pEnt->m_iChaosID == iChaosID)
			return pEnt;
		pEnt = gEntList.NextEnt(pEnt);
	}
	return NULL;
}

bool CChaosEffect::MapGoodForCrane(const char* pMapName)
{
	//Disallow maps that wouldn't let a crane do much. cranes need wide open land to graze on.

	//trigger_physics_trap can cause the magnet to become vaporized
	CBaseEntity* pRemover = gEntList.FindEntityByClassname(NULL, "trigger_physics_trap");
	if (pRemover)
		return false;
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strnicmp("d1", pMapName, 2))
		{
			if (!Q_strcmp(pMapName, "d1_trainstation_03") || !Q_strcmp(pMapName, "d1_trainstation_04") || !Q_strcmp(pMapName, "d1_trainstation_05")
				|| !Q_strcmp(pMapName, "d1_canals_02") || !Q_strcmp(pMapName, "d1_canals_03")
				|| !Q_strcmp(pMapName, "d1_eli_01")
				|| !Q_strcmp(pMapName, "d1_town_04"))
				return false;
		}
		else
		{
			if ((!Q_strnicmp("d2_p", pMapName, 4) && Q_strcmp(pMapName, "d2_prison_01"))//don't allow any prison map except 01 since that's outdoors
				|| !Q_strcmp(pMapName, "d3_c17_01") || !Q_strcmp(pMapName, "d3_c17_05") || !Q_strcmp(pMapName, "d3_c17_06a") || !Q_strcmp(pMapName, "d3_c17_06b") || !Q_strcmp(pMapName, "d3_c17_10b")
				|| !Q_strcmp(pMapName, "d3_citadel_02") || !Q_strcmp(pMapName, "d3_citadel_05")
				|| !Q_strcmp(pMapName, "d3_breen_01"))
				return false;
		}
	}
	else
	{
		if (!Q_strnicmp("ep1", pMapName, 3))
		{
			if (!Q_strcmp(pMapName, "ep1_citadel_02b") || !Q_strcmp(pMapName, "ep1_citadel_03")
				|| !Q_strcmp(pMapName, "ep1_c17_00") || !Q_strcmp(pMapName, "ep1_c17_00a") || !Q_strcmp(pMapName, "ep1_c17_02a"))
				return false;
		}
		else
		{
			if (!Q_strcmp(pMapName, "ep2_outland_01a") || !Q_strcmp(pMapName, "ep2_outland_02") || !Q_strcmp(pMapName, "ep2_outland_03") || !Q_strcmp(pMapName, "ep2_outland_04")
				|| !Q_strcmp(pMapName, "ep2_outland_11") || !Q_strcmp(pMapName, "ep2_outland_11a") || !Q_strcmp(pMapName, "ep2_outland_11b") || !Q_strcmp(pMapName, "ep2_outland_12a"))
				return false;
		}
	}
	return true;
}

bool CChaosEffect::SafeCloneNPCs(const char* pMapName)
{
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strcmp(pMapName, "d1_trainstation_01") || !Q_strnicmp("d1_e", pMapName, 4) || !Q_strcmp(pMapName, "d3_breen_01"))
			return false;
	}
	else//ep2 or ep1 goes here
	{
		if (!Q_strcmp(pMapName, "ep1_citadel_00") || !Q_strcmp(pMapName, "ep1_citadel_01") || !Q_strcmp(pMapName, "ep1_citadel_03") || !Q_strcmp(pMapName, "ep1_citadel_04") || !Q_strcmp(pMapName, "ep1_c17_00"))
			return false;
	}
	return true;
}

bool CChaosEffect::MapHasImportantPickups(const char* pMapName)
{
	//d1_trainstation_05 suit to progress cutscene
	//d2_coast_10 rocket crate for gunship
	//d2_prison_01 rocket crate for gunships
	//d3_c17_09 grenades to save barney
	//d3_c17_10b explosives for ground turrets
	//d3_c17_11 rocket crate for gunship
	//d3_c17_13 rocket crate for striders
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strcmp(pMapName, "d1_trainstation_05") || !Q_strcmp(pMapName, "d2_coast_10") || !Q_strcmp(pMapName, "d2_prison_01")
			|| !Q_strcmp(pMapName, "d3_c17_09") || !Q_strcmp(pMapName, "d3_c17_10b") || !Q_strcmp(pMapName, "d3_c17_11") || !Q_strcmp(pMapName, "d3_c17_13"))
			return true;
	}
	//ep1_c17_00 pistol and shotgun to shoot lock
	//ep1_c17_05 rocket crate for sniper
	//ep1_c17_06 rocket crate for strider
	//ep2_outland_02 too hard
	//ep2_outland_09 grenades for autogun
	//ep2_outland_12 removing seems to break the respawn system?
	else
	{
		if (!Q_strcmp(pMapName, "ep1_c17_00") || !Q_strcmp(pMapName, "ep1_c17_05") || !Q_strcmp(pMapName, "ep1_c17_06")
			|| !Q_strcmp(pMapName, "ep2_outland_02") || !Q_strcmp(pMapName, "ep2_outland_09") || !Q_strcmp(pMapName, "ep2_outland_12"))
			return true;
	}
	return false;
}

bool CChaosEffect::QuickclipProblems(const char* pMapName)
{
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strcmp(pMapName, "d2_prison_05") || !Q_strcmp(pMapName, "d2_prison_06") || !Q_strcmp(pMapName, "d2_prison_08")
			|| !Q_strcmp(pMapName, "d3_citadel_03") || !Q_strcmp(pMapName, "d3_citadel_04") || !Q_strcmp(pMapName, "d3_breen_01"))
			return true;//bad map
	}
	else
	{
		if (!Q_strcmp(pMapName, "ep1_citadel_01") || !Q_strcmp(pMapName, "ep1_citadel_03") || !Q_strcmp(pMapName, "ep1_c17_00a")
			|| !Q_strcmp(pMapName, "ep2_outland_04") || !Q_strcmp(pMapName, "ep2_outland_03") || !Q_strcmp(pMapName, "ep2_outland_12a"))
			return true;//bad map
	}
	return false;
}

bool CChaosEffect::PhysConvertSoftlock(const char* pMapName)
{
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strcmp(pMapName, "d1_trainstation_05") || !Q_strcmp(pMapName, "d1_canals_11")
			|| !Q_strcmp(pMapName, "d1_eli_01") || !Q_strcmp(pMapName, "d1_town_01")
			|| !Q_strcmp(pMapName, "d2_prison_08") || !Q_strcmp(pMapName, "d3_c17_08"))
			return true;//bad map
	}
	else
	{
		if (!Q_strcmp(pMapName, "ep1_citadel_03") || !Q_strcmp(pMapName, "ep1_c17_00a")
			|| !Q_strcmp(pMapName, "ep2_outland_03") || !Q_strcmp(pMapName, "ep2_outland_11") || !Q_strcmp(pMapName, "ep2_outland_11b"))
			return true;//bad map
	}
	return false;
}

bool CChaosEffect::CombatBreaksScene(const char* pMapName)
{
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strcmp(pMapName, "d1_trainstation_01") || !Q_strcmp(pMapName, "d1_trainstation_04") || !Q_strcmp(pMapName, "d1_canals_03") || !Q_strcmp(pMapName, "d1_eli_01")
			|| !Q_strcmp(pMapName, "d2_coast_10") || !Q_strcmp(pMapName, "d2_coast_11") || !Q_strcmp(pMapName, "d3_breen_01"))
			return true;//bad map
	}
	else
	{
		if (!Q_strcmp(pMapName, "ep1_citadel_03") || !Q_strcmp(pMapName, "ep1_c17_02b")
			|| !Q_strcmp(pMapName, "ep2_outland_01") || !Q_strcmp(pMapName, "ep2_outland_07") || !Q_strcmp(pMapName, "ep2_outland_08") || !Q_strcmp(pMapName, "ep2_outland_10a"))
			return true;//bad map
	}
	return false;
}

bool CChaosEffect::NeedPhysgun(const char* pMapName)
{
	if (GlobalEntity_GetState("super_phys_gun") == GLOBAL_ON)
		return true;
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strcmp(pMapName, "d3_c17_07") || !Q_strcmp(pMapName, "d3_c17_08") || !Q_strcmp(pMapName, "d3_c17_10b")
			|| !Q_strcmp(pMapName, "d3_citadel_03") || !Q_strcmp(pMapName, "d3_citadel_04") || !Q_strcmp(pMapName, "d3_breen_01"))
			return true;//bad time to lose the gravity gun
	}
	else
	{
		if (!Q_strcmp(pMapName, "ep1_citadel_00") || !Q_strcmp(pMapName, "ep1_citadel_01") || !Q_strcmp(pMapName, "ep1_citadel_03") || !Q_strcmp(pMapName, "ep1_citadel_04")
			|| !Q_strcmp(pMapName, "ep1_c17_00") || !Q_strcmp(pMapName, "ep1_c17_00a") || !Q_strcmp(pMapName, "ep1_c17_01") || !Q_strcmp(pMapName, "ep1_c17_02")
			|| !Q_strcmp(pMapName, "ep2_outland_01"))
			return true;//bad time to lose the gravity gun
	}
	return false;
}

bool CChaosEffect::DontTeleportPlayer(const char* pMapName)
{
	//don't need to check map list if we would be forcing out of a car, cause we don't teleport (far) in that case, unless you did the ladder bug thing
	if (m_nID == EFFECT_FORCE_INOUT_CAR && UTIL_GetLocalPlayer()->IsInAVehicle())
		return false;
	if (!Q_strnicmp("d", pMapName, 1))//hl2
	{
		if (!Q_strcmp(pMapName, "d1_trainstation_01") || !Q_strcmp(pMapName, "d1_trainstation_04") || !Q_strcmp(pMapName, "d1_trainstation_05")
			|| !Q_strcmp(pMapName, "d1_eli_01") || !Q_strcmp(pMapName, "d1_town_05")
			|| !Q_strcmp(pMapName, "d2_coast_11") || !Q_strcmp(pMapName, "d2_prison_08")
			|| !Q_strcmp(pMapName, "d3_c17_13")
			|| !Q_strcmp(pMapName, "d3_breen_01"))
			return true;//no
	}
	else
	{
		if (!Q_strnicmp("ep1", pMapName, 3))
		{
			if (!Q_strcmp(pMapName, "ep1_citadel_01") || !Q_strcmp(pMapName, "ep1_citadel_03") || !Q_strcmp(pMapName, "ep1_citadel_04")
				|| !Q_strcmp(pMapName, "ep1_c17_00") || !Q_strcmp(pMapName, "ep1_c17_00a") || !Q_strcmp(pMapName, "ep1_c17_01"))
				return true;//no
		}
		else
		{
			if (!Q_strcmp(pMapName, "ep2_outland_01") || !Q_strcmp(pMapName, "ep2_outland_11") || !Q_strcmp(pMapName, "ep2_outland_11b") || !Q_strcmp(pMapName, "ep2_outland_12a"))
				return true;//no
		}
	}
	return false;
}

//make evil NPCs stay evil
//can't do in the effect's MaintainEffect() because that only lasts 80 seconds
void CHL2_Player::MaintainEvils()
{
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		if (pEnt->IsNPC())
		{
			CAI_BaseNPC* pNPC = static_cast<CAI_BaseNPC*>(pEnt);
			if (pNPC->m_bEvil)
			{
				const int MAX_HANDLED = 512;
				CUtlVectorFixed<CBaseCombatCharacter*, MAX_HANDLED> targetList;
				//Search players first
				for (int i = 1; i <= gpGlobals->maxClients; i++)
				{
					CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
					if (pPlayer)
					{
						targetList.AddToTail(pPlayer);
					}
				}
				for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
				{
					CAI_BaseNPC* pNPC = (g_AI_Manager.AccessAIs())[i];
					if (pNPC)
					{
						targetList.AddToTail(pNPC);
					}
				}
				if (targetList.Count() == 0)
				{
					Warning("Found no targets for evil rels!?\n");
					return;
				}
				CBaseCombatCharacter* pSubject = pNPC;
				for (int j = 0; j < targetList.Count(); j++)
				{
					CBaseCombatCharacter* pTarget = targetList[j];
					if (pSubject == pTarget)
						continue;
					//verbose due to debugging
					int reltype = (int)pSubject->IRelationType(pTarget);
					int priority = pSubject->IRelationPriority(pTarget);
					//Msg("reltype %i\n", reltype);
					//Msg("priority %i\n", priority);
					if (reltype != D_HT || priority != 100)
					{
						pSubject->AddEntityRelationship(pTarget, D_HT, 100);
						if (!pSubject->ClassMatches("npc_cra*"))//don't try to attack a crane driver, you're not winning
							pTarget->AddEntityRelationship(pSubject, D_HT, 100);
						else
							pTarget->AddEntityRelationship(pSubject, D_NU, 100);
						//Msg("Applying relationship to %s and %s\n", STRING(pSubject->GetEntityName()), STRING(pTarget->GetEntityName()));
					}
				}
			}
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
}
CAI_BaseNPC* CChaosEffect::ChaosSpawnNPC(const char* className, string_t strActualName, int iSpawnType, const char* strModel, const char* strTargetname, const char* strWeapon, int flags)
{
	bool bEvil = (flags & CSF_EVIL) != 0;
	float flDistAway;
	float flExtraHeight;
	switch (iSpawnType)
	{
	case SPAWNTYPE_HIDEINCOVER:
	case SPAWNTYPE_EYELEVEL_REGULAR:
	case SPAWNTYPE_EYELEVEL_SPECIAL:
	case SPAWNTYPE_CEILING:
	case SPAWNTYPE_ONGROUND:
		flDistAway = 128;
		flExtraHeight = 64;
		break;
	case SPAWNTYPE_BIGFLYER:
		flDistAway = 192;
		flExtraHeight = 256;
		break;
	case SPAWNTYPE_STRIDER:
		flDistAway = 128;
		flExtraHeight = 512;
		break;
	default:
		return NULL;
	}
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();//This will always give back some actual XY numbers because the camera is actually limited to 89 degrees up or down, not 90
	//then again something weird COULD happen to set the angle to straight 90 up/down, but idk what that would be
	CAI_BaseNPC* pNPC = (CAI_BaseNPC*)CreateEntityByName(className);
	if (pNPC)
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * flDistAway + Vector(0, 0, flExtraHeight);
		trace_t wallTrace;
		UTIL_TraceLine(pPlayer->GetAbsOrigin(), vecOrigin, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &wallTrace);
		vecOrigin = wallTrace.endpos;
		if (iSpawnType == SPAWNTYPE_CEILING)//put the NPC on the ceiling
		{
			trace_t tr;
			UTIL_TraceLine(vecOrigin, vecOrigin + Vector(0, 0, 100000), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);
			vecOrigin = tr.endpos + Vector(0, 0, -2);//move down a bit so barnacle looks right
		}
		float flPitch = 0;
		QAngle aAngles;
		if (iSpawnType == SPAWNTYPE_ONGROUND)//put the NPC on/in the ground
		{
			trace_t tr;
			UTIL_TraceLine(vecOrigin, vecOrigin - Vector(0, 0, 100000), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);
			vecOrigin = tr.endpos + Vector(0, 0, 1);//a little above the ground to avoid z fighting
			Vector xaxis(1.0f, 0.0f, 0.0f);
			pPlayer->EyeVectors(&xaxis);
			xaxis = -xaxis;
			Vector yaxis;
			CrossProduct(tr.plane.normal, xaxis, yaxis);
			if (VectorNormalize(yaxis) < 1e-3)
			{
				xaxis.Init(0.0f, 0.0f, 1.0f);
				CrossProduct(tr.plane.normal, xaxis, yaxis);
				VectorNormalize(yaxis);
			}
			CrossProduct(yaxis, tr.plane.normal, xaxis);
			VectorNormalize(xaxis);
			VMatrix entToWorld;
			entToWorld.SetBasisVectors(xaxis, yaxis, tr.plane.normal);
			MatrixToAngles(entToWorld, aAngles);
		}
		else
		{
			aAngles = QAngle(flPitch, pPlayer->GetAbsAngles().y - 180, 0);
		}
		pNPC->SetAbsOrigin(vecOrigin);
		pNPC->SetAbsAngles(aAngles);
		pNPC->m_bEvil = bEvil;
		if (FStrEq(className, "npc_alyx")) pNPC->KeyValue("ShouldHaveEMP", "1");
		else if (FStrEq(className, "npc_cscanner")) pNPC->KeyValue("ShouldInspect", "1");
		else if (FStrEq(className, "npc_sniper")) pNPC->AddSpawnFlags(65536);
		else if (FStrEq(className, "npc_strider")) pNPC->AddSpawnFlags(65536);
		else if (FStrEq(className, "npc_vortigaunt")) pNPC->KeyValue("ArmorRechargeEnabled", bEvil ? "0" : "1");
		else if (FStrEq(className, "npc_apcdriver"))
		{
			pNPC->KeyValue("vehicle", "apc");
			pNPC->KeyValue("parentname", "apc");//once i saw an APC do nothing when it should have. maybe the driver was stuck in the wall and couldn't see, while the APC was teleported with GetUnstuck?
		}
		else if (FStrEq(className, "npc_antlion"))
		{
			if (RandomInt(0, 1) == 1)//cave variant
				pNPC->AddSpawnFlags(262144);
			pNPC->KeyValue("startburrowed", "0");
		}
		else if (FStrEq(className, "npc_antlionguard"))
		{
			if (RandomInt(0, 1) == 1)//cave variant
			{
				pNPC->KeyValue("cavernbreed", "1");
				pNPC->KeyValue("incavern", "1");
			}
			pNPC->KeyValue("startburrowed", "0");
			pNPC->KeyValue("allowbark", "1");
			pNPC->KeyValue("shovetargets", "*");
		}
		else if (FStrEq(className, "npc_citizen"))
		{
			pNPC->AddSpawnFlags(65536);//follow player
			pNPC->AddSpawnFlags(262144);//random head
			if (RandomInt(0, 1) == 1)//medic
				pNPC->AddSpawnFlags(131072);
			if (RandomInt(0, 1) == 1)//ammo resupplier
			{
				float nRandom = RandomInt(0, 10);
				if (nRandom == 0) pNPC->KeyValue("ammosupply", "AR2");
				if (nRandom == 1) pNPC->KeyValue("ammosupply", "Pistol");
				if (nRandom == 2) pNPC->KeyValue("ammosupply", "SMG1");
				if (nRandom == 3) pNPC->KeyValue("ammosupply", "357");
				if (nRandom == 4) pNPC->KeyValue("ammosupply", "XBowBolt");
				if (nRandom == 5) pNPC->KeyValue("ammosupply", "Buckshot");
				if (nRandom == 6) pNPC->KeyValue("ammosupply", "RPG_Round");
				if (nRandom == 7) pNPC->KeyValue("ammosupply", "SMG1_Grenade");
				if (nRandom == 8) pNPC->KeyValue("ammosupply", "Grenade");
				if (nRandom == 9) pNPC->KeyValue("ammosupply", "Battery");
				if (nRandom == 10) pNPC->KeyValue("ammosupply", "AR2AltFire");
				pNPC->AddSpawnFlags(524288);
				pNPC->KeyValue("ammoamount", "100");
			}
			int nRandom = RandomInt(0, 6);//weapon
			if (nRandom == 0) pNPC->KeyValue("additionalequipment", "weapon_ar2");
			if (nRandom == 1) pNPC->KeyValue("additionalequipment", "weapon_citizenpackage");
			if (nRandom == 2) pNPC->KeyValue("additionalequipment", "weapon_citizensuitcase");
			if (nRandom == 3) pNPC->KeyValue("additionalequipment", "weapon_crowbar");
			if (nRandom == 4) pNPC->KeyValue("additionalequipment", "weapon_rpg");
			if (nRandom == 5) pNPC->KeyValue("additionalequipment", "weapon_shotgun");
			if (nRandom == 6) pNPC->KeyValue("additionalequipment", "weapon_smg1");

			nRandom = RandomInt(0, 3);//clothing
			if (nRandom == 0) pNPC->KeyValue("citizentype", "0");
			if (nRandom == 1) pNPC->KeyValue("citizentype", "1");
			if (nRandom == 2) pNPC->KeyValue("citizentype", "2");
			if (nRandom == 3) pNPC->KeyValue("citizentype", "3");

			pNPC->KeyValue("expressiontype", "0");
		}
		else if (FStrEq(className, "npc_combine_s"))
		{
			pNPC->KeyValue("NumGrenades", "100");
			int nRandom = RandomInt(0, 2);//model/elite status
			if (nRandom == 0) pNPC->KeyValue("model", "models/combine_soldier.mdl");
			if (nRandom == 1) pNPC->KeyValue("model", "models/combine_super_soldier.mdl");
			if (nRandom == 2) pNPC->KeyValue("model", "models/combine_soldier_prisonguard.mdl");

			nRandom = RandomInt(0, 2);//weapon
			if (nRandom == 0) pNPC->KeyValue("additionalequipment", "weapon_ar2");
			if (nRandom == 1) pNPC->KeyValue("additionalequipment", "weapon_shotgun");
			if (nRandom == 2) pNPC->KeyValue("additionalequipment", "weapon_smg1");
		}
		else if (FStrEq(className, "npc_combinedropship"))
		{
			int nRandom = 1;// RandomInt(-3, 1);//cargo type. avoid 0 cause that does nothing
			Msg("crate type %i\n", nRandom);
			if (nRandom == -3) pNPC->KeyValue("CrateType", "-3");//jeep
			if (nRandom == -2)//apc
			{
				pNPC->KeyValue("CrateType", "-2");
				ChaosSpawnVehicle("prop_vehicle_apc", strActualName, SPAWNTYPE_VEHICLE, "models/combine_apc.mdl", "chaos_dropship_apc", "scripts/vehicles/apc_npc.txt");
				pNPC->KeyValue("APCVehicleName", "chaos_dropship_apc");
				variant_t emptyVariant;
				g_EventQueue.AddEvent("combinedropship", "DropAPC", emptyVariant, 1, pNPC, pNPC);
			}
			if (nRandom == -1)//strider
			{
				pNPC->KeyValue("CrateType", "-1");
				variant_t emptyVariant;
				g_EventQueue.AddEvent("combinedropship", "DropStrider", emptyVariant, 1, pNPC, pNPC);
			}
			if (nRandom == 0) pNPC->KeyValue("CrateType", "2");//nothing
			if (nRandom == 1)//soldier crate
			{
				pNPC->KeyValue("CrateType", "1");
				pNPC->KeyValue("GunRange", "2000");
				//soldiers are spawned in CNPC_CombineDropship::SpawnTroop
				CBaseEntity* pTarget = CreateEntityByName("info_target");
				pTarget->KeyValue("targetname", "dropship_target");
				pTarget->SetAbsOrigin(vecOrigin);
				pTarget->SetAbsAngles(aAngles);
				DispatchSpawn(pTarget);
				pNPC->KeyValue("LandTarget", "dropship_target");
				variant_t variant;
				variant.SetInt(6);
				if (RandomInt(0, 1))
					g_EventQueue.AddEvent("combinedropship", "LandLeaveCrate", variant, 1, pNPC, pNPC);
				else
					g_EventQueue.AddEvent("combinedropship", "LandTakeCrate", variant, 1, pNPC, pNPC);
			}
		}
		else if (FStrEq(className, "npc_dog"))
		{
			CBaseEntity* pTarget = CreateEntityByName("info_radar_target");
			pTarget->KeyValue("radius", "3000");
			pTarget->KeyValue("type", "2");
			pTarget->SetParent(pNPC);
			pTarget->SetAbsOrigin(vecOrigin);
			pTarget->SetAbsAngles(aAngles);
			DispatchSpawn(pTarget);
		}
		else if (FStrEq(className, "npc_metropolice"))
		{
			pNPC->KeyValue("manhacks", "100");

			int nRandom = RandomInt(0, 2);//weapon
			if (nRandom == 0) pNPC->KeyValue("additionalequipment", "weapon_smg1");
			if (nRandom == 1) pNPC->KeyValue("additionalequipment", "weapon_pistol");
			if (nRandom == 2) pNPC->KeyValue("additionalequipment", "weapon_stunstick");
		}
		else if (FStrEq(className, "npc_stalker"))
		{
			int nRandom = RandomInt(0, 2);
			if (nRandom == 0) pNPC->KeyValue("BeamPower", "0");
			if (nRandom == 1) pNPC->KeyValue("BeamPower", "1");
			if (nRandom == 2) pNPC->KeyValue("BeamPower", "2");
		}
		else if (FStrEq(className, "npc_turret_ceiling"))
		{
			pNPC->SetMaxHealth(700);
			pNPC->SetHealth(700);
			pNPC->AddSpawnFlags(32);
		}
		else if (FStrEq(className, "npc_turret_ground"))
		{
			CBaseEntity* pMover = CreateEntityByName("func_movelinear");
			pMover->SetAbsOrigin(vecOrigin);

			//get a vector for what's "upward" relative to the surface we're on
			Vector forward, right, up;
			AngleVectors(aAngles, &forward, &right, &up);
			//entity wants a QAngle though... so put the direction back to an angle. dumb!
			QAngle angUp;
			VectorAngles(up, angUp);

			char buf[512];
			Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", angUp.x, angUp.y, angUp.z);
			pMover->KeyValue("movedir", buf);
			pMover->KeyValue("movedistance", "32");
			pMover->KeyValue("speed", "40");
			pMover->KeyValue("startposition", "0");
			pMover->KeyValue("startsound", "Streetwar.d3_c17_10b_doormove3");
			pMover->KeyValue("stopsound", "Streetwar.d3_c17_10b_metal_stop1");
			pMover->KeyValue("targetname", "turret_lift");
			pMover->KeyValue("OnFullyOpen", "turret_ground,Enable,,0,-1");
			g_iChaosSpawnCount++; pMover->KeyValue("chaosid", g_iChaosSpawnCount);
			pNPC->SetAbsOrigin(vecOrigin - (up * 8));//put turret below top part of the cover
			pNPC->SetAbsAngles(aAngles);
			pMover->m_bChaosPersist = true;
			pMover->m_bChaosSpawned = true;
			DispatchSpawn(pMover);
			pMover->Activate();
			CBaseEntity* pProp = CreateEntityByName("prop_dynamic_override");
			pProp->SetAbsOrigin(vecOrigin);
			pProp->SetAbsAngles(aAngles);
			pProp->KeyValue("model", "models/props_c17/turretcover.mdl");
			pProp->KeyValue("disableshadows", "1");
			pProp->KeyValue("solid", "6");
			g_iChaosSpawnCount++; pProp->KeyValue("chaosid", g_iChaosSpawnCount);
			pProp->m_bChaosPersist = true;
			pProp->m_bChaosSpawned = true;
			DispatchSpawn(pProp);
			pProp->Activate();
			pNPC->SetParent(pMover);
			pProp->SetParent(pMover);
			variant_t sVariant;
			pMover->AcceptInput("Open", pMover, pMover, sVariant, 0);
			CBaseEntity* pSound1 = CreateEntityByName("ambient_generic");
			pSound1->SetAbsOrigin(vecOrigin);
			pSound1->KeyValue("targetname", "turret_detected_sound");
			pSound1->KeyValue("message", "Streetwar.d3_c17_10b_alarm1");
			pSound1->KeyValue("spawnflags", "16");
			g_iChaosSpawnCount++; pSound1->KeyValue("chaosid", g_iChaosSpawnCount);
			pSound1->m_bChaosPersist = true;
			pSound1->m_bChaosSpawned = true;
			DispatchSpawn(pSound1);
			pSound1->Activate();
			g_EventQueue.AddEvent("turret_detected_sound", "PlaySound", sVariant, 0.01, pPlayer, pPlayer);
			g_EventQueue.AddEvent("turret_detected_sound", "StopSound", sVariant, 1.5, pPlayer, pPlayer);
			pNPC->KeyValue("OnDeath", "turret_lift,Close,,0,-1");
		}

		if (!FStrEq(strModel, "_"))
			pNPC->KeyValue("model", strModel);
		if (!FStrEq(strWeapon, "_"))
			pNPC->KeyValue("additionalequipment", strWeapon);
		pNPC->KeyValue("targetname", strTargetname);
		pNPC->m_bChaosSpawned = true;
		pNPC->m_bChaosPersist = true;
		g_iChaosSpawnCount++; pNPC->CBaseEntity::KeyValue("chaosid", g_iChaosSpawnCount);

		DispatchSpawn(pNPC);

		//AI functions are expecting certain things to be set up which only happen after properly spawning (namely having capabilities set)
		if (iSpawnType == SPAWNTYPE_HIDEINCOVER)
		{
			Vector vResult = vec3_origin;
			if (pNPC->GetTacticalServices()->FindLateralCover(pPlayer->EyePosition(), 0, &vResult))
			{
				vecOrigin = vResult;
			}
			else
			{
				//set ground capability on flying NPCs so that they are allowed to use a ground node as "cover" temporarily
				bool bRemoveGroundCap = false;
				if (!(pNPC->CapabilitiesGet() & bits_CAP_MOVE_GROUND))
				{
					bRemoveGroundCap = true;
					pNPC->CapabilitiesAdd(bits_CAP_MOVE_GROUND);
				}
				if (pNPC->GetTacticalServices()->FindCoverPos(pPlayer->GetAbsOrigin(), pPlayer->EyePosition(), 0, FLT_MAX, &vResult, false))
				{
					vecOrigin = vResult;
				}
				else
				{
					Msg("Couldn't find cover...\n");
				}
				if (bRemoveGroundCap)
					pNPC->CapabilitiesRemove(bits_CAP_MOVE_GROUND);
			}
		}

		pNPC->Activate();
		if (iSpawnType != SPAWNTYPE_ONGROUND)//taken care of
			pNPC->Teleport(&vecOrigin, &aAngles, NULL);
		m_strHudName = strActualName;

		if (iSpawnType != SPAWNTYPE_ONGROUND)//put the NPC in the ground
			pNPC->GetUnstuck(500);

		if ((flags & CSF_SQUAD) != 0)
		{
			CNPC_PlayerCompanion* pPC = dynamic_cast<CNPC_PlayerCompanion*>(pNPC);
			if (pPC)
			{
				if (!pPC->GetFollowBehavior().GetOuter())//avoid a crash with gman
				{
					//pPC->GetFollowBehavior().SetOuter(pNPC);
					//pPC->GetFollowBehavior().SetBackBridge(pNPC);
					pPC->AddBehavior(&pPC->GetFollowBehavior());
				}
				pPC->GetFollowBehavior().SetFollowTarget(UTIL_GetLocalPlayer());
				if (!pPC->GetRunningBehavior())
					pPC->DeferSchedulingToBehavior(&pPC->GetFollowBehavior());//ChangeBehaviorTo(&pPC->GetFollowBehavior());
			}
			else
			{
				Assert(0);
			}
		}

	}
	return pNPC;
}
void CEPullToPlayer::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		if (gEntList.FindEntityByName(NULL, "chaos_pull_to_player") == NULL)
		{
			CPhysExplosion* pExplo = (CPhysExplosion*)CBaseEntity::Create("env_physexplosion", pPlayer->GetAbsOrigin() + Vector(0, 0, 32), QAngle(0, 0, 0), pPlayer);
			pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
			pExplo->m_radius = 600;
			pExplo->m_bInvert = true;
			pExplo->AddSpawnFlags(SF_PHYSEXPLOSION_NODAMAGE);
			pExplo->SetParent(pPlayer);
			pExplo->SetName(MAKE_STRING("chaos_pull_to_player"));
			pExplo->m_bConstant = true;
			variant_t emptyVariant;
			g_EventQueue.AddEvent("chaos_pull_to_player", "Explode", emptyVariant, 0, pPlayer, pPlayer, 0);
		}
	}
}
void CEPullToPlayer::StopEffect()
{
	CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "chaos_pull_to_player");
	while (pEnt)
	{
		UTIL_Remove(pEnt);
		//there is another
		pEnt = gEntList.FindEntityByName(pEnt, "chaos_pull_to_player");
	}
}
void CEPullToPlayer::TransitionEffect()
{
	StartEffect();
}
bool CEPullToPlayer::CheckStrike(const CTakeDamageInfo& info)
{
	return (info.GetDamageType() & DMG_CRUSH | DMG_SLASH | DMG_BLAST) != 0;
}
void CEPushFromPlayer::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		if (gEntList.FindEntityByName(NULL, "chaos_push_from_player") == NULL)
		{
			CPhysExplosion* pExplo = (CPhysExplosion*)CBaseEntity::Create("env_physexplosion", pPlayer->GetAbsOrigin() + Vector(0, 0, 32), QAngle(0, 0, 0), pPlayer);
			pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
			pExplo->m_radius = 500;
			pExplo->m_bInvert = false;
			pExplo->AddSpawnFlags(SF_PHYSEXPLOSION_NODAMAGE);
			pExplo->SetParent(pPlayer);
			pExplo->SetName(MAKE_STRING("chaos_push_from_player"));
			pExplo->m_bConstant = true;
			variant_t emptyVariant;
			g_EventQueue.AddEvent("chaos_push_from_player", "Explode", emptyVariant, 0, pPlayer, pPlayer, 0);
		}
	}
}
void CEPushFromPlayer::StopEffect()
{
	CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "chaos_push_from_player");
	while (pEnt)
	{
		UTIL_Remove(pEnt);
		//there is another
		pEnt = gEntList.FindEntityByName(pEnt, "chaos_push_from_player");
	}
}
void CEPushFromPlayer::TransitionEffect()
{
	StartEffect();
}
void CERandomWeaponGive::StartEffect()
{
	UTIL_GetLocalPlayer()->EquipSuit();
	int nRandom;
	//TODO: harpoon, alyxgun, annabelle, citizenpackage, citizensuitcase
	for (int iWeaponAttempts = 0; iWeaponAttempts <= 30; iWeaponAttempts++)
	{
		nRandom = chaos_rng1.GetInt() == -1 ? RandomInt(0, 14) : chaos_rng1.GetInt();
		if (nRandom == 0) if (ChaosSpawnWeapon("weapon_crowbar", MAKE_STRING("#hl2c_give_crowbar"))) return;
		if (nRandom == 1) if (ChaosSpawnWeapon("weapon_physcannon", MAKE_STRING("#hl2c_give_gravgun"))) return;
		if (nRandom == 2) if (ChaosSpawnWeapon("weapon_pistol", MAKE_STRING("#hl2c_give_pistol"), 255, "Pistol")) return;
		if (nRandom == 3) if (ChaosSpawnWeapon("weapon_357", MAKE_STRING("#hl2c_give_357"), 32, "357")) return;
		if (nRandom == 4) if (ChaosSpawnWeapon("weapon_smg1", MAKE_STRING("#hl2c_give_smg"), 255, "SMG1", 3, "smg1_grenade")) return;
		if (nRandom == 5) if (ChaosSpawnWeapon("weapon_ar2", MAKE_STRING("#hl2c_give_ar2"), 255, "AR2", 5, "AR2AltFire")) return;
		if (nRandom == 6) if (ChaosSpawnWeapon("weapon_shotgun", MAKE_STRING("#hl2c_give_shotgun"), 255, "Buckshot")) return;
		if (nRandom == 7) if (ChaosSpawnWeapon("weapon_crossbow", MAKE_STRING("#hl2c_give_crossbow"), 16, "XBowBolt")) return;
		if (nRandom == 8) if (ChaosSpawnWeapon("weapon_frag", MAKE_STRING("#hl2c_give_frag"), 5, "grenade")) return;
		if (nRandom == 9) if (ChaosSpawnWeapon("weapon_rpg", MAKE_STRING("#hl2c_give_rpg"), 3, "rpg_round")) return;
		if (nRandom == 10)
		{
			if (ChaosSpawnWeapon("weapon_bugbait", MAKE_STRING("#hl2c_give_bugbait")))
			{
				GlobalEntity_Add(MAKE_STRING("antlion_allied"), gpGlobals->mapname, GLOBAL_ON);//antlions become friendly
				return;
			}
		}
		if (nRandom == 11) if (ChaosSpawnWeapon("weapon_slam", MAKE_STRING("#hl2c_give_slam"), 5, "slam")) return;
		if (nRandom == 12) if (ChaosSpawnWeapon("weapon_cubemap", MAKE_STRING("#hl2c_give_cubemap"))) return;
		if (nRandom == 13) if (ChaosSpawnWeapon("weapon_stunstick", MAKE_STRING("#hl2c_give_stunstick"))) return;
		if (nRandom == 14) if (ChaosSpawnWeapon("weapon_oldmanharpoon", MAKE_STRING("Give Harpoon"))) return;
	}
}
void CERandomVehicle::StartEffect()
{
	variant_t sVariant;
	int nRandom;
	nRandom = chaos_rng1.GetInt() == -1 ? RandomInt(0, 5) : chaos_rng1.GetInt();
	if (nRandom == 5)
	{
		CBaseEntity* pJalopy;
		if (UTIL_GetLocalPlayer()->GetModelScale() == 2)
			pJalopy = ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("#hl2c_spawn_jalopy"), SPAWNTYPE_VEHICLE, "models/vehicle_2.mdl", "jalopy", "scripts/vehicles/jalopy_2.txt");
		else if (UTIL_GetLocalPlayer()->GetModelScale() == 0.5)
			pJalopy = ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("#hl2c_spawn_jalopy"), SPAWNTYPE_VEHICLE, "models/vehicle_0_5.mdl", "jalopy", "scripts/vehicles/jalopy_0_5.txt");
		else
			pJalopy = ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("#hl2c_spawn_jalopy"), SPAWNTYPE_VEHICLE, "models/vehicle.mdl", "jalopy", "scripts/vehicles/jalopy.txt");
		pJalopy->AcceptInput("EnableRadar", pJalopy, pJalopy, sVariant, 0);
		pJalopy->AcceptInput("EnableRadarDetectEnemies", pJalopy, pJalopy, sVariant, 0);
		pJalopy->AcceptInput("AddBusterToCargo", pJalopy, pJalopy, sVariant, 0);
		CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
		CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
		pHL2Player->SetLocatorTargetEntity(pJalopy);
	}
	if (nRandom == 4)
	{
		if (gEntList.FindEntityByClassname(NULL, "trigger_physics_trap"))//avoid dissolver triggers, easy crash
		{
			nRandom = chaos_rng1.GetInt() == -1 ? RandomInt(0, 3) : chaos_rng1.GetInt();
		}
		else
		{
			//crane
			CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
			CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
			g_iChaosSpawnCount++;
			char szName[2048];

			//crane magnet
			Vector vecOrigin = pHL2Player->RotatedOffset(Vector(1034, 164, 750) * pHL2Player->GetModelScale(), true);
			QAngle vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
			CBaseEntity* pMagnet = CreateEntityByName("phys_magnet");
			if (pHL2Player->GetModelScale() == 2)
				pMagnet->KeyValue("model", "models/props_wasteland/cranemagnet01a_2.mdl");
			else if (pHL2Player->GetModelScale() == 0.5)
				pMagnet->KeyValue("model", "models/props_wasteland/cranemagnet01a_0_5.mdl");
			else
				pMagnet->KeyValue("model", "models/props_wasteland/cranemagnet01a.mdl");
			pMagnet->KeyValue("massScale", "1000");
			Q_snprintf(szName, sizeof(szName), "crane_magnet_%i", g_iChaosSpawnCount);
			pMagnet->KeyValue("targetname", szName);
			pMagnet->KeyValue("overridescript", "damping,0.2,rotdamping,0.2,inertia,0.3");
			DispatchSpawn(pMagnet);
			pMagnet->Activate();
			pMagnet->Teleport(&vecOrigin, &vecAngles, NULL);
			pMagnet->GetUnstuck(500 * pHL2Player->GetModelScale(), UF_NO_NODE_TELEPORT);

			//crane
			vecOrigin = pHL2Player->RotatedOffset(Vector(400, 0, 64) * pHL2Player->GetModelScale(), true);
			vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
			CBaseEntity* pVehicle = CreateEntityByName("prop_vehicle_crane");
			if (pHL2Player->GetModelScale() == 2)
				pVehicle->KeyValue("model", "models/Cranes/crane_docks_2.mdl");
			else if (pHL2Player->GetModelScale() == 0.5)
				pVehicle->KeyValue("model", "models/Cranes/crane_docks_0_5.mdl");
			else
				pVehicle->KeyValue("model", "models/Cranes/crane_docks.mdl");
			pVehicle->KeyValue("solid", "6");
			pVehicle->KeyValue("magnetname", szName);
			Q_snprintf(szName, sizeof(szName), "crane%i", g_iChaosSpawnCount);
			pVehicle->KeyValue("targetname", szName);
			pVehicle->KeyValue("vehiclescript", "scripts/vehicles/crane.txt");
			pVehicle->KeyValue("PlayerOn", "chaos_ladder,Disable,,0,-1");
			pVehicle->KeyValue("PlayerOff", "chaos_ladder,Enable,,5,-1");
			pVehicle->Teleport(&vecOrigin, &vecAngles, NULL);

			//crane ladder
			vecOrigin = pHL2Player->RotatedOffset(Vector(524, 84, 90) * pHL2Player->GetModelScale(), true);
			vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
			CFuncLadder* pLadder = (CFuncLadder*)CreateEntityByName("func_useableladder");
			pLadder->KeyValue("parentname", szName);
			pLadder->SetEndPoints(pHL2Player->RotatedOffset(Vector(524, 84, 90) * pHL2Player->GetModelScale(), true), pHL2Player->RotatedOffset(Vector(524, 84, 4) * pHL2Player->GetModelScale(), true));
			pLadder->KeyValue("targetname", "chaos_ladder");
			pLadder->KeyValue("spawnflags", "1");
			Q_snprintf(szName, sizeof(szName), "crane%i,ForcePlayerIn,,0,-1", g_iChaosSpawnCount);
			pLadder->KeyValue("OnPlayerGotOnLadder", szName);
			DispatchSpawn(pLadder);
			pLadder->Activate();
			pLadder->Teleport(&vecOrigin, &vecAngles, NULL);

			//activate crane last so everything works correctly
			DispatchSpawn(pVehicle);
			pVehicle->Activate();
			m_strHudName = MAKE_STRING("#hl2c_spawn_crane");
		}
	}
	if (nRandom == 0)
	{
		if (UTIL_GetLocalPlayer()->GetModelScale() == 2)
			ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("#hl2c_spawn_buggy"), SPAWNTYPE_VEHICLE, "models/buggy_2.mdl", "jeep", "scripts/vehicles/jeep_test_2.txt");
		else if (UTIL_GetLocalPlayer()->GetModelScale() == 0.5)
			ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("#hl2c_spawn_buggy"), SPAWNTYPE_VEHICLE, "models/buggy_0_5.mdl", "jeep", "scripts/vehicles/jeep_test_0_5.txt");
		else
			ChaosSpawnVehicle("prop_vehicle_jeep", MAKE_STRING("#hl2c_spawn_buggy"), SPAWNTYPE_VEHICLE, "models/buggy.mdl", "jeep", "scripts/vehicles/jeep_test.txt");
	}
	if (nRandom == 1)
	{
		if (UTIL_GetLocalPlayer()->GetModelScale() == 2)
			ChaosSpawnVehicle("prop_vehicle_airboat", MAKE_STRING("#hl2c_spawn_airboat"), SPAWNTYPE_VEHICLE, "models/airboat_2.mdl", "airboat", "scripts/vehicles/airboat_2.txt");
		else if (UTIL_GetLocalPlayer()->GetModelScale() == 0.5)
			ChaosSpawnVehicle("prop_vehicle_airboat", MAKE_STRING("#hl2c_spawn_airboat"), SPAWNTYPE_VEHICLE, "models/airboat_0_5.mdl", "airboat", "scripts/vehicles/airboat_0_5.txt");
		else
			ChaosSpawnVehicle("prop_vehicle_airboat", MAKE_STRING("#hl2c_spawn_airboat"), SPAWNTYPE_VEHICLE, "models/airboat.mdl", "airboat", "scripts/vehicles/airboat.txt");
	}
	if (nRandom == 2)
	{
		if (UTIL_GetLocalPlayer()->GetModelScale() == 2)
			ChaosSpawnVehicle("prop_vehicle_prisoner_pod", MAKE_STRING("#hl2c_spawn_pod"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/vehicles/prisoner_pod_inner_2.mdl", "pod", "scripts/vehicles/prisoner_pod_2.txt");
		else if (UTIL_GetLocalPlayer()->GetModelScale() == 0.5)
			ChaosSpawnVehicle("prop_vehicle_prisoner_pod", MAKE_STRING("#hl2c_spawn_pod"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/vehicles/prisoner_pod_inner_0_5.mdl", "pod", "scripts/vehicles/prisoner_pod_0_5.txt");
		else
			ChaosSpawnVehicle("prop_vehicle_prisoner_pod", MAKE_STRING("#hl2c_spawn_pod"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/vehicles/prisoner_pod_inner.mdl", "pod", "scripts/vehicles/prisoner_pod.txt");
	}
	if (nRandom == 3)
	{
		if (UTIL_GetLocalPlayer()->GetModelScale() == 2)
			ChaosSpawnVehicle("prop_vehicle_apc", MAKE_STRING("#hl2c_spawn_apc"), SPAWNTYPE_VEHICLE, "models/combine_apc_2.mdl", "apc", "scripts/vehicles/apc_npc_2.txt");
		else if (UTIL_GetLocalPlayer()->GetModelScale() == 0.5)
			ChaosSpawnVehicle("prop_vehicle_apc", MAKE_STRING("#hl2c_spawn_apc"), SPAWNTYPE_VEHICLE, "models/combine_apc_0_5.mdl", "apc", "scripts/vehicles/apc_npc_0_5.txt");
		else
			ChaosSpawnVehicle("prop_vehicle_apc", MAKE_STRING("#hl2c_spawn_apc"), SPAWNTYPE_VEHICLE, "models/combine_apc.mdl", "apc", "scripts/vehicles/apc_npc.txt");
	}
}
void CERandomNPC::StartEffect()
{
	//CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	variant_t emptyVariant;
	variant_t sVariant;
	int nRandom;
	//TODO: blob
	nRandom = chaos_rng1.GetInt() == -1 ? RandomInt(0, 46) : chaos_rng1.GetInt();
	if (nRandom == 0)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_alyx", MAKE_STRING("#hl2c_spawn_alyx"), SPAWNTYPE_EYELEVEL_REGULAR, "models/alyx.mdl", "alyx", "weapon_alyxgun", CSF_SQUAD)->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 1) m_iSavedChaosID = ChaosSpawnNPC("npc_antlion", MAKE_STRING("#hl2c_spawn_antlion"), SPAWNTYPE_HIDEINCOVER, "_", "antlion", "_")->m_iChaosID;
	if (nRandom == 2) m_iSavedChaosID = ChaosSpawnNPC("npc_antlionguard", MAKE_STRING("Spawn Antlionguard"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "antlionguard", "_")->m_iChaosID;
	if (nRandom == 3) m_iSavedChaosID = ChaosSpawnNPC("npc_barnacle", MAKE_STRING("#hl2c_spawn_barnacle"), SPAWNTYPE_CEILING, "_", "barnacle", "_")->m_iChaosID;
	if (nRandom == 4)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_barney", MAKE_STRING("#hl2c_spawn_barney"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "barney", "weapon_ar2", CSF_SQUAD)->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 5) m_iSavedChaosID = ChaosSpawnNPC("npc_breen", MAKE_STRING("#hl2c_spawn_breen"), SPAWNTYPE_EYELEVEL_REGULAR, "models/breen.mdl", "breen", "_")->m_iChaosID;
	if (nRandom == 6)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_citizen", MAKE_STRING("#hl2c_spawn_citizen"), SPAWNTYPE_EYELEVEL_REGULAR, "models/Humans/Group02/Male_05.mdl", "citizen", "_", CSF_SQUAD)->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 7) m_iSavedChaosID = ChaosSpawnNPC("npc_combine_s", MAKE_STRING("#hl2c_spawn_combine_s"), SPAWNTYPE_HIDEINCOVER, "_", "combine_s", "_")->m_iChaosID;
	if (nRandom == 8) m_iSavedChaosID = ChaosSpawnNPC("npc_combinedropship", MAKE_STRING("#hl2c_spawn_dropship"), SPAWNTYPE_BIGFLYER, "_", "combinedropship", "_")->m_iChaosID;
	if (nRandom == 9) m_iSavedChaosID = ChaosSpawnNPC("npc_combinegunship", MAKE_STRING("#hl2c_spawn_gunship"), SPAWNTYPE_BIGFLYER, "_", "combinegunship", "_")->m_iChaosID;
	if (nRandom == 10) m_iSavedChaosID = ChaosSpawnNPC("npc_crow", MAKE_STRING("#hl2c_spawn_crow"), SPAWNTYPE_EYELEVEL_REGULAR, "models/crow.mdl", "crow", "_")->m_iChaosID;
	if (nRandom == 11) m_iSavedChaosID = ChaosSpawnNPC("npc_cscanner", MAKE_STRING("#hl2c_spawn_scanner"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/combine_scanner.mdl", "cscanner", "_")->m_iChaosID;
	if (nRandom == 12)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_dog", MAKE_STRING("#hl2c_spawn_dog"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "dog", "_")->m_iChaosID;
		GetEntityWithID(m_iSavedChaosID)->AcceptInput("StartCatchThrowBehavior", GetEntityWithID(m_iSavedChaosID), GetEntityWithID(m_iSavedChaosID), emptyVariant, 0);
	}
	if (nRandom == 13) m_iSavedChaosID = ChaosSpawnNPC("npc_eli", MAKE_STRING("#hl2c_spawn_eli"), SPAWNTYPE_EYELEVEL_REGULAR, "models/eli.mdl", "eli", "_", CSF_SQUAD)->m_iChaosID;
	if (nRandom == 14) m_iSavedChaosID = ChaosSpawnNPC("npc_fastzombie", MAKE_STRING("#hl2c_spawn_fastzombie"), SPAWNTYPE_HIDEINCOVER, "_", "fastzombie", "_")->m_iChaosID;
	if (nRandom == 15) m_iSavedChaosID = ChaosSpawnNPC("npc_gman", MAKE_STRING("#hl2c_spawn_gman"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "gman", "_", CSF_SQUAD)->m_iChaosID;
	if (nRandom == 16) m_iSavedChaosID = ChaosSpawnNPC("npc_headcrab", MAKE_STRING("#hl2c_spawn_headcrab"), SPAWNTYPE_HIDEINCOVER, "_", "headcrab", "_")->m_iChaosID;
	if (nRandom == 17) m_iSavedChaosID = ChaosSpawnNPC("npc_headcrab_black", MAKE_STRING("#hl2c_spawn_headcrab_p"), SPAWNTYPE_HIDEINCOVER, "_", "headcrab_black", "_")->m_iChaosID;
	if (nRandom == 18) m_iSavedChaosID = ChaosSpawnNPC("npc_headcrab_fast", MAKE_STRING("#hl2c_spawn_headcrab_f"), SPAWNTYPE_HIDEINCOVER, "_", "headcrab_fast", "_")->m_iChaosID;
	if (nRandom == 19) m_iSavedChaosID = ChaosSpawnNPC("npc_helicopter", MAKE_STRING("#hl2c_spawn_heli"), SPAWNTYPE_BIGFLYER, "_", "helicopter", "_")->m_iChaosID;
	if (nRandom == 20) m_iSavedChaosID = ChaosSpawnNPC("npc_ichthyosaur", MAKE_STRING("#hl2c_spawn_ich"), SPAWNTYPE_EYELEVEL_SPECIAL, "_", "ichthyosaur", "_")->m_iChaosID;
	if (nRandom == 21) m_iSavedChaosID = ChaosSpawnNPC("npc_kleiner", MAKE_STRING("#hl2c_spawn_dr_k"), SPAWNTYPE_EYELEVEL_REGULAR, "models/kleiner.mdl", "kleiner", "_", CSF_SQUAD)->m_iChaosID;
	if (nRandom == 22) m_iSavedChaosID = ChaosSpawnNPC("npc_manhack", MAKE_STRING("#hl2c_spawn_manhack"), SPAWNTYPE_HIDEINCOVER, "_", "manhack", "_")->m_iChaosID;
	if (nRandom == 23) m_iSavedChaosID = ChaosSpawnNPC("npc_metropolice", MAKE_STRING("#hl2c_spawn_metrocop"), SPAWNTYPE_HIDEINCOVER, "_", "metropolice", "_")->m_iChaosID;
	if (nRandom == 24) m_iSavedChaosID = ChaosSpawnNPC("npc_monk", MAKE_STRING("#hl2c_spawn_grigori"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "monk", "weapon_annabelle", CSF_SQUAD)->m_iChaosID;
	if (nRandom == 25) m_iSavedChaosID = ChaosSpawnNPC("npc_mossman", MAKE_STRING("#hl2c_spawn_mossman"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "mossman", "_", CSF_SQUAD)->m_iChaosID;
	if (nRandom == 26) m_iSavedChaosID = ChaosSpawnNPC("npc_pigeon", MAKE_STRING("#hl2c_spawn_pigeon"), SPAWNTYPE_EYELEVEL_REGULAR, "models/pigeon.mdl", "pigeon", "_")->m_iChaosID;
	if (nRandom == 27) m_iSavedChaosID = ChaosSpawnNPC("npc_poisonzombie", MAKE_STRING("#hl2c_spawn_poison_z"), SPAWNTYPE_HIDEINCOVER, "_", "poisonzombie", "_")->m_iChaosID;
	if (nRandom == 28) m_iSavedChaosID = ChaosSpawnNPC("npc_rollermine", MAKE_STRING("#hl2c_spawn_rollermine"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "rollermine", "_")->m_iChaosID;
	if (nRandom == 29) m_iSavedChaosID = ChaosSpawnNPC("npc_seagull", MAKE_STRING("#hl2c_spawn_seagull"), SPAWNTYPE_EYELEVEL_REGULAR, "models/seagull.mdl", "seagull", "_")->m_iChaosID;
	if (nRandom == 30) m_iSavedChaosID = ChaosSpawnNPC("npc_sniper", MAKE_STRING("#hl2c_spawn_sniper"), SPAWNTYPE_EYELEVEL_SPECIAL, "_", "sniper", "_")->m_iChaosID;
	if (nRandom == 31) m_iSavedChaosID = ChaosSpawnNPC("npc_stalker", MAKE_STRING("#hl2c_spawn_stalker"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "stalker", "_")->m_iChaosID;
	if (nRandom == 32) m_iSavedChaosID = ChaosSpawnNPC("npc_strider", MAKE_STRING("#hl2c_spawn_strider"), SPAWNTYPE_STRIDER, "models/combine_strider.mdl", "strider", "_")->m_iChaosID;
	if (nRandom == 33) m_iSavedChaosID = ChaosSpawnNPC("npc_turret_ceiling", MAKE_STRING("#hl2c_spawn_turret_c"), SPAWNTYPE_CEILING, "_", "turret_ceiling", "_")->m_iChaosID;
	if (nRandom == 34) m_iSavedChaosID = ChaosSpawnNPC("npc_turret_floor", MAKE_STRING("#hl2c_spawn_turret_f"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "turret_floor", "_")->m_iChaosID;
	if (nRandom == 35) m_iSavedChaosID = ChaosSpawnNPC("npc_turret_ground", MAKE_STRING("#hl2c_spawn_turret_g"), SPAWNTYPE_ONGROUND, "_", "turret_ground", "_")->m_iChaosID;
	if (nRandom == 36)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_vortigaunt", MAKE_STRING("#hl2c_spawn_vortigaunt"), SPAWNTYPE_EYELEVEL_REGULAR, "models/vortigaunt.mdl", "vortigaunt", "_", CSF_SQUAD)->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 37) m_iSavedChaosID = ChaosSpawnNPC("npc_zombie", MAKE_STRING("#hl2c_spawn_zombie"), SPAWNTYPE_HIDEINCOVER, "_", "zombie", "_")->m_iChaosID;
	if (nRandom == 38) m_iSavedChaosID = ChaosSpawnNPC("npc_zombie_torso", MAKE_STRING("#hl2c_spawn_zombie_t"), SPAWNTYPE_HIDEINCOVER, "_", "zombie_torso", "_")->m_iChaosID;
	if (nRandom == 39)
	{
		m_iSavedChaosID = ChaosSpawnNPC("npc_fisherman", MAKE_STRING("#hl2c_spawn_fisherman"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "fisherman", "weapon_oldmanharpoon", CSF_SQUAD)->m_iChaosID;
		RandomizeReadiness(GetEntityWithID(m_iSavedChaosID));
	}
	if (nRandom == 40)
	{
		ChaosSpawnVehicle("prop_vehicle_apc", MAKE_STRING("#hl2c_spawn_apc_enemy"), SPAWNTYPE_VEHICLE, "models/combine_apc.mdl", "apc", "scripts/vehicles/apc_npc.txt");
		m_iSavedChaosID = ChaosSpawnNPC("npc_apcdriver", MAKE_STRING("#hl2c_spawn_apc_enemy"), SPAWNTYPE_EYELEVEL_SPECIAL, "_", "apcdriver", "_")->m_iChaosID;
		//make apc follow player

		sVariant.SetFloat(500);
		g_EventQueue.AddEvent("apcdriver", "SetDriversMaxSpeed", sVariant, 0, NULL, NULL, 0);
		sVariant.SetString(MAKE_STRING("OnUser1 !self:FireUser1::10:-1"));
		g_EventQueue.AddEvent("apcdriver", "AddOutput", sVariant, 0, NULL, NULL, 0);
		sVariant.SetString(MAKE_STRING("OnUser1 !self:GoToPathCorner:!player:0:-1"));
		g_EventQueue.AddEvent("apcdriver", "AddOutput", sVariant, 0, NULL, NULL, 0);
		g_EventQueue.AddEvent("apcdriver", "FireUser1", sVariant, 0, NULL, NULL, 0);
	}
	//ep1
	if (nRandom == 41) m_iSavedChaosID = ChaosSpawnNPC("npc_zombine", MAKE_STRING("#hl2c_spawn_zombine"), SPAWNTYPE_HIDEINCOVER, "_", "zombine", "_")->m_iChaosID;
	//ep2
	if (nRandom == 42) m_iSavedChaosID = ChaosSpawnNPC("npc_advisor", MAKE_STRING("#hl2c_spawn_advisor"), SPAWNTYPE_EYELEVEL_SPECIAL, "models/advisor.mdl", "advisor", "_")->m_iChaosID;
	if (nRandom == 43) m_iSavedChaosID = ChaosSpawnNPC("npc_antlion_grub", MAKE_STRING("Spawn Antliongrub"), SPAWNTYPE_ONGROUND, "_", "antlion_grub", "_")->m_iChaosID;
	if (nRandom == 44) m_iSavedChaosID = ChaosSpawnNPC("npc_fastzombie_torso", MAKE_STRING("#hl2c_spawn_fastzombie_t"), SPAWNTYPE_HIDEINCOVER, "_", "fastzombie_torso", "_")->m_iChaosID;
	if (nRandom == 45) m_iSavedChaosID = ChaosSpawnNPC("npc_hunter", MAKE_STRING("#hl2c_spawn_hunter"), SPAWNTYPE_EYELEVEL_REGULAR, "_", "hunter", "_")->m_iChaosID;
	if (nRandom == 46) m_iSavedChaosID = ChaosSpawnNPC("npc_magnusson", MAKE_STRING("#hl2c_spawn_magnusson"), SPAWNTYPE_EYELEVEL_REGULAR, "models/magnusson.mdl", "magnusson", "_", CSF_SQUAD)->m_iChaosID;
}
bool CERandomNPC::CheckStrike(const CTakeDamageInfo& info)
{
	return info.GetAttacker() == GetEntityWithID(m_iSavedChaosID);
}
void CERandomNPC::MaintainEffect()
{
	CBaseEntity* pNPC = GetEntityWithID(m_iSavedChaosID);
	if (!pNPC)//if NPC died
		return;
}
void CELockVehicles::DoOnVehicles(CPropVehicleDriveable* pVehicle)
{
	variant_t emptyVariant;
	if (m_flTimeRem < 1 || !m_bActive)
		pVehicle->AcceptInput("Unlock", pVehicle, pVehicle, emptyVariant, 0);
	else
		pVehicle->AcceptInput("Lock", pVehicle, pVehicle, emptyVariant, 0);
}
void CELockVehicles::StartEffect()
{
	IterUsableVehicles(false);
}
void CELockVehicles::StopEffect()
{
	IterUsableVehicles(false);
}
void CESuperMovement::StartEffect()
{
	sv_maxspeed.SetValue(4000);//320
	hl2_normspeed.SetValue(4000);//190
	hl2_sprintspeed.SetValue(4000);//320
	hl2_walkspeed.SetValue(4000);//150
	hl2_duckspeed.SetValue(4000);//64
	static ConVar* pCVcl_forwardspeed = (ConVar*)cvar->FindVar("cl_forwardspeed");
	pCVcl_forwardspeed->SetValue(4000);//450
	static ConVar* pCVcl_sidespeed = (ConVar*)cvar->FindVar("cl_sidespeed");
	pCVcl_sidespeed->SetValue(4000);//450
	static ConVar* pCVcl_upspeed = (ConVar*)cvar->FindVar("cl_upspeed");
	pCVcl_upspeed->SetValue(4000);//320
	static ConVar* pCVcl_backspeed = (ConVar*)cvar->FindVar("cl_backspeed");
	pCVcl_backspeed->SetValue(4000);//450
	UTIL_GetLocalPlayer()->SetMaxSpeed(HL2_NORM_SPEED);
}
void CESuperMovement::StopEffect()
{
	sv_maxspeed.SetValue(320);//
	hl2_normspeed.SetValue(190);//
	hl2_sprintspeed.SetValue(320);//
	hl2_walkspeed.SetValue(150);//
	hl2_duckspeed.SetValue(64);//
	static ConVar* pCVcl_forwardspeed = (ConVar*)cvar->FindVar("cl_forwardspeed");
	pCVcl_forwardspeed->SetValue(450);//
	static ConVar* pCVcl_sidespeed = (ConVar*)cvar->FindVar("cl_sidespeed");
	pCVcl_sidespeed->SetValue(450);//
	static ConVar* pCVcl_upspeed = (ConVar*)cvar->FindVar("cl_upspeed");
	pCVcl_upspeed->SetValue(320);//
	static ConVar* pCVcl_backspeed = (ConVar*)cvar->FindVar("cl_backspeed");
	pCVcl_backspeed->SetValue(450);//
	UTIL_GetLocalPlayer()->SetMaxSpeed(HL2_NORM_SPEED);
	if (g_ChaosEffects[EFFECT_NO_MOVEMENT]->m_bActive && g_ChaosEffects[EFFECT_NO_MOVEMENT]->m_flTimeRem > 1)
	{
		//EFFECT_NO_MOVEMENT is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_NO_MOVEMENT]->StartEffect();
	}
}
void CESolidTriggers::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	{
		CBaseEntity* pEnt = gEntList.FirstEnt();
		while (pEnt)
		{
			CBaseTrigger* pBaseTrigger = dynamic_cast<CBaseTrigger*>(pEnt);
			CTriggerVPhysicsMotion* pTriggerVPhysicsMotion = dynamic_cast<CTriggerVPhysicsMotion*>(pEnt);
			bool bIsTrigger = (pBaseTrigger != NULL) || (pTriggerVPhysicsMotion != NULL);
			if (bIsTrigger && ((pBaseTrigger != NULL) ? !pBaseTrigger->m_bDisabled : !pTriggerVPhysicsMotion->m_bDisabled))
			{
				pEnt->RemoveEffects(EF_NODRAW);
				if (pEnt->VPhysicsGetObject())
				{
					pEnt->VPhysicsGetObject()->EnableCollisions(true);
				}
				pEnt->RemoveSolidFlags(FSOLID_TRIGGER);
				pEnt->RemoveSolidFlags(FSOLID_NOT_SOLID);
				pEnt->PhysicsTouchTriggers();
			}
			pEnt = gEntList.NextEnt(pEnt);
		}
	}
	//if we're still stuck, then we're probably in some HUGE triggers. just make them unsolid again
	//500 is as far as i feel comfortable teleporting the player. HUGE triggers could make us teleport outside of entire buildings and such
	while (!pPlayer->GetUnstuck(500, UF_NO_NODE_TELEPORT))
	{
		trace_t	trace;
		Vector vecGoodSpot = pPlayer->GetAbsOrigin() + Vector(0, 0, 1);
		int iOldCollisionGroup = pPlayer->GetCollisionGroup();
		pPlayer->SetCollisionGroup(COLLISION_GROUP_PLAYER);
		pPlayer->GetIntersectingEntity(vecGoodSpot, false, trace);
		pPlayer->SetCollisionGroup(iOldCollisionGroup);

		//make sure we're really stuck in a trigger and not some other thing we should actually move out of
		//otherwise, we get into an infinite loop on d2_prison_01 because somehow we think we're stuck in the world!
		CBaseTrigger* pBaseTrigger = dynamic_cast<CBaseTrigger*>(trace.m_pEnt);
		CTriggerVPhysicsMotion* pTriggerVPhysicsMotion = dynamic_cast<CTriggerVPhysicsMotion*>(trace.m_pEnt);
		bool bIsTrigger = (pBaseTrigger != NULL) || (pTriggerVPhysicsMotion != NULL);
		if (bIsTrigger)
		{
			trace.m_pEnt->AddEffects(EF_NODRAW);
			if (trace.m_pEnt->VPhysicsGetObject())
			{
				trace.m_pEnt->VPhysicsGetObject()->EnableCollisions(true);
			}
			CTriggerVPhysicsMotion* pTriggerVPhysicsMotion = dynamic_cast<CTriggerVPhysicsMotion*>(trace.m_pEnt);
			if (!pTriggerVPhysicsMotion)//trigger_vphysics_motion doesn't use this according to a comment in CBaseVPhysicsTrigger::Spawn()
				trace.m_pEnt->AddSolidFlags(FSOLID_TRIGGER);
			trace.m_pEnt->AddSolidFlags(FSOLID_NOT_SOLID);
			trace.m_pEnt->PhysicsTouchTriggers();
		}
		else
			break;
	}
}
void CESolidTriggers::StopEffect()
{
	g_bEndSolidTriggers = true;
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		CBaseTrigger* pBaseTrigger = dynamic_cast<CBaseTrigger*>(pEnt);
		CTriggerVPhysicsMotion* pTriggerVPhysicsMotion = dynamic_cast<CTriggerVPhysicsMotion*>(pEnt);
		bool bBool = (pBaseTrigger != NULL) || (pTriggerVPhysicsMotion != NULL);
		if (bBool && ((pBaseTrigger != NULL) ? !pBaseTrigger->m_bDisabled : !pTriggerVPhysicsMotion->m_bDisabled))
		{
			pEnt->AddEffects(EF_NODRAW);
			if (pEnt->VPhysicsGetObject())
			{
				pEnt->VPhysicsGetObject()->EnableCollisions(true);
			}
			if (!pTriggerVPhysicsMotion)//trigger_vphysics_motion doesn't use this according to a comment in CBaseVPhysicsTrigger::Spawn()
				pEnt->AddSolidFlags(FSOLID_TRIGGER);//TODO: this may also fix the crash related to the use of g_bEndSolidTriggers
			pEnt->AddSolidFlags(FSOLID_NOT_SOLID);
			pEnt->PhysicsTouchTriggers();
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
	g_bEndSolidTriggers = false;
}
void CESolidTriggers::TransitionEffect()
{
	StartEffect();
}
void CECredits::StartEffect()
{
	//hack effect length so we can play song 2
	m_flDuration = 100;
	m_flTimeRem = 100;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	//visual
	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();
	UserMessageBegin(user, "CreditsMsg");
	WRITE_BYTE(3);
	MessageEnd();
	//audio
	CPASAttenuationFilter filter(pPlayer);
	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	ep.m_pSoundName = "*#music/hl2_song3.mp3";
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NORM;
	ep.m_nPitch = PITCH_NORM;
	pPlayer->EmitSound(filter, pPlayer->entindex(), ep);
}
void CECredits::MaintainEffect()
{
	if (m_flTimeRem > 13.5 || m_bPlayedSecondSong)//song 2 delayed by ? seconds
		return;
	m_bPlayedSecondSong = true;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CPASAttenuationFilter filter(pPlayer);
	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	ep.m_pSoundName = "*#music/hl1_song25_remix3.mp3";
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NORM;
	ep.m_nPitch = PITCH_NORM;
	pPlayer->EmitSound(filter, pPlayer->entindex(), ep);
}
void CECredits::RestoreEffect()
{
	//if we reload, the credits visual disappears and as far as i know we can't put it back to where it was at time of load, so we don't bother restoring it
	//playing song 2 would make no sense without the visual
	m_bPlayedSecondSong = true;
}
void CECredits::TransitionEffect()
{
	//if we reload, the credits visual disappears and as far as i know we can't put it back to where it was at time of load, so we don't bother restoring it
	//playing song 2 would make no sense without the visual
	m_bPlayedSecondSong = true;
}
void CESuperhot::FastThink()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer->pl.deadflag)
	{
		cvar->FindVar("host_timescale")->SetValue(1);
		return;
	}
	CBaseEntity* pVehicle = pPlayer->GetVehicleEntity();
	Vector vecVelocity;
	if (pVehicle && pVehicle->VPhysicsGetObject())
	{
		pVehicle->VPhysicsGetObject()->GetVelocity(&vecVelocity, NULL);
	}
	else
	{
		vecVelocity = pPlayer->GetAbsVelocity();
	}
	//If input locks up, change 0.08 to something higher
	float flNum = min(3, 2 * max(0.08, 1 / (hl2_normspeed.GetFloat() / max(hl2_normspeed.GetFloat() * 0.05, vecVelocity.Length()))));
	cvar->FindVar("host_timescale")->SetValue(flNum);
}
void CESupercold::FastThink()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer->pl.deadflag)
	{
		cvar->FindVar("host_timescale")->SetValue(1);
		return;
	}
	CBaseEntity* pVehicle = pPlayer->GetVehicleEntity();
	Vector vecVelocity;
	if (pVehicle && pVehicle->VPhysicsGetObject())
	{
		pVehicle->VPhysicsGetObject()->GetVelocity(&vecVelocity, NULL);
	}
	else
	{
		vecVelocity = pPlayer->GetAbsVelocity();
	}
	float flNum = max(0.3, (hl2_normspeed.GetFloat() / max(hl2_normspeed.GetFloat() * 0.3, vecVelocity.Length())));
	cvar->FindVar("host_timescale")->SetValue(flNum);
}
void CEPullToPlayer::MaintainEffect()
{
	CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "chaos_pull_to_player");
	if (pEnt)
	{
		CPhysExplosion* pExplo = static_cast<CPhysExplosion*>(pEnt);
		pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
	}
	else
	{
		StartEffect();
	}
}
void CEPushFromPlayer::MaintainEffect()
{
	CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, "chaos_push_from_player");
	if (pEnt)
	{
		CPhysExplosion* pExplo = static_cast<CPhysExplosion*>(pEnt);
		pExplo->m_damage = sv_gravity.GetFloat() * chaos_pushpull_strength.GetFloat();
	}
	else
	{
		StartEffect();
	}
}
void CEColors::TransitionEffect()
{
	StartEffect();
}
void CEColors::StartEffect()
{
	for (CBaseEntity* pEnt = gEntList.FirstEnt(); pEnt; pEnt = gEntList.NextEnt(pEnt))
	{
		if (pEnt->ClassMatches("env_fo*"))
		{
			//change fog!
			variant_t colorVariant;
			colorVariant.SetColor32(RandomInt(0, 255), RandomInt(0, 255), RandomInt(0, 255), pEnt->GetRenderColor().a);
			pEnt->AcceptInput("SetColor", pEnt, pEnt, colorVariant, 0);
			colorVariant.SetColor32(RandomInt(0, 255), RandomInt(0, 255), RandomInt(0, 255), pEnt->GetRenderColor().a);
			pEnt->AcceptInput("SetColorSecondary", pEnt, pEnt, colorVariant, 0);
		}
		ChangeEntity(pEnt);
	}
}
void CEColors::ChangeEntity(CBaseEntity* pEntity)
{
	char szcolor[2048];
	variant_t colorVariant;
	int r = RandomInt(0, 255);
	int g = RandomInt(0, 255);
	int b = RandomInt(0, 255);
	Q_snprintf(szcolor, sizeof(szcolor), "%i %i %i", r, g, b);
	Msg("%s\n", szcolor);
	colorVariant.SetString(MAKE_STRING(szcolor));
	pEntity->AcceptInput("Color", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), colorVariant, 0);
}
void CEColors::OnEntitySpawned(CBaseEntity* pEntity)
{
	ChangeEntity(pEntity);
}
//is the given character one that the game will force you to keep alive
bool IsPlayerAlly(CBaseCombatCharacter* pCharacter)
{
	if (pCharacter->m_bChaosSpawned)
		return false;

	if (pCharacter->IsNPC())
	{
		if (pCharacter->GetMaxHealth() < 10)//talkers (max health of 8 for all of them apparently?)
			return true;
		if (pCharacter->MyNPCPointer()->IsPlayerAlly())//fighters
		{
			//damage filter pertains to some invulnerable citizens in some maps
			if (pCharacter->m_hDamageFilter)
				return true;
			if (pCharacter->ClassMatches("npc_a*") || pCharacter->ClassMatches("npc_b*") || pCharacter->ClassMatches("npc_mon*") || pCharacter->ClassMatches("npc_v*"))
				return true;
		}
	}
	return false;
}
void CENPCRels::DoNPCRels(int disposition, bool bRevert)
{
	const int MAX_HANDLED = 512;
	CUtlVectorFixed<CBaseCombatCharacter*, MAX_HANDLED> subjectList;
	CUtlVectorFixed<CBaseCombatCharacter*, MAX_HANDLED> targetList;
	//Search players first
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			subjectList.AddToTail(pPlayer);
			targetList.AddToTail(pPlayer);
		}
	}
	for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
	{
		CAI_BaseNPC* pNPC = (g_AI_Manager.AccessAIs())[i];
		if (pNPC)
		{
			subjectList.AddToTail(pNPC);
			targetList.AddToTail(pNPC);
		}
	}
	if (subjectList.Count() == 0)
	{
		Warning("Found no subjects for chaos effect!?\n");
		return;
	}
	else if (targetList.Count() == 0)
	{
		Warning("Found no targets for chaos effect!?\n");
		return;
	}
	for (int i = 0; i < subjectList.Count(); i++)
	{
		CBaseCombatCharacter* pSubject = subjectList[i];
		for (int j = 0; j < targetList.Count(); j++)
		{
			CBaseCombatCharacter* pTarget = targetList[j];
			if (pSubject == pTarget)
				continue;

			if (disposition == D_HT || disposition == D_FR)
			{
				//don't make vital allies like alyx and barney hostile to player
				//you can't just kill them!
				bool bTargetIsPlayer = pTarget->IsPlayer();
				bool bSubjectIsAlly = IsPlayerAlly(pSubject);
				if (bTargetIsPlayer && bSubjectIsAlly)
					continue;

				//check other way too so player can't kill them by friendly fire cause that's just dumb
				bool bSubjectIsPlayer = pSubject->IsPlayer();
				bool bTargetIsAlly = IsPlayerAlly(pTarget);
				if (bSubjectIsPlayer && bTargetIsAlly)
					continue;

				//no fighting amongst themselves either
				if (bSubjectIsAlly && bTargetIsAlly)
					continue;

				//don't try to attack a crane driver, you're not winning
				if (pTarget->ClassMatches("npc_cra*"))
					pSubject->AddEntityRelationship(pTarget, D_NU, 100);
			}

			if (bRevert)
			{
				pSubject->RemoveEntityRelationship(pTarget);
			}
			else
			{
				if (pSubject->IRelationType(pTarget) != disposition || pSubject->IRelationPriority(pTarget) != 100)
				{
					pSubject->AddEntityRelationship(pTarget, (Disposition_t)disposition, 100);
					//Msg("Applying relationship to %s and %s\n", STRING(pSubject->GetEntityName()), STRING(pTarget->GetEntityName()));
				}
			}
		}
		if (disposition == D_FR)
		{
			variant_t emptyVariant;
			pSubject->AcceptInput("SetReadinessPanic", pSubject, pSubject, emptyVariant, 0);
		}
	}
}
void CENPCRels::StopEffect()
{
	DoNPCRels(D_ER, true);
	if (m_nID == EFFECT_NPC_LIKE)
		ai_block_damage.SetValue(false);
}
void CENPCRels::MaintainEffect()
{
	StartEffect();
}
void CENPCRels::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_NPC_HATE:
		DoNPCRels(D_HT, false);
		break;
	case EFFECT_NPC_LIKE:
		DoNPCRels(D_LI, false);
		ai_block_damage.SetValue(true);
		break;
	case EFFECT_NPC_NEUTRAL:
		DoNPCRels(D_NU, false);
		break;
	case EFFECT_NPC_FEAR:
		DoNPCRels(D_FR, false);
		break;
	}
}
static const char* zombieNPCs[] = { "npc_zombie", "npc_zombie_torso", "npc_poisonzombie", "npc_fastzombie", "npc_zombine", "npc_fastzombie_torso" };
void CEZombieSpam::MaintainEffect()
{
	if (bNewWay)
	{
		//new way
		m_strHudName = MAKE_STRING("#hl2c_zombie_spam_new");

		int iZombieType, nRandMax;

		nRandMax = 5;

		while (true)
		{
			//find a random hidden node
			CAI_Node* pNode = g_pBigAINet->GetNode(RandomInt(0, g_pBigAINet->NumNodes() - 1));
			trace_t	tr;
			UTIL_TraceLine(UTIL_GetLocalPlayer()->EyePosition(), pNode->GetPosition(HULL_HUMAN), MASK_VISIBLE, UTIL_GetLocalPlayer(), COLLISION_GROUP_NONE, &tr);
			if (tr.DidHit())//can't see
			{
				if (chaos_rng1.GetInt() == -1)
					iZombieType = RandomInt(0, nRandMax);
				else
					iZombieType = chaos_rng1.GetInt() % nRandMax;

				CBaseEntity* pNPC = (CBaseEntity*)CreateEntityByName(zombieNPCs[iZombieType]);

				if (!pNPC)
					return;

				pNPC->SetAbsOrigin(pNode->GetOrigin());
				pNPC->KeyValue("targetname", "l4d_zombie");
				g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
				DispatchSpawn(pNPC);
				pNPC->Activate();
				pNPC->m_bChaosSpawned = true;
				pNPC->m_bChaosPersist = true;
				return;
			}
		}
	}
}
void CEZombieSpam::StartEffect()
{
	bNewWay = (chaos_rng1.GetInt() == -1 ? RandomInt(0, 1) : chaos_rng1.GetInt()) == 1;
	if (bNewWay)
		return;

	//old way
	m_strHudName = MAKE_STRING("#hl2c_zombie_spam");

	int iZombieType, nRandMax;

	nRandMax = 5;//ep2

	CAI_Node* pNode;
	CNodeList* result = GetNearbyNodes(20);
	for (; result->Count(); result->RemoveAtHead())
	{
		pNode = g_pBigAINet->GetNode(result->ElementAtHead().nodeIndex);

		int rng;
		if (chaos_rng1.GetInt() == -1)
			rng = chaos_rng1.GetInt();
		else
			rng = RandomInt(0, nRandMax);

		iZombieType = (rng + result->ElementAtHead().nodeIndex) % (nRandMax + 1);
		if (iZombieType < 0 || iZombieType > nRandMax)//just incase?
			return;

		CBaseEntity* pNPC = (CBaseEntity*)CreateEntityByName(zombieNPCs[iZombieType]);

		if (!pNPC)
			return;

		pNPC->SetAbsOrigin(pNode->GetOrigin());
		pNPC->KeyValue("targetname", "l4d_zombie");
		g_iChaosSpawnCount++; pNPC->KeyValue("chaosid", g_iChaosSpawnCount);
		DispatchSpawn(pNPC);
		pNPC->Activate();
		pNPC->m_bChaosSpawned = true;
		pNPC->m_bChaosPersist = true;
	}
}
void CEBottle::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	QAngle vecAngles = QAngle(0, 0, 0);
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();
	CBaseAnimating* pEnt = (CBaseAnimating*)CreateEntityByName("prop_physics_override");
	pEnt->SetModel("models/props_junk/garbage_glassbottle003a.mdl");
	Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 128;
	trace_t	trDown;
	UTIL_TraceLine(vecOrigin, vecOrigin - Vector(0, 0, 1000), MASK_SOLID, pEnt, COLLISION_GROUP_DEBRIS, &trDown);
	vecOrigin = trDown.endpos + Vector(0, 0, 1);
	Vector vecLastGoodPos = vecOrigin;
	pEnt->SetAbsOrigin(vecOrigin);
	trace_t	trace;
	int i = 0;
	//try to make bottle as big as possible with whatever space is in front of player
	do {
		//NDebugOverlay::Cross3D(pEnt->GetAbsOrigin(), 16, 0, 255, 0, true, 30);
		pEnt->SetModelScale(i + 1);
		vecLastGoodPos = pEnt->GetAbsOrigin();
		//only go up to 20 units away from previous position, instead of 500, which risks the beer spawning in some other random place, which is bad, i guess
		i++;
	} while (pEnt->GetUnstuck(20, UF_NO_NODE_TELEPORT) && i < chaos_beer_size_limit.GetInt());//yes this i limit actually matters, or else we will create a beer so big it hits a max coord related assert and brings the whole game to a screeching halt. what the fuck.
	if (i > 1)
		pEnt->SetModelScale(i - 1);
	pEnt->SetMaxHealth(100 * i);
	pEnt->SetHealth(100 * i);
	DispatchSpawn(pEnt);
	pEnt->Activate();
	vecOrigin = vecLastGoodPos;
	pEnt->Teleport(&vecOrigin, &vecAngles, NULL);
	IPhysicsObject* pPhys = pEnt->VPhysicsGetObject();
	pPhys->EnableDrag(false);
	pPhys->SetMass(i * 10);
	if (i > 40)
		m_strHudName = MAKE_STRING("#hl2c_beer_bottle_huge");
}
void CEEvilNPC::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_EVIL_ALYX:
		m_iSavedChaosID = ChaosSpawnNPC("npc_alyx", MAKE_STRING("#hl2c_evil_alyx"), SPAWNTYPE_HIDEINCOVER, "models/alyx.mdl", "alyx", "weapon_alyxgun", CSF_EVIL)->m_iChaosID;
		break;
	case EFFECT_EVIL_NORIKO:
		EvilNoriko();
		break;
	case EFFECT_EVIL_BARNEY:
		m_iSavedChaosID = ChaosSpawnNPC("npc_barney", MAKE_STRING("#hl2c_evil_barney"), SPAWNTYPE_HIDEINCOVER, "_", "barney", "weapon_ar2", CSF_EVIL)->m_iChaosID;
		break;
	case EFFECT_EVIL_KLEINER:
		m_iSavedChaosID = ChaosSpawnNPC("npc_kleiner", MAKE_STRING("#hl2c_evil_dr_k"), SPAWNTYPE_HIDEINCOVER, "models/kleiner.mdl", "kleiner", "weapon_shotgun", CSF_EVIL)->m_iChaosID;
		break;
	case EFFECT_EVIL_GRIGORI:
		m_iSavedChaosID = ChaosSpawnNPC("npc_monk", MAKE_STRING("#hl2c_evil_grig"), SPAWNTYPE_HIDEINCOVER, "_", "monk", "weapon_annabelle", CSF_EVIL)->m_iChaosID;
		break;
	case EFFECT_EVIL_MOSSMAN:
		m_iSavedChaosID = ChaosSpawnNPC("npc_mossman", MAKE_STRING("#hl2c_evil_mossman"), SPAWNTYPE_HIDEINCOVER, "_", "mossman", "weapon_crowbar", CSF_EVIL)->m_iChaosID;
		break;
	case EFFECT_EVIL_VORT:
		m_iSavedChaosID = ChaosSpawnNPC("npc_vortigaunt", MAKE_STRING("#hl2c_evil_vort"), SPAWNTYPE_HIDEINCOVER, "models/vortigaunt.mdl", "vortigaunt", "_", CSF_EVIL)->m_iChaosID;
		break;
	case EFFECT_EVIL_BREEN:
		m_iSavedChaosID = ChaosSpawnNPC("npc_breen", MAKE_STRING("#hl2c_evil_breen"), SPAWNTYPE_HIDEINCOVER, "models/breen.mdl", "breen", "weapon_physcannon", CSF_EVIL)->m_iChaosID;
		break;
		/*
	case EFFECT_EVIL_ELI:
		m_iSavedChaosID = ChaosSpawnNPC("npc_eli", MAKE_STRING("Evil Eli"), SPAWNTYPE_HIDEINCOVER, "models/eli.mdl", "eli", "weapon_crossbow", true)->m_iChaosID;
		break;*/
	}
}
bool CEEvilNPC::CheckStrike(const CTakeDamageInfo& info)
{
	return info.GetAttacker() == GetEntityWithID(m_iSavedChaosID);
}
void CEEvilNPC::EvilNoriko()
{
	variant_t sVariant;
	sVariant.SetString(MAKE_STRING("d1_t02_Plaza_Sit02"));
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	g_iChaosSpawnCount++;
	char szName[2048];

	//crane magnet
	Vector vecOrigin = pHL2Player->RotatedOffset(Vector(0, 164, 750), true);
	QAngle vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
	CBaseEntity* pMagnet = CreateEntityByName("phys_magnet");
	pMagnet->KeyValue("model", "models/props_wasteland/cranemagnet01a.mdl");
	pMagnet->KeyValue("massScale", "1000");
	Q_snprintf(szName, sizeof(szName), "crane_magnet_%i", g_iChaosSpawnCount);
	pMagnet->KeyValue("targetname", szName);
	pMagnet->KeyValue("overridescript", "damping,0.5,rotdamping,0.2,inertia,0.3");
	pMagnet->m_bChaosSpawned = true;
	DispatchSpawn(pMagnet);
	pMagnet->Activate();
	pMagnet->Teleport(&vecOrigin, &vecAngles, NULL);
	pMagnet->GetUnstuck(500, UF_NO_NODE_TELEPORT);

	//crane
	vecOrigin = pHL2Player->RotatedOffset(Vector(634, 0, 64), true);
	vecAngles = QAngle(0, pPlayer->GetAbsAngles().y + 90, 0);
	CBaseEntity* pVehicle = CreateEntityByName("prop_vehicle_crane");
	pVehicle->KeyValue("model", "models/Cranes/crane_docks.mdl");
	pVehicle->KeyValue("solid", "6");
	pVehicle->KeyValue("magnetname", szName);
	Q_snprintf(szName, sizeof(szName), "crane%i", g_iChaosSpawnCount);
	pVehicle->KeyValue("targetname", szName);
	pVehicle->KeyValue("vehiclescript", "scripts/vehicles/crane.txt");
	pVehicle->m_bChaosSpawned = true;
	pVehicle->Teleport(&vecOrigin, &vecAngles, NULL);

	//noriko
	vecOrigin = pHL2Player->RotatedOffset(Vector(480, 85, 135), true);
	vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 180, 0);
	CBaseEntity* pNoriko = CreateEntityByName("cycler");
	pNoriko->KeyValue("model", "models/Humans/Group02/Female_04.mdl");
	pNoriko->KeyValue("targetname", "chaos_crane_driver");
	pNoriko->m_bChaosSpawned = true;
	DispatchSpawn(pNoriko);
	pNoriko->Activate();
	pNoriko->Teleport(&vecOrigin, &vecAngles, NULL);
	pNoriko->AcceptInput("SetSequence", pNoriko, pNoriko, sVariant, 0);

	//hello dr. freeman
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "play vo/noriko_hello.wav");

	//driver
	vecOrigin = pHL2Player->RotatedOffset(Vector(534, 64, 128), true);
	vecAngles = QAngle(0, pPlayer->GetAbsAngles().y - 90, 0);
	CAI_BaseNPC* pDriver = (CAI_BaseNPC*)CreateEntityByName("npc_cranedriver");
	pDriver->KeyValue("vehicle", szName);
	pDriver->KeyValue("releasepause", "0");
	pDriver->m_bChaosSpawned = true;
	pDriver->m_bEvil = true;
	DispatchSpawn(pDriver);
	pDriver->Activate();
	pDriver->Teleport(&vecOrigin, &vecAngles, NULL);

	//activate crane last so everything works correctly
	DispatchSpawn(pVehicle);
	pVehicle->Activate();
	pNoriko->SetParent(pVehicle);
	m_iSavedChaosID = g_iChaosSpawnCount;
}
void CERandomSong::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	const char* sSongName = "*#music/hl1_song10.mp3";
	CPASAttenuationFilter filter(pPlayer);
	EmitSound_t ep;
	ep.m_nChannel = CHAN_STATIC;
	ep.m_flVolume = 1;
	ep.m_SoundLevel = SNDLVL_NONE;
	ep.m_nPitch = PITCH_NORM;
	int nRandom;
	char modDir[MAX_PATH];
	if (UTIL_GetModDir(modDir, sizeof(modDir)) == false)
		return;
	if (!Q_strcmp(modDir, "ep2chaos"))
		nRandom = RandomInt(0, 71);
	else if (!Q_strcmp(modDir, "ep1chaos"))
		nRandom = RandomInt(0, 58);
	else
		nRandom = RandomInt(0, 47);
	Msg("Song number %i\n", nRandom);
	if (nRandom == 0) sSongName = "*#music/hl1_song10.mp3";
	else if (nRandom == 1) sSongName = "*#music/hl1_song11.mp3";
	else if (nRandom == 2) sSongName = "*#music/hl1_song14.mp3";
	else if (nRandom == 3) sSongName = "*#music/hl1_song15.mp3";
	else if (nRandom == 4) sSongName = "*#music/hl1_song17.mp3";
	else if (nRandom == 5) sSongName = "*#music/hl1_song19.mp3";
	else if (nRandom == 6) sSongName = "*#music/hl1_song20.mp3";
	else if (nRandom == 7) sSongName = "*#music/hl1_song21.mp3";
	else if (nRandom == 8) sSongName = "*#music/hl1_song24.mp3";
	else if (nRandom == 9) sSongName = "*#music/hl1_song25_remix3.mp3";
	else if (nRandom == 10) sSongName = "*#music/hl1_song26.mp3";
	else if (nRandom == 11) sSongName = "*#music/hl1_song3.mp3";
	else if (nRandom == 12) sSongName = "*#music/hl1_song5.mp3";
	else if (nRandom == 13) sSongName = "*#music/hl1_song6.mp3";
	else if (nRandom == 14) sSongName = "*#music/hl1_song9.mp3";
	else if (nRandom == 15) sSongName = "*#music/hl2_ambient_1.wav";
	else if (nRandom == 16) sSongName = "*#music/hl2_intro.mp3";
	else if (nRandom == 17) sSongName = "*#music/hl2_song0.mp3";
	else if (nRandom == 18) sSongName = "*#music/hl2_song1.mp3";
	else if (nRandom == 19) sSongName = "*#music/hl2_song2.mp3";
	else if (nRandom == 20) sSongName = "*#music/hl2_song3.mp3";
	else if (nRandom == 21) sSongName = "*#music/hl2_song4.mp3";
	else if (nRandom == 22) sSongName = "*#music/hl2_song6.mp3";
	else if (nRandom == 23) sSongName = "*#music/hl2_song7.mp3";
	else if (nRandom == 24) sSongName = "*#music/hl2_song8.mp3";
	else if (nRandom == 25) sSongName = "*#music/hl2_song10.mp3";
	else if (nRandom == 26) sSongName = "*#music/hl2_song11.mp3";
	else if (nRandom == 27) sSongName = "*#music/hl2_song12_long.mp3";
	else if (nRandom == 28) sSongName = "*#music/hl2_song13.mp3";
	else if (nRandom == 29) sSongName = "*#music/hl2_song14.mp3";
	else if (nRandom == 30) sSongName = "*#music/hl2_song15.mp3";
	else if (nRandom == 31) sSongName = "*#music/hl2_song16.mp3";
	else if (nRandom == 32) sSongName = "*#music/hl2_song17.mp3";
	else if (nRandom == 33) sSongName = "*#music/hl2_song19.mp3";
	else if (nRandom == 34) sSongName = "*#music/hl2_song20_submix0.mp3";
	else if (nRandom == 35) sSongName = "*#music/hl2_song20_submix4.mp3";
	else if (nRandom == 36) sSongName = "*#music/hl2_song23_suitsong3.mp3";
	else if (nRandom == 37) sSongName = "*#music/hl2_song26.mp3";
	else if (nRandom == 38) sSongName = "*#music/hl2_song26_trainstation1.mp3";
	else if (nRandom == 39) sSongName = "*#music/hl2_song27_trainstation2.mp3";
	else if (nRandom == 40) sSongName = "*#music/hl2_song28.mp3";
	else if (nRandom == 41) sSongName = "*#music/hl2_song29.mp3";
	else if (nRandom == 42) sSongName = "*#music/hl2_song30.mp3";
	else if (nRandom == 43) sSongName = "*#music/hl2_song31.mp3";
	else if (nRandom == 44) sSongName = "*#music/hl2_song32.mp3";
	else if (nRandom == 45) sSongName = "*#music/hl2_song33.mp3";
	else if (nRandom == 46) sSongName = "*#music/radio1.mp3";
	else if (nRandom == 47) sSongName = "*#music/ravenholm_1.mp3";
	//ep1
	else if (nRandom == 48) sSongName = "*#music/vlvx_song1.mp3";
	else if (nRandom == 49) sSongName = "*#music/vlvx_song2.mp3";
	else if (nRandom == 50) sSongName = "*#music/vlvx_song4.mp3";
	else if (nRandom == 51) sSongName = "*#music/vlvx_song8.mp3";
	else if (nRandom == 52) sSongName = "*#music/vlvx_song11.mp3";
	else if (nRandom == 53) sSongName = "*#music/vlvx_song12.mp3";
	else if (nRandom == 54) sSongName = "*#music/vlvx_song18.mp3";
	else if (nRandom == 55) sSongName = "*#music/vlvx_song19a.mp3";
	else if (nRandom == 56) sSongName = "*#music/vlvx_song19b.mp3";
	else if (nRandom == 57) sSongName = "*#music/vlvx_song20.mp3";
	else if (nRandom == 58) sSongName = "*#music/vlvx_song21.mp3";
	//ep2
	else if (nRandom == 59) sSongName = "*#music/vlvx_song0.mp3";
	else if (nRandom == 60) sSongName = "*#music/vlvx_song3.mp3";
	else if (nRandom == 61) sSongName = "*#music/vlvx_song9.mp3";
	else if (nRandom == 62) sSongName = "*#music/vlvx_song15.mp3";
	else if (nRandom == 63) sSongName = "*#music/vlvx_song20.mp3";
	else if (nRandom == 64) sSongName = "*#music/vlvx_song22.mp3";
	else if (nRandom == 65) sSongName = "*#music/vlvx_song23.mp3";
	else if (nRandom == 66) sSongName = "*#music/vlvx_song23ambient.mp3";
	else if (nRandom == 67) sSongName = "*#music/vlvx_song24.mp3";
	else if (nRandom == 68) sSongName = "*#music/vlvx_song25.mp3";
	else if (nRandom == 69) sSongName = "*#music/vlvx_song26.mp3";
	else if (nRandom == 70) sSongName = "*#music/vlvx_song27.mp3";
	else if (nRandom == 71) sSongName = "*#music/vlvx_song28.mp3";
	ep.m_pSoundName = sSongName;
	pPlayer->PrecacheSound(sSongName);//because precaching every single song on spawn is not the winning move
	pPlayer->EmitSound(filter, pPlayer->entindex(), ep);
}
void CETreeSpam::StartEffect()
{
	CAI_Node* pNode;
	CNodeList* result = GetNearbyNodes(80);
	Msg("list has %i nodes\n", result->Count());
	CUtlVector<Vector> vecTreeSpots;//positions of trees we've placed, or other places we'd like to avoid
	vecTreeSpots.AddToTail(UTIL_GetLocalPlayer()->GetAbsOrigin());
	//track doors because often nodes are placed around doorways, and trees often block necessary doorways
	CBaseEntity* pDoor = gEntList.FindEntityByClassname(NULL, "prop_door_rotating");
	while (pDoor)
	{
		vecTreeSpots.AddToTail(pDoor->GetAbsOrigin());
		pDoor = gEntList.FindEntityByClassname(pDoor, "prop_door_rotating");
	}

	for (; result->Count(); result->RemoveAtHead())
	{
		pNode = g_pBigAINet->GetNode(result->ElementAtHead().nodeIndex);
		Msg("node %i\n", pNode->GetId());
		CBaseEntity* pEnt = CreateEntityByName("prop_dynamic");
		trace_t tr;
		Vector vecNodePos = pNode->GetOrigin();
		UTIL_TraceLine(vecNodePos + Vector(0, 0, 16), vecNodePos - Vector(0, 0, 100), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);
		if (tr.m_pEnt && (tr.m_pEnt->GetMoveType() == MOVETYPE_VPHYSICS || tr.m_pEnt->IsNPC()))
		{
			Msg("Tree (node %i) on bad ground\n", pNode->GetId());
			continue;
		}
		bool bDone = false;
		for (int i = 0; vecTreeSpots.Size() >= i + 1; i++)
		{
			if ((vecTreeSpots[i] - tr.endpos).Length2D() < 90)
			{
				bDone = true;
				break;
			}
		}
		if (bDone)
			continue;
		pEnt->SetAbsOrigin(tr.endpos);
		vecTreeSpots.AddToTail(pEnt->GetAbsOrigin());
		pEnt->KeyValue("model", "models/props_foliage/tree_pine04.mdl");
		pEnt->KeyValue("disableshadows", "1");//shadows may cause a surprising amount of lag
		pEnt->KeyValue("solid", "6");
		g_iChaosSpawnCount++; pEnt->KeyValue("chaosid", g_iChaosSpawnCount);
		pEnt->m_bChaosPersist = true;
		pEnt->m_bChaosSpawned = true;
		DispatchSpawn(pEnt);
		pEnt->Activate();
	}
	UTIL_GetLocalPlayer()->GetUnstuck(500, UF_NO_NODE_TELEPORT);//despite earlier check, we can still end up stuck in a tree if there are multiple nodes very closeby
}
void CETreeSpam::StopEffect()
{
	CBaseEntity* pEnt = gEntList.FindEntityByClassname(NULL, "prop_dynamic");
	while (pEnt)
	{
		if (pEnt->m_bChaosPersist && !Q_strcmp(STRING(pEnt->GetModelName()), "models/props_foliage/tree_pine04.mdl"))
			pEnt->Remove();
		pEnt = gEntList.FindEntityByClassname(pEnt, "prop_dynamic");
	}
}
void CEMountedGun::StartEffect()
{
	CBaseEntity* pTank;
	//float flDistAway = 128;
	//float flExtraHeight = 32;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	vecForward.z = 0;
	vecForward.NormalizeInPlace();
	Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 128 + Vector(0, 0, 16);
	trace_t tr;
	UTIL_TraceLine(UTIL_GetLocalPlayer()->EyePosition(), vecOrigin, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
	Vector vecTargetOrigin = tr.endpos - vecForward * 40;
	QAngle vecAngles(0, pPlayer->GetAbsAngles().y, 0);
	//pTank->SetAbsOrigin(vecOrigin);
	//pTank->SetAbsAngles(vecAngles);
	vecOrigin = tr.endpos - vecForward * 5 + Vector(0, 0, 32);//move back out of walls so we're guaranteed to be usable
	CBaseEntity* pProp = CreateEntityByName("prop_dynamic");
	if (pProp)
	{
		pProp->KeyValue("targetname", "gunmodel");
		pProp->KeyValue("DefaultAnim", "idle_inactive");
		pProp->KeyValue("parentname", "tank");
	}
	CBaseEntity* pTarget = CreateEntityByName("info_target");
	if (pTarget)
	{
		pTarget->KeyValue("targetname", "tank_npc_spot");
		pTarget->KeyValue("targetname", "gunmodel");
		DispatchSpawn(pTarget);
		pTarget->Activate();
		pTarget->Teleport(&vecTargetOrigin, &vecAngles, NULL);
	}
	int nRandom;
	nRandom = chaos_rng1.GetInt() == -1 ? RandomInt(0, 4) : chaos_rng1.GetInt();
	if (nRandom == 0)
	{
		pTank = CreateEntityByName("func_tank");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 15);
			pTank->KeyValue("bullet_damage", 3);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("bullet", 3);
			pTank->KeyValue("effecthandling", 1);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("ammotype", "AR2");
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", 8);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pProp->KeyValue("rendercolor", "255 255 255");
			pProp->KeyValue("model", "models/props_combine/bunker_gun01.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	if (nRandom == 1)
	{
		pTank = CreateEntityByName("func_tankairboatgun");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 66);
			pTank->KeyValue("bullet_damage", 250);
			pTank->KeyValue("bullet_damage_vs_player", 25);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", "0");
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pProp->KeyValue("rendercolor", "255 255 255");
			pProp->KeyValue("model", "models/Airboatgun.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	if (nRandom == 2)
	{
		pTank = CreateEntityByName("func_tankmortar");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 0.22);
			pTank->KeyValue("bullet_damage", 0.0);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", 8);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pTank->KeyValue("spriteflash", "materials/Sprites/redglow1.vmt");
			pTank->KeyValue("firedelay", 1.5);
			pTank->KeyValue("firestartsound", "Weapon_Mortar.Single");
			pTank->KeyValue("incomingsound", "Weapon_Mortar.Incomming");
			pTank->KeyValue("warningtime", 1);
			pTank->KeyValue("firevariance", 1);
			pProp->KeyValue("rendercolor", "255 255 0");
			pProp->KeyValue("model", "models/props_combine/bunker_gun01.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	if (nRandom == 3)
	{
		pTank = CreateEntityByName("func_tankrocket");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 1.5);
			pTank->KeyValue("bullet_damage", 100);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("bullet", 3);
			pTank->KeyValue("effecthandling", 0.0);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("ammotype", "AR2");
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 31);
			pTank->KeyValue("barrelz", 8);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pTank->KeyValue("rocketspeed", "1500");
			pProp->KeyValue("rendercolor", "255 0 0");
			pProp->KeyValue("model", "models/props_combine/bunker_gun01.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
	//ep2
	if (nRandom == 4)
	{
		pTank = CreateEntityByName("func_tank_combine_cannon");
		if (pTank)
		{
			pTank->AddSpawnFlags(32 + 1024 + 32768);
			pTank->KeyValue("firerate", 15);
			pTank->KeyValue("bullet_damage", 50);
			pTank->KeyValue("bullet_damage_vs_player", 25);
			pTank->KeyValue("pitchrange", 90);
			pTank->KeyValue("pitchrate", 300);
			pTank->KeyValue("yawrange", 180);
			pTank->KeyValue("yawrate", 300);
			pTank->KeyValue("bullet", 3);
			pTank->KeyValue("effecthandling", 2);
			pTank->KeyValue("ammo_count", -1);
			pTank->KeyValue("ammotype", "CombineHeavyCannon");
			pTank->KeyValue("gun_base_attach", "aimrotation");
			pTank->KeyValue("gun_barrel_attach", "muzzle");
			pTank->KeyValue("gun_yaw_pose_param", "aim_yaw");
			pTank->KeyValue("gun_pitch_pose_param", "aim_pitch");
			pTank->KeyValue("gun_pitch_pose_center", 7.5);
			pTank->KeyValue("gun_yaw_pose_center", 0.0);
			pTank->KeyValue("barrel", 20);
			pTank->KeyValue("barrelz", 13);
			pTank->KeyValue("firespread", 1);
			pTank->KeyValue("targetname", "tank");
			pTank->KeyValue("npc_man_point", "tank_npc_spot");
			pTank->KeyValue("manual", true);
			pProp->KeyValue("rendercolor", "255 255 255");
			pProp->KeyValue("model", "models/combine_turrets/combine_cannon_gun.mdl");
			DispatchSpawn(pTank);
			pTank->Activate();
			pTank->Teleport(&vecOrigin, &vecAngles, NULL);
			DispatchSpawn(pProp);
			pProp->Activate();
			pProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pProp->SetParent(pTank);
		}
	}
}
void CERestartLevel::StartEffect()
{
	g_bGoBackLevel = true;
	g_bAvoidExtreme = true;
	if (completed_town03.GetBool() && !strcmp(gpGlobals->mapname.ToCStr(), "d1_town_02"))
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "map d1_town_02 setpos -3648 0 -3419 setang 0 90 0");
	else if (completed_coast08.GetBool() && !strcmp(gpGlobals->mapname.ToCStr(), "d2_coast_07"))
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "map d2_coast_07 setpos 3008 5184 1572 setang 0 180 0");
	else
	{
		const char* pMapName = STRING(gpGlobals->mapname);
		engine->ClientCommand(engine->PEntityOfEntIndex(1), "changelevel %s", pMapName);
	}
}
void CERemovePickups::StartEffect()
{
	CBaseEntity* pPickup = gEntList.FindEntityByClassname(NULL, "it*");
	while (pPickup)
	{
		pPickup->Remove();
		pPickup = gEntList.FindEntityByClassname(pPickup, "it*");
	}
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)gEntList.FindEntityByClassname(NULL, "we*");
	while (pWeapon)
	{
		if (!pWeapon->GetOwner())
			pWeapon->Remove();
		pWeapon = (CBaseCombatWeapon*)gEntList.FindEntityByClassname(pWeapon, "we*");
	}
}
void CECloneNPCs::StartEffect()
{
	//find NPCs to clone
	CUtlVector<CChaosStoredEnt> vNPCs;
	CBaseEntity* pNPC = gEntList.FindEntityByClassname(NULL, "n*");
	while (pNPC)
	{
		//avoid cloning non-NPC entities with an "npc_" prefix, like npc_maker
		//avoid npc_furniture, it goes bad
		if (pNPC->IsNPC() && !pNPC->ClassMatches("npc_furniture"))
			vNPCs.AddToTail(*StoreEnt(pNPC));
		pNPC = gEntList.FindEntityByClassname(pNPC, "n*");
	}
	//clone them
	for (int i = 0; vNPCs.Size() >= i + 1; i++)
	{
		CBaseEntity* pCloneNPC = RetrieveStoredEnt(&vNPCs[i], false);
		if (pCloneNPC)
		{
			Vector vecOrigin = pCloneNPC->GetAbsOrigin();
			if (pCloneNPC->ClassMatches("npc_barnacle"))
			{
				//we don't want barnacles to be in the same spot as their original cause that just breaks both of them
				//GetUnstuck doesn't work for whatever reason, so instead we manually find a good piece of ceiling nearby
				trace_t tr;
				int iAttempts = 0;
				do
				{
					iAttempts++;
					QAngle aBarnacleOffset = QAngle(0, RandomInt(1, 360), 0);
					Vector vecBarnacleOffset;
					AngleVectors(aBarnacleOffset, &vecBarnacleOffset);
					vecBarnacleOffset *= 30;
					UTIL_TraceLine(pCloneNPC->GetAbsOrigin() - Vector(0, 0, 4), pCloneNPC->GetAbsOrigin() + vecBarnacleOffset, MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_DEBRIS, &tr);
				} while (tr.DidHit() && iAttempts < 10);
				UTIL_TraceLine(tr.endpos, tr.endpos + Vector(0, 0, 100), MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_DEBRIS, &tr);
				vecOrigin = tr.endpos - Vector(0, 0, 2);
				pCloneNPC->SetAbsOrigin(vecOrigin);
			}
			QAngle vecAngle = pCloneNPC->GetAbsAngles();
			DispatchSpawn(pCloneNPC);
			pCloneNPC->Activate();
			pCloneNPC->Teleport(&vecOrigin, &vecAngle, NULL);
		}
	}
}
void CEBumpy::FastThink()
{
	if (IterUsableVehicles(false))
		m_bReverse = !m_bReverse;
}
void CEBumpy::DoOnVehicles(CPropVehicleDriveable* pVehicle)
{
	int iSpheres = 0;
	CBaseEntity* pSphere = gEntList.FindEntityByClassname(NULL, "prop_physics");
	while (pSphere)
	{
		if (pSphere->m_bChaosSpawned && !strcmp(STRING(pSphere->GetModelName()), "models/props_c17/oildrum001.mdl"))
		{
			iSpheres++;
			//get rid of props that are far away
			if ((pSphere->GetAbsOrigin() - pVehicle->GetAbsOrigin()).Length2D() > 300 || !pVehicle->GetDriver())
			{
				UTIL_Remove(pSphere);
				iSpheres--;
			}
		}
		pSphere = gEntList.FindEntityByClassname(pSphere, "prop_physics");
	}
	if (iSpheres < 7 && pVehicle->GetDriver())
	{
		//put invisible props in front of the car
		QAngle angFacing = pVehicle->GetAbsAngles();
		float flSidewaysPos = RandomInt(-50, 50);
		float flForwardPos = RandomInt(pVehicle->GetPhysics()->GetVehicleOperatingParams().speed >= 0 ? 200 : -200, pVehicle->GetPhysics()->GetVehicleOperatingParams().speed >= 0 ? 300 : -300);
		Vector vecOffset = Vector(flSidewaysPos, flForwardPos, 0);
		Vector vecRotated;
		VectorRotate(vecOffset, angFacing, vecRotated);
		trace_t tr;
		UTIL_TraceLine(vecRotated + pVehicle->GetAbsOrigin(), vecRotated + pVehicle->GetAbsOrigin() - Vector(0, 0, 1000), MASK_SOLID, pVehicle, COLLISION_GROUP_DEBRIS, &tr);
		CBaseEntity* pEnt = CreateEntityByName("prop_physics");
		pEnt->SetAbsOrigin(tr.endpos - Vector(0, 0, 7));
		pEnt->SetAbsAngles(QAngle(90, angFacing.y, 0));
		pEnt->SetModel("models/props_c17/oildrum001.mdl");
		pEnt->AddEffects(EF_NODRAW);
		g_iChaosSpawnCount++;
		pEnt->m_iChaosID = g_iChaosSpawnCount;
		pEnt->m_bChaosSpawned = true;
		pEnt->AddSpawnFlags(8);
		DispatchSpawn(pEnt);
	}
}
void CEGravitySet::StartEffect()
{
	bool bNegative = g_ChaosEffects[EFFECT_INVERTG]->m_bActive;
	switch (m_nID)
	{
	case EFFECT_ZEROG:
		sv_gravity.SetValue(0);
		Msg("Setting sv_gravity to 0\n");
		break;
	case EFFECT_SUPERG:
		sv_gravity.SetValue(bNegative ? -1800 : 1800);
		Msg("Setting sv_gravity to %i\n", bNegative ? -1800 : 1800);
		break;
	case EFFECT_LOWG:
		sv_gravity.SetValue(bNegative ? -200 : 200);
		Msg("Setting sv_gravity to %i\n", bNegative ? -200 : 200);
		break;
	case EFFECT_INVERTG:
		Msg("Setting sv_gravity to %i\n", -sv_gravity.GetInt());
		sv_gravity.SetValue(-sv_gravity.GetInt());
		break;
	}
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
void CEGravitySet::FixVehicleGravity(const char* szClassname, float flTargetGravity, float flScale)
{
	CBaseEntity* pVehicle = gEntList.FindEntityByClassname(NULL, szClassname);
	while (pVehicle)
	{
		float flCounteract = flTargetGravity - pVehicle->m_flGravityAtActivation;
		//Msg("TG: %f - GAA: %f = CA: %f\n", flTargetGravity, pVehicle->m_flGravityAtActivation, flCounteract);
		if (flCounteract == 0)
		{
			pVehicle = gEntList.FindEntityByClassname(pVehicle, szClassname);
			continue;
		}
		Vector vecAbsVelocity = vec3_origin;
		vecAbsVelocity.z = -(flCounteract * flScale * gpGlobals->frametime);
		pVehicle->ApplyAbsVelocityImpulse(vecAbsVelocity);
		pVehicle = gEntList.FindEntityByClassname(pVehicle, szClassname);
	}
}
//airboat scale for 0g: 0.6545f
//jeep scale for 0g: 0.5f (ALL types)
//real APCs, crane, pod: don't need to fix. fixing breaks them.
void CEGravitySet::FastThink()
{
	bool bNegative = g_ChaosEffects[EFFECT_INVERTG]->m_bActive;
	bool bOtherEffectOn = g_ChaosEffects[EFFECT_ZEROG]->m_bActive || g_ChaosEffects[EFFECT_SUPERG]->m_bActive || g_ChaosEffects[EFFECT_LOWG]->m_bActive;
	//if i'm invert and someone else is on, they handle it
	if (m_nID == EFFECT_INVERTG && bOtherEffectOn)
		return;
	//vehicles are reluctant to gravity changes
	//artificially apply the correct force here
	//these numbers are an amount we would add to 600 to arrive at the target gravity
	float flTargetGravity = 0;
	switch (m_nID)
	{
	case EFFECT_ZEROG:
		flTargetGravity = 0;
		break;
	case EFFECT_SUPERG:
		flTargetGravity = bNegative ? -1800 : 1800;
		break;
	case EFFECT_LOWG:
		flTargetGravity = bNegative ? -200 : 200;
		break;
	case EFFECT_INVERTG:
		flTargetGravity = -600;
		break;
	}
	FixVehicleGravity("prop_vehicle_airboat", flTargetGravity, 0.6545f);
	FixVehicleGravity("prop_vehicle_jeep", flTargetGravity, 0.5f);
}
void CEGravitySet::StopEffect()
{
	bool bNegative = g_ChaosEffects[EFFECT_INVERTG]->m_bActive;
	switch (m_nID)
	{
	case EFFECT_ZEROG:
	case EFFECT_SUPERG:
	case EFFECT_LOWG:
		sv_gravity.SetValue(bNegative ? -600 : 600);
		Msg("Unsetting sv_gravity to %i\n", bNegative ? -600 : 600);
		break;
	case EFFECT_INVERTG:
		Msg("Unsetting sv_gravity to %i\n", -sv_gravity.GetInt());
		sv_gravity.SetValue(-sv_gravity.GetInt());
		break;
	}
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
bool CEGravitySet::CheckStrike(const CTakeDamageInfo& info)
{
	return (info.GetDamageType() & DMG_FALL) != 0;
}
void CEGravitySet::MaintainEffect()
{
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
void CEPhysSpeedSet::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_PHYS_PAUSE:
		phys_timescale.SetValue(0);
		break;
	case EFFECT_PHYS_FAST:
		phys_timescale.SetValue(4);
		break;
	case EFFECT_PHYS_SLOW:
		phys_timescale.SetValue(0.25f);
		break;
	}
}
void CEPhysSpeedSet::StopEffect()
{
	phys_timescale.SetValue(1);
}
void CEStop::StartEffect()
{
	sv_maxspeed.SetValue(0);
	IterUsableVehicles(false);
}
void CEStop::DoOnVehicles(CPropVehicleDriveable* pVehicle)
{
	variant_t emptyVariant;
	if (m_flTimeRem < 1)
		pVehicle->AcceptInput("TurnOn", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), emptyVariant, 0);
	else
		pVehicle->AcceptInput("TurnOff", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), emptyVariant, 0);
}
void CEStop::StopEffect()
{
	sv_maxspeed.SetValue(320);
	UTIL_GetLocalPlayer()->SetMaxSpeed(HL2_NORM_SPEED);
	IterUsableVehicles(false);
	if (g_ChaosEffects[EFFECT_SUPER_MOVEMENT]->m_bActive && g_ChaosEffects[EFFECT_SUPER_MOVEMENT]->m_flTimeRem > 1)
	{
		//EFFECT_SUPER_MOVEMENT is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_SUPER_MOVEMENT]->StartEffect();
	}
}
void CESwimInAir::StartEffect()
{
	UTIL_GetLocalPlayer()->m_bSwimInAir = true;
}
void CESwimInAir::StopEffect()
{
	UTIL_GetLocalPlayer()->m_bSwimInAir = false;
}
void CESwimInAir::FastThink()
{
	//TODO: can we move this to MaintainEffect? it's just been here since creation.
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer->IsInAVehicle())
	{
		pPlayer->SetWaterLevel(WL_Eyes);
		pPlayer->SetPlayerUnderwater(true);
		pPlayer->AddFlag(FL_INWATER);
		pPlayer->AddFlag(FL_SWIM);
	}
	//other half of this logic is in prethink
}
bool CESwimInAir::CheckStrike(const CTakeDamageInfo& info)
{
	return (info.GetDamageType() & DMG_DROWN) != 0;
}
void CELockPVS::StartEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_lockpvs 0; exec portalsopenall; wait 100; r_lockpvs 1; r_portalsopenall 1\n");
}
void CELockPVS::StopEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "r_lockpvs 0; exec portalsopenall\n");
}
void CELockPVS::TransitionEffect()
{
	StartEffect();
}
void CEPlayerBig::StartEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(2);
	UTIL_GetLocalPlayer()->GetUnstuck(500);//done here so that the player won't be stuck when reloading a save that was made before the effect was on
}
void CEPlayerBig::StopEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(1);
	if (g_ChaosEffects[EFFECT_PLAYER_SMALL]->m_bActive && g_ChaosEffects[EFFECT_PLAYER_SMALL]->m_flTimeRem > 1)
	{
		//EFFECT_PLAYER_SMALL is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_PLAYER_SMALL]->StartEffect();
	}
}
void CEPlayerBig::MaintainEffect()
{
	//if (!UTIL_GetLocalPlayer()->IsInAVehicle())
	//	UTIL_GetLocalPlayer()->GetUnstuck(500);
}
void CEPlayerSmall::StartEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(0.5);
}
void CEPlayerSmall::StopEffect()
{
	UTIL_GetLocalPlayer()->SetModelScale(1);
	UTIL_GetLocalPlayer()->GetUnstuck(500);
	if (g_ChaosEffects[EFFECT_PLAYER_BIG]->m_bActive && g_ChaosEffects[EFFECT_PLAYER_BIG]->m_flTimeRem > 1)
	{
		//EFFECT_PLAYER_BIG is active and will outlast me, restore its effects
		g_ChaosEffects[EFFECT_PLAYER_BIG]->StartEffect();
	}
}
void CEPlayerSmall::MaintainEffect()
{
	if (!UTIL_GetLocalPlayer()->IsInAVehicle())
		UTIL_GetLocalPlayer()->GetUnstuck(500);
}
void CESuperGrab::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	player_use_dist.SetValue(8000);
	player_throwforce.SetValue(50000);
	if (pHL2Player)
		pHL2Player->m_bSuperGrab = true;
}
void CESuperGrab::StopEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player* pHL2Player = static_cast<CHL2_Player*>(pPlayer);
	player_use_dist.SetValue(80);
	player_throwforce.SetValue(1000);
	if (pHL2Player)
		pHL2Player->m_bSuperGrab = false;
}
void CEWeaponsDrop::StartEffect()
{
	m_bDone = false;
}
void CEWeaponsDrop::FastThink()
{
	if (m_bDone)
		return;
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CBaseCombatWeapon* pActiveWeapon = pPlayer->GetActiveWeapon();
	QAngle gunAngles;
	VectorAngles(pPlayer->BodyDirection2D(), gunAngles);
	Vector vecForward;
	AngleVectors(gunAngles, &vecForward, NULL, NULL);
	float flDiameter = sqrt(pPlayer->CollisionProp()->OBBSize().x * pPlayer->CollisionProp()->OBBSize().x +
		pPlayer->CollisionProp()->OBBSize().y * pPlayer->CollisionProp()->OBBSize().y);
	for (int i = 0; i < MAX_WEAPONS; ++i)
	{
		CBaseCombatWeapon* pWeapon = pPlayer->m_hMyWeapons[i];
		if (!pWeapon)
			continue;
		// Have to drop this after we've dropped everything else, so autoswitch doesn't happen
		if (pWeapon == pActiveWeapon)
			continue;
		pPlayer->DropWeaponForWeaponStrip(pWeapon, vecForward, gunAngles, flDiameter);

		//a little speculative fix. we had an unexplainable engine crash once when dropping all weapons simultaneously
		//so now we're dropping one per tick to hopefully fix that
		return;
	}
	m_bDone = true;
	// Drop the active weapon normally...
	if (pActiveWeapon)
	{
		// Nowhere in particular; just drop it.
		Vector vecThrow;
		pPlayer->ThrowDirForWeaponStrip(pActiveWeapon, vecForward, &vecThrow);
		// Throw a little more vigorously; it starts closer to the player
		vecThrow *= RandomFloat(800.0f, 1000.0f);
		pPlayer->Weapon_Drop(pActiveWeapon, NULL, &vecThrow);
		pActiveWeapon->SetRemoveable(false);
	}
}
void CEEarthquake::StartEffect()
{
	UTIL_ScreenShake(UTIL_GetLocalPlayer()->WorldSpaceCenter(), 50 * UTIL_GetLocalPlayer()->GetModelScale(), 2, m_flTimeRem, 375, SHAKE_START, true);
}
void CEEarthquake::TransitionEffect()
{
	StartEffect();
}
void CE420Joke::StartEffect()
{
	UTIL_GetLocalPlayer()->SetHealth(4);
	UTIL_GetLocalPlayer()->SetArmorValue(20);
}
void CEQuickclip::StartEffect()
{
	switch (m_nID)
	{
	case EFFECT_QUICKCLIP_ON:
		UTIL_GetLocalPlayer()->SetCollisionGroup(COLLISION_GROUP_IN_VEHICLE);
		break;
	case EFFECT_QUICKCLIP_OFF:
		UTIL_GetLocalPlayer()->SetCollisionGroup(COLLISION_GROUP_PLAYER);
		break;
	}
}
ConVar groundeffect_tracedist("groundeffect_tracedist", "4");
GroundState CEFloorEffect::GroundShouldActivateEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	//have to be on solid ground
	if (pPlayer->IsInAVehicle())
	{
		//Msg("In a vehicle\n");
		return GS_OFFGROUND;
	}

	if (m_nID == EFFECT_FLOOR_IS_LAVA)
	{
		//don't hurt if in water
		if (pPlayer->GetWaterLevel() >= WL_Feet)
			return GS_DONT_ACTIVATE;

		//allow all grating and clipping
		if (pPlayer->m_chTextureType == 'G' || pPlayer->m_chTextureType == 'I')
			return GS_DONT_ACTIVATE;
	}

	//trace hull lets us see when player is surfing. ground entity is null when surfing, so it's not reliable.
	float flHullWidth = 16 * pPlayer->GetModelScale();
	Vector vStart = pPlayer->GetAbsOrigin() + Vector(0, 0, 1);
	Vector vEnd = pPlayer->GetAbsOrigin() - Vector(0, 0, groundeffect_tracedist.GetFloat());
	Vector vHullWidth = Vector(flHullWidth, flHullWidth, 0);
	trace_t	trace;
	UTIL_TraceHull(vStart, vEnd, -vHullWidth, vHullWidth, CONTENTS_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace);

	//have to be on solid ground
	if (!trace.m_pEnt)
	{
		//Msg("Off the ground\n");
		return GS_OFFGROUND;
	}

	if (m_nID == EFFECT_FLOOR_IS_LAVA)
	{
		//all entities get a free pass
		if (!trace.m_pEnt->IsWorld())
			return GS_DONT_ACTIVATE;

		//don't count static props
		if (!strcmp(trace.surface.name, "**studio**"))
			return GS_DONT_ACTIVATE;
	}

	//test in center
	//for some reason this distance used to be 20 units down. if there was a reason for this, write it here when we re-discover it.
	trace_t	trace2;
	UTIL_TraceLine(vStart, vEnd, CONTENTS_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace2);

	//if you're on sky or nodraw, then whatever
	if ((trace2.surface.flags & SURF_SKY) || (trace2.surface.flags & SURF_NODRAW))
	{
		//Msg("Standing on sky/nodraw\n");
		return GS_DONT_ACTIVATE;
	}

	//want grass only
	if (m_nID == EFFECT_GRASS_HEAL)
	{
		if (pPlayer->m_chTextureType == 'J' || pPlayer->m_chTextureType == 'K')
		{
			//Msg("Texture type %c (grass)\n", pPlayer->m_chTextureType);
			return GS_ACTIVATE_EFFECT;
		}
		else
		{
			//Msg("Texture type %c (not grass)\n", pPlayer->m_chTextureType);
			return GS_DONT_ACTIVATE;
		}
	}

	//Msg("activating effect\n");
	return GS_ACTIVATE_EFFECT;
}
void CEFloorEffect::FastThink()
{
	GroundState eState = GroundShouldActivateEffect();
	if (eState == GS_OFFGROUND)
	{
		m_iJumped = true;
	}
	else
	{
		CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
		CTakeDamageInfo infoBurn(pPlayer, pPlayer, m_iJumped ? 2 : 0.125, DMG_BURN);
		m_iJumped = false;
		if (eState == GS_ACTIVATE_EFFECT)
		{
			switch (m_nID)
			{
			case EFFECT_FLOOR_IS_LAVA:
				pPlayer->TakeDamage(infoBurn);
				break;
			case EFFECT_GRASS_HEAL:
				pPlayer->TakeHealth(m_iJumped ? 2 : 0.125, DMG_GENERIC);
				break;
			}
		}
	}
}
bool CEFloorEffect::CheckStrike(const CTakeDamageInfo& info)
{
	switch (m_nID)
	{
	case EFFECT_FLOOR_IS_LAVA:
		return (info.GetDamageType() & DMG_BURN) != 0;
	default:
		return false;
	}
}
void CEUseSpam::FastThink()
{
	UTIL_GetLocalPlayer()->m_afButtonPressed |= IN_USE;
}
void CENoBrake::StartEffect()
{
	r_handbrake_allowed.SetValue(0);
	r_vehicleBrakeRate.SetValue(0);
}
void CENoBrake::StopEffect()
{
	r_handbrake_allowed.SetValue(1);
	r_vehicleBrakeRate.SetValue(1.5f);
}
void CEForceInOutCar::StartEffect()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CPropVehicleDriveable* pVehicle = (CPropVehicleDriveable*)pPlayer->GetVehicleEntity();
	if (pVehicle)
	{
		//get out
		bool bWasLocked = pVehicle->IsLocked();
		pVehicle->SetLocked(false);
		pPlayer->GetVehicle()->HandlePassengerExit(pPlayer);
		pVehicle->SetLocked(bWasLocked);
		m_strHudName = MAKE_STRING("#hl2c_force_out_veh");
	}
	else
	{
		//get in
		IterUsableVehicles(false);
	}

}
void CEForceInOutCar::DoOnVehicles(CPropVehicleDriveable* pVehicle)
{
	if (m_bFoundOne)
		return;
	//find a vehicle first
	if (pVehicle)
	{
		UTIL_GetLocalPlayer()->ForceDropOfCarriedPhysObjects();
		bool bWasLocked = pVehicle->IsLocked();
		pVehicle->SetLocked(false);
		pVehicle->GetServerVehicle()->HandlePassengerEntry(UTIL_GetLocalPlayer(), true);
		pVehicle->SetLocked(bWasLocked);
		m_strHudName = MAKE_STRING("#hl2c_force_in_veh");
		m_bFoundOne = true;
	}
}
void CEWeaponRemove::StartEffect()
{
	if (UTIL_GetLocalPlayer()->GetActiveWeapon() == NULL)
		return;
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)gEntList.FindEntityByClassname(NULL, "we*");
	CBaseCombatWeapon* pWeapon1 = NULL;
	CBaseCombatWeapon* pWeapon2 = NULL;
	while (pWeapon && !pWeapon2)
	{
		if (pWeapon->GetOwner() && pWeapon->GetOwner()->IsPlayer())
		{
			if (pWeapon1)
			{
				pWeapon2 = pWeapon;
				break;
			}
			else
			{
				pWeapon1 = pWeapon;
			}
		}
		pWeapon = (CBaseCombatWeapon*)gEntList.FindEntityByClassname(pWeapon, "we*");
	}
	if (!pWeapon2)
		return;
	pWeapon = RandomInt(0, 1) ? pWeapon1 : pWeapon2;
	//hide model
	CBaseViewModel* vm = UTIL_GetLocalPlayer()->GetViewModel(pWeapon->m_nViewModelIndex);
	if (vm && pWeapon == UTIL_GetLocalPlayer()->GetActiveWeapon())
		vm->AddEffects(EF_NODRAW);
	pWeapon->Delete();
}
void CEPhysConvert::StartEffect()
{
	//door-linked areaportals become permanently open since the door is now free to move
	CBaseEntity* pPortal = NULL;
	while ((pPortal = gEntList.FindEntityByClassname(pPortal, "func_a*")) != NULL)
	{
		if (pPortal->m_target != NULL_STRING)
		{
			// USE_ON means open the portal, off means close it
			pPortal->Use(pPortal, pPortal, USE_ON, 0);
			pPortal->m_target = NULL_STRING;
		}
	}
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		//disable physics constraints and break ropes for maximum breaking
		if (pEnt->ClassMatches("ph*") || pEnt->ClassMatches("mov*") || pEnt->ClassMatches("k*"))
		{
			variant_t emptyVariant;
			pEnt->AcceptInput("Break", pEnt, pEnt, emptyVariant, 0);
			pEnt->AcceptInput("Turnoff", pEnt, pEnt, emptyVariant, 0);//Also hit phys_thruster and torque
			if (pEnt->ClassMatches("phys_spring"))//no break or turnoff input, just directly kill i guess
				pEnt->AcceptInput("Kill", pEnt, pEnt, emptyVariant, 0);
			pEnt = gEntList.NextEnt(pEnt);
			continue;
		}
		//tell crane drivers that their magnet just dropped stuff, or they may act odd
		if (pEnt->ClassMatches("npc_cra*"))
		{
			variant_t emptyVariant;
			pEnt->AcceptInput("MagnetDropped", pEnt, pEnt, emptyVariant, 0);
			pEnt = gEntList.NextEnt(pEnt);
			continue;
		}
		//objects have to be real "things"
		//no world
		//no vehicles
		//no players
		//no children (some props are visual representations of invisible brush entities)
		//and don't be an invisible brush entity or something along those lines
		if (pEnt->IsEffectActive(EF_NODRAW) || !(pEnt->GetSolid() == SOLID_BSP || pEnt->GetSolid() == SOLID_VPHYSICS) || pEnt->IsSolidFlagSet(FSOLID_NOT_SOLID) ||
			pEnt->IsWorld() || pEnt->IsPlayer() ||
			!pEnt->GetModel() || pEnt->GetServerVehicle() || pEnt->GetParent())
		{
			pEnt = gEntList.NextEnt(pEnt);
			continue;
		}
		//we DO let vphysics entities pass because we still want to wake them

		IPhysicsObject* pPhysicsObject = pEnt->VPhysicsGetObject();
		if (pPhysicsObject && pPhysicsObject->IsStatic())
		{
			pEnt->VPhysicsDestroyObject();
			pEnt->VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);
			pPhysicsObject = pEnt->VPhysicsGetObject();
		}
		if (!pPhysicsObject)
			pPhysicsObject = pEnt->VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);
		if (pPhysicsObject != NULL)
		{
			pPhysicsObject->SetShadow(1e4, 1e4, true, true);
			pEnt->SetSolid(SOLID_VPHYSICS);
			pEnt->SetMoveType(MOVETYPE_VPHYSICS);
			pPhysicsObject->RemoveShadowController();
			pEnt->VPhysicsUpdate(pPhysicsObject);
			pPhysicsObject->EnableMotion(true);
			pPhysicsObject->RecheckCollisionFilter();
			pPhysicsObject->Wake();
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
}
void CEIncline::StartEffect()
{
	steepness_limit.SetValue(0.96f);//1.0 is too aggressive, we will slide around on seemingly flat things
	sv_airaccelerate.SetValue(1);
	UTIL_GetLocalPlayer()->m_Local.m_flStepSize = 0;
	chaos_disable_ladders.SetValue(true);
}
void CEIncline::StopEffect()
{
	steepness_limit.SetValue(0.7f);
	sv_airaccelerate.SetValue(10);
	UTIL_GetLocalPlayer()->m_Local.m_flStepSize = 18;
	chaos_disable_ladders.SetValue(false);
}
void CEDeathWater::FastThink()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer->GetWaterLevel() >= WL_Feet && !pPlayer->IsInAVehicle())
	{
		CTakeDamageInfo info(pPlayer, pPlayer, 1000, DMG_SONIC);
		pPlayer->TakeDamage(info);
	}
}
bool CEDeathWater::CheckStrike(const CTakeDamageInfo& info)
{
	return (info.GetDamageType() & DMG_SONIC) != 0;
}
void CEBarrelShotgun::StartEffect()
{
	chaos_barrel_shotgun.SetValue(1);
}
void CEBarrelShotgun::StopEffect()
{
	chaos_barrel_shotgun.SetValue(0);
}
bool CEBarrelShotgun::CheckStrike(const CTakeDamageInfo& info)
{
	return !strcmp(STRING(info.GetAttacker()->GetModelName()), "models/props_c17/oildrum001_explosive.mdl") && info.GetAttacker()->m_bChaosSpawned;
}
void CEDejaVu::StartEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "reload setpos");
}
void CERandomCC::StartEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "cc_generate");
}
void CERandomCC::StopEffect()
{
	CBaseEntity* pCC = gEntList.FindEntityByName(NULL, "chaos_cc");
	while (pCC)
	{
		variant_t emptyVariant;
		UTIL_Remove(pCC);
		pCC = gEntList.FindEntityByName(pCC, "chaos_cc");
	}
}
void CERandomCC::TransitionEffect()
{
	StopEffect();
	DisplayGeneratedCC();
}
void CERandomCC::RestoreEffect()
{
	StopEffect();
	DisplayGeneratedCC();
}
void CESecondarySpam::MaintainEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1),
		"+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;+attack2;wait 5;-attack2;wait 5;\n");
}
void CESuitSwap::StartEffect()
{
	int iHealth = UTIL_GetLocalPlayer()->GetHealth();
	int iSuit = UTIL_GetLocalPlayer()->m_ArmorValue;
	//Msg("health %i suit %i\n", iHealth, iSuit);
	UTIL_GetLocalPlayer()->SetHealth(iSuit);
	UTIL_GetLocalPlayer()->SetArmorValue(iHealth);
	if (iSuit == 0)
	{
		//so now player is dead, but damage them a bit more so the game properly registers their death
		variant_t variant;
		variant.SetInt(-1);
		UTIL_GetLocalPlayer()->AcceptInput("SetHealth", UTIL_GetLocalPlayer(), UTIL_GetLocalPlayer(), variant, 0);
	}
}
void CEGiveAllRPG::StartEffect()
{
	//player
	ChaosSpawnWeapon("weapon_rpg", MAKE_STRING("#hl2c_giveallrpgs"), 3, "rpg_round");
	//NPCs
	CBaseEntity* pEnt = gEntList.FindEntityByClassname(NULL, "npc*");
	while (pEnt)
	{
		CAI_BaseNPC* pNPC = dynamic_cast<CAI_BaseNPC*>(pEnt);
		if (pNPC)
		{
			pNPC->GiveWeapon(MAKE_STRING("weapon_rpg"));
			pNPC->CapabilitiesAdd(bits_CAP_WEAPON_RANGE_ATTACK1);
		}
		pEnt = gEntList.FindEntityByClassname(pEnt, "npc*");
	}
}
void CEChangePitch::MaintainEffect()
{
	//the convar allows us to hit instanced scenes at their moment of creation.
	float flPitch = RandomFloat(0.25, 2.5);
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "scene_pitch_default %f; ent_fire logic_choreographed_scene pitchshift %f\n", flPitch, flPitch);
}
void CEChangePitch::StopEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "scene_pitch_default 1; ent_fire logic_choreographed_scene pitchshift 1\n");
}
void CELogicExplode::StartEffect()
{
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		//don't change these, too likely to fuck things up
		if (!strcmp(pEnt->GetClassname(), "hl2_gamerules"))
		{
			pEnt = gEntList.NextEnt(pEnt);
			continue;
		}
		pEnt->LogicExplode();
		pEnt = gEntList.NextEnt(pEnt);
	}
}
void CECameraTextures::StartEffect()
{
	//entities (studio only for now)
	CBaseEntity* pEnt = gEntList.FirstEnt();
	while (pEnt)
	{
		model_t* pModel = pEnt->GetModel();
		CBaseAnimating* pAnimating = pEnt->GetBaseAnimating();
		if (pModel && modelinfo->GetModelType(pModel) == mod_studio && pAnimating)
		{
			IMaterial* pMaterials[128];
			int materialCount = g_pStudioRender->GetMaterialListFromBodyAndSkin(modelinfo->GetCacheHandle(pModel), pAnimating->m_nSkin, pAnimating->m_nBody, ARRAYSIZE(pMaterials), pMaterials);
			for (int i = 0; i < materialCount; i++)
			{
				if (pMaterials[i] != NULL)
				{
					bool foundVar;
					IMaterialVar* pTexName = pMaterials[i]->FindVar("$basetexture", &foundVar, false);
					if (foundVar)
					{
						ITexture* pSrc = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
						pTexName->SetTextureValue(pSrc);
					}
				}
			}
		}
		pEnt = gEntList.NextEnt(pEnt);
	}
	//static props
	ICollideable* pCollideable = staticpropmgr->GetStaticPropByIndex(0);
	for (int i = 1; pCollideable; i++)
	{
		const model_t* pModel = pCollideable->GetCollisionModel();
		if (pModel)
		{
			IMaterial* pMaterials[128];
			int materialCount = g_pStudioRender->GetMaterialListFromBodyAndSkin(modelinfo->GetCacheHandle(pModel), 0, 0, ARRAYSIZE(pMaterials), pMaterials);
			for (int i = 0; i < materialCount; i++)
			{
				if (pMaterials[i] != NULL)
				{
					bool foundVar;
					IMaterialVar* pTexName = pMaterials[i]->FindVar("$basetexture", &foundVar, false);
					if (foundVar)
					{
						ITexture* pSrc = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
						pTexName->SetTextureValue(pSrc);
					}
				}
			}
		}

		pCollideable = staticpropmgr->GetStaticPropByIndex(i);
	}
}
void CECameraTextures::TransitionEffect()
{
	StartEffect();
}
void CECameraTextures::StopEffect()
{
	engine->ClientCommand(engine->PEntityOfEntIndex(1), "mat_reloadallmaterials");
}
void CECameraGravity::FastThink()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	Vector vecAngles;
	QAngle angleCamera = pPlayer->GetAbsAngles();
	AngleVectors(angleCamera, &vecAngles);
	physenv->SetGravity(vecAngles * GetCurrentGravity());
}
void CECameraGravity::StopEffect()
{
	physenv->SetGravity(Vector(0, 0, -GetCurrentGravity()));
}
void CEHL1Phys::ChangeEntity(CBaseEntity* pEntity)
{
	if (pEntity->VPhysicsGetObject() && pEntity->VPhysicsGetObject()->IsMotionEnabled())
	{
		float speeddamping;
		float rotdamping;
		pEntity->VPhysicsGetObject()->GetDamping(&speeddamping, &rotdamping);
		Vector inertia = pEntity->VPhysicsGetObject()->GetInertia();

		//print for debugging
		//Msg("speed damp: %f rot damp: %f inertia: %0.1f %0.1f %0.1f\n", speeddamping, rotdamping, inertia.x, inertia.y, inertia.z);

		rotdamping = FLT_MAX;

		pEntity->VPhysicsGetObject()->SetDamping(&speeddamping, &rotdamping);
		pEntity->VPhysicsGetObject()->SetInertia(Vector(1e30, 1e30, 1e30));
	}
}
void CEHL1Phys::RevertEntity(CBaseEntity* pEntity)
{
	//haven't been able to make this work right. don't know what to set the values back to
	//if (pEntity->VPhysicsGetObject() && pEntity->VPhysicsGetObject()->IsMotionEnabled())
	{
		//pEntity->VPhysicsGetObject()->SetInertia(Vector(1, 1, 1));
		//pEntity->VPhysicsGetObject()->SetDamping(0, 0);
	}
}
void CEHL1Phys::StartEffect()
{
	for (CBaseEntity* pEnt = gEntList.FirstEnt(); pEnt; pEnt = gEntList.NextEnt(pEnt))
	{
		ChangeEntity(pEnt);
	}
}
void CEHL1Phys::OnEntitySpawned(CBaseEntity* pEntity)
{
	ChangeEntity(pEntity);
}
void CEHL1Phys::StopEffect()
{
	//haven't been able to make this work right. don't know what to set the values back to
	//for (CBaseEntity* pEnt = gEntList.FirstEnt(); pEnt; pEnt = gEntList.NextEnt(pEnt))
	{
		//RevertEntity(pEnt);
	}
}
void CEHL1Phys::TransitionEffect()
{
	StartEffect();
}
void CEDVDCrosshair::StartEffect()
{
	dvdcross.SetValue(true);
}
void CEDVDCrosshair::StopEffect()
{
	dvdcross.SetValue(false);
}