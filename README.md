<div align="center">
  <h1><code>TourneyCTRL</code></h1>
  <p>
    <strong>TF2 tournament control plugin for automated match flow, demos, and reporting</strong>
  </p>
</div>


## Requirements ##
- SourceMod and MetaMod
- Extensions: SteamWorks, RIPExt
- Include deps bundled in scripting/include (json, ripext, discordWebhookAPI)


## Installation ##
1. Compile TourneyCTRL.sp with spcomp and place the output in addons/sourcemod/plugins.
2. Copy the addons/ and translations/ folders into your TF2 server root.
3. Edit addons/sourcemod/configs/tourneyctrl.cfg and fill required values.
4. Set required environment variables before server start.
5. Restart the server or load with `sm plugins load TourneyCTRL`.

## Configuration ##
- Required config keys in addons/sourcemod/configs/tourneyctrl.cfg:
  - discord_webhook_url
  - discord_username
  - discord_avatar_url
  - api_upload_demo_url
  - api_send_scores_url
  - api_player_stats_url
  - api_secret
- Required environment variables:
  - MATCH_ID
  - ROUND_ID
  - HOME_TEAM
  - HOME_TEAM_ID
  - AWAY_TEAM
  - AWAY_TEAM_ID
  - MIN_PLAYERS
  - MAX_PLAYERS
- Optional environment variables:
  - WIN_LIMIT
- You can modify phrases in addons/sourcemod/translations/tourneyctrl.phrase.txt.

## Files ##
- scripting/TourneyCTRL.sp: main plugin entry, events, timers, and shared globals.
- scripting/include/tourneyctrl_config.inc: config loading and API secret helper.
- scripting/include/tourneyctrl_util.inc: API retry helpers and request handlers.
- scripting/include/tourneyctrl_recording.inc: demo recording, upload, and archive.
- scripting/include/tourneyctrl_teams.inc: team assignment, team enforcement, ready checks.
- scripting/include/tourneyctrl_stats.inc: player stat tracking and upload.
- scripting/include/tourneyctrl_web.inc: Discord webhook and score submission flow.
- scripting/include/udl.inc: shared UDL helpers used across plugins.
- addons/sourcemod/configs/tourneyctrl.cfg: required secrets and endpoints.
- translations/tourneyctrl.phrase.txt: chat message translations.
- scripting/include/discordWebhookAPI.inc, json/, ripext/: bundled dependencies.

## Usage ##
1. Ensure the environment variables are set before match start.
2. The plugin will assign players, enforce teams, and start recording on ready-up.
3. On match end, it posts scores and player stats, uploads the demo, and kicks players.
4. Admin commands:
   - tc_record: start recording
   - tc_stoprecord: stop recording
   - tc_restart: restart tournament
   - tc_reset_assigned: clear assigned players
