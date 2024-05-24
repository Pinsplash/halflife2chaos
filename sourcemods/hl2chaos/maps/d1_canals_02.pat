"Entities"
{
	"replace_entity"
	{
	"origin" "-199.681 -976.443 -1070.72"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "2466119"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-231.044 -979.053 -1070.72"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "2466121"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "-218.307 -962.159 -1070.72"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_ammo_pistol"
	"hammerid" "2470099"
	"OnCacheInteraction" "relay_achievement_cache_2,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "-202.501 -952 -1031.22"
	"targetname" "achievement_cache_2"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_SEWERGRATE"
	"classname" "logic_achievement"
	"hammerid" "2510153"
	}
	"entity"
	{
	"origin" "-202.501 -952 -1011.22"
	"targetname" "relay_achievement_cache_2"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "2510155"
	"OnTrigger" "achievement_cache_2,FireEvent,,0,-1"
	}
}
