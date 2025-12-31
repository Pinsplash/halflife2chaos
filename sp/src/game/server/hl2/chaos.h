#ifndef CHAOS_H
#define CHAOS_H
#pragma once
#include "ai_node.h"
#include "ai_network.h"
#include "vehicle_base.h"
#include <vector>
#define MAX_ACTIVE_EFFECTS 64
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
	EFFECT_ZOMBIE_SPAM_CLOSE,
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
	EFFECT_RESTART_LEVEL,
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
	EFFECT_RANDOM_CC,
	EFFECT_EVIL_BARNEY,
	EFFECT_GOOD_GMAN,
	EFFECT_EVIL_KLEINER,
	EFFECT_EVIL_GRIGORI,
	EFFECT_EVIL_MOSSMAN,
	EFFECT_EVIL_VORT,
	EFFECT_SECONDARY_SPAM,
	EFFECT_STEAL_HEALTH,
	EFFECT_SUIT_SWAP,
	EFFECT_YAWROLL,
	EFFECT_NORMAL_VISION,
	EFFECT_GIVE_ALL_RPG,
	EFFECT_GRASS_HEAL,
	EFFECT_CHANGE_PITCH,
	EFFECT_LOGIC_EXPLODE,
	EFFECT_CAMERA_TEXTURES,
	EFFECT_CAMERA_GRAVITY,
	EFFECT_HL1_PHYSICS,
	EFFECT_DVD_CROSSHAIR,
	EFFECT_EVIL_BREEN,
	EFFECT_ZOMBIE_SPAM_FAR,
	EFFECT_COP_SPAM,
	EFFECT_SCANNER_SPAM,
	EFFECT_HOMING_AR2,
	EFFECT_CLIMB_ANYWHERE,
	EFFECT_TIMESKIP,
	//EFFECT_EVIL_ELI,

	NUM_EFFECTS
};

//effect contexts
#define EC_NONE				0
#define EC_BOAT				1
#define EC_BUGGY			2
#define EC_WATER			4
#define EC_EXTREME			8
#define EC_NO_INVULN		16
#define EC_HAS_WEAPON		32
#define EC_QC_OFF			64
#define EC_FAR_ENEMY		128
#define EC_PICKUPS			256
#define EC_NEED_PHYSGUN		512
#define EC_PLR_TELE			1024
#define EC_NO_VEHICLE		2048

//flags for spawning chaos npcs
#define CSF_EVIL	1
#define CSF_SQUAD	2

//cache context result, for cases where code is intentionally being run many many times. kind of stepping on the toes of something in PickEffect()...
enum
{
	C_STATUS_BAD,
	C_STATUS_GOOD,
	C_STATUS_UNKNOWN,
};
enum
{
	SPAWNTYPE_EYELEVEL_REGULAR,
	SPAWNTYPE_EYELEVEL_SPECIAL,
	SPAWNTYPE_CEILING,
	SPAWNTYPE_BIGFLYER,
	SPAWNTYPE_STRIDER,
	SPAWNTYPE_VEHICLE,
	SPAWNTYPE_ONGROUND,
	SPAWNTYPE_HIDEINCOVER
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
	bool WasShufflePicked();
	bool m_bActive = false;
	int m_nID;
	//this name changes (i.e. 'Spawn Alyx' from Spawn Random NPC)
	string_t m_strHudName;
	//this name does not change (i.e. Spawn Random NPC)
	string_t m_strGeneralName;
	int m_iExclude[NUM_EFFECTS];
	int m_iExcludeCount = 0;
	int m_nContext;
	float m_flDuration;
	//float m_flStartTime;
	float m_flTimeRem;
	//int m_iWeight;
	int m_iMaxWeight;
	int m_iCurrentWeight;
	int m_iStrikes;
	bool m_bTransient;
	int m_iContextStatusCache = C_STATUS_UNKNOWN;
	//only to be set true when selecting the 4 effects to use.
	//g_arriVoteEffects will remember the effects that are in it until a new set of 4 need to be put in. other code is written with that in mind.
	//the difference matters because we don't want to exclude an effect simply for being a current option.
	bool m_bInVoteList = false;
	CBaseEntity *ChaosSpawnVehicle(const char *className, string_t strActualName, int iSpawnType, const char *strModel, const char *strTargetname, const char *strScript);
	CAI_BaseNPC *ChaosSpawnNPC(const char *className, string_t strActualName, int iSpawnType, const char *strModel, const char *strTargetname, const char *strWeapon, int flags = 0);
	bool ChaosSpawnWeapon(const char *className, string_t strActualName, int iCount = 0, const char *strAmmoType = NULL, int iCount2 = 0, const char *strAmmoType2 = NULL);
	void RandomTeleport(bool bPlayerOnly);
	CNodeList *GetNearbyNodes(int iNodes);
	CAI_Node *NearestNodeToPoint(const Vector &vPosition, bool bCheckVisibility);
	bool IterUsableVehicles(bool bFindOnly);
	virtual void DoOnVehicles(CPropVehicleDriveable *pVehicle){};
	CBaseEntity *GetEntityWithID(int iChaosID);
	bool MapIsLong(const char *pMapName);
	bool MapGoodForCrane(const char *pMapName);
	bool SafeCloneNPCs(const char *pMapName);
	bool MapHasImportantPickups(const char *pMapName);
	bool QuickclipProblems(const char *pMapName);
	bool PhysConvertSoftlock(const char *pMapName);
	bool CombatBreaksScene(const char *pMapName);
	bool NeedPhysgun(const char *pMapName);
	bool DontTeleportPlayer(const char *pMapName);
	virtual void OnEntitySpawned(CBaseEntity* pEntity){};
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
	int body;

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
	bool crabs[3];
	int crabcount;

	bool antlion = false;
	//bool burrowed = false;

	bool antlionguard = false;
	//bool burrowed = false;
	bool cavernbreed = false;
};
//CUtlVector<int>				g_iActiveEffects;
int g_iActiveEffects[MAX_ACTIVE_EFFECTS];
int g_iShufflePicked[NUM_EFFECTS];
CUtlVector<CChaosEffect *>	g_ChaosEffects;
int							g_iChaosSpawnCount = 0;
CUtlVector<int>				g_iTerminated;//list of chaos ids to NOT restore from txt. used to remember which NPCs are dead as it would not make sense for them to come back to life.
CUtlVector<CChaosStoredEnt *> g_PersistEnts;
float						g_flEffectThinkRem;
float						g_flNextEffectRem = -1;
int							g_arriVoteEffects[4];
int							g_arriVotes[4];
int							g_iVoteNumber = 0; // acts as a unique number for the external client
CChaosStoredEnt *StoreEnt(CBaseEntity *pEnt);
CBaseEntity *RetrieveStoredEnt(CChaosStoredEnt *pStoredEnt, bool bPersist);
bool						g_bGoBackLevel = false;
bool						g_bAvoidExtreme = false;

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
class CERestartLevel : public CChaosEffect
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
	void TransitionEffect() override;
	void OnEntitySpawned(CBaseEntity* pEntity) override;
	void ChangeEntity(CBaseEntity* pEntity);
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
class CE_NPC_Spam : public CChaosEffect
{
public:
	int PickRandomClass(int iNode);
	void SpawnNPC(Vector vPos, int iNPCType);
	void SpawnNPC(CAI_Node* pNode);
	std::vector<const char*> m_sSpawnNPCs;
	const char* m_sTargetname = "";
	const char* m_sWeapon = "";
};
class CE_NPC_SpamClose : public CE_NPC_Spam
{
public:
	void InitialSpawn();
};
class CE_NPC_SpamFar : public CE_NPC_Spam
{
public:
	void MaintainEffect() override;
};
class CEZombieSpamClose : public CE_NPC_SpamClose
{
public:
	void StartEffect() override;
};
class CEZombieSpamFar : public CE_NPC_SpamFar
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
	void FastThink() override;
	void FixVehicleGravity(const char* szClassname, float flTargetGravity, float flScale);
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
	void StartEffect() override;
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
enum GroundState
{
	GS_OFFGROUND,
	GS_ACTIVATE_EFFECT,
	GS_DONT_ACTIVATE
};
class CEFloorEffect : public CChaosEffect
{
public:
	void FastThink() override;
	bool m_iJumped = false;
	bool CheckStrike(const CTakeDamageInfo &info) override;
	GroundState GroundShouldActivateEffect();
};
class CEUseSpam : public CChaosEffect
{
public:
	void FastThink() override;
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
};
class CERandomCC : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
	void TransitionEffect() override;
	void RestoreEffect() override;
};
class CESecondarySpam : public CChaosEffect
{
public:
	void MaintainEffect() override;
};
class CESuitSwap : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEGiveAllRPG : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CEChangePitch : public CChaosEffect
{
public:
	void MaintainEffect() override;
	void StopEffect() override;
};
class CELogicExplode : public CChaosEffect
{
public:
	void StartEffect() override;
};
class CECameraTextures : public CChaosEffect
{
public:
	void StartEffect() override;
	void TransitionEffect() override;
	void StopEffect() override;
};
class CECameraGravity : public CChaosEffect
{
public:
	void FastThink() override;
	void StopEffect() override;
};
class CEHL1Phys : public CChaosEffect
{
public:
	void StartEffect() override;
	void OnEntitySpawned(CBaseEntity* pEntity) override;
	void ChangeEntity(CBaseEntity* pEntity);
	void RevertEntity(CBaseEntity* pEntity);
	void StopEffect() override;
	void TransitionEffect() override;
};
class CEDVDCrosshair : public CChaosEffect
{
public:
	void StartEffect() override;
	void StopEffect() override;
};
class CECopSpam : public CE_NPC_SpamFar
{
public:
	void StartEffect() override;
};
class CEScannerSpam : public CE_NPC_SpamClose
{
public:
	void StartEffect() override;
};
#endif