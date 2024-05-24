"Entities"
{
	"replace_entity"
	{
	"origin" "-142.201 -478.451 -982.132"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "2511062"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-150.828 -496.686 -985.265"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "2511064"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-133.502 -407.484 -1008.29"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_ammo_smg1"
	"hammerid" "2511074"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-115.174 -513.954 -976.49"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_battery"
	"hammerid" "2517842"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-127.672 -517.563 -974.306"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_battery"
	"hammerid" "2517844"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-125 -495 -969"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_ammo_smg1_grenade"
	"hammerid" "2525497"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "-260.666 -440 -920.368"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_STEAMPIPE"
	"classname" "logic_achievement"
	"hammerid" "2804296"
	}
	"entity"
	{
	"origin" "-260.666 -440 -900.368"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "2804298"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
}
