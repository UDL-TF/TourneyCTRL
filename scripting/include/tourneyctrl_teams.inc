#if defined _tourneyctrl_teams_included
 #endinput
#endif
#define _tourneyctrl_teams_included

void AssignPlayersToTeams()
{
  char steamIDs[64][64];

  int homeCount = ExplodeString(g_HomeTeam, ",", steamIDs, sizeof(steamIDs), sizeof(steamIDs[]));
  for (int i = 0; i < homeCount; i++)
  {
    LogMessage("SteamID For Red %s", steamIDs[i]);
    g_AssignedRedTeam.PushString(steamIDs[i]);
  }

  int awayCount = ExplodeString(g_AwayTeam, ",", steamIDs, sizeof(steamIDs), sizeof(steamIDs[]));
  for (int i = 0; i < awayCount; i++)
  {
    LogMessage("SteamID For Blue %s", steamIDs[i]);
    g_AssignedBlueTeam.PushString(steamIDs[i]);
  }
}

int AmountOfPlayersInTeam(TFTeam team)
{
  int count = 0;
  for (int i = 1; i <= MaxClients; i++)
  {
    if (IsClientInGame(i) && GetClientTeam(i) == view_as<int>(team))
    {
      count++;
    }
  }
  return count;
}

void CancelMinuteAnnouncements()
{
  if (g_MinuteAnnouncementTimer != null)
  {
    KillTimer(g_MinuteAnnouncementTimer);
    g_MinuteAnnouncementTimer = null;
  }
}

void UpdateMinuteAnnouncements()
{
  if (g_MaxPlayers == 0 || g_GameFinished)
  {
    CancelMinuteAnnouncements();
    return;
  }

  int redCount = AmountOfPlayersInTeam(TFTeam_Red);
  int blueCount = AmountOfPlayersInTeam(TFTeam_Blue);

  bool redFull = redCount >= g_MaxPlayers;
  bool blueFull = blueCount >= g_MaxPlayers;

  if (redFull && blueFull)
  {
    CancelMinuteAnnouncements();
    return;
  }

  if ((redFull && !blueFull) || (blueFull && !redFull))
  {
    if (g_MinuteAnnouncementTimer == null)
    {
      g_MinuteAnnouncementTimer = CreateTimer(60.0, Timer_MinuteAnnouncement, _, TIMER_REPEAT);
    }
    return;
  }

  CancelMinuteAnnouncements();
}

public Action Timer_MinuteAnnouncement(Handle timer, any data)
{
  if (g_MaxPlayers == 0 || g_GameFinished)
  {
    g_MinuteAnnouncementTimer = null;
    return Plugin_Stop;
  }

  int redCount = AmountOfPlayersInTeam(TFTeam_Red);
  int blueCount = AmountOfPlayersInTeam(TFTeam_Blue);

  bool redFull = redCount >= g_MaxPlayers;
  bool blueFull = blueCount >= g_MaxPlayers;

  if ((redFull && !blueFull) || (blueFull && !redFull))
  {
    char waitingName[255];
    if (redFull && !blueFull)
    {
      g_CvarRedTeamName.GetString(waitingName, sizeof(waitingName));
    }
    else
    {
      g_CvarBlueTeamName.GetString(waitingName, sizeof(waitingName));
    }

    CPrintToChatAll("Waiting for teams to join. Red: %i/%i, Blue: %i/%i.", redCount, g_MaxPlayers, blueCount, g_MaxPlayers);
    return Plugin_Continue;
  }
  else
  {
    // Both teams are either full or empty, stop the timer
    g_MinuteAnnouncementTimer = null;
    return Plugin_Stop;
  }
}

public Action Timer_CheckPlayerTeams(Handle timer, any data)
{
  int realPlayers = 0;
  for (int i = 1; i <= MaxClients; i++)
  {
    if (IsClientInGame(i))
    {
      if (!IsFakeClient(i) && !IsClientSourceTV(i))
      {
        realPlayers++;
      }

      char steamID[64];
      GetSteamId(i, steamID, sizeof(steamID));

      int team = GetClientTeam(i);

      if (g_AssignedRedTeam.FindString(steamID) != -1)
      {
        if (AmountOfPlayersInTeam(TFTeam_Red) > g_MaxPlayers && g_MaxPlayers != 0)
        {
          KickClient(i, "Red team is full.");
          continue;
        }
        if (team != view_as<int>(TFTeam_Red))
        {
          ChangeClientTeam(i, view_as<int>(TFTeam_Red));
          TF2_SetPlayerClass(i, TFClass_Pyro, false, true);
        }
      }
      else if (g_AssignedBlueTeam.FindString(steamID) != -1)
      {
        if (AmountOfPlayersInTeam(TFTeam_Blue) > g_MaxPlayers && g_MaxPlayers != 0)
        {
          KickClient(i, "Blue team is full.");
          continue;
        }
        if (team != view_as<int>(TFTeam_Blue))
        {
          ChangeClientTeam(i, view_as<int>(TFTeam_Blue));
          TF2_SetPlayerClass(i, TFClass_Pyro, false, true);
        }
      }
      else
      {
        ChangeClientTeam(i, view_as<int>(TFTeam_Spectator));
        TF2_SetPlayerClass(i, TFClass_Pyro, false, true);
      }
    }
  }

  if (g_IsRecording && !g_GameFinished && realPlayers == 0)
  {
    LogMessage("All players left. Stopping recording and archiving demo.");
    SourceTV_StopRecording();
    ArchiveDemoFile();
  }

  UpdateMinuteAnnouncements();

  return Plugin_Continue;
}

public Action OnPlayerTeam(Event event, const char[] eventName, bool dontBroadcast)
{
  int client = GetClientOfUserId(event.GetInt("userid"));
  if (client <= 0 || !IsClientInGame(client))
  {
    return Plugin_Continue;
  }

  int team = event.GetInt("team");

  char steamID[64];
  GetSteamId(client, steamID, sizeof(steamID));

  LogMessage("Checking if %s is in, index red: %i, index blue: %i", steamID, g_AssignedRedTeam.FindString(steamID), g_AssignedBlueTeam.FindString(steamID));

  if (g_AssignedRedTeam.FindString(steamID) != -1)
  {
    if (team != view_as<int>(TFTeam_Red))
    {
      LogMessage("Moving client %s to RED team", steamID);
      ChangeClientTeam(client, view_as<int>(TFTeam_Red));
      return Plugin_Continue;
    }
  }

  if (g_AssignedBlueTeam.FindString(steamID) != -1)
  {
    if (team != view_as<int>(TFTeam_Blue))
    {
      LogMessage("Moving client %s to BLUE team", steamID);
      ChangeClientTeam(client, view_as<int>(TFTeam_Blue));
      return Plugin_Continue;
    }
  }

  return Plugin_Continue;
}

public Action OnTournamentStateUpdate(Event event, const char[] eventName, bool dontBroadcast)
{
  int readyState = event.GetInt("readyState");
  char blueName[255];
  char redName[255];

  g_CvarBlueTeamName.GetString(blueName, sizeof(blueName));
  g_CvarRedTeamName.GetString(redName, sizeof(redName));

  if (readyState)
  {
    char recordName[PLATFORM_MAX_PATH];
    GetRecordName(recordName, sizeof(recordName));

    SourceTV_StartRecording(recordName);

    CPrintToChatAll("%t", "tc_readyup");
    
    if (AmountOfPlayersInTeam(TFTeam_Red) < g_MinPlayers)
    {
      CPrintToChatAll("Team %s does not have enough players to ready up.", redName);
      ServerCommand("mp_tournament_restart");
      return Plugin_Handled;
    }
    if (AmountOfPlayersInTeam(TFTeam_Blue) < g_MinPlayers)
    {
      CPrintToChatAll("Team %s does not have enough players to ready up.", blueName);
      ServerCommand("mp_tournament_restart");
      return Plugin_Handled;
    }
  }

  return Plugin_Continue;
}
