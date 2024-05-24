"Entities"
{
	"entity"
	{
	"origin" "151.263 4321.08 -1400"
	"targetname" "achievement_lambda_loc"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_LAMDACACHE_BMEDOCK"
	"classname" "logic_achievement"
	"hammerid" "2605848"
	}
	"entity"
	{
	"model" "*130"
	"StartDisabled" "0"
	"spawnflags" "1"
	"origin" "-384 4168 -1368"
	"classname" "trigger_once"
	"hammerid" "2605866"
	"OnTrigger" "achievement_lambda_loc,FireEvent,,0,-1"
	}
}