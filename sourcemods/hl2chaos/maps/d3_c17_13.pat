"Entities"
{
	"replace_entity"
	{
	"origin" "5031 284 96"
	"targetname" "allstridersdead_counter"
	"startvalue" "0"
	"min" "0"
	"max" "2"
	"classname" "math_counter"
	"hammerid" "509166"
	"OnHitMax" "endstreet_citizens_template,ForceSpawn,,0,-1"
	"OnHitMax" "allstridersdead_song,PlaySound,,0,-1"
	"OnHitMax" "stridersnotdead_brush,Kill,,0,-1"
	"OnHitMax" "endstreet_explosion_conditions,Enable,,10,-1"
	"OnHitMax" "rooftop_soldier_balcony_maker,Kill,,0,-1"
	"OnHitMax" "rooftop_soldier_1_maker,Kill,,0,-1"
	"OnHitMax" "rooftop_soldier_2_maker,Kill,,0,-1"
	"OnHitMax" "rooftop_trigger,Kill,,0,-1"
	"OnHitMax" "allstridersdead_save,Save,,3,-1"
	"OnHitMax" "citizen_horse_hint_end_maker_timer,Enable,,0,-1"
	"OnHitMax" "horse_hint_end_lcs_trigger,Enable,,5,-1"
	"OnHitMax" "achievement,FireEvent,,0,-1"
	}
	"entity"
	{
	"origin" "5070.65 231 80.9337"
	"targetname" "achievement"
	"AchievementEvent" "ACHIEVEMENT_EVENT_HL2_BEAT_C1713STRIDERSTANDOFF"
	"classname" "logic_achievement"
	"hammerid" "818782"
	}
}