"Entities"
{
	"replace_entity"
	{
	"origin" "6959.83 -9281.54 -295"
	"targetname" "tower1_break_relay"
	"classname" "logic_relay"
	"hammerid" "1391659"
	"OnTrigger" "tower1_boards,EnableMotion,,0,-1"
	"OnTrigger" "tower1_boards_breakable,EnableMotion,,0,-1"
	"OnTrigger" "tower1_boards_breakable,Break,,2.5,-1"
	"OnTrigger" "tower1_boards,Break,,2.5,-1"
	"OnTrigger" "relay_achievement_cache_1,Trigger,,1,-1"
	}
	"replace_entity"
	{
	"origin" "7002 -9312 3"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 75 0"
	"classname" "item_item_crate"
	"hammerid" "1392721"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "6960 -9312 3"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 103 0"
	"classname" "item_item_crate"
	"hammerid" "1392751"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "6966.35 -9336 -197.706"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "1497922"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "6966.35 -9336 -217.706"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_TUNNELPLATFORMS"
	"classname" "logic_achievement"
	"hammerid" "1497924"
	}
}