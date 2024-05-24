"Entities"
{
	"replace_entity"
	{
	"origin" "10294.6 -7432.69 -315.719"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "775095"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "10294 -7415.39 -315.719"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "775097"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "10317.5 -7414.08 -315.724"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_battery"
	"hammerid" "775099"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "10317.1 -7431.22 -315.724"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_battery"
	"hammerid" "775101"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "10308.8 -7396.21 -315.724"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_ammo_357"
	"hammerid" "775103"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-6833.27 704.689 -591.696"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 24.5 0"
	"classname" "item_item_crate"
	"hammerid" "523417"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-6833.27 640.689 -591.696"
	"spawnflags" "256"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"inertiaScale" "1.0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 291 0"
	"classname" "item_item_crate"
	"hammerid" "523419"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "-6643.61 584 -503.521"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "1028799"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "-6643.61 584 -523.521"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_RAILWAYBRIDGE"
	"classname" "logic_achievement"
	"hammerid" "1028801"
	}
	"entity"
	{
	"origin" "10244.4 -7384 -318.719"
	"targetname" "relay_achievement_cache_2"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "1028863"
	"OnTrigger" "achievement_cache_2,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "10244.4 -7384 -338.719"
	"targetname" "achievement_cache_2"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_COUNTERWEIGHT"
	"classname" "logic_achievement"
	"hammerid" "1028865"
	}
}