/*
 * setup.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef SETUP_H
#define SETUP_H

#include <vdr/plugin.h>

class cHddArchiveConfig
{
   public:
      cHddArchiveConfig();
      ~cHddArchiveConfig();
      bool SetupParse(const char *Name, const char *Value);
      int HideMainmenuEntry;
      int ReplaceRecmenu;
      char ArchiveDevice[NAME_MAX + 1];
      char ArchiveMountpoint[NAME_MAX + 1];
};

class cHddArchiveSetup : public cMenuSetupPage
{
   public:
      cHddArchiveSetup();
      virtual ~cHddArchiveSetup();
      int HideMainmenuEntry;
      int ReplaceRecmenu;
      char ArchiveDevice[NAME_MAX + 1];
      char ArchiveMountpoint[NAME_MAX + 1];
   private:
      void Setup(void);
      virtual void Store(void);
      virtual eOSState ProcessKey(eKeys Key);
      cHddArchiveConfig tmpHddArchiveConfig;
};

extern cHddArchiveConfig HddArchiveConfig;

#endif
