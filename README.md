# Half-Life 2 Chaos

## Installing a binary release
1. Install `Source SDK Base 2013 Singleplayer` from Steam.
1. Switch SDK to upcoming beta. Press right click -> `Properties...` -> `Betas` and change the dropdown `None` to `upcoming - upcoming`. Changing this setting will prevent immediate crashes.
1. Download latest `hl2c.zip` file from [releases](https://github.com/Pinsplash/halflife2chaos/releases).
1. Extract `hl2chaos` from the dowloaded zip file into `Steam/steamapps/sourcemods/`.

    The final directory tree should look like this: `Steam/steamapps/sourcemods/hl2chaos/gameinfo.txt`.

    Do the same for `ep1chaos` and `ep2chaos` if you want to play Chaos in Half-Life 2: Episode One/Two.
1. Restart Steam for the mod(s) to appear in your library.
1. Copy map (`.bsp`) files from `Steam/steamapps/common/Half-Life 2/___/maps/` into `Steam/steamapps/sourcemods/***/maps/`.

    In place of `___` and `***` you will want to fill:
    * `hl2` and `hl2chaos` for Half-Life 2
    * `episodic` and `ep1chaos` for Half-Life 2: Episode One
    * `ep2` and `ep2chaos` for Half-Life 2: Episode Two

    Maps from mods (and Lost Coast) can also be used if you wish, though some things may not function correctly and additional original content (like textures and models) will need to be copied as well


## Building from source for Linux
No build enviroment setup required.
1. `git clone https://github.com/Pinsplash/halflife2chaos`
2. `cd halflife2chaos`
3. `./sp/src/createallprojects` // TODO: this likely can be done with creategameprojects.
4. `make -f ./sp/src/everything.mak client_hl2 server_hl2`
5. `ln -s $(pwd)/sp/game/mod_hl2/bin ./sourcemods/hl2chaos/` Link bin directory from our build directory
6. `mkdir ~/.steam/steam/steamapps/sourcemods/hl2chaos; sudo mount --bind $(pwd)/sourcemods/hl2chaos ~/.steam/steam/steamapps/sourcemods/hl2chaos` Note the use of the bind mount instead of a symbolic link. Steam won't detect symbolic links. You can use `mv` or `cp` instead, if you don't care.

Compiling for the episodes is an excercise for the reader. (Should be similar but with `mod_episodic`, `client_episodic` and `server_episodic`.)

## Tips
* If issues arise while playing, try `chaos_restart` in the console. This should set everything back to normal and restart the map.
* To modify things about Chaos like effect duration and probability, edit `hl2chaos/cfg/autoexec.cfg`.
* Effect groups can be changed in `groups.cfg`.
* Want a specific effect? Use `chaos_test_effect` followed by a number. You can get any effect's number through `chaos_print`.
* Any CFG files you change must have their change reflected in the same file in all three mods if you wish for the change to apply in all of them. You can simply copy the file into the other two mod folders and replace the old version.
* Saving often will help you progress faster.
* If necessary, you can leave important NPCs behind. They will teleport into the next level with you.
* Enemy NPCs spawned by Chaos are gone forever once killed and remain wounded forever once hurt (unless they regenerate health), even if you reload a save, so don't give up on them.
* Fast weapon switch is best left off to easily know which weapons you have.
* Quickclip will not disable weapon switching if enabled by Chaos.
* Doing ep1_citadel_03 out of order is not recommended.
* Make keybinds for loading quicksaves and autosaves.

Some effect-specific advice, if you don't wish to figure it out on your own:
* **Zero Gravity** and **Invert Gravity**: You can stick to the ground as long as you don't jump or move off it. Land in water to avoid fall damage. Landing on slopes can prevent fall damage.
* **Water World**: Only the crowbar, gravity gun, pistol, and crossbow can be used underwater. Drowning damage heals once you leave water. If **Superhot** is on, stay still to minimize drowning damage. If **Supercold** is on, move around to minimize drowning damage. You can't drown in vehicles.
* **Annoying Alyx**: Don't use burst damage weapons such as the shotgun. Alyx can only lose 25% of her health at once and regenerates health, so will take at least 5 hits to kill.
* Sometimes things are invisible with **Orthographic Camera**. Turning on the flashlight may fix it.

Certain console variables are changed in this mod that you might want your own settings for.
These settings have been put in their own CFG files so you can control what they are, except when chaos effects modify them.
* `pitch.cfg`: Sets `m_pitch`, the vertical sensitivity. Default 0.022.
* `yaw.cfg`: Sets `m_yaw`, the horizontal sensitivity. Default 0.022.
* `portalsopenall.cfg`: Sets `r_portalsopenall`. Force-opens all areaportals. Most users will want to use 1 because if 0, areas of levels may become invisible until you enter them. This was intended to help performance, but Chaos can disrupt this feature due to sequence breaking. Normally the default is 0, but in Chaos, it's 1.

## Known issues
* Sometimes the bar at the top won't appear after loading a save/map. The mod is still working. The bar should appear once the next effect starts.
* The "Saved..." message may not appear. The save is still made.
* The car compass HUD element does not display correctly.
* If you play maps from something that isn't _Half-Life 2/Episode One/Two_, the "Node graph out of date" message will appear every time you go to a new map for the first time. This message is harmless, but if you wish, you can copy the `.AIN` files from the mod as well. It will also stop appearing after you've visited each map a single time, because the mod has now generated its own copy of the node graph. The `.AIN` files are included for _Half-Life 2/Episode One/Two_.
* If you take a weird path through d3_c17_13, you may not activate the strider in the car pit. You will have to kill it before the intended route can continue.
* It's easy to get stuck with **Player is Huge**, but only temporarily. The issue seems to only happen when jumping and not holding crouch.
* **Super Grab** sometimes makes it harder to grab things.
* Sometimes things are invisible with **Orthographic Camera**. Turning on the flashlight may fix it.
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
