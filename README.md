# Half-Life 2 Chaos
## Installing
1. Install `Source SDK Base 2013 Singleplayer` from Steam.
2. Switch the SDK to the 'upcoming' beta. Press right click -> `Properties...` -> `Betas` and change the dropdown `None` to `upcoming - upcoming`. Changing this setting will prevent immediate crashes.
3. Download latest `hl2c.zip` file from [releases](https://github.com/Pinsplash/halflife2chaos/releases).
4. Extract `hl2chaos` from the dowloaded zip file into `Steam/steamapps/sourcemods/`. The path to gameinfo.txt should look like this: `Steam/steamapps/sourcemods/hl2chaos/gameinfo.txt`. Do the same for `ep1chaos` and `ep2chaos` if you want to play Chaos in Half-Life 2: Episode One/Two.
5. Restart Steam for the mod(s) to appear in your library.

As of version 0.2.2, you no longer need to copy map files into the `maps` folders.

## Building from source for Linux
No build enviroment setup required.
1. `git clone --depth=1 https://github.com/Pinsplash/halflife2chaos`
2. `cd halflife2chaos`
3. `./sp/src/createallprojects` // TODO: this likely can be done with creategameprojects.
4. `make -f ./sp/src/everything.mak client_hl2 server_hl2`
5. `ln -s $(pwd)/sp/game/mod_hl2/bin ./sourcemods/hl2chaos/` Link bin directory from our build directory
6. `mkdir ~/.steam/steam/steamapps/sourcemods/hl2chaos; sudo mount --bind $(pwd)/sourcemods/hl2chaos ~/.steam/steam/steamapps/sourcemods/hl2chaos` Note the use of the bind mount instead of a symbolic link. Steam won't detect symbolic links. You can use `mv` or `cp` instead, if you don't care.

Compiling for the episodes is an excercise for the reader. (Should be similar but with `mod_episodic`, `client_episodic` and `server_episodic`.)

# Effect Voting on Twitch
If you stream on Twitch, you can make it so that viewers can vote for effects by sending chat messages. These instructions are specifically for Windows. If you're on Linux, you can figure it out.
1. Download and install [Python](https://www.python.org/) 3.10.11 from [here](https://www.python.org/downloads/release/python-31011/).
2. Download the code. This is the green button that says "code" at the top. Click "Download ZIP" unless you want to use GitHub Desktop for some reason.
3. Extract the contents to any place on your computer. The only part you need are the files in the `twitch-integration` folder.
4. Press the Windows logo on your keyboard and R at the same time. It should bring up a small window named 'Run'. Type `cmd` in the text box and then click 'OK'. It should bring up a black text box.
5. Select the extracted requirements.txt in File Explorer and view the file's properties. Copy the file path next to "Location".
6. In cmd.exe, type `cd` followed by a space, then paste the file path and press Enter.
7. Now type `pip install -r requirements.txt`. It should do a bunch of stuff.
8. Go to [twitch dev apps](https://dev.twitch.tv/console/apps) and register a new app.
9. The name can be anything. In "OAuth Redirect URLs" put `http://localhost:17563`. The Category should be Game Integration.
10. Press 'Manage' on your newly created app.
11. Copy 'Client ID', create a new secret, and copy 'Client Secret'. Both of these strings will need to be remembered.
12. In OBS, create a new text source.
13. Open Tools -> Scripts -> Python Settings. Change the Install Path to the folder where Python 3.10 is installed. If you allowed it to install to it's default place, this should be `C:/Users/___/AppData/Local/Programs/Python/Python310`. Replace `___` with your Windows user name. You can find what the name is by simply looking in `C:/Users`. If set correctly, it should say "Loaded Python Version: 3.10".
14. Click on "Scripts", click the `+` button, and select `twitch_integration.py` from the `twitch-integration` folder you extracted before.
15. Fill in 'App id' with the Client ID from Twitch and 'App secret' with the Client Secret from Twitch. Set 'Target channel' to your channel (i.e `acuifex` for `twitch.tv/acuifex`). Set 'Text Source' to your newly created text source's name. Set 'RCON password' to a password of your choice.
16. Press 'Reconnect to twitch' if you didn't automatically connect.
17. The script should open Twitch authorization page. Allow the app to do things.
18. In Steam, right click on the mod and then click "Properties...". In 'LAUNCH OPTIONS', put `+developer 0 -usercon +ip 127.0.0.1 +rcon_password ___ +net_start +chaos_vote_enable 1`. Replace `___` with the RCON password that you entered in OBS. The `+developer 0` part is necessary for some reason, despite not seeming like it should be. You will also have to do this in Ep1 & 2 chaos if you want to use Twitch voting in them.

It should work now. If it doesn't, 'Script Log' button in OBS might have useful info. You can test voting commands by typing in your Twitch chat, even if you're not streaming.

## Tips
* If issues arise while playing, try `chaos_restart` in the console. This should set everything back to normal and restart the map.
* To modify things about Chaos like effect duration and probability, edit `hl2chaos/cfg/autoexec.cfg`.
* Effect groups can be changed in `groups.cfg`.
* Want a specific effect? Use `chaos_test_effect` followed by a number. You can get any effect's number through `chaos_print`.
* Any CFG files you change must have their change reflected in the same file in all three mods if you wish for the change to apply in all of them. You can simply copy the file into the other two mod folders and replace the old version.
* Saving often will help you progress faster.
* If necessary, you can leave important NPCs behind, except Alyx in the driving parts of Episode Two. They will teleport into the next level with you.
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
* tmob03
* ntrf