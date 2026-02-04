#if defined _tourneyctrl_config_included
 #endinput
#endif
#define _tourneyctrl_config_included

void LoadTourneyConfig()
{
  char configPath[PLATFORM_MAX_PATH];
  BuildPath(Path_SM, configPath, sizeof(configPath), "configs/tourneyctrl.cfg");

  KeyValues kv = new KeyValues("TourneyCTRL");
  if (!FileToKeyValues(kv, configPath))
  {
    delete kv;
    SetFailState("Failed to read config: %s", configPath);
    return;
  }

  kv.GetString("discord_webhook_url", g_DiscordWebhookUrl, sizeof(g_DiscordWebhookUrl), "");
  kv.GetString("discord_username", g_DiscordUsername, sizeof(g_DiscordUsername), "");
  kv.GetString("discord_avatar_url", g_DiscordAvatarUrl, sizeof(g_DiscordAvatarUrl), "");
  kv.GetString("api_upload_demo_url", g_ApiUploadDemoUrl, sizeof(g_ApiUploadDemoUrl), "");
  kv.GetString("api_send_scores_url", g_ApiSendScoresUrl, sizeof(g_ApiSendScoresUrl), "");
  kv.GetString("api_player_stats_url", g_ApiPlayerStatsUrl, sizeof(g_ApiPlayerStatsUrl), "");
  kv.GetString("api_secret", g_ApiSecret, sizeof(g_ApiSecret), "");

  delete kv;

  if (g_DiscordWebhookUrl[0] == '\0')
  {
    SetFailState("Missing discord_webhook_url in %s", configPath);
  }

  if (g_DiscordUsername[0] == '\0')
  {
    SetFailState("Missing discord_username in %s", configPath);
  }

  if (g_DiscordAvatarUrl[0] == '\0')
  {
    SetFailState("Missing discord_avatar_url in %s", configPath);
  }

  if (g_ApiUploadDemoUrl[0] == '\0')
  {
    SetFailState("Missing api_upload_demo_url in %s", configPath);
  }

  if (g_ApiSendScoresUrl[0] == '\0')
  {
    SetFailState("Missing api_send_scores_url in %s", configPath);
  }

  if (g_ApiPlayerStatsUrl[0] == '\0')
  {
    SetFailState("Missing api_player_stats_url in %s", configPath);
  }

  if (g_ApiSecret[0] == '\0')
  {
    SetFailState("Missing api_secret in %s", configPath);
  }
}

void AppendApiSecret(HTTPRequest request)
{
  if (g_ApiSecret[0] == '\0')
  {
    return;
  }

  request.AppendQueryParam("secret_password", g_ApiSecret);
}
