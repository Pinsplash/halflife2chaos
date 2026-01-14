# Half-Life 2 Chaos
## Installing
1. Install `Source SDK Base 2013 Singleplayer` from Steam.
2. Switch the SDK to the 'upcoming' beta. Right click on it in Steam and then click `Properties...` -> `Betas` and change the dropdown `None` to `upcoming - upcoming`. Changing this setting will prevent immediate crashes.
3. Download the latest `hl2c.zip` file from the [releases](https://github.com/Pinsplash/halflife2chaos/releases) page.
4. Extract the `hl2chaos` folder from the zip file into `Steam/steamapps/sourcemods/`. The path to gameinfo.txt should look like this: `Steam/steamapps/sourcemods/hl2chaos/gameinfo.txt` (see picture below). Do the same for `ep1chaos` and `ep2chaos` if you want to play Chaos in Half-Life 2: Episode One/Two.
5. Restart Steam for the mod(s) to appear in your library.

![image](https://github.com/Pinsplash/halflife2chaos/assets/39359267/854d1f1d-7c23-4b87-bd0d-ea840ea2e873)

You must install Half-Life 2 and have it under the same Steam library folder (`steamapps`) as the mod. The same must be done for the Episodes as well to play Episode One/Two Chaos. In one case, someone was not able to do this but got around this limitation by making a symbolic link between their two `sourcemods` folders with this cmd: `mklink /D "C:\Program Files (x86)\Steam\steamapps\sourcemods" "D:\somewhereelse\steamapps\sourcemods"`

To play Episode One Chaos, you must also install Half-Life 2 Chaos, and if you wish to play Episode Two Chaos, you must also install Half-Life 2 Chaos and Episode One Chaos.

If you want to play Chaos in other mods or Half-Life: Source, you can try, but there can be several issues that we wouldn't be able to fix.

To uninstall Chaos, you should delete the entire `hl2chaos` folder. To install a new version, you should do the same before installing the new version. If you don't do this, you may experience some issues that were only in older versions or entirely new issues.

## Issues

If you experience bugs with the mod, you can submit a bug report [here](https://github.com/Pinsplash/halflife2chaos/issues).

## Wiki

Check the GitHub [wiki](https://github.com/Pinsplash/halflife2chaos/wiki) if you're looking for info about how Half-Life 2 Chaos works.

## Building from source for Linux
No build environment setup required.
1. `git clone --depth=1 https://github.com/Pinsplash/halflife2chaos`
2. `cd halflife2chaos`
3. `./sp/src/creategameprojects`
4. `make -f ./sp/src/games.mak -j$(grep processor /proc/cpuinfo | wc -l) client_hl2 server_hl2`
5. `ln -s $(pwd)/sp/game/bin ./sourcemods/hl2chaos/` Link bin directory from our build directory
6. `mkdir ~/.steam/steam/steamapps/sourcemods/hl2chaos; sudo mount --bind $(pwd)/sourcemods/hl2chaos ~/.steam/steam/steamapps/sourcemods/hl2chaos` Note the use of the bind mount instead of a symbolic link. Steam won't detect symbolic links. You can use `mv` or `cp` instead, if you don't care.

Compiling for the episodes is an excercise for the reader. (Should be similar but with `mod_episodic`, `client_episodic` and `server_episodic`.)

## Effect Voting on Twitch and YouTube
If you stream on Twitch or YouTube, you can make it so that viewers can vote for effects by sending chat messages. These instructions are specifically for Windows. If you're on Linux, you can figure it out.

NOTE: Currently, you can only have voting integration for Twitch *or* YouTube.

1. Download the code. This is the green button that says "code" at the top. Click "Download ZIP".
2. Extract the contents to any place on your computer. The only parts you need are the files in the `(website)-integration` folder.
3. Double click on `setup.bat` in the `(website)-integration` folder. It will install [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) and set up OBS to use it. If you run into any issue using this script, it should tell you which of the following steps you need to complete manually. If it works perfectly, you can skip to step 9.
---
These steps can be skipped depending on how much `setup.bat` accomplishes.

4. Download and install Python 3.11 from [here](https://www.python.org/downloads/release/python-3113/).
5. Navigate in File Explorer to the extracted `(website)-integration` folder and go to File -> Open Windows PowerShell.
6. Now type `py -m pip install -U -r requirements.txt`. It should do a bunch of stuff.
7. In OBS, open Tools -> Scripts -> Python Settings. Change the Install Path to the folder where Python 3.11 is installed. If you allowed it to install to its default place, this should be `C:/Users/___/AppData/Local/Programs/Python/Python311`. Replace `___` with your Windows user name. You can find what the name is by simply looking in `C:/Users`. If set correctly, it should say "Loaded Python Version: 3.11".
8. Click on "Scripts", click the `+` button, and select `(website)-integration.py`.
---
9. In OBS, create a new text source.
10. Open Tools -> Scripts and click on `(website)-integration.py`. Set 'Text Source' to your newly created text source's name. Set 'RCON password' to a password of your choice.

11a. For Twitch, set 'Target channel' to your channel (i.e `acuifex` for `twitch.tv/acuifex`).

11b. For YouTube, set 'Stream URL' to the URL of your stream. This must be updated every time you start a new stream.

12. Press 'Reconnect to (website)' if you didn't automatically connect.
13. In Steam, right click on the mod and then click "Properties...". In 'LAUNCH OPTIONS', put `+developer 0 -usercon +ip 127.0.0.1 +rcon_password ___ +net_start +chaos_vote_enable 1`. Replace `___` with the RCON password that you entered in OBS. The `+developer 0` part is necessary for some reason, despite not seeming like it should be. You will also have to add these options in Ep1 & 2 chaos if you want to use Twitch or YouTube voting in them.
It should work now. If it doesn't, the 'Script Log' button in OBS might have useful info. You can test voting commands by typing in your stream chat, even if you're not actively streaming.

To remove stream integration, you can simply clear out the mod's launch options and select `(website)-integration.py` in OBS and then click the minus button.

Voting will not work if a map is not loaded. Menu background maps count. If no map loads upon opening the mod, this is not normal. Please open an issue for it even if a closed one already exists, or contact Pinsplash in any other way.

## Tips
* If issues arise while playing, try `chaos_restart` in the console. This should set everything back to normal and restart the map.
* If you become physically stuck in something, the game will try to get you out of it the next time an effect starts. You can also use the console command `getunstuck`, or use `noclip` as is usual for Source games.
* To modify things about Chaos like effect duration and probability, edit `hl2chaos/cfg/autoexec.cfg`. Restart the game for changes to take effect. Simply using `exec` is NOT sufficient depending on the circumstances. Effect groups can be changed in `groups.cfg`.
* Want a specific effect? Use `chaos_test_effect` followed by a number. You can get any effect's number through `chaos_print`.
* 1.0: Changes to CFG files in the `hl2chaos` folder now affect all 3 mods. If you want different settings for ep1/2, copy the relevant file into that mod's `cfg` folder.
* Saving often will help you progress faster.
* If necessary, you can leave important NPCs behind, except Alyx in some parts of Episode Two. They will teleport into the next level with you.
* Enemy NPCs spawned by Chaos are gone forever once killed and remain wounded forever once hurt (unless they regenerate health), even if you reload a save, so don't give up on them.
* Fast weapon switch is best left off to easily know which weapons you have. You can hold jump to continuously jump in Chaos, so there's no need to change the scroll wheel binds to bunnyhop.
* Quickclip will not disable weapon switching if enabled by Chaos.
* Make keybinds for loading quicksaves and autosaves.

Certain console variables are changed by this mod during gameplay that you might want your own default settings for.
These settings have been put in their own CFG files so you can control what they are, except when chaos effects modify them.
* `pitch.cfg`: Sets `m_pitch`, the vertical sensitivity. Default 0.022.
* `yaw.cfg`: Sets `m_yaw`, the horizontal sensitivity. Default 0.022. Set the inverse in `negative_yaw.cfg`.
* `portalsopenall.cfg`: Sets `r_portalsopenall`. Force-opens all areaportals. Most users will want to use 1 because if 0, areas of levels may become invisible until you enter them. This was intended to help performance, but Chaos can disrupt this feature due to sequence breaking. Normally the default is 0, but in Chaos, it's 1.

## Known issues
* To ensure all effects work right, sv_cheats is automatically set to 1. Don't change it.
* The "Saved..." message may not appear. The save is still made.
* If you play maps from something that isn't _Half-Life 2/Episode One/Two_, the "Node graph out of date" message will appear every time you go to a new map for the first time. This message is harmless, but if you wish, you can copy the `.AIN` files from the mod as well. The mod sets `ai_norebuildgraph 1` by default to avoid issues with rebuilding node graphs on Valve maps. The `.AIN` files are included for _Half-Life 2/Episode One/Two_.
* If you take a weird path through certain maps, you may find that some things won't work as intended and softlocks will potentially exist.
* **Didn't Skip Arm Day** sometimes makes it harder to grab things.
* Sometimes things are invisible with **Orthographic Camera**. Turning on the flashlight may fix it.
* Collisions can be weird when **Slow Physics** is on.

## Localization
The mod's strings are localized only for English and Spanish (Spain/LatAm). Contributions for other languages are welcome.

## Acknowledgements for indirect contributions - Thank you!
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
* metalisai
* Owlrazum

See [here](https://github.com/Pinsplash/halflife2chaos/wiki/Open-Source-Information) for details on some original code in Half-Life 2 Chaos.