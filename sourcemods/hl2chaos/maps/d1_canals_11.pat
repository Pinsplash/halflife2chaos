"Entities"
{
	"replace_entity"
	{
	"origin" "6344 4856 -864.176"
	"spawnflags" "0"
	"targetname" "logic_labVortBeam_Off"
	"StartDisabled" "0"
	"classname" "logic_relay"
	"OnTrigger" "Sprite_labVort_Beam1,Kill,,0.2,-1"
	"OnTrigger" "beam_labvortCharge,Kill,,0.1,-1"
	"OnTrigger" "Sprite_labVort_Beam2,Kill,,0,-1"
	"OnTrigger" "global_newgame_spawner_airboat,Unlock,,0.3,-1"
	"OnTrigger" "mark_vort_Lookat_Work,Kill,,5,-1"
	"hammerid" "2198590"
	"OnTrigger" "achievement_get_airboat_gun,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "6220.7 5179.89 -887"
	"targetname" "achievement_get_airboat_gun"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_GET_AIRBOATGUN"
	"classname" "logic_achievement"
	"hammerid" "2507484"
	}
}