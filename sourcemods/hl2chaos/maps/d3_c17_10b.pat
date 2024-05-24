"Entities"
{
	"replace_entity"
	{
	"origin" "3096 -696 560"
	"targetname" "switches_counter"
	"startvalue" "0"
	"min" "0"
	"max" "3"
	"classname" "math_counter"
	"hammerid" "94751"
	"OnHitMax" "switches_success_relay,Trigger,,0,-1"
	"OnHitMax" "achievement,FireEvent,,5,-1"
	}
	"entity"
	{
	"origin" "2799 100.246 618.361"
	"targetname" "achievement"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_BEAT_SUPRESSIONDEVICE"
	"classname" "logic_achievement"
	"hammerid" "895175"
	}
}
