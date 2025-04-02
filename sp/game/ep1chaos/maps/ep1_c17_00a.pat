"Entities"
{
	"entity"
	{
		"origin" "4418.94 3518.75 417"
		"targetname" "achiev_elevator"
		"AchievementEvent" "ACHIEVEMENT_EVENT_EP1_BEAT_GARAGEELEVATORSTANDOFF"
		"classname" "logic_achievement"
		"hammerid" "1881423"
	}
	
	"replace_entity"
	{
		"origin" "4583 3444 535.298"
		"onplayerdeath" "1"
		"busyactor" "1"
		"SceneFile" "scenes/episode_1/al_c17_00_elev_arrive_02.vcd"
		"targetname" "lcs_elevator_05a"
		"classname" "logic_choreographed_scene"
		"OnStart" "alyx,StartScripting,,0,-1"
		"OnTrigger2" "ss_alyx_elevator_lean,BeginSequence,,0.01,-1"
		"OnTrigger2" "elevator_shake_move_1,StartShake,,0,-1"
		"OnTrigger2" "autosave_01,Save,,0.8,-1"
		"OnTrigger2" "alyx_follow,Deactivate,,0,-1"
		"OnTrigger2" "alyx,StartScripting,,0,-1"
		"OnTrigger2" "elevator_garage,StartForward,,0.75,-1"
		"OnTrigger3" "lcs_elevator_05a,Pause,,0,-1"
		"OnTrigger4" "lcs_elevator_05a,Pause,,0,-1"
		"OnCompletion" "ss_alyx_elevator_lean_exit,BeginSequence,,0.01,-1"
		"OnCompletion" "ss_alyx_elevator_lean,CancelSequence,,0,-1"
		"OnTrigger3" "//lcs_elevator_10,Start,,0,-1"
		"OnTrigger2" "ep_song9,FadeOut,3,0.75,-1"
		"OnTrigger2" "achiev_elevator,FireEvent,,5.5,-1"
	}
}

