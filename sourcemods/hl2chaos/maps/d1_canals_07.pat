"Entities"
{
	"replace_entity"
	{
	"origin" "-4432.39 -14872.3 -882"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_battery"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"CrateType" "0"
	"angles" "0 275 0"
	"classname" "item_item_crate"
	"hammerid" "1399085"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-4390.48 -14861.9 -882.998"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "2"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"CrateType" "0"
	"angles" "0 30 0"
	"classname" "item_item_crate"
	"hammerid" "1399097"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "-4755.3 -14309.4 -871.627"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_BELOWAPCS"
	"classname" "logic_achievement"
	"hammerid" "2097133"
	}
	"entity"
	{
	"origin" "-4755.3 -14309.4 -851.627"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "2097135"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
}