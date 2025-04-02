"Entities"
{
	"replace_entity"
	{
		"origin" "1760 8312 -2550"
		"SceneFile" "scenes/episode_1/c17/al_gship_holyterror.vcd"
		"targetname" "lcs_al_openseason"
		"busyactor" "1"
		"classname" "logic_choreographed_scene"
		"OnCompletion" "lcs_gship_crowbar01,Start,,0.25,-1"
		"OnCompletion" "achiev_gunship,FireEvent,,0,-1"
	}

	"entity"
	{
	"origin" "1782.57 8348.24 -2551"
	"targetname" "achiev_gunship"
	"AchievementEvent" "ACHIEVEMENT_EVENT_EP1_BEAT_HOSPITALATTICGUNSHIP"
	"classname" "logic_achievement"
	"hammerid" "2284031"
	}
}