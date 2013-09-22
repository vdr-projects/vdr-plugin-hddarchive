/*
 * archive.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __ARCHIVE_H
#define __ARCHIVE_H

#include <vdr/menu.h>
#include <vdr/recording.h>
#include <vdr/tools.h>
#include "mymenu.h"

class cMyMenuRecordingItem;

class cArchive {
   private:
      static char buf[8];
      static bool FindUniqueFolder(const char *Parent, const char *Target);
   public:
      static cString archivePath;
      static bool Filecopy(const char *Sourcefile, const char *Destfile);
      static const char *GetArchiveId(const cRecording *Recording);
      static bool MountArchive(void);
      static bool UnmountArchive(void);
      static bool LinkArchive(const cRecording *Recording);
      static bool UnlinkArchive(const cRecording *Recording);
};

#endif //__ARCHIVE_H
