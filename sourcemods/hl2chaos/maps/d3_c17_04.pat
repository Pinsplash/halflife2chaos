"Entities"
{
	"replace_entity"
	{
	"origin" "-1035.83 -4878.15 325.439"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"hammerid" "258300"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-1042.92 -4878.49 325.439"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"hammerid" "258304"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-1042.03 -4890.63 322.69"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_ammo_smg1_grenade"
	"hammerid" "258306"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-1041.09 -4917.5 321.002"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_battery"
	"hammerid" "258308"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "-1050.57 -4873 350.464"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_STREETWARSHACK"
	"classname" "logic_achievement"
	"hammerid" "471622"
	}
	"entity"
	{
	"origin" "-1050.57 -4873 370.464"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "471624"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
}
