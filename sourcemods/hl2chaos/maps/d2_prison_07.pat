"Entities"
{
	"replace_entity"
	{
	"origin" "3932 -4016 -416"
	"targetname" "logic_room5_assault_finished"
	"StartDisabled" "1"
	"spawnflags" "0"
	"classname" "logic_relay"
	"hammerid" "200587"
	"OnTrigger" "logic_alyxdrop,Trigger,,3,1"
	"OnTrigger" "mic_alyx_2,Disable,,0,-1"
	"OnTrigger" "achievement_turret_standoff,FireEvent,,1,-1"
	}
	"entity"
	{
	"origin" "4225.22 -3977 -491.615"
	"targetname" "achievement_turret_standoff"
	"StartDisabled" "0"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_BEAT_TURRETSTANDOFF2"
	"classname" "logic_achievement"
	"hammerid" "766222"
	}
}