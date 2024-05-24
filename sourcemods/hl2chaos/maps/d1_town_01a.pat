"Entities"
{
	"replace_entity"
	{
	"origin" "101.995 187.048 -3298.56"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_frag"
	"hammerid" "2387325"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "101.9 213.809 -3303.01"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "2387327"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"entity"
	{
	"origin" "92.5311 144 -3277.51"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_RAVENHOLMATTIC"
	"classname" "logic_achievement"
	"hammerid" "2511613"
	}
	"entity"
	{
	"origin" "92.5311 144 -3257.51"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "2511615"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
}
