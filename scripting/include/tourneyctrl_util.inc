#if defined _tourneyctrl_util_included
 #endinput
#endif
#define _tourneyctrl_util_included

#define API_RETRY_MAX 3
#define API_RETRY_DELAY 5.0

void SendScoresRequest(TFTeam team, int attempt)
{
  HTTPRequest request = new HTTPRequest(g_ApiSendScoresUrl);
  request.SetHeader("Content-Type", "application/json");
  AppendApiSecret(request);

  JSONObject jsonData = new JSONObject();
  int matchId = StringToInt(g_MatchId);
  int roundId = StringToInt(g_RoundId);
  int homeId = StringToInt(g_HomeTeamId);
  int awayId = StringToInt(g_AwayTeamId);
  int homeScore = GetTeamScore(view_as<int>(TFTeam_Red));
  int awayScore = GetTeamScore(view_as<int>(TFTeam_Blue));

  jsonData.SetInt("match_id", matchId);
  jsonData.SetInt("round_id", roundId);
  jsonData.SetInt("winner_team_id", (team == TFTeam_Red) ? homeId : awayId);
  jsonData.SetInt("loser_team_id", (team == TFTeam_Red) ? awayId : homeId);
  jsonData.SetInt("away_points", awayScore);
  jsonData.SetInt("home_points", homeScore);

  DataPack pack = new DataPack();
  pack.WriteCell(attempt);
  pack.WriteCell(view_as<int>(team));
  request.Post(jsonData, OnSendScoresCompleted, pack);
  delete jsonData;
}

public Action Timer_RetrySendScores(Handle timer, any data)
{
  DataPack pack = view_as<DataPack>(data);
  pack.Reset();
  int attempt = pack.ReadCell();
  TFTeam team = view_as<TFTeam>(pack.ReadCell());
  delete pack;
  SendScoresRequest(team, attempt);
  return Plugin_Stop;
}

public void OnSendScoresCompleted(HTTPResponse response, any value)
{
  DataPack pack = view_as<DataPack>(value);
  pack.Reset();
  int attempt = pack.ReadCell();
  TFTeam team = view_as<TFTeam>(pack.ReadCell());

  if (response.Status == HTTPStatus_NoContent || response.Status == HTTPStatus_OK)
  {
    PrintToServer("Request completed successfully.");
    delete pack;
    return;
  }

  PrintToServer("Request failed with status: %d", response.Status);
  char responseBody[1024];
  response.Data.ToString(responseBody, sizeof(responseBody));
  PrintToServer("Response body: %s", responseBody);

  int nextAttempt = attempt + 1;
  delete pack;
  if (nextAttempt < API_RETRY_MAX)
  {
    DataPack retryPack = new DataPack();
    retryPack.WriteCell(nextAttempt);
    retryPack.WriteCell(view_as<int>(team));
    CreateTimer(API_RETRY_DELAY, Timer_RetrySendScores, retryPack);
  }
}

void SendPlayerStatsRequest(DataPack pack)
{
  pack.Reset();
  int attempt = pack.ReadCell();
  char steamId[128];
  pack.ReadString(steamId, sizeof(steamId));
  int matchId = pack.ReadCell();
  int kills = pack.ReadCell();
  int deaths = pack.ReadCell();
  int deflects = pack.ReadCell();
  int timeAlive = pack.ReadCell();

  JSONObject body = new JSONObject();
  body.SetString("steam_id", steamId);
  body.SetInt("league_match_id", matchId);
  body.SetInt("kills", kills);
  body.SetInt("deaths", deaths);
  body.SetInt("deflects", deflects);
  body.SetInt("time_alive_seconds", timeAlive);

  HTTPRequest req = new HTTPRequest(g_ApiPlayerStatsUrl);
  req.SetHeader("Content-Type", "application/json");
  AppendApiSecret(req);
  pack.Reset();
  pack.WriteCell(attempt);
  pack.WriteString(steamId);
  pack.WriteCell(matchId);
  pack.WriteCell(kills);
  pack.WriteCell(deaths);
  pack.WriteCell(deflects);
  pack.WriteCell(timeAlive);
  req.Post(body, OnPlayerStatsCompleted, pack);
  delete body;
}

public Action Timer_RetryPlayerStats(Handle timer, any data)
{
  DataPack pack = view_as<DataPack>(data);
  SendPlayerStatsRequest(pack);
  return Plugin_Stop;
}

public void OnPlayerStatsCompleted(HTTPResponse response, any value)
{
  DataPack pack = view_as<DataPack>(value);
  pack.Reset();
  int attempt = pack.ReadCell();
  char steamId[128];
  pack.ReadString(steamId, sizeof(steamId));
  int matchId = pack.ReadCell();
  int kills = pack.ReadCell();
  int deaths = pack.ReadCell();
  int deflects = pack.ReadCell();
  int timeAlive = pack.ReadCell();

  if (response.Status == HTTPStatus_NoContent || response.Status == HTTPStatus_OK)
  {
    PrintToServer("Player stats posted successfully.");
    delete pack;
    return;
  }

  PrintToServer("Player stats post failed with status: %d", response.Status);
  char responseBody[1024];
  response.Data.ToString(responseBody, sizeof(responseBody));
  PrintToServer("Response body: %s", responseBody);

  int nextAttempt = attempt + 1;
  delete pack;
  if (nextAttempt < API_RETRY_MAX)
  {
    DataPack retryPack = new DataPack();
    retryPack.WriteCell(nextAttempt);
    retryPack.WriteString(steamId);
    retryPack.WriteCell(matchId);
    retryPack.WriteCell(kills);
    retryPack.WriteCell(deaths);
    retryPack.WriteCell(deflects);
    retryPack.WriteCell(timeAlive);
    CreateTimer(API_RETRY_DELAY, Timer_RetryPlayerStats, retryPack);
  }
}

void UploadDemoFileWithRetry(int attempt)
{
  char filePath[PLATFORM_MAX_PATH];
  Format(filePath, sizeof(filePath), "%s/%s.dem", g_DemoPath, g_CurrentRecording);

  HTTPRequest request = new HTTPRequest(g_ApiUploadDemoUrl);
  request.SetHeader("Content-Type", "application/octet-stream");
  AppendApiSecret(request);

  DataPack pack = new DataPack();
  pack.WriteCell(attempt);
  request.UploadFile(filePath, OnDemoUploadCompletedRetry, pack);
}

public Action Timer_RetryUploadDemo(Handle timer, any data)
{
  DataPack pack = view_as<DataPack>(data);
  pack.Reset();
  int attempt = pack.ReadCell();
  delete pack;
  UploadDemoFileWithRetry(attempt);
  return Plugin_Stop;
}

public void OnDemoUploadCompletedRetry(HTTPStatus status, any value)
{
  DataPack pack = view_as<DataPack>(value);
  pack.Reset();
  int attempt = pack.ReadCell();

  if (status == HTTPStatus_OK)
  {
    PrintToServer("Demo upload completed successfully.");
    delete pack;
    return;
  }

  PrintToServer("Demo upload failed with status: %d", status);
  int nextAttempt = attempt + 1;
  delete pack;
  if (nextAttempt < API_RETRY_MAX)
  {
    DataPack retryPack = new DataPack();
    retryPack.WriteCell(nextAttempt);
    CreateTimer(API_RETRY_DELAY, Timer_RetryUploadDemo, retryPack);
  }
}
