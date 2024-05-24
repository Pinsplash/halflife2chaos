"Entities"
{
	"replace_entity"
	{
	"origin" "2520.63 -4250.86 129.281"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_item_crate"
	"hammerid" "499015"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "2472 -4248 129.281"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_item_crate"
	"hammerid" "499019"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "2500 -4248 168"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_item_crate"
	"hammerid" "499023"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "2504.23 -4344 259.124"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_STREETWARFENCE"
	"classname" "logic_achievement"
	"hammerid" "843333"
	}
	"entity"
	{
	"origin" "2504.23 -4344 279.124"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "843335"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "2484 -4352 203.122"
	"texture" "decals/lambdaspray_2a"
	"classname" "infodecal"
	"hammerid" "539541"
	}
}