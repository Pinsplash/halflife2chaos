"Entities"
{
	"replace_entity"
	{
	"origin" "-11406.5 4242.82 1045.44"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"hammerid" "44242"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-11410.9 4234.54 1045.44"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"hammerid" "44244"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-11407.3 4272.23 1044.41"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 90 0"
	"classname" "weapon_357"
	"hammerid" "44246"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"model" "*45"
	"StartDisabled" "0"
	"spawnflags" "1"
	"origin" "-32 7328 1120.23"
	"classname" "trigger_once"
	"hammerid" "280328"
	"OnTrigger" "gman_aisc,Enable,,0,-1"
	"OnTrigger" "gman_sighting_achievement_aisc,Enable,,0,-1"
	}
	"entity"
	{
	"origin" "-11402.3 4307.08 1048"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_MINETUNNELEXIT"
	"classname" "logic_achievement"
	"hammerid" "342507"
	}
	"entity"
	{
	"origin" "-11402.3 4307.08 1068"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "342509"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "44.1073 7511.19 1160"
	"targetname" "achievement_gman_sighting_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_GMAN_TRAINTRACKS"
	"classname" "logic_achievement"
	"hammerid" "345205"
	}
	"entity"
	{
	"origin" "44.1073 7511.19 1180"
	"targetname" "relay_achievement_gman_sighting_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "345207"
	"OnTrigger" "achievement_gman_sighting_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "141.032 7511.82 1161"
	"targetname" "gman_sighting_achievement_aisc"
	"StartDisabled" "1"
	"spawnflags" "0"
	"ScriptStatus" "2"
	"RequiredTime" "2"
	"PlayerTargetProximity" "0"
	"PlayerTargetLOS" "2"
	"PlayerTargetFOVTrueCone" "1"
	"PlayerTargetFOV" "360"
	"PlayerInVehicle" "2"
	"PlayerBlockingActor" "2"
	"PlayerActorLOS" "1"
	"PlayerActorFOVTrueCone" "1"
	"PlayerActorFOV" "15"
	"MinimumState" "1"
	"MaximumState" "3"
	"ActorSeeTarget" "2"
	"ActorSeePlayer" "2"
	"ActorInVehicle" "2"
	"ActorInPVS" "2"
	"Actor" "gman"
	"classname" "ai_script_conditions"
	"hammerid" "345209"
	"OnConditionsSatisfied" "relay_achievement_gman_sighting_1,Trigger,,0,-1"
	"OnConditionsSatisfied" "gman_sighting_achievement_aisc,Disable,,0.5,-1"
	}
	"replace_entity"
	{
	"origin" "2687.74 7316.12 897"
	"spawnflags" "608"
	"m_iszEntity" "gman"
	"targetname" "gman_ss_1"
	"m_fMoveTo" "1"
	"angles" "0 0 0"
	"classname" "scripted_sequence"
	"OnBeginSequence" "gman,Kill,,0,-1"
	"OnBeginSequence" "gman_sighting_achievement_aisc,Enable,,0,-1"
	}
	"entity"
	{
	"model" "*47"
	"StartDisabled" "0"
	"spawnflags" "1"
	"origin" "-11054 4352 958"
	"classname" "trigger_once"
	"hammerid" "350986"
	"OnTrigger" "achievement_gravgun_only,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "-11164.6 4420.47 957.451"
	"targetname" "achievement_gravgun_only"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_BEAT_RAVENHOLM_NOWEAPONS_END"
	"classname" "logic_achievement"
	"hammerid" "351060"
	}
}