#if defined _tourneyctrl_recording_included
 #endinput
#endif
#define _tourneyctrl_recording_included

void StartRecord()
{
  if (g_CvarTvEnabled.BoolValue && !g_IsRecording && !g_GameFinished)
  {
    char recordName[PLATFORM_MAX_PATH];
    GetRecordName(recordName, sizeof(recordName));

    ServerCommand("tv_record \"%s/%s\"", g_DemoPath, g_CurrentRecording);
    g_IsRecording = true;

    SourceTvStatus();

    LogMessage("Recording to %s", g_CurrentRecording);
  }
}

void StopRecord()
{
  if (g_CvarTvEnabled.BoolValue)
  {
    ServerCommand("tv_stoprecord");
    g_IsRecording = false;
  }
}

void UploadDemoFile()
{
  UploadDemoFileWithRetry(0);
}

void ArchiveDemoFile()
{
  char filePath[PLATFORM_MAX_PATH];
  Format(filePath, sizeof(filePath), "%s/%s.dem", g_DemoPath, g_CurrentRecording);

  char archivePath[PLATFORM_MAX_PATH] = "demo/archive";

  if (!DirExists(archivePath))
  {
    InitDirectory(archivePath);
  }

  char newFilePath[PLATFORM_MAX_PATH];
  Format(newFilePath, sizeof(newFilePath), "%s/%s.dem", archivePath, g_CurrentRecording);

  if (FileExists(filePath))
  {
    if (RenameFile(newFilePath, filePath))
    {
      LogMessage("Archived demo file to %s", newFilePath);
    }
    else
    {
      LogError("Failed to archive demo file from %s to %s", filePath, newFilePath);
    }
  }
}
