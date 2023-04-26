# Twitch integration

This is a script for OBS to allow effect voting from chat.

It uses RCON commands to communicate with the game.

# Usage

1. `pip install -r requirements.txt`
1. Go to [twitch dev apps](https://dev.twitch.tv/console/apps) and register a new app
1. Choose name, category, and set redirect url to `http://localhost:17563`
1. Press `Manage` on your newly created app
1. Copy `Client ID`, create new secret, copy `Client Secret`  
 NOTE: OBS doesn't persist script's parameters. You might want to save those values. 
1. In OBS: create a new `Text` source
1. Open `Tools` -> `Scripts`, press `+`, and select `twitch_integration.py`
1. Fill in `App id` and `App secret`  
 Set `Target channel` to your channel (i.e `acuifex` for `twitch.tv/acuifex`)  
 Set `Text Source` to your newly created text source's name  
 Set `RCON password` to a password of your choice
1. Press `Reconnect to twitch` if you didn't automatically connect.
1. The script should open Twitch authorization page. Allow the app to do things
1. Set your mod's launch options to `+developer 0 -usercon +ip 127.0.0.1 +rcon_password <PASSWORD> +net_start +chaos_vote_enable 1`  
 Do not ask me about `+developer 0`. I have no idea why it doesn't work without it.
1. Launch your game

It should work now. If it doesn't, check `Script Log` in OBS for possibly more info