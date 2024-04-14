# Half-Life 2 Chaos
## Installing
1. Install `Source SDK Base 2013 Singleplayer` from Steam.
2. Switch the SDK to the 'upcoming' beta. Right click on it in Steam and then click `Properties...` -> `Betas` and change the dropdown `None` to `upcoming - upcoming`. Changing this setting will prevent immediate crashes.
3. Download latest `hl2c.zip` file from the [releases](https://github.com/Pinsplash/halflife2chaos/releases) page.
4. Extract the `hl2chaos` folder from the zip file into `Steam/steamapps/sourcemods/`. The path to gameinfo.txt should look like this: `Steam/steamapps/sourcemods/hl2chaos/gameinfo.txt` (see picture below). Do the same for `ep1chaos` and `ep2chaos` if you want to play Chaos in Half-Life 2: Episode One/Two.
5. Restart Steam for the mod(s) to appear in your library.

![image](https://github.com/Pinsplash/halflife2chaos/assets/39359267/854d1f1d-7c23-4b87-bd0d-ea840ea2e873)

For Half-Life 2 Chaos, but not the Episodes, you must have Half-Life 2 installed for the mod to find the levels.

As of version 0.2.2, you no longer need to copy map files into the `maps` folders.

If you want to play Chaos in other mods or Half-Life: Source, you can try, but there can be several issues that we wouldn't be able to fix.

To uninstall Chaos, you should delete the entire `hl2chaos` folder. To install a new version, you should do the same before installing the new version. If you don't do this, you may experience some issues that were only in older versions or entirely new issues.

## Issues

If you experience bugs with the mod, you can submit a bug report [here](https://github.com/Pinsplash/halflife2chaos/issues).

## Wiki

Check our GitHub [wiki](https://github.com/Pinsplash/halflife2chaos/wiki) if you're looking for info about how Half-Life 2 Chaos works.

## Building from source for Linux
No build enviroment setup required.
1. `git clone --depth=1 https://github.com/Pinsplash/halflife2chaos`
2. `cd halflife2chaos`
3. `./sp/src/creategameprojects`
4. `make -f ./sp/src/games.mak -j$(grep processor /proc/cpuinfo | wc -l) client_hl2 server_hl2`
5. `ln -s $(pwd)/sp/game/mod_hl2/bin ./sourcemods/hl2chaos/` Link bin directory from our build directory
6. `mkdir ~/.steam/steam/steamapps/sourcemods/hl2chaos; sudo mount --bind $(pwd)/sourcemods/hl2chaos ~/.steam/steam/steamapps/sourcemods/hl2chaos` Note the use of the bind mount instead of a symbolic link. Steam won't detect symbolic links. You can use `mv` or `cp` instead, if you don't care.

Compiling for the episodes is an excercise for the reader. (Should be similar but with `mod_episodic`, `client_episodic` and `server_episodic`.)

## Effect Voting on Twitch and YouTube
If you stream on Twitch or YouTube, you can make it so that viewers can vote for effects by sending chat messages. These instructions are specifically for Windows. If you're on Linux, you can figure it out.

NOTE: Currently, you can only have voting integration for Twitch *or* YouTube.

1. Download the code. This is the green button that says "code" at the top. Click "Download ZIP" unless you want to use GitHub Desktop for some reason.
2. Extract the contents to any place on your computer. The only part you need are the files in the `twitch-integration` or `youtube-integration` folder.
3. Double click on `setup.bat` in the `twitch-integration` or `youtube-integration` folder. It will install [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) and set up OBS to use it. If you run into any issue using this script, it should tell you which of the following steps you need to complete manually. If it works perfectly, you can skip to step 11.
---
These steps can be skipped depending on how much `setup.bat` accomplishes.

4. Download and install Python 3.11 from [here](https://www.python.org/downloads/release/python-3113/).
5. Press the Windows logo on your keyboard and R at the same time. It should bring up a small window named 'Run'. Type `cmd` in the text box and then click 'OK'. It should bring up a black text box.
6. Select the extracted requirements.txt in File Explorer and view the file's properties. Copy the file path next to "Location".
7. In cmd.exe, type `cd` followed by a space, then paste the file path and press Enter.
8. Now type `py -m pip install -U -r requirements.txt`. It should do a bunch of stuff.
9. Open Tools -> Scripts -> Python Settings. Change the Install Path to the folder where Python 3.11 is installed. If you allowed it to install to its default place, this should be `C:/Users/___/AppData/Local/Programs/Python/Python311`. Replace `___` with your Windows user name. You can find what the name is by simply looking in `C:/Users`. If set correctly, it should say "Loaded Python Version: 3.11".
10. Click on "Scripts", click the `+` button, and select `twitch_integration.py` from the `twitch-integration` folder or `youtube_integration.py` from the `youtube-integration` folder you extracted before.
---
11. In OBS, create a new text source.
12. Open Tools -> Scripts and click on `twitch-integration.py` or  `youtube-integration.py`. Set 'Text Source' to your newly created text source's name. Set 'RCON password' to a password of your choice.

13a. For Twitch, set 'Target channel' to your channel (i.e `acuifex` for `twitch.tv/acuifex`).

13b. For YouTube, set 'Steam URL' to the URL of your stream. This must be updated every time you start a new stream.

14. Press 'Reconnect to twitch/youtube' if you didn't automatically connect.
15. In Steam, right click on the mod and then click "Properties...". In 'LAUNCH OPTIONS', put `+developer 0 -usercon +ip 127.0.0.1 +rcon_password ___ +net_start +chaos_vote_enable 1`. Replace `___` with the RCON password that you entered in OBS. The `+developer 0` part is necessary for some reason, despite not seeming like it should be. You will also have to add these options in Ep1 & 2 chaos if you want to use Twitch or YouTube voting in them.
It should work now. If it doesn't, the 'Script Log' button in OBS might have useful info. You can test voting commands by typing in your Twitch or YouTube chat, even if you're not actively streaming.

To remove Twitch or YouTube integration, you can simply clear out the mod's launch options and select `twitch-integration.py` or `youtube-integration.py` in OBS and then click the minus button.

Voting will not work if a map is not loaded. Menu background maps count. If no map loads upon opening the mod, please open an issue for it even if a closed one already exists, or contact Pinsplash in any other way.

## Tips
* If issues arise while playing, try `chaos_restart` in the console. This should set everything back to normal and restart the map.
* If you become physically stuck in something, the game will try to get you out of it the next time an effect starts. You can also use the console command `getunstuck`, or use `noclip` as is usual for Source games.
* To modify things about Chaos like effect duration and probability, edit `hl2chaos/cfg/autoexec.cfg`. Restart the game for changes to take effect. Simply using `exec` is NOT sufficient depending on the circumstances. Effect groups can be changed in `groups.cfg`.
* Want a specific effect? Use `chaos_test_effect` followed by a number. You can get any effect's number through `chaos_print`.
* Any CFG files you change must have their change reflected in the same file in all three mods if you wish for the change to apply in all of them. You can simply copy the file into the other two mod folders and replace the old version.
* Saving often will help you progress faster.
* If necessary, you can leave important NPCs behind, except Alyx in the driving parts of Episode Two. They will teleport into the next level with you.
* Enemy NPCs spawned by Chaos are gone forever once killed and remain wounded forever once hurt (unless they regenerate health), even if you reload a save, so don't give up on them.
* Fast weapon switch is best left off to easily know which weapons you have. You can hold jump to continuously jump in Chaos, so there's no need to change the scroll wheel binds to bunnyhop.
* Quickclip will not disable weapon switching if enabled by Chaos.
* Make keybinds for loading quicksaves and autosaves.

Some effect-specific advice, if you don't wish to figure it out on your own:
* **Zero Gravity** and **Invert Gravity**: You can stick to the ground as long as you don't jump or move off it. Land in water to avoid fall damage. Landing on slopes can prevent fall damage.
* **Water World**: Only the crowbar, gravity gun, pistol, and crossbow can be used underwater. Drowning damage heals once you leave water. If **Superhot** is on, stay still to minimize drowning damage. If **Supercold** is on, move around to minimize drowning damage. You can't drown in vehicles.
* **Annoying Alyx** and similar: Don't use burst damage weapons such as the shotgun. Alyx can only lose 25% of her health at once and regenerates health, so will take at least 5 hits to kill.
* Sometimes things are invisible with **Orthographic Camera**. Turning on the flashlight may fix it.

Certain console variables are changed by this mod during gameplay that you might want your own default settings for.
These settings have been put in their own CFG files so you can control what they are, except when chaos effects modify them.
* `pitch.cfg`: Sets `m_pitch`, the vertical sensitivity. Default 0.022.
* `yaw.cfg`: Sets `m_yaw`, the horizontal sensitivity. Default 0.022.
* `portalsopenall.cfg`: Sets `r_portalsopenall`. Force-opens all areaportals. Most users will want to use 1 because if 0, areas of levels may become invisible until you enter them. This was intended to help performance, but Chaos can disrupt this feature due to sequence breaking. Normally the default is 0, but in Chaos, it's 1.

## Known issues
* To ensure all effects work right, sv_cheats is automatically set to 1. Don't change it.
* The "Saved..." message may not appear. The save is still made.
* If you play maps from something that isn't _Half-Life 2/Episode One/Two_, the "Node graph out of date" message will appear every time you go to a new map for the first time. This message is harmless, but if you wish, you can copy the `.AIN` files from the mod as well. It will also stop appearing after you've visited each map a single time, because the mod has now generated its own copy of the node graph. The `.AIN` files are included for _Half-Life 2/Episode One/Two_.
* If you take a weird path through certain maps, you may find that some things won't work as intended and softlocks will potentially exist.
* **Didn't Skip Arm Day** sometimes makes it harder to grab things.
* Sometimes things are invisible with **Orthographic Camera**. Turning on the flashlight may fix it.
* Collisions can be weird when **Slow Physics** is on.
* **Teleporter Bullets** does not apply to vehicle guns while a fix is in the works for a serious issue.

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
