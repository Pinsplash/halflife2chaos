"Entities"
{
	"replace_entity"
	{
	"origin" "9641 9674.03 -650.42"
	"targetname" "counter_everyone_in_place_for_barney_goodbye"
	"max" "5"
	"classname" "math_counter"
	"hammerid" "1168137"
	"OnHitMax" "lcs_rearsoldierslock,Start,,0,1"
	"OnHitMax" "achiev_cit_end,FireEvent,,0,1"
	}

	"replace_entity"
	{
	"model" "*84"
	"StartDisabled" "0"
	"spawnflags" "1"
	"origin" "10176 11968 -641"
	"classname" "trigger_once"
	"hammerid" "1646649"
	"OnTrigger" "ss_barney_stand,BeginSequence,,0,-1"
	"OnTrigger" "lcs_al_c17_05_barneyoverhere,Start,,1,-1"
	"OnTrigger" "ss_barney_stand,Kill,,1,-1"
	"OnTrigger" "pen_camera_1,Disable,,0,-1"
	"OnTrigger" "achiev_cit_start,FireEvent,,0,-1"
	}

	"entity"
	{
	"origin" "10088 12344 -622.896"
	"targetname" "achiev_cit_start"
	"AchievementEvent" "ACHIEVEMENT_EVENT_EP1_BEAT_CITIZENESCORT_NOCITIZENDEATHS_START"
	"classname" "logic_achievement"
	"hammerid" "1969696"
	}
	
	"entity"
	{
	"origin" "10104 12344 -622.896"
	"targetname" "achiev_cit_end"
	"AchievementEvent" "ACHIEVEMENT_EVENT_EP1_BEAT_CITIZENESCORT_NOCITIZENDEATHS_END"
	"classname" "logic_achievement"
	"hammerid" "1969698"
	}
}