"Entities"
{
	"replace_entity"
	{
	"origin" "-9616.43 -3497.92 600"
	"targetname" "crowbar_weapon"
	"spawnflags" "0"
	"fadescale" "1"
	"fademindist" "-1"
	"angles" "0 0 0"
	"classname" "weapon_crowbar"
	"hammerid" "643434"
	"OnPlayerPickup" "achievement_get_crowbar,FireEvent,,0,-1"
	"OnPlayerPickup" "trigger_attack,Enable,,0,-1"
	}
	"entity"
	{
	"origin" "-9616 -3514 600"
	"targetname" "achievement_get_crowbar"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_GET_CROWBAR"
	"classname" "logic_achievement"
	"hammerid" "774473"
	}
}