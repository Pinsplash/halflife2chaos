# Version history

## v0.1.2

### General

* Transient effects will no longer show their remaining time.  
 If you want them to show despite technically not being necessary, you can use `chaos_alwaysShowEffectTime 1`.
* Effects can now be in more than one group.
* Effect groups are now user-modifiable. You can edit groups in `groups.cfg`.
* Fixed issue where the game could give the player a large amount of speed when teleported inside a prop.

### Game

* Vehicles now prefer to put the player below them instead of above them when exiting.  
 This prevents an issue with **Invert Gravity** on ep2_outland_12.
* The game no longer automatically resets on d2_coast_04 when the player or buggy falls into a bad place.  
 This prevents an issue with **Invert Gravity** on this map.

### Contexts

* Several contexts have been deleted and had their criteria assigned specifically to the effect they were made for, 
 as they were probably never going to be used by any other effects: 
 **Invert Gravity**, **Bullet Teleport**, **Shotgun**, **Quickclip On**, **Crane Spawn**, **No Long Map**, **Clone NPCs**, 
 and **NPC Teleport**.  
 The effects' underlying criteria remain the same except as noted.
* **No Lava** and **No Slow Physics** have been deleted because their purpose is now fulfilled by a group.
* **Pickups**:
  * Made the unsuitable map criterion specific to **Remove All Pickups**, so that **Water World** isn't pointlessly restricted on some maps.
* **Player Teleport**:
  * Added a few more maps to the list.
* **Quickclip Off**:
  * Made the unsuitable map criterion specific to **Enable Quickclip**, 
 so that **Floor Is Lava** isn't pointlessly restricted on some maps.
  * **Physics** has been removed and **Ran Out Of Glue** and **Pause Physics** now use their own separate map lists that cater to them individually.

### Groups

* **Superhot** has been split into two groups: one that includes **Superhot**, **Supercold**, and **Stop**, 
and one that includes **Superhot**, **Supercold**, and **Super Speed**. The goal of this is to make **Stop**
and **Super Speed** not mutually exclusive.
* **Barrel** has been split into two groups: 
one that includes **Double Barrel Shotgun** and **Black Hole**, and 
one that includes **Double Barrel Shotgun** and **Repulsive**. 
The goal of this is to make **Black Hole** and **Repulsive** not mutually exclusive.
* A new group has been created which includes **Slow Physics** and **Floor Is Lava**, 
which previously couldn't exist because **Slow Physics** was already in a group.
* The **Vision** group was removed.

### Effects

* **Go Back a Level**:
 Fixed being sent to d1_town_03 from d1_town_04 when you should be sent to d1_town_02a.
* **Spawn Random Vehicle** and **Noriko, No!**:
 Spawned cranes will no longer attempt to use the same magnet as another crane.
* **No Climbing**:
 Now prevents moving on ladders.
* **Remove All Pickups**:
 More maps have been added to the list of unsuitable maps.
* **Floor Is Lava**:
 No longer uses the *No Citadel* context as the effect's understanding of "floor" was improved in v0.1.1.
* **Teleporter Bullets**:
 No longer puts the shooting object slightly in the air when not needed.
* **World of Love**:
 NPCs can no longer be hurt when active. Previously, it was possible to still hurt enemy NPCs while they were being friendly.
* **Give All Weapons**:
 Now gives less ammo.
* **Orthographic Camera**:
 "Field of view" now wider.
* **Invert Gravity**:
 No longer restricted on d2_coast_04 and ep2_outland_12 as the issues with those maps have been fixed.

## v0.1.1

### Effects

* **NPCs Explode on Death**:
 Actually fixed crash when scanners died.
* **Floor Is Lava**:
 Improved understanding of what is and isn't the floor. Mainly, static props now count as not being part of the floor.

## v0.1.0

### General

* Effects are much less likely to bug out when loading saves or going to new levels.
* Added timers showing how long effects have left until they expire.
* Certain spawned objects can now persist through saves and carry their state over from when the load button was pressed. 
This means things like zombies spawned by **Left 4 Dead** are easier to fight as the dead zombies will stay dead, 
and wounded zombies will stay wounded, even if you die initially.
* Things from the _Episodes_ and _Lost Coast_ have been added. 
Episodic things will only appear in _Episode One/Two Chaos_. 
You may play the base HL2 campaign in _Episode One/Two Chaos_ if you wish, but expect issues.
* Time between new effects has been lowered from 30 seconds to 20 seconds.
* Effect duration has been lowered from 90 seconds to 80 seconds.
* Effects can now have individually modified lengths.
* Effects can now have individually modified probabilities. (Including being entirely disabled.)
* Effects that are already active cannot be chosen again. This existed in the first version but I don't think it was functional.
* Added effect groups, which prevent effects in the same group from being chosen if one from the group is already active. This existed in the first version but I don't think it was functional.
* Effects are now less likely to happen again after recently occurring.
* Due to being necessary for some effects, 
Chaos now operates at the correct real life speed, independent of `host_timescale`.
* If an effect causes the player to die too many times, it will be aborted. 
The number of times can be controlled with `chaos_strike_max`. Default 5.
* Added convars for controlling bar color, text color, and positioning of text.
* Text for effects can now fade to a different color as the effect gets closer to expiring.
* Text for effects now renders normally instead of additively to make it easier to read.
* Fixed top bar becoming thicker as it grew.
* The game now tries to get the player unstuck every time an effect starts. If you become stuck, try waiting for the next effect.
* The game now sets `sv_cheats` to 1 every time an effect starts as it would interfere with some effects.

* Created new algorithm for getting the player and other objects out of places where they're stuck.
  * Based off original algorithm used by noclip.
  * Tests for open space above, below, to the north, south, east, and west of the object and all combinations of those directions.
  * Avoids placing objects through walls.
  * Avoids placing objects above nodraw and skybox textures.
  * Optional failsafe to placing object at the nearest appropriate AI node.
  * Failsafe keeps the object in the same node zone if possible.
  * Recognizes quickclip and preserves it while still getting the player unstuck.
  * Forces player to crouch if necessary.

Since it wasn't tracked well before, here is a readout of all current effect **contexts**

* **Boat**: An airboat must be present in the map.  
 Used by **Lock Vehicles** and **Force In/Out Vehicle**.  
 If an effect uses both **Boat** and **Buggy**, only one of them is required.
* **Buggy**: A buggy or jalopy must be present in the map.  
 Used by **Lock Vehicles**, **Bumpy Road**, **Broken Brakes**, and **Force In/Out Vehicle**.
* **Water**: The map must have water.  
 Used by **Death Water**.
* **Physics**: The map must not be one where important things can become broken if their 
physics are messed with.  
 Used by **Ran Out Of Glue** and **Pause Physics**.
* **No Invuln**: Player must not be invulnerable.  
 Used by Grenade Guns, Funny Number, and Floor Is Lava.
* **Shotgun**: There must a shotgun, held or unheld, in the map.  
 Used by **Double Barrel Shotgun**.
* **Has Weapon**: Player must have at least one weapon.  
 Used by **Drop Weapons**, **Left 4 Dead**, **Annoying Alyx**, **Remove Random Weapon**, **No One Can 
  Reload**, and **Remove All Pickups**.
* **Quickclip Off**: Quickclip must be off and the map must not be one that can easily softlock if quickclip is on.  
 Used by **Enable Quickclip** and **Floor Is Lava**.
* **Quickclip On**: Quickclip must be on.  
 Used by **Disable Quickclip**.
* **No Cutscene**: Must not be on a map that's mostly cutscenes.  
 Used by **World of Hate**, **World of Fear**, and **You Teleport?**.
* **Pickups**: A pickup must exist on the map, and the map must not be one with pickups that are needed to progress or are very hard without pickups.  
 Used by **Remove All Pickups** and **Water World**.  
 This only looks for entities with classnames beginning with "it", so suit and health chargers count as pickups.  
 Weapon entities do not count. To clarify, **Remove All Pickups** does remove unheld weapons despite them not being counted by the context's criteria.
* **Invert Gravity**: The map must not be one that is known to have softlock issues with this effect.  
 Used by **Invert Gravity**.
* **Crane Spawn**: Map must generally be spacious enough to fit a crane.  
 Used by **Noriko, No!**.
* **Bullet Teleport**: The map must not be one that has certain NPCs which are vital to progressing.  
 Used by **Teleporter Bullets**.  
 NPCs on these maps are required to be killed in order to progress on the intended route, and **Teleporter Bullets** can cause them to teleport to places that make it difficult or impossible to find them.
* **No Citadel**: Map must not be one of the gameplay-focussed Citadel maps.  
 Used by **Drop Weapons**, **Remove Random Weapon**, **Remove All Pickups**, and **Floor Is Lava**.  
 **Floor Is Lava** currently allows all metal surfaces to be stepped on, and wouldn't make much sense in the Citadel.
* **Player Teleport**: The map must not be one where a softlock could occur if the player was teleported somewhere.  
 Used by **Teleport to Random Place**, **Force In/Out Vehicle**, and **Deja Vu?**.
* **No Vehicle**: Must not be an airboat, buggy, or jalopy in the map.  
 Used by **Pause Physics** and **Grabby**.
* **NPC Teleport**: The map must not be one there NPCs can still create issues even when teleporting in a manner more controlled than **Teleporter Bullets**.  
 Used by **You Teleport?**.
* **Clone NPCs**: Must not be on certain maps that rely on an NPC moving to a certain place.  
 Used by **Suppression Field Hiccup**. When multiple of an NPC are requested by a script, the game only chooses one of them to use at a time. The NPC that's chosen at any particular moment may be stuck elsewhere because it was not chosen to do some scripted movement that it would be expected to have done by now.
* **No Long Map**: Map cannot be one that takes "too long".  
 Used by **Go Back a Level**. If this effect would take you back to a "long map", it will just 
restart the current map instead.  
 A convar is planned to be introduced in later versions to control what counts as "too long".
* **No Slow Physics**: The physics timescale cannot be below 1.0. 
(So basically, it just avoids Pause Physics and Slow Physics, mainly intended for the 2nd one.)  
 Used by **Floor Is Lava**.
* **No Lava**: **Floor Is Lava** must not be on.  
 Used by **Slow Physics**.  
 **No Slow Physics** and **No Lava exist** because physics objects can interact weirdly with the player if the physics timescale is lower than 1.  
 **Floor Is Lava** and **Slow Physics** could not be made mutually exclusive via **groups** because 
both were already in groups, and effects currently cannot be in multiple groups.

Since it wasn't tracked well before, here is a readout of all current effect **groups**

* **Gravity**: Used by **Zero Gravity**, **Super Gravity**, and **Low Gravity**.
* **Physics Speed**: Used by **Pause Physics**, **Fast Physics**, and **Slow Physics**.
* **NPC Relationships**: Used by **World of Hate**, **World of Love**, **World of Apathy**, and **World of Fear**.
* **Superhot**: Used by **Superhot**, **Supercold**, **Stop**, and **Super Speed**.
* **Barrel**: Used by **Black Hole**, **Repulsive**, and **Double Barrel Shotgun**.
* **Vision**: (AKA shit that annoyed me on ep2_outland_12) Used by 
**Where Are The Objects?**, **Drop Weapons**, **Floor Is Lava**, **Orthographic Camera**, **Surprise Reforestation!**, and **Vision Machine Broke**.

### Game

* Stalkers in HL2 now use the Ep1 model as the original one did not ragdoll well and could cause lag.
* If a vehicle is being grabbed by a crane or resting on the ceiling due to altered gravity, the player will now exit the vehicle from below instead of getting jammed into the thing above it.
* Removed ability of `setpos_exact` (and `reload setpos`) to enable `noclip` when stuck.
* Removed acid screen flashing.
* Fixed Alyx "falling to her death" when gravity is not pointing downward in the Episodes.

### Effects

* Gravity modifiers:  
 Physically-simulated objects now react immediately to changes in gravity.  
 Previously they only fell according to `sv_gravity` at the time they spawned/activated at.  
 Now less likely to set `sv_gravity` to something incorrect.
* **Zero Gravity**:  
 Fixed minor issues related to NPCs not understanding **Zero Gravity**.
* **Black Hole** and **Repulsive**:  
 Now acknowledge the strength of global gravity and scale with it.  
 The base strength of **Black Hole** and **Repulsive** can be adjusted with `chaos_pushpull_strength`.
* **Stop** and **Super Speed**:  
 Now only affect the "leg speed" instead of affecting all movement. Secondary things such as gravity and damage affect player movement how they normally would.
* **Stop**:  
 Now affects vehicles too.
* **Super Speed**:  
 Speed increased from 10 times the regular speed values to 4000 ups in all modes and directions.  
 Fall damage has been reduced.
* **Lock Vehicle**:  
 Has been renamed to "**Lock Vehicles**".  
 Locks vehicles the whole time it's active, not just when it is picked.
* **World of ...**:  
 Fixed issue where relationships didn't apply to newly spawned NPCs.  
 NPCs will no longer have relationships applied to themselves. This means no more NPCs attempting to attack themselves or running away from themselves.  
 Negative relationships will no longer apply between vital ally NPCs and the player since killing them would mean resetting the game. This could be changed in the future since you can just hide from them.  
 NPCs will no longer care about crane drivers, since they're immortal.
* **Player is Huge**, **Player is Tiny**, and **Teleport to Random Place**:  
 If the player gets stuck, the game will try to get them unstuck.
* **Teleport to Random Place**:  
 Now works when in vehicles.
* **Spawn Random Vehicle** and **Spawn Random NPC**:  
 APCs can now spawn.  
 Spawned things ignore the camera's vertical angle to avoid spawning in the ground or directly above the player.  
 The game will try to avoid putting NPCs and vehicles inside solid objects.
* **Spawn Random Vehicle**:  
  * Cranes can now spawn.  
  * Jalopies can now spawn in Episode Two.  
  * Vehicles spawn farther away to avoid hitting the player on the head.
  * Airboats will now have their gun.
* **Spawn Random NPC**:
  * NPCs spawn facing player instead of looking to the side.
  * Large flying NPCs will now spawn farther away.
  * Barnacles and ceiling turrets now spawn on ceilings as opposed to floating in the air.
  * Antlions and antlion guardians may randomly be the acid variants in Episode Two.
  * Alyx can now use her EMP device to reprogram rollermines.
  * Alyx, Barney, citizens, vortigaunts and fisherman now have a randomized readiness level.
  * Citizens will now follow the player and have a randomized medic status, ammo resupplier status, type of ammo they give, clothing, expression, and weapon.
  * Soldiers now have 100 grenades to throw and have a randomized model/elite status and weapon.
  * Scanners are now allowed to chase enemies.
  * Dog will now try to play fetch.
  * Metrocops now have 100 manhacks to deploy and have a randomized weapon.
  * Stalkers now have a randomized beam power.
  * Striders are now allowed to impale the player.
  * Ceiling turrets now actually do stuff.
  * Speculative fix for a crash related to spawned turrets.
  * Vortigaunts can now recharge the player's suit.
  * Zombines can now spawn in Episode One and Two.
  * Advisors, antlion grubs, fast zombie torsos, hunters, and Dr. Magnusson can now spawn in Episode Two.
* **Water World**:  
 Drowning damage will stop when in a vehicle. It already did this sometimes so might as well make it a consistent thing.
* **No Looking Left/Right** and **No Looking Up/Down**:  
 Now ensuring that sv_cheats is always 1 as this seemed to be the cause of an issue where the sensitivity would become inverted.
* **Super Grab**:  
 Objects can now be picked up regardless of weight.
* **Give All Weapons** and **Give Random Weapon**:  
 Now give HEV suit and (more) ammo as well.
* **Give Random Weapon**:
 Removed alyxgun, annabelle, citizenpackage, citizensuitcase, and cubemap as they don't work very well.  
 Will no longer try to give weapons you already have.
* **Drop Weapons**:  
 Speculative fix for crash.
* **Drunk Camera**:  
 Has been renamed to "Wobbly".  
 Will now apply even in the air.
* **Left 4 Dead**:  
 Zombies no longer appear to fall down immediately upon spawning.  
 Zombies will no longer mysteriously be all the same type.  
 Zombines can now spawn in Episode One and Two.  
 Fast zombie torsos can now spawn in Episode Two.
* **NPCs Explode on Death**:  
 Antlion grubs now explode.
 Fixed a crash when scanners died.
* **Teleporter Bullets**:  
 If the player or an NPC gets stuck, the game will try to get them unstuck.
 Floor turrets now work a bit better, but will not attempt to teleport when tipped over.
 Mounted guns will no longer teleport because they break.
* **Credits**:  
 Second song now plays at appropriate time.

Added effects:

* **Grenade Guns**
* **Superhot**
* **Supercold**
* **Double Barrel Shotgun**
* **Enable Quickclip**
* **Disable Quickclip**
* **Solid Triggers**
* **Pretty Colors**
* **Beer I owed ya**
* **Annoying Alyx**
* **Noriko, No!**
* **Why So Rushed?**
* **Floor Is Lava**
* **Play Random Song**
* **Grabby**
* **Orthographic Camera**
* **Surprise Reforestation!**
* **Spawn Mounted Gun**
* **Go Back a Level**
* **Remove All Pickups**
* **Suppression Field Hiccup**
* **Vision Machine Broke**
* **Deja Vu?**
* **Bumpy Road**
* **Broken Brakes**
* **Force In/Out Vehicle**
* **Remove Random Weapon**
* **Laggy NPCs**
* **Ran Out Of Glue**
* **No Climbing**
* **No Saving**
* **No One Can Reload**
* **You Teleport?**
* **Death Water**

## v0.0.0

### Effects

Added effects:

* **Zero Gravity**
* **Super Gravity**
* **Low Gravity**
* **Invert Gravity**
* **Pause Physics**
* **Fast Physics**
* **Slow Physics**
* **Black Hole**
* **Repulsive**
* **Stop**
* **Super Speed**
* **Lock Vehicle**
* **World of Hate**
* **World of Love**
* **World of Apathy**
* **World of Fear**
* **Teleport to Random Place**
* **Spawn Random Vehicle**
* **Spawn Random NPC**
* **Water World**
* **Where Are The Objects?**
* **Ultra Low Detail**
* **Player is Huge**
* **Player is Tiny**
* **No Looking Left/Right**
* **No Looking Up/Down**
* **Super Grab**
* **Give Random Weapon**
* **Give All Weapons**
* **Drop Weapons**
* **Drunk Camera**
* **Funny Number**
* **Left 4 Dead**
* **NPCs Explode on Death**
* **Teleporter Bullets**
* **Credits**
