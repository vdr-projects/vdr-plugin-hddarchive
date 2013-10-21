/*
 * hddarchive.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include <vdr/recording.h>
#include "mymenu.h"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "Archive HDD functions";
static const char *MAINMENUENTRY  = "HDD-Archive";
cHddArchiveConfig HddArchiveConfig;

class cPluginHddarchive : public cPlugin {
   private:
   public:
      cPluginHddarchive(void);
      virtual ~cPluginHddarchive();
      virtual const char *Version(void) { return VERSION; }
      virtual const char *Description(void) { return DESCRIPTION; }
      virtual const char *CommandLineHelp(void);
      virtual bool ProcessArgs(int argc, char *argv[]);
      virtual bool Initialize(void);
      virtual bool Start(void);
      virtual void Stop(void);
      virtual void Housekeeping(void);
      virtual void MainThreadHook(void);
      virtual cString Active(void);
      virtual time_t WakeupTime(void);
      virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
      virtual cOsdObject *MainMenuAction(void);
      virtual cMenuSetupPage *SetupMenu(void);
      virtual bool SetupParse(const char *Name, const char *Value);
      virtual bool Service(const char *Id, void *Data = NULL);
      virtual const char **SVDRPHelpPages(void);
      virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
};

cPluginHddarchive::cPluginHddarchive(void)
{

}

cPluginHddarchive::~cPluginHddarchive()
{
}

const char *cPluginHddarchive::CommandLineHelp(void)
{
   return NULL;
}

bool cPluginHddarchive::ProcessArgs(int argc, char *argv[])
{
   return true;
}

bool cPluginHddarchive::Initialize(void)
{
   return true;
}

bool cPluginHddarchive::Start(void)
{
   return true;
}

void cPluginHddarchive::Stop(void)
{
}

void cPluginHddarchive::Housekeeping(void)
{
}

void cPluginHddarchive::MainThreadHook(void)
{
}

cString cPluginHddarchive::Active(void)
{
   return NULL;
}

time_t cPluginHddarchive::WakeupTime(void)
{
   return 0;
}

cOsdObject *cPluginHddarchive::MainMenuAction(void)
{
   return new cMyMenuRecordings();
}

cMenuSetupPage *cPluginHddarchive::SetupMenu(void)
{
   return new cHddArchiveSetup();
}

bool cPluginHddarchive::SetupParse(const char *Name, const char *Value)
{
   return HddArchiveConfig.SetupParse(Name, Value);
}

struct Hddarchive_archiveid_v1_0
{
   const cRecording *recording;
   bool isarchive;
   const char *archiveid;
};

struct Hddarchive_play_v1_0
{
   const cRecording *recording;
};

bool cPluginHddarchive::Service(const char *Id, void *Data)
{
   if (!strcmp(Id, "Hddarchive-archiveid_v1.0")) {
      if (Data == NULL)
         return true;
      Hddarchive_archiveid_v1_0 *archive = (Hddarchive_archiveid_v1_0 *)Data;
      archive->archiveid = cArchive::GetArchiveId(archive->recording);
      archive->isarchive = strcmp(archive->archiveid, "") ? true : false;
      return true;
   }

   if (!strcmp(Id, "Hddarchive-play_v1.0")) {
      if (Data == NULL)
         return false;
      Hddarchive_play_v1_0 *play = (Hddarchive_play_v1_0 *)Data;
      cMyMenuRecordings player(NULL, 0, false, false);
      player.Play(play->recording, false);
      return true;
   }
   return false;
}

const char **cPluginHddarchive::SVDRPHelpPages(void)
{
   return NULL;
}

cString cPluginHddarchive::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
   return NULL;
}

VDRPLUGINCREATOR(cPluginHddarchive);
