#ifndef CHAOS_H
#define CHAOS_H
#pragma once
#include "ai_node.h"
#include "ai_network.h"
#include "vehicle_base.h"
#define MAX_ACTIVE_EFFECTS 64
#define MAX_GROUPS 32
#define MAX_EFFECTS_IN_GROUP 32
enum Effect_T
{
	EFFECT_ERROR,
	EFFECT_ZEROG,
	EFFECT_SUPERG,
	EFFECT_LOWG,
	EFFECT_INVERTG,
	EFFECT_PHYS_PAUSE,
	EFFECT_PHYS_FAST,
	EFFECT_PHYS_SLOW,
	EFFECT_PULL_TO_PLAYER,
	EFFECT_PUSH_FROM_PLAYER,
	EFFECT_NO_MOVEMENT,
	EFFECT_SUPER_MOVEMENT,
	EFFECT_LOCK_VEHICLE,
	EFFECT_NPC_HATE,
	EFFECT_NPC_LIKE,
	EFFECT_NPC_NEUTRAL,
	EFFECT_NPC_FEAR,
	EFFECT_TELEPORT_RANDOM,
	EFFECT_SPAWN_VEHICLE,
	EFFECT_SPAWN_NPC,
	EFFECT_SWIM_IN_AIR,
	EFFECT_ONLY_DRAW_WORLD,
	EFFECT_LOW_DETAIL,
	EFFECT_PLAYER_BIG,
	EFFECT_PLAYER_SMALL,
	EFFECT_NO_MOUSE_HORIZONTAL,
	EFFECT_NO_MOUSE_VERTICAL,
	EFFECT_SUPER_GRAB,
	EFFECT_GIVE_WEAPON,
	EFFECT_GIVE_ALL_WEAPONS,
	EFFECT_DROP_WEAPONS,
	//EFFECT_CROSSBOW_GUNS,//crossbow guns - replace bullets with crossbow bolts. requires moving some code around and i don't wanna deal with it right now
	EFFECT_NADE_GUNS,//grenade guns - replace bullets with grenades. same story as crossbow
	//rapid weapon fire - that will take a while to do
	EFFECT_EARTHQUAKE,
	//EFFECT_WINDY,//broken for unknown reasons
	EFFECT_420_JOKE,
	EFFECT_ZOMBIE_SPAM,
	//EFFECT_LOW_FOV,
	//EFFECT_HIGH_FOV,
	EFFECT_EXPLODE_ON_DEATH,
	EFFECT_BULLET_TELEPORT,
	EFFECT_CREDITS,
	//EFFECT_SANTIAGO,
	EFFECT_SUPERHOT,
	EFFECT_SUPERCOLD,
	EFFECT_BARREL_SHOTGUN,
	EFFECT_QUICKCLIP_ON,
	EFFECT_QUICKCLIP_OFF,
	EFFECT_SOLID_TRIGGERS,
	EFFECT_RANDOM_COLORS,
	EFFECT_BEER_BOTTLE,
	EFFECT_EVIL_ALYX,
	EFFECT_EVIL_NORIKO,
	EFFECT_CANT_LEAVE_MAP,
	EFFECT_FLOOR_IS_LAVA,
	EFFECT_PLAY_MUSIC,
	EFFECT_USE_SPAM,
	EFFECT_ORTHO_CAM,
	EFFECT_FOREST,
	EFFECT_SPAWN_MOUNTED_GUN,
	EFFECT_BACK_LEVEL,
	EFFECT_REMOVE_PICKUPS,
	EFFECT_CLONE_NPCS,
	EFFECT_LOCK_PVS,
	EFFECT_RELOAD_DEJA_VU,
	EFFECT_BUMPY,
	EFFECT_NO_BRAKE,
	EFFECT_FORCE_INOUT_CAR,
	EFFECT_WEAPON_REMOVE,
	EFFECT_INTERP_NPCS,
	EFFECT_PHYS_CONVERT,
	EFFECT_INCLINE,
	EFFECT_DISABLE_SAVE,
	EFFECT_NO_RELOAD,
	EFFECT_NPC_TELEPORT,
	EFFECT_DEATH_WATER,

	NUM_EFFECTS
};

//effect contexts
#define EC_NONE				0
#define EC_BOAT				1
#define EC_BUGGY			2
#define EC_WATER			4
#define EC_PHYSICS			8
#define EC_NO_INVULN		16
#define EC_HAS_WEAPON		32
#define EC_QC_OFF			64
#define EC_NO_CUTSCENE		128
#define EC_PICKUPS			256
#define EC_NO_CITADEL		512
#define EC_PLAYER_TELEPORT	1024
#define EC_NO_VEHICLE		2048

enum
{
	SPAWNTYPE_EYELEVEL_REGULAR,
	SPAWNTYPE_EYELEVEL_SPECIAL,
	SPAWNTYPE_CEILING,
	SPAWNTYPE_BIGFLYER,
	SPAWNTYPE_STRIDER,
	SPAWNTYPE_VEHICLE,
};
class CChaosEffect
{
public:
	//DECLARE_CLASS_NOBASE(CChaosEffect);
	virtual void StartEffect();
	virtual void StopEffect();
	void AbortEffect();
	virtual void MaintainEffect(){};
	virtual void FastThink(){};
	virtual void TransitionEffect(){};
	virtual bool CheckStrike(const CTakeDamageInfo &info){ return false; };
	virtual void RestoreEffect();
	bool CheckEffectContext();
	bool DoRestorationAbort();
	bool m_bActive = false;
	int m_nID;
	string_t m_strHudName;
	string_t m_strGeneralName;
	//int m_nGroup;
	int m_nContext;
	float m_flDuration;
	//float m_flStartTime;
	float m_flTimeRem;
	//int m_iWeight;
	int m_iMaxWeight;
	int m_iCurrentWeight;
	int m_iStrikes;
	bool m_bTransient;
	CBaseEntity *ChaosSpawnVehicle(const char *className, string_t strActualName, int iSpawnType, const char *strModel, const char *strTargetname, const char *strScript);
	CAI_BaseNPC *ChaosSpawnNPC(const char *className, string_t strActualName, int iSpawnType, const char *strModel, const char *strTargetname, const char *strWeapon, bool bEvil = false);
	bool ChaosSpawnWeapon(const char *className, string_t strActualName, int iCount = 0, const char *strAmmoType = NULL, int iCount2 = 0, const char *strAmmoType2 = NULL);
	void RandomTeleport(bool bPlayerOnly);
	CNodeList *GetNearbyNodes(int iNodes);
	CAI_Node *NearestNodeToPoint(const Vector &vPosition, bool bCheckVisibility);
	bool IterUsableVehicles(bool bFindOnly);
	virtual void DoOnVehicles(CPropVehicleDriveable *pVehicle){};
	CBaseEntity *GetEntityWithID(int iChaosID);
	bool MapIsCutsceneMap(const char *pMapName);
	bool MapIsLong(const char *pMapName);
	bool MapGoodForCrane(const char *pMapName);
	bool MapHasImportantPhysics(const char *pMapName);
	bool MapHasElevator(const char *pMapName);
	//DECLARE_SIMPLE_DATADESC();
};
//this is our macabre method of remembering persist entities. it's a holdover from when data was being stored in a txt file instead of global variables
//preferably this will be replaced with whatever point_template does
//i HIGHLY doubt storing the entire raw CBaseEntity would work well
class CChaosStoredEnt
{
public:
	//DECLARE_CLASS_NOBASE(CChaosStoredEnt);
	//string_t classname;
	const char *strClassname;
	string_t targetname;
	int chaosid;
	Vector origin;
	QAngle angle;
	int health;
	int max_health;
	int spawnflags;
	string_t model;
	int effects;
	int rendermode;
	int renderamt;
	Color rendercolor;
	int renderfx;
	int modelindex;
	float speed;
	int solid;
	//pKVData->SetInt("touchStamp", pEnt->touchStamp);//this was a speculative fix for some kind of touchlink related crash. if that crash comes back, put this back in.

	bool animating = false;
	int skin;

	bool combatcharacter = false;

	bool npc = false;
	bool evil = false;

	bool hasweapon = false;
	string_t additionalequipment;

	bool headcrab = false;
	bool burrowed = false;
	bool hiding = false;
	bool ceiling = false;

	bool poisonzombie = false;
	int crabcount;

	bool antlion = false;
	//bool burrowed = false;

	bool antlionguard = false;
	//bool burrowed = false;
	bool cavernbreed = false;
};
//CUtlVector<int>				g_iActiveEffects;
int g_iActiveEffects[MAX_ACTIVE_EFFECTS];
CUtlVector<CChaosEffect *>	g_ChaosEffects;
int							g_iChaosSpawnCount = 0;
CUtlVector<int>				g_iTerminated;//list of chaos ids to NOT restore from txt. used to remember which NPCs are dead as it would not make sense for them to come back to life.
CUtlVector<CChaosStoredEnt *> g_PersistEnts;
float						g_flEffectThinkRem;
float						g_flNextEffectRem = -1;
CChaosStoredEnt *StoreEnt(CBaseEntity *pEnt);
CBaseEntity *RetrieveStoredEnt(CChaosStoredEnt *pStoredEnt, bool bPersist);
bool						g_bGoBackLevel = false;
int							g_iGroups[MAX_GROUPS][MAX_EFFECTS_IN_GROUP];

class CEBumpy : public CChaosEffect
{
public:
	void FastThink() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
	bool m_bReverse;
};
class CECloneNPCs : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CERemovePickups : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEBackLevel : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEMountedGun : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CETreeSpam : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CERandomSong : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEEvilNPC : public CChaosEffect
{
public:
	void StartEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
	void EvilNoriko();
	int m_iSavedChaosID;
};
class CEBottle : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEColors : public CChaosEffect
{
public:
	void StartEffect() override;
	void MaintainEffect() override;
};
class CEPushFromPlayer : public CChaosEffect
{
public:
	void MaintainEffect() override;
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
};
class CEPullToPlayer : public CChaosEffect
{
public:
	void MaintainEffect() override;
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CESuperhot : public CChaosEffect
{
public:
	void FastThink() override;
};
class CESupercold : public CChaosEffect
{
public:
	void FastThink() override;
};
class CECredits : public CChaosEffect
{
public:
	void StartEffect() override;
	void MaintainEffect() override;
	void RestoreEffect() override;
	void TransitionEffect() override;
	bool m_bPlayedSecondSong = false;
};
class CESolidTriggers : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
};
class CESuperMovement : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CELockVehicles : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
};
class CERandomNPC : public CChaosEffect
{
public:
	void StartEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
	int m_iSavedChaosID;
	void MaintainEffect() override;
};
class CERandomVehicle : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CERandomWeaponGive : public CChaosEffect
{
public:
	void StartEffect() override;
};
//intentionally no strike check because the chance that you can't kill at least one zombie per life is super low
//maybe we could do something but ehhhh. the prospect of dying and reloading from this effect is part of the appeal.
//maybe instead of abort removing all zombies, just remove the one that did the final blow
class CEZombieSpam : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CENPCRels : public CChaosEffect
{
public:
	void DoNPCRels(int disposition, bool bRevert);
	void StartEffect() override;
	void StopEffect() override;
	void MaintainEffect() override;
};
class CEGravitySet : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
	void MaintainEffect() override;
};
class CEPhysSpeedSet : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEStop : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
};
class CESwimInAir : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void FastThink() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CELockPVS : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
};
class CEPlayerBig : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void MaintainEffect() override;
};
class CEPlayerSmall : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void MaintainEffect() override;
};
class CESuperGrab : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEWeaponsDrop : public CChaosEffect
{
public:
	void FastThink() override;
	bool m_bDone = false;
};
class CEEarthquake : public CChaosEffect
{
public:
	void StartEffect() override;
	void TransitionEffect() override;
};
class CE420Joke : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEQuickclip : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEFloorIsLava : public CChaosEffect
{
public:
	void FastThink() override;
	int m_iSkipTicks = 0;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CEUseSpam : public CChaosEffect
{
public:
	void MaintainEffect() override;
	float m_flLastUseThink = -1;
};
class CENoBrake : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEForceInOutCar : public CChaosEffect
{
public:
	void StartEffect() override;
	void DoOnVehicles(CPropVehicleDriveable *pVehicle);
	bool m_bFoundOne = false;
};
class CEWeaponRemove : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEPhysConvert : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEIncline : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CEDeathWater : public CChaosEffect
{
public:
	void FastThink() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CEBarrelShotgun : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	bool CheckStrike(const CTakeDamageInfo &info) override;
};
class CEDejaVu : public CChaosEffect
{
public:
	void StartEffect() override;
	void MaintainEffect() override;
	bool m_bDone = false;
};
#endif