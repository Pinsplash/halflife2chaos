"Entities"
{
	entity
	{
		"id" "1063408"
		"classname" "point_teleport"
		"angles" "0 0 0"
		"spawnflags" "0"
		"target" "!player"
		"targetname" "teleport_to_test_chamber"
		"origin" "-11537.7 -11060.7 -1475"
		editor
		{
			"color" "220 30 220"
			"visgroupshown" "1"
			"visgroupautoshown" "1"
			"logicalpos" "[0 0]"
		}
	}
	entity
	{
		"id" "1063418"
		"classname" "logic_auto"
		"spawnflags" "1"
		connections
		{
			"OnNewGame" "teleport_to_test_chamber,Teleport,,0.5,-1"
			"OnNewGame" "teleport_to_citadel,Teleport,,1,-1"
			"OnNewGame" "teleport_to_train,Teleport,,1.5,-1"
			"OnNewGame" "teleport_to_trainstation,Teleport,,2,-1"
			"OnNewGame" "teleport_to_start,Teleport,,2.5,-1"
		}
		"origin" "-14080 -13952 0"
		editor
		{
			"color" "220 30 220"
			"visgroupshown" "1"
			"visgroupautoshown" "1"
			"logicalpos" "[0 500]"
		}
	}
	entity
	{
		"id" "1063428"
		"classname" "point_teleport"
		"angles" "0 90 0"
		"spawnflags" "0"
		"target" "!player"
		"targetname" "teleport_to_citadel"
		"origin" "-10584.9 -15068 -1660"
		editor
		{
			"color" "220 30 220"
			"visgroupshown" "1"
			"visgroupautoshown" "1"
			"logicalpos" "[0 0]"
		}
	}
	entity
	{
		"id" "1063432"
		"classname" "point_teleport"
		"angles" "0 0 0"
		"spawnflags" "0"
		"target" "!player"
		"targetname" "teleport_to_train"
		"origin" "-6488.38 -6014.77 24"
		editor
		{
			"color" "220 30 220"
			"visgroupshown" "1"
			"visgroupautoshown" "1"
			"logicalpos" "[0 0]"
		}
	}
	entity
	{
		"id" "1063436"
		"classname" "point_teleport"
		"angles" "0 0 0"
		"spawnflags" "0"
		"target" "!player"
		"targetname" "teleport_to_trainstation"
		"origin" "-6879.26 -2131.11 -24"
		editor
		{
			"color" "220 30 220"
			"visgroupshown" "1"
			"visgroupautoshown" "1"
			"logicalpos" "[0 0]"
		}
	}
	entity
	{
		"id" "1063444"
		"classname" "point_teleport"
		"angles" "0 90 0"
		"spawnflags" "0"
		"target" "!player"
		"targetname" "teleport_to_start"
		"origin" "-14576 -14208 -1292"
		editor
		{
			"color" "220 30 220"
			"visgroupshown" "1"
			"visgroupautoshown" "1"
			"logicalpos" "[0 0]"
		}
	}
	
	"replace_entity"
	{
		"id" "809723"
		"classname" "logic_choreographed_scene"
		"busyactor" "1"
		"SceneFile" "scenes/trainyard/security_intro01.vcd"
		"target1" "barneyroom_door_cop_1"
		"targetname" "security_intro_01"
		connections
		{
			"OnCompletion" "security_intro_01b,Start,,0,-1"
			"OnTrigger3" "barney_door_2_slot,SetAnimation,close,1,-1"
			"OnTrigger3" "barney_door_2_slot,SetAnimation,open,0,-1"
			"OnTrigger2" "barney_ss_2,BeginSequence,,0,-1"
			"OnTrigger1" "barney_door_2,Open,,1,-1"
			"OnTrigger1" "barney_door_2,Unlock,,0,-1"
			"OnStart" "barneyroom_door_cop_1,StartScripting,,0,-1"
			"OnTrigger1" "logic_comblock_1,Trigger,,0,-1"
		}
		"origin" "-3600 -392 72"
		editor
		{
			"color" "0 0 255"
			"visgroupshown" "1"
			"visgroupautoshown" "1"
			"logicalpos" "[9500 10000]"
		}
	}
}