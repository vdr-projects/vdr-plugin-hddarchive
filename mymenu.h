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
      void SetHelpKeys(void);
      void Set(bool Refresh = false);
      cString DirectoryName(void);
      bool Open(bool OpenSubMenus = false);
      bool Prepare(const cRecording *Recording);
      eOSState Play(void);
      eOSState Archive(void);
      eOSState Delete(void);
      eOSState Info(void);
      eOSState Commands(eKeys Key = kNone);
      eOSState Sort(void);
   public:
      cMyMenuRecordings(const char *Base = NULL, int Level = 0, bool OpenSubMenus = false, bool Show = true);
      // when Show = false, the menu works in background without being displayed
      virtual ~cMyMenuRecordings();
      virtual eOSState ProcessKey(eKeys Key);
      eOSState Play(const cRecording *Recording, bool IsPluginReplay = true);
};

class cMyMenuRecordingItem : public cOsdItem {
   private:
      cRecording *recording;
      int level;
      char *name;
      int totalEntries;
      int newEntries;
   public:
      cMyMenuRecordingItem(cRecording *Recording, int Level);
      virtual ~cMyMenuRecordingItem();
      void IncrementCounter(bool New);
      virtual void SetMenuItem(cSkinDisplayMenu *DisplayMenu, int Index, bool Current, bool Selectable);
      const char *Name(void) { return name; }
      int Level(void) { return level; }
      cRecording *Recording(void) { return recording; }
      bool IsDirectory(void) { return name != NULL; }
};

class cMenuRecording : public cOsdMenu {
   private:
      cRecording *recording;
      cString originalFileName;
      int recordingsState;
      bool withButtons;
      bool RefreshRecording(void);
   public:
      cMenuRecording(cRecording *Recording, bool WithButtons = false);
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

class cMenuPathEdit : public cOsdMenu {
   private:
      cString path;
      char folder[PATH_MAX];
      char name[NAME_MAX];
      cMenuEditStrItem *folderItem;
      int pathIsInUse;
      eOSState SetFolder(void);
      eOSState Folder(void);
      eOSState ApplyChanges(void);
   public:
      cMenuPathEdit(const char *Path);
      virtual eOSState ProcessKey(eKeys Key);
};

#endif // __MYMENU_H
