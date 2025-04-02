"Entities"
{
	"replace_entity"
	{
		"origin" "3864 4702 -6710"
		"targetname" "alyx_exit_elevator_vcd_1"
		"SceneFile" "scenes/episode_1/citadel/al_lift_close.vcd"
		"busyactor" "1"
		"classname" "logic_choreographed_scene"
		"hammerid" "651016"
		"OnCompletion" "achiev_elevator,FireEvent,,0,-1"
	}

	"entity"
	{
		"origin" "3880 4702 -6711"
		"targetname" "achiev_elevator"
		"AchievementEvent" "ACHIEVEMENT_EVENT_EP1_BEAT_MAINELEVATOR"
		"classname" "logic_achievement"
		"hammerid" "845412"
	}
}