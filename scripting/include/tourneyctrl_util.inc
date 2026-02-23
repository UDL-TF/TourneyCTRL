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
