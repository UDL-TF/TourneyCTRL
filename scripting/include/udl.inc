#if defined _udl_included
 #endinput
#endif
#define _udl_included

public Action
  Timer_CallSingleFunction(Handle timer, DataPack data)
{
  data.Reset();
  Call_StartFunction(INVALID_HANDLE, data.ReadFunction());
  Call_Finish();
  delete data;
  return Plugin_Stop;
}

void GetIp(int maxTries = 0, int currentTry = 0)
{
  int ipaddress[4];
  SteamWorks_GetPublicIP(ipaddress);
  Format(PublicIp, sizeof(PublicIp), "%d.%d.%d.%d", ipaddress[0], ipaddress[1], ipaddress[2], ipaddress[3]);
  ServerPort = GetConVarInt(FindConVar("hostport"));
}

public void OnWebHookExecuted(HTTPResponse response, DataPack pack)
{
  if (response.Status == HTTPStatus_NoContent || response.Status == HTTPStatus_OK)
  {
    PrintToServer("Webhook executed successfully.");
  }
  else
  {
    PrintToServer("Webhook execution failed with status: %d", response.Status);
    char responseBody[1024];
    response.Data.ToString(responseBody, sizeof(responseBody));
    PrintToServer("Response body: %s", responseBody);
  }
}

stock void GetSteamId(int client, char[] steamID, int len)
{
  GetClientAuthId(client, AuthId_SteamID64, steamID, len);
}

stock bool IsValidPlayer(int client, bool alive = false)
{
  if (client >= 1 && client <= MaxClients && IsClientConnected(client) && IsClientInGame(client) && !IsFakeClient(client) && (alive == false || IsPlayerAlive(client)))
  {
    return true;
  }

  return false;
}

void InitDirectory(const char[] sDir)
{
  char sPieces[32][PLATFORM_MAX_PATH];
  char sPath[PLATFORM_MAX_PATH];
  int  iNumPieces = ExplodeString(sDir, "/", sPieces, sizeof(sPieces), sizeof(sPieces[]));

  for (int i = 0; i < iNumPieces; i++)
  {
    Format(sPath, sizeof(sPath), "%s/%s", sPath, sPieces[i]);
    if (!DirExists(sPath))
    {
      CreateDirectory(sPath, 509);
    }
  }
}

void GetRecordName(char[] name, int size)
{
  char time[16];
  char teamNameBlue[32];
  char teamNameRed[32];

  MpRedTeamName.GetString(teamNameRed, sizeof(teamNameRed));
  MpBlueTeamName.GetString(teamNameBlue, sizeof(teamNameBlue));

  FormatTime(time, sizeof(time), "%Y%m%d-%H%M%S", GetTime());

  char recordName[PLATFORM_MAX_PATH];
  Format(recordName, sizeof(recordName), "auto-%s-%s_vs_%s-match-%s-round-%s", time, teamNameRed, teamNameBlue, MatchID, RoundID);

  Format(name, size, "%s", recordName);
  Format(CurrentRecording, sizeof(CurrentRecording), "%s", recordName);
}
