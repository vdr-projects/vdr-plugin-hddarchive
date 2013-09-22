/*
 * mymenu.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __MYMENU_H
#define __MYMENU_H

#include <vdr/cutter.h>
#include <vdr/interface.h>
#include <vdr/menu.h>
#include <vdr/osdbase.h>
#include <vdr/recording.h>
#include <vdr/tools.h>
#include <vdr/videodir.h>
#include "archive.h"
#include "setup.h"

class cMyMenuRecordingItem;

class cMyMenuRecordings : public cOsdMenu {
   private:
      char *base;
      int level;
      bool show;
      int recordingsState;
      int helpKeys;
      bool isPluginReplay;
      void SetHelpKeys(void);
      void Set(bool Refresh = false);
      cString DirectoryName(void);
      bool Open(bool OpenSubMenus = false);
      bool Prepare(cMyMenuRecordingItem *Ri);
      eOSState Play(void);
      eOSState Rewind(void);
      eOSState Archive(void);
      eOSState Info(void);
      eOSState Sort(void);
   public:
      cMyMenuRecordings(const char *Base = NULL, int Level = 0, bool OpenSubMenus = false, bool Show = true);
      // when Show = false, the menu works in background wothout being displayed
      virtual ~cMyMenuRecordings();
      virtual void SetCurrent(cOsdItem *Item, bool IsPluginReplay = true);
      virtual eOSState ProcessKey(eKeys Key);
};

class cMyMenuRecordingItem : public cOsdItem {
   private:
      cRecording *recording;
      int level;
      char *name;
      int totalEntries;
      int newEntries;
      bool isArchive = false;
      bool isMounted = false;
      bool isLinked = false;
      char *archiveId;
      char *uniqueFolder;
   public:
      cMyMenuRecordingItem(cRecording *Recording, int Level);
      virtual ~cMyMenuRecordingItem();
      void IncrementCounter(bool New);
      virtual void SetMenuItem(cSkinDisplayMenu *DisplayMenu, int Index, bool Current, bool Selectable);
      const char *Name(void) { return name; }
      cRecording *Recording(void) { return recording; }
      bool IsDirectory(void) { return name != NULL; }
      bool IsArchive(void) { return isArchive; }
      bool IsMounted(void) { return isMounted; }
      bool IsLinked(void) { return isLinked; }
      const char *ArchiveId(void) { return archiveId; }
      const char *UniqueFolder(void) { return uniqueFolder; }
};

class cMenuRecording : public cOsdMenu {
   private:
      const cRecording *recording;
      bool withButtons;
   public:
      cMenuRecording(const cRecording *Recording, bool WithButtons = false);
      virtual void Display(void);
      virtual eOSState ProcessKey(eKeys Key);
};

class cMyReplayControl : public cReplayControl {
   private:
      bool isPluginReplay;
   public:
      cMyReplayControl(bool IsPluginReplay = true);
      virtual ~cMyReplayControl();
};

#endif // __MYMENU_H
