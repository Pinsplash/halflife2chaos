# Half-Life 2 Chaos
Download from here: https://github.com/Pinsplash/halflife2chaos/releases  

## Installation

Put the `hl2chaos` folder into `Steam/steamapps/sourcemods/`.

The path to `gameinfo.txt` should be `Steam/steamapps/sourcemods/hl2chaos/gameinfo.txt`.  
Do the same with the `ep1chaos` and `ep2chaos` folders if you wish to play Chaos in _Episode One/Two_.  
In all places in this guide, replace "hl2chaos" with "ep1chaos" or "ep2chaos" if using _Episode One/Two_.

Restart Steam for the mod(s) to appear in your library.

When running you will be prompted to download `Source SDK Base 2013 Singleplayer` if you don't have it already.  
This is normal.  
Download it, then right click on it and go to `Properties...`, `BETAS`, and change the dropdown from `None` to `upcoming - upcoming`.  
Changing this setting will prevent immediate crashes.  
You must copy the `.BSP` files from your installation of _Half-Life 2/Episode One/Two_.  
The mod may attempt to load maps without this step, but there will likely be issues.  
To do this go to `Steam/steamapps/common/Half-Life 2/___/maps/`.  
Fill in the blank with which game you want to copy maps for:

* **hl2** for _Half-Life 2_
* **episodic** for _Episode One_
* **ep2** for _Episode Two_

Maps from mods (and _Lost Coast_) can also be used if you wish, though some things may not function correctly and additional original content (like textures and models) will need to be copied as well.

Copy the map files and paste them in `Steam/steamapps/sourcemods/hl2chaos/maps/`.

## Tips

* If issues arise while playing, try `chaos_restart` in the console.  
 This should set everything back to normal and restart the map.
* To modify things about Chaos like effect duration and probability, edit `hl2chaos/cfg/autoexec.cfg`.
* Effect groups can be changed in `groups.cfg`.
* Want a specific effect? Use `chaos_test_effect` followed by a number. You can get any effect's number through `chaos_print`.
* Any CFG files you change must have their change reflected in the same file in all three mods if you wish for the change to apply in all of them. You can simply copy the file into the other two mod folders and replace the old version.
* Saving often will help you progress faster.
* If necessary, you can leave important NPCs behind. They will teleport into the next level with you.
* Enemy NPCs spawned by Chaos are gone forever once killed and remain wounded forever once hurt (unless they regenerate health), even if you reload a save, so don't give up on them.
* Turn off Fast weapon switch to easily know which weapons you have.
* Quickclip will not disable weapon switching if enabled by Chaos.
* Doing ep1_citadel_03 out of order is not recommended.
* Make keybinds for loading quicksaves and autosaves.

Some effect-specific advice, if you don't wish to figure it out on your own:

* **Zero Gravity** and **Invert Gravity**:  
 You can stick to the ground as long as you don't jump or move off it. 
 Land in water to avoid fall damage. 
 Landing on slopes can prevent fall damage.
* **Water World**:  
 Only the crowbar, gravity gun, pistol, and crossbow can be used underwater.  
 Drowning damage heals once you leave water.  
 If **Superhot** is on, stay still to minimize drowning damage.  
 If **Supercold** is on, move around to minimize drowning damage. You can't drown in vehicles.
* **Annoying Alyx**:  
 Don't use burst damage weapons such as the shotgun. Alyx can only lose 25% of her health at once and regenerates health, so will take at least 5 hits to kill.
* Sometimes things are invisible with **Orthographic Camera**. Turning on the flashlight may fix it.

Certain console variables are changed in this mod that you might want your own settings for.  
These settings have been put in their own CFG files so you can control what they are, except when chaos effects modify them.

* `pitch.cfg`: Sets `m_pitch`, the vertical sensitivity. Default 0.022.
* `yaw.cfg`: Sets `m_yaw`, the horizontal sensitivity. Default 0.022.
* `portalsopenall.cfg`: Sets `r_portalsopenall`. Force-opens all areaportals.  
 Most users will want to use 1 because if 0, areas of levels may become invisible until you enter them.  
 This was intended to help performance, but Chaos can disrupt this feature due to sequence breaking.  
 Normally the default is 0, but in Chaos, it's 1.

## Known issues
* Sometimes the bar at the top won't appear after loading a save/map. The mod is still working. The bar should appear once the next effect starts.
* The "Saved..." message may not appear. The save is still made.
* The car compass HUD element does not display correctly.
* If you play maps from something that isn't _Half-Life 2/Episode One/Two_, 
 the "Node graph out of date" message will appear every time you go to a new map for the first time. 
 This message is harmless, but if you wish, you can copy the `.AIN` files from the mod as well. 
 It will also stop appearing after you've visited each map a single time, 
 because the mod has now generated its own copy of the node graph. 
 The `.AIN` files are included for _Half-Life 2/Episode One/Two_.
* If you take a weird path through d3_c17_13, you may not activate the strider in the car pit. You will have to kill it before the intended route can continue.
* It's easy to get stuck with **Player is Huge**, but only temporarily.  
 The issue seems to only happen when jumping and not holding crouch.
* **Super Grab** sometimes makes it harder to grab things.
* Sometimes things are invisible with **Orthographic Camera**. Turning on the flashlight may fix 
it.
* Collisions can be weird when **Slow Physics** is on.

## Thanks for big and small bits of help

* Blixibon
* tmp64
* BoxFigs
* B.A.S.E
* TeamSpen210
* craftablescience
* EchoesForeAndAft
* ender
* 2838
* UncraftedName
