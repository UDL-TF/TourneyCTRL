#if defined _tourneyctrl_web_included
 #endinput
#endif
#define _tourneyctrl_web_included

void AnnounceWinner(TFTeam team, TFTeam loserTeam)
{
  if (team == TFTeam_Unassigned || loserTeam == TFTeam_Unassigned)
  {
    return;
  }

  char teamName[32];
  char loserName[32];
  int winnerPoints = GetTeamScore(view_as<int>(team));
  int loserPoints = GetTeamScore(view_as<int>(loserTeam));

  if (team == TFTeam_Red)
  {
    g_CvarRedTeamName.GetString(teamName, sizeof(teamName));
    g_CvarBlueTeamName.GetString(loserName, sizeof(loserName));
  }
  else
  {
    g_CvarBlueTeamName.GetString(teamName, sizeof(teamName));
    g_CvarRedTeamName.GetString(loserName, sizeof(loserName));
  }

  char serverMessage[128];
  Format(serverMessage, sizeof(serverMessage), "%s won the game!", teamName);

  Webhook webhook = new Webhook("");
  webhook.SetUsername(g_DiscordUsername);
  webhook.SetAvatarURL(g_DiscordAvatarUrl);

  Embed embed = new Embed(serverMessage);
  embed.SetColor(0x00FF00);

  EmbedField teamField = new EmbedField("Team", teamName);
  embed.AddField(teamField);

  char winnerPointsString[32];
  Format(winnerPointsString, sizeof(winnerPointsString), "%i", winnerPoints);
  EmbedField scoreField = new EmbedField("Score", winnerPointsString);
  embed.AddField(scoreField);

  for (int i = 1; i <= MaxClients; i++)
  {
    if (!IsValidPlayer(i))
    {
      continue;
    }
    TFTeam clientTeam = view_as<TFTeam>(GetClientTeam(i));
    if (clientTeam == team)
    {
      char playerName[32];
      GetClientName(i, playerName, sizeof(playerName));
      char steamID[32];
      GetSteamId(i, steamID, sizeof(steamID));

      char message[128];
      Format(message, sizeof(message), "%s (%s)", playerName, steamID);

      EmbedField playerField = new EmbedField("Player", message);
      playerField.SetInline(true);
      embed.AddField(playerField);
    }
  }

  webhook.AddEmbed(embed);

  char losingMessage[128];
  Format(losingMessage, sizeof(losingMessage), "%s lost the game!", loserName);

  Embed loserEmbed = new Embed(losingMessage);
  loserEmbed.SetColor(0xFF0000);

  EmbedField loserTeamField = new EmbedField("Team", loserName);
  loserEmbed.AddField(loserTeamField);

  char loserPointsString[32];
  Format(loserPointsString, sizeof(loserPointsString), "%i", loserPoints);
  EmbedField loserScoreField = new EmbedField("Score", loserPointsString);
  loserEmbed.AddField(loserScoreField);

  for (int i = 1; i <= MaxClients; i++)
  {
    if (!IsValidPlayer(i))
    {
      continue;
    }
    TFTeam clientTeam = view_as<TFTeam>(GetClientTeam(i));
    if (clientTeam == loserTeam)
    {
      char playerName[32];
      GetClientName(i, playerName, sizeof(playerName));
      char steamID[32];
      GetSteamId(i, steamID, sizeof(steamID));

      char message[128];
      Format(message, sizeof(message), "%s (%s)", playerName, steamID);

      EmbedField loserPlayerField = new EmbedField("Player", message);
      loserPlayerField.SetInline(true);
      loserEmbed.AddField(loserPlayerField);
    }
  }

  webhook.AddEmbed(loserEmbed);

  char contentMessage[128];
  char serverIp[128];
  Format(serverIp, sizeof(serverIp), "%s:%i", g_PublicIp, g_ServerPort);

  char hostname[128];
  GetConVarString(FindConVar("hostname"), hostname, sizeof(hostname));

  Format(contentMessage, sizeof(contentMessage), "Demo: %s\nServer IP: %s\nHostname: %s", g_CurrentRecording, serverIp, hostname);
  webhook.SetContent(contentMessage);
  webhook.Execute(g_DiscordWebhookUrl, OnWebHookExecuted);
  SendScoresRequest(team, 0);
}
