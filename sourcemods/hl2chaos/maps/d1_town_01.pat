"Entities"
{
	"replace_entity"
	{
	"world_maxs" "5200 1280 -2048"
	"world_mins" "-64 -3648 -4160"
	"maxpropscreenwidth" "-1"
	"chaptertitle" "CHAPTER6_TITLE"
	"MaxRange" "4096"
	"sounds" "1"
	"skyname" "sky_day01_09_hdr"
	"classname" "worldspawn"
	}
	"replace_entity"
	{
	"origin" "664.614 -1639.42 -3642.56"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_ammo_smg1_grenade"
	"hammerid" "2297855"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "661.247 -1618.52 -3642.56"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"hammerid" "2297857"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "806.573 1094.32 -3642.56"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"hammerid" "2521911"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "801.288 1114.6 -3647.01"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "2521913"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "672.486 -1628.24 -3646.69"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_battery"
	"hammerid" "2548938"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "803.71 1076.85 -3646.69"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_battery"
	"hammerid" "2611390"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "744 -1608 -3568.64"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_GENERATORS"
	"classname" "logic_achievement"
	"hammerid" "2861487"
	}
	"entity"
	{
	"origin" "744 -1608 -3548.64"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "2861489"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "784 1097.55 -3595.47"
	"targetname" "achievement_cache_2"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_CARCRUSHERARENA"
	"classname" "logic_achievement"
	"hammerid" "2865746"
	}
	"entity"
	{
	"origin" "784 1097.55 -3575.47"
	"targetname" "relay_achievement_cache_2"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "2865748"
	"OnTrigger" "achievement_cache_2,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "4498.45 -2808 -3690.6"
	"targetname" "achievement_gravgun_only"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_BEAT_RAVENHOLM_NOWEAPONS_START"
	"classname" "logic_achievement"
	"hammerid" "2883740"
	}
	"entity"
	{
	"origin" "4497 -2807 -3672"
	"spawnflags" "1"
	"classname" "logic_auto"
	"hammerid" "2883774"
	"OnMapSpawn" "achievement_gravgun_only,FireEvent,,0,-1"
	}
}
