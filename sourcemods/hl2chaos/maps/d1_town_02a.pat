"Entities"
{
	"replace_entity"
	{
	"model" "*27"
	"origin" "-7504 -304 -3344"
	"spawnflags" "1"
	"StartDisabled" "0"
	"classname" "trigger_once"
	"OnTrigger" "monk,StartScripting,,0,-1"
	"OnTrigger" "monk_health_timer,Enable,,0,-1"
	"OnTrigger" "graveyard_exitgate_monk_assault,Activate,,0.2,-1"
	"OnTrigger" "graveyard_monk_scene_exit_nag1,Kill,,0,-1"
	"OnTrigger" "graveyard_exitgate_monk_assault,BeginAssault,,0.3,-1"
	"OnTrigger" "graveyard_exit_monk_seq,Kill,,0.1,-1"
	"OnTrigger" "graveyard_exit_monk_nag_timer,Kill,,0,-1"
	"OnTrigger" "g_dead_fade,Kill,,0,-1"
	"OnTrigger" "g_dead_text,Kill,,0,-1"
	"OnTrigger" "graveyard_exit_monk_seq,CancelSequence,,0,-1"
	"OnTrigger" "graveyard_exit_momentary_wheel,Unlock,,0,-1"
	"OnTrigger" "graveyard_exit_wheel_wav2,StopSound,,0,-1"
	"OnTrigger" "graveyard_exit_wheel_wav3,PlaySound,,0,-1"
	"OnTrigger" "graveyard_exit_momentary_wheel,SetPosition,0,1,-1"
	"OnTrigger" "graveyard_monk_scene_exit_nag3,Kill,,0,-1"
	"OnTrigger" "graveyard_monk_scene_exit_nag2,Kill,,0,-1"
	"OnTrigger" "achievement,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "-7500.36 -328.111 -3271"
	"targetname" "achievement"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_BEAT_CEMETERY"
	"classname" "logic_achievement"
	"hammerid" "2790029"
	}
}
 