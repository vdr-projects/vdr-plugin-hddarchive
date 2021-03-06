/*
 * setup.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include "setup.h"

// --- cHddArchiveConfig -------------------------------------------------------

cHddArchiveConfig::cHddArchiveConfig()
{
   HideMainmenuEntry = false;
   ReplaceRecmenu = false;
   strcpy(ArchiveDevice, "/dev/usb0");
   strcpy(ArchiveMountpoint, "/media/usb0");
}

cHddArchiveConfig::~cHddArchiveConfig()
{
}

bool cHddArchiveConfig::SetupParse(const char *Name, const char *Value)
{
   if (!strcasecmp(Name, "HideMainmenuEntry"))
      HideMainmenuEntry = atoi(Value);
   else if (!strcasecmp(Name, "ReplaceRecmenu"))
      ReplaceRecmenu = atoi(Value);
   else if (!strcasecmp(Name, "ArchiveDevice"))
      strn0cpy(ArchiveDevice, Value, sizeof(ArchiveDevice));
   else if (!strcasecmp(Name, "ArchiveMountpoint"))
      strn0cpy(ArchiveMountpoint, Value, sizeof(ArchiveMountpoint));
   else
      return false;
   return true;
}

// --- cHddArchiveSetup --------------------------------------------------------

cHddArchiveSetup::cHddArchiveSetup()
{
   tmpHddArchiveConfig = HddArchiveConfig;
   Setup();
}

cHddArchiveSetup::~cHddArchiveSetup()
{
}

void cHddArchiveSetup::Setup(void)
{
   Add(new cMenuEditBoolItem(tr("Hide main menu entry"), &tmpHddArchiveConfig.HideMainmenuEntry));
   Add(new cMenuEditBoolItem(tr("Replace original recmenu"), &tmpHddArchiveConfig.ReplaceRecmenu));
   Add(new cMenuEditStrItem(tr("Archive device"), tmpHddArchiveConfig.ArchiveDevice, sizeof(tmpHddArchiveConfig.ArchiveDevice)));
   Add(new cMenuEditStrItem(tr("Archive mountpoint"), tmpHddArchiveConfig.ArchiveMountpoint, sizeof(tmpHddArchiveConfig.ArchiveMountpoint)));
}

void cHddArchiveSetup::Store(void)
{
   HddArchiveConfig = tmpHddArchiveConfig;
   SetupStore("HideMainmenuEntry", HddArchiveConfig.HideMainmenuEntry);
   SetupStore("ReplaceRecmenu", HddArchiveConfig.ReplaceRecmenu);
   SetupStore("ArchiveDevice", HddArchiveConfig.ArchiveDevice);
   SetupStore("ArchiveMountpoint", HddArchiveConfig.ArchiveMountpoint);
}

eOSState cHddArchiveSetup::ProcessKey(eKeys Key)
{
   eOSState state = cMenuSetupPage::ProcessKey(Key);
   if (Key == kOk)
      Store();
   return state;
}
