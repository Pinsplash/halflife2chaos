"Entities"
{
	"replace_entity"
	{
	"origin" "517.731 10936.7 505"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "1000"
	"fademaxdist" "1200"
	"disableshadows" "1"
	"angles" "0 244.5 0"
	"classname" "item_item_crate"
	"hammerid" "727346"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "513.731 10940.7 537"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "1000"
	"fademaxdist" "1200"
	"disableshadows" "1"
	"angles" "0 270 0"
	"classname" "item_item_crate"
	"hammerid" "854596"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"model" "*40"
	"StartDisabled" "0"
	"spawnflags" "1"
	"origin" "428.23 5100.23 464"
	"classname" "trigger_once"
	"hammerid" "1371450"
	"OnTrigger" "relay_ss_gman_zombies,Trigger,,0,-1"
	"OnTrigger" "scriptcond_gman,Enable,,0,-1"
	"OnTrigger" "gman_sighting_achievement_aisc,Enable,,0,-1"
	}
	"entity"
	{
	"origin" "456 10968.6 588.836"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_CHANNELSPLIT"
	"classname" "logic_achievement"
	"hammerid" "1850356"
	}
	"entity"
	{
	"origin" "456 10968.6 608.836"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "1850358"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "5.33454 7735.37 727.538"
	"targetname" "achievement_gman_sighting_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_GMAN_CATWALK"
	"classname" "logic_achievement"
	"hammerid" "1852893"
	}
	"entity"
	{
	"origin" "5.33454 7735.37 747.538"
	"targetname" "relay_achievement_gman_sighting_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "1852895"
	"OnTrigger" "achievement_gman_sighting_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "48 7736 736"
	"targetname" "gman_sighting_achievement_aisc"
	"StartDisabled" "1"
	"spawnflags" "0"
	"ScriptStatus" "2"
	"RequiredTime" ".5"
	"PlayerTargetProximity" "0"
	"PlayerTargetLOS" "2"
	"PlayerTargetFOVTrueCone" "1"
	"PlayerTargetFOV" "360"
	"PlayerInVehicle" "2"
	"PlayerBlockingActor" "2"
	"PlayerActorLOS" "2"
	"PlayerActorFOVTrueCone" "1"
	"PlayerActorFOV" "15"
	"MinimumState" "1"
	"MaximumState" "3"
	"ActorSeeTarget" "2"
	"ActorSeePlayer" "2"
	"ActorInVehicle" "2"
	"ActorInPVS" "2"
	"Actor" "gman_zombies"
	"classname" "ai_script_conditions"
	"hammerid" "1852897"
	"OnConditionsSatisfied" "relay_achievement_gman_sighting_1,Trigger,,0,-1"
	"OnConditionsSatisfied" "gman_sighting_achievement_aisc,Disable,,0.5,-1"
	}
}