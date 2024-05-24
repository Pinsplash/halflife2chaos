"Entities"
{
	"replace_entity"
	{
	"origin" "11194.6 399.071 -383.014"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "11207.4 421.633 -383.014"
	"angles" "0 23 0"
	"classname" "item_healthkit"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "11223.1 342.738 -382.998"
	"angles" "0 0 0"
	"classname" "item_battery"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "10889.1 15.1108 -378.561"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "10897.7 34.0481 -379.561"
	"angles" "90 30 0"
	"classname" "weapon_frag"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "11856 -1888 -192"
	"fadescale" "1"
	"spawnflags" "256"
	"fademindist" "-1"
	"inertiaScale" "1.0"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_ammo_pistol"
	"angles" "0 66 0"
	"classname" "item_item_crate"
	"hammerid" "1441707"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "11920 -1888 -192"
	"fadescale" "1"
	"spawnflags" "256"
	"fademindist" "-1"
	"inertiaScale" "1.0"
	"physdamagescale" "0.1"
	"ItemCount" "1"
	"ItemClass" "item_dynamic_resupply"
	"angles" "0 94 0"
	"classname" "item_item_crate"
	"hammerid" "1441735"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "11164 9 -309.72"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "1643790"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "11164 9 -329.72"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_CANALWALL"
	"classname" "logic_achievement"
	"hammerid" "1643792"
	}
	"entity"
	{
	"origin" "11996 -1911 -133.72"
	"targetname" "relay_achievement_cache_2"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "1644086"
	"OnTrigger" "achievement_cache_2,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "11996 -1911 -153.72"
	"targetname" "achievement_cache_2"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_BANKEDCANAL"
	"classname" "logic_achievement"
	"hammerid" "1644088"
	}
	"replace_entity"
	{
	"model" "*74"
	"targetname" "gate4_wheel"
	"startposition" "0"
	"startdirection" "Forward"
	"speed" "64"
	"spawnflags" "1088"
	"sounds" "24"
	"solidbsp" "1"
	"returnspeed" "30"
	"rendermode" "0"
	"renderfx" "0"
	"rendercolor" "255 255 255"
	"renderamt" "255"
	"origin" "4606.46 9647.34 -63.84"
	"distance" "360"
	"disablereceiveshadows" "0"
	"angles" "0 0 0"
	"classname" "momentary_rot_button"
	"hammerid" "1561206"
	"Position" "canals_brush_boathouselift1,SetPosition,,0,-1"
	"OnFullyClosed" "gate4_wheel,Lock,,0,1"
	"OnPressed" "prop_vehicle_airboat,wake,,0,-1"
	}
}