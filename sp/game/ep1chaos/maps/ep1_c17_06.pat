"Entities"
{
	"replace_entity"
	{
		"origin" "11856 8224 -678.687"
		"targetname" "lcs_al_finale_1"
		"SceneFile" "scenes/episode_1/c17/al_finale_01.vcd"
		"busyactor" "2"
		"classname" "logic_choreographed_scene"
		"hammerid" "1823992"
		"OnCompletion" "achievement_ep1_end,FireEvent,,2,-1"
	}

	"entity"
	{
		"origin" "-14536 -6232 -656"
		"targetname" "achievement_ep1_end"
		"StartDisabled" "0"
		"AchievementEvent" "ACHIEVEMENT_EVENT_EP1_BEAT_GAME"
		"classname" "logic_achievement"
		"hammerid" "1932796"
	}
	"replace_entity"
	{
	"origin" "12024 8285.62 -682.042"
	"targetname" "lcs_al_leavingOnTrain"
	"SceneFile" "scenes/episode_1/c17/al_finale_wediditg.vcd"
	"busyactor" "1"
	"classname" "logic_choreographed_scene"
	"hammerid" "1305563"
	}
}