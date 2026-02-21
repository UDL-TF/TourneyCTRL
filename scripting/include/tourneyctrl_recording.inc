#if defined _tourneyctrl_recording_included
 #endinput
#endif
#define _tourneyctrl_recording_included

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
