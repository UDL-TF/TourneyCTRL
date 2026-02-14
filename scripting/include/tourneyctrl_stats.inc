#if defined _tourneyctrl_stats_included
 #endinput
#endif
#define _tourneyctrl_stats_included

void SendAllPlayerStatsToBackend()
{
  for (int client = 1; client <= MaxClients; client++)
  {
    if (!IsClientInGame(client) || IsFakeClient(client))
    {
      continue;
    }

    char steamID[128];
    GetSteamId(client, steamID, sizeof(steamID));

    if (IsPlayerAlive(client))
    {
      int spawnTime = g_PlayerStats[client][PlayerStat_LastSpawnTime];
      if (spawnTime > 0)
      {
        g_PlayerStats[client][PlayerStat_TimeAlive] += GetTime() - spawnTime;
      }
    }

    DataPack pack = new DataPack();
    pack.WriteCell(0);
    pack.WriteString(steamID);
    pack.WriteCell(StringToInt(g_MatchId));
    pack.WriteCell(g_PlayerStats[client][PlayerStat_Kills]);
    pack.WriteCell(g_PlayerStats[client][PlayerStat_Deaths]);
    pack.WriteCell(g_PlayerStats[client][PlayerStat_Deflects]);
    pack.WriteCell(g_PlayerStats[client][PlayerStat_TimeAlive]);
    SendPlayerStatsRequest(pack);
  }
}

public void OnPlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
  int client = GetClientOfUserId(event.GetInt("userid"));
  if (!IsClientInGame(client) || IsFakeClient(client))
  {
    return;
  }

  g_PlayerStats[client][PlayerStat_LastSpawnTime] = GetTime();
}

public void OnPlayerDeath(Event event, const char[] name, bool dontBroadcast)
{
  int victim = GetClientOfUserId(event.GetInt("userid"));
  int attacker = GetClientOfUserId(event.GetInt("attacker"));

  if (victim > 0 && victim <= MaxClients)
  {
    g_PlayerStats[victim][PlayerStat_Deaths]++;
    int spawnTime = g_PlayerStats[victim][PlayerStat_LastSpawnTime];
    if (spawnTime > 0)
    {
      g_PlayerStats[victim][PlayerStat_TimeAlive] += GetTime() - spawnTime;
    }
  }

  if (attacker > 0 && attacker <= MaxClients && attacker != victim)
  {
    g_PlayerStats[attacker][PlayerStat_Kills]++;
  }
}

void ResetAllPlayerStats()
{
  for (int client = 1; client <= MaxClients; client++)
  {
    for (int stat = 0; stat <= view_as<int>(PlayerStat_LastSpawnTime); stat++)
    {
      g_PlayerStats[client][stat] = 0;
    }
  }
}

public void TFDB_OnRocketDeflect(int index, int entity, int owner)
{
  OnPlayerDeflect(owner);
}

void OnPlayerDeflect(int client)
{
  if (client > 0 && client <= MaxClients)
  {
    g_PlayerStats[client][PlayerStat_Deflects]++;
  }
}
