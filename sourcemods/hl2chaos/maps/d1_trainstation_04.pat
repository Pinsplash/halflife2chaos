"Entities"
{
	"replace_entity"
	{
	"model" "*39"
	"StartDisabled" "0"
	"spawnflags" "1"
	"origin" "-6974 -4250 584"
	"classname" "trigger_once"
	"hammerid" "503371"
	"OnTrigger" "!player,SetHealth,100,0,-1"
	"OnTrigger" "npc_windowscanner,SetFollowTarget,target_scanner_inspection_final,0,-1"
	"OnTrigger" "gordon_criminal_global,TurnOn,,0,-1"
	"OnTrigger" "achivement_escape_raid,FireEvent,,1,-1"
	}
	"entity"
	{
	"origin" "-7017.18 -4111.54 528"
	"targetname" "achivement_escape_raid"
	"StartDisabled" "0"
	"spawnflags" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_ESCAPE_APARTMENTRAID"
	"classname" "logic_achievement"
	"hammerid" "712762"
	}
}
