"Entities"
{
	"replace_entity"
	{
	"origin" "3619.91 -1805.09 -318.561"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 90"
	"classname" "weapon_frag"
	"hammerid" "315035"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "3643 -1813 -318"
	"spawnflags" "0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 90"
	"classname" "weapon_ar2"
	"hammerid" "315048"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "3626 -1832 -318.561"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 90"
	"classname" "weapon_frag"
	"hammerid" "378584"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "3602.93 -1752.39 -322.746"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "404522"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "3612.8 -1774.69 -322.746"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "404524"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"origin" "3586.8 -1758.68 -322.746"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "item_healthkit"
	"hammerid" "404526"
	"OnCacheInteraction" "relay_achievement_cache_1,Trigger,,0,-1"
	}
	"replace_entity"
	{
	"model" "*71"
	"StartDisabled" "0"
	"spawnflags" "1"
	"origin" "2560 4904 -168"
	"classname" "trigger_once"
	"hammerid" "486329"
	"OnTrigger" "slime_timer,Disable,,0,-1"
	"OnTrigger" "achievement,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "2505 4420.03 -91.8923"
	"targetname" "achievement"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_BEAT_TOXICTUNNEL"
	"classname" "logic_achievement"
	"hammerid" "595672"
	}
	"entity"
	{
	"origin" "3781.96 -1785.83 -243.223"
	"targetname" "achievement_cache_1"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_FREEWAYTUNNEL"
	"classname" "logic_achievement"
	"hammerid" "607449"
	}
	"entity"
	{
	"origin" "3781.96 -1785.83 -223.223"
	"targetname" "relay_achievement_cache_1"
	"StartDisabled" "0"
	"spawnflags" "1"
	"classname" "logic_relay"
	"hammerid" "607451"
	"OnTrigger" "achievement_cache_1,FireEvent,,0,-1"
	}
}