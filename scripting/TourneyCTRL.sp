#include <ripext>
#include <sdktools>
#include <sourcemod>
#include <multicolors>
#include <tf2>
#include <tf2_stocks>
#include <discordWebhookAPI>
#include <SteamWorks>
#include <env_variables>
#include <json>

#pragma newdecls required
#pragma semicolon 1

ConVar g_CvarRedTeamName;
ConVar g_CvarBlueTeamName;
ConVar g_CvarWinLimit;
ConVar g_CvarTvEnabled;

bool g_IsRecording = false;
bool g_GameFinished = false;

char g_DemoPath[PLATFORM_MAX_PATH] = "demos";
char g_CurrentRecording[PLATFORM_MAX_PATH];

char g_PublicIp[32];
int g_ServerPort;

int g_MinPlayers;
int g_MaxPlayers;

char g_MatchId[128];
char g_RoundId[128];
char g_AwayTeam[256];
char g_AwayTeamId[128];
char g_HomeTeam[256];
char g_HomeTeamId[128];
char g_WinLimit[64];

ArrayList g_AssignedRedTeam;
ArrayList g_AssignedBlueTeam;

enum PlayerStatField
{
  PlayerStat_Kills,
  PlayerStat_Deaths,
  PlayerStat_Deflects,
  PlayerStat_TimeAlive,
  PlayerStat_LastSpawnTime
};

int g_PlayerStats[MAXPLAYERS + 1][PlayerStatField];

char g_DiscordWebhookUrl[256];
char g_DiscordUsername[64];
char g_DiscordAvatarUrl[256];
char g_ApiUploadDemoUrl[256];
char g_ApiSendScoresUrl[256];
char g_ApiPlayerStatsUrl[256];
char g_ApiSecret[128];

#define PublicIp g_PublicIp
#define ServerPort g_ServerPort
#define MpRedTeamName g_CvarRedTeamName
#define MpBlueTeamName g_CvarBlueTeamName
#define MatchID g_MatchId
#define RoundID g_RoundId
#define CurrentRecording g_CurrentRecording

#include <udl>
#include "tourneyctrl_config"
#include "tourneyctrl_util"
#include "tourneyctrl_recording"
#include "tourneyctrl_teams"
#include "tourneyctrl_stats"
#include "tourneyctrl_web"

public Plugin myinfo =
{
  name        = "TourneyCTRL",
  author      = "Tolfx",
  description = "Tournament Control Plugin",
  version     = "2.0.0",
  url         = "https://github.com/Tolfx/TourneyCTRL"
};

public void OnPluginStart()
{
  LoadTranslations("tourneyctrl.phrase.txt");

  RegAdminCmd("tc_record", CommandRecord, ADMFLAG_GENERIC, "Starts recording a demo");
  RegAdminCmd("tc_stoprecord", CommandStopRecord, ADMFLAG_GENERIC, "Stops recording a demo");
  RegAdminCmd("tc_restart", CommandRestart, ADMFLAG_GENERIC, "Restarts the tournament");
  RegAdminCmd("tc_reset_assigned", CommandResetAssigned, ADMFLAG_GENERIC, "Resets assigned players");

  g_CvarRedTeamName = FindConVar("mp_tournament_redteamname");
  g_CvarBlueTeamName = FindConVar("mp_tournament_blueteamname");
  g_CvarWinLimit = FindConVar("mp_winlimit");
  g_CvarTvEnabled = FindConVar("tv_enable");

  g_AssignedBlueTeam = new ArrayList(10);
  g_AssignedRedTeam = new ArrayList(10);

  if (!DirExists(g_DemoPath))
  {
    InitDirectory(g_DemoPath);
  }

  HookEvent("player_team", OnPlayerTeam, EventHookMode_Pre);
  HookEvent("teamplay_round_start", OnRoundStart);
  HookEvent("tf_game_over", OnGameOver);
  HookEvent("tournament_stateupdate", OnTournamentStateUpdate, EventHookMode_Pre);
  HookEvent("player_spawn", OnPlayerSpawn, EventHookMode_Post);
  HookEvent("player_death", OnPlayerDeath, EventHookMode_Post);

  LoadTourneyConfig();
  LoadMatchEnvironment();

  if (g_WinLimit[0] != '\0')
  {
    int winLimit = StringToInt(g_WinLimit);
    if (winLimit > 0)
    {
      ServerCommand("sm_cvar mp_winlimit %i", winLimit);
    }
  }

  AssignPlayersToTeams();
  GetIp();

  CreateTimer(1.0, Timer_CheckPlayerTeams, _, TIMER_REPEAT);
  CreateTimer(5.0, Timer_CheckWinLimitCorrector, _, TIMER_REPEAT);
}

void LoadMatchEnvironment()
{
  char minPlayers[32];
  char maxPlayers[32];

  if (!ReadEnvironmentValue("MATCH_ID", g_MatchId, sizeof(g_MatchId), true)) return;
  if (!ReadEnvironmentValue("ROUND_ID", g_RoundId, sizeof(g_RoundId), true)) return;
  if (!ReadEnvironmentValue("AWAY_TEAM", g_AwayTeam, sizeof(g_AwayTeam), true)) return;
  if (!ReadEnvironmentValue("AWAY_TEAM_ID", g_AwayTeamId, sizeof(g_AwayTeamId), true)) return;
  if (!ReadEnvironmentValue("HOME_TEAM", g_HomeTeam, sizeof(g_HomeTeam), true)) return;
  if (!ReadEnvironmentValue("HOME_TEAM_ID", g_HomeTeamId, sizeof(g_HomeTeamId), true)) return;
  ReadEnvironmentValue("WIN_LIMIT", g_WinLimit, sizeof(g_WinLimit), false);

  if (!ReadEnvironmentValue("MIN_PLAYERS", minPlayers, sizeof(minPlayers), true)) return;
  if (!ReadEnvironmentValue("MAX_PLAYERS", maxPlayers, sizeof(maxPlayers), true)) return;

  g_MaxPlayers = StringToInt(maxPlayers);
  g_MinPlayers = StringToInt(minPlayers);

  LogMessage("Home Team SteamIDs: %s", g_HomeTeam);
  LogMessage("Away Team SteamIDs: %s", g_AwayTeam);

  ServerCommand("con_logfile \"udl_%s_%s\"", g_MatchId, g_RoundId);
  ServerCommand("con_timestamp \"1\"");
}

bool ReadEnvironmentValue(const char[] name, char[] buffer, int bufferLength, bool required)
{
  GetEnvironmentVariable(name, buffer, bufferLength);
  if (buffer[0] == '\0')
  {
    if (required)
    {
      char message[128];
      Format(message, sizeof(message), "Failed to get %s env", name);
      SetFailState(message);
    }
    return false;
  }
  return true;
}

public void OnClientPostAdminCheck(int client)
{
  char steamID[64];
  GetSteamId(client, steamID, sizeof(steamID));

  LogMessage("SteamID %s, Client %i", steamID, client);

  if (GetUserAdmin(client) != INVALID_ADMIN_ID || IsClientObserver(client) || IsClientSourceTV(client))
  {
    return;
  }

  if (g_MaxPlayers == 0)
  {
    return;
  }

  if (g_AssignedRedTeam.FindString(steamID) != -1)
  {
    if (AmountOfPlayersInTeam(TFTeam_Red) >= g_MaxPlayers)
    {
      KickClient(client, "Your team is already full.");
      return;
    }
  }
  else if (g_AssignedBlueTeam.FindString(steamID) != -1)
  {
    if (AmountOfPlayersInTeam(TFTeam_Blue) >= g_MaxPlayers)
    {
      KickClient(client, "Your team is already full.");
      return;
    }
  }
  else
  {
    KickClient(client, "You are not allowed to join this match.");
  }

  SourceTvStatus();
}

public Action CommandResetAssigned(int client, int args)
{
  g_AssignedRedTeam.Clear();
  g_AssignedBlueTeam.Clear();

  CPrintToChat(client, "%t", "tc_reset_success");

  return Plugin_Continue;
}

public Action CommandRecord(int client, int args)
{
  if (g_IsRecording)
  {
    CPrintToChat(client, "%t", "tc_already_recording");
    return Plugin_Handled;
  }

  StartRecord();

  CPrintToChat(client, "%t", "tc_started_recording", g_CurrentRecording);

  return Plugin_Continue;
}

public Action CommandStopRecord(int client, int args)
{
  if (!g_IsRecording)
  {
    CPrintToChat(client, "%t", "tc_not_recording");
    return Plugin_Handled;
  }

  StopRecord();

  CPrintToChat(client, "%t", "tc_stopped_recording", g_CurrentRecording);

  return Plugin_Continue;
}

public Action CommandRestart(int client, int args)
{
  g_AssignedRedTeam.Clear();
  g_AssignedBlueTeam.Clear();
  ServerCommand("mp_tournament_restart");
  return Plugin_Handled;
}

public Action OnRoundStart(Event event, char[] eventName, bool dontBroadcast)
{
  char redTeamName[32];
  char blueTeamName[32];

  g_CvarRedTeamName.GetString(redTeamName, sizeof(redTeamName));
  g_CvarBlueTeamName.GetString(blueTeamName, sizeof(blueTeamName));

  int bestOf = g_CvarWinLimit.IntValue;

  CPrintToChatAll("%t", "tc_roundstart", redTeamName, blueTeamName, bestOf);

  SourceTvStatus();

  return Plugin_Handled;
}

public Action OnGameOver(Event event, char[] eventName, bool dontBroadcast)
{
  int    redScore  = GetTeamScore(view_as<int>(TFTeam_Red));
  int    blueScore = GetTeamScore(view_as<int>(TFTeam_Blue));

  TFTeam winner    = TFTeam_Unassigned;
  TFTeam loser     = TFTeam_Unassigned;

  if (redScore > blueScore)
  {
    winner = TFTeam_Red;
    loser  = TFTeam_Blue;
  }

  if (blueScore > redScore)
  {
    winner = TFTeam_Blue;
    loser  = TFTeam_Red;
  }

  if (winner != TFTeam_Unassigned)
  {
    char teamName[32];
    if (winner == TFTeam_Red)
      g_CvarRedTeamName.GetString(teamName, sizeof(teamName));
    else
      g_CvarBlueTeamName.GetString(teamName, sizeof(teamName));

    CPrintToChatAll("%t", "tc_gameover", teamName);
  }

  AnnounceWinner(winner, loser);
  StopRecord();
  UploadDemoFile();

  // Send player stats to backend
  SendAllPlayerStatsToBackend();
  ResetAllPlayerStats();

  g_GameFinished = true;

  CreateTimer(10.0, Timer_KickPlayers, _, TIMER_REPEAT);

  return Plugin_Continue;
}
// Register event hooks for player stats
public void OnMapStart()
{
  ResetAllPlayerStats();
}
public Action Timer_CheckWinLimitCorrector(Handle timer, any data)
{
  g_CvarWinLimit = FindConVar("mp_winlimit");

  if (!ReadEnvironmentValue("WIN_LIMIT", g_WinLimit, sizeof(g_WinLimit), true))
  {
    return Plugin_Continue;
  }

  LogMessage("The server has requested %s, and we currently have %i", g_WinLimit, g_CvarWinLimit.IntValue);

  int winLimit = StringToInt(g_WinLimit);

  if (winLimit != g_CvarWinLimit.IntValue)
  {
    LogError("WinLimit is not identical!");
    ServerCommand("sm_cvar mp_winlimit %i", winLimit);
  }
  else {
    LogMessage("Both are the same, stopping timer");
    return Plugin_Stop;
  }

  return Plugin_Continue;
}

public Action Timer_KickPlayers(Handle timer, any data)
{
  char kickMessage[255];
  Format(kickMessage, sizeof(kickMessage), "Thanks for playing, scores have been updated.");

  for (int i = 1; i <= MaxClients; i++)
  {
    if (IsClientInGame(i) && !IsClientSourceTV(i))
    {
      KickClientEx(i, kickMessage);
    }
  }

  return Plugin_Stop;
}

public void SourceTvStatus()
{
  // Ensure MaxClients is valid
  if (MaxClients <= 0)
  {
    return;
  }

  for (int client = 1; client <= MaxClients; client++)
  {
    if (IsClientSourceTV(client))
    {
      FakeClientCommand(client, "status");
    }
  }
}
