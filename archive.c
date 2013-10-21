/*
 * archive.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/interface.h>
#include <vdr/videodir.h>
#include "archive.h"

// --- cArchive ----------------------------------------------------------------

cString cArchive::archivePath = NULL;
char cArchive::buf[8] = "";

bool cArchive::Filecopy(const char *Sourcefile, const char *Destfile)
{
   FILE *source = fopen(Sourcefile, "r");
   FILE *dest = fopen(Destfile, "w");
   if (!source)
      return false;
   if (!dest) {
      fclose(source);
      return false;
   }
   size_t l1;
   size_t l2;
   unsigned char buffer[8192];
   while ((l1 = fread(buffer, 1, sizeof buffer, source)) > 0) {
      l2 = fwrite(buffer, 1, l1, dest);
      if (l2 < 0 || l2 < l1) {
         fclose(source);
         fclose(dest);
         return false;
      }
   }
   fclose(source);
   fclose(dest);
   return true;
}

const char *cArchive::GetArchiveId(const cRecording *Recording)
{
   buf[0] = 0;
   cString archiveFileName = cString::sprintf("%s%s", Recording->FileName(), "/hdd");
   FILE *f = fopen(archiveFileName, "r");
   if (f) {
      if (fgets(buf, sizeof(buf), f)) {
         char *p = strchr(buf, '\n');
         if (p)
            *p = 0;
      }
      fclose(f);
   }
   return buf;
}

bool cArchive::MountArchive(void)
{
   char *cmd = NULL;
   asprintf(&cmd, "mount %s", HddArchiveConfig.ArchiveMountpoint);
   if (SystemExec(cmd)) {
      free(cmd);
      return false;
   }
   free(cmd);
   return true;
}

bool cArchive::UnmountArchive(void)
{
   char *cmd = NULL;
   asprintf(&cmd, "umount %s", HddArchiveConfig.ArchiveMountpoint);
   if (SystemExec(cmd)) {
      free(cmd);
      return false;
   }
   free(cmd);
   return true;
}

bool cArchive::LinkArchive(const cRecording *Recording)
{
   const char *uniqueFolder = strrchr(Recording->FileName(), '/') + 1;
   if (!FindUniqueFolder(HddArchiveConfig.ArchiveMountpoint, uniqueFolder))
      return false;
   cString videoPath = Recording->FileName();
   cReadDir dir(archivePath);
   struct dirent *e;
   cString sourcefile;
   cString destfile;
   while (e = dir.Next()) {
      if (e->d_type == DT_REG) {
         sourcefile = AddDirectory(archivePath, e->d_name);
         destfile = AddDirectory(videoPath, e->d_name);
         symlink((const char*)sourcefile, (const char*)destfile);
      }
   }
   return true;
}

bool cArchive::UnlinkArchive(const cRecording *Recording)
{
   cString videoPath = Recording->FileName();
   cReadDir dir(videoPath);
   struct dirent *e;
   cString file;
   while (e = dir.Next()) {
      if (e->d_type == DT_LNK) {
         file = AddDirectory(videoPath, e->d_name);
         if (remove((const char*)file))
            return false;
      }
   }
   return true;
}

bool cArchive::FindUniqueFolder(const char *Parent, const char *Target)
{
   cReadDir dir(Parent);
   struct dirent *e;
   while (e = dir.Next()) {
      if (!e->d_type == DT_DIR || e->d_type == DT_LNK)
         continue;
      cString child = AddDirectory(Parent, e->d_name);
      if (!strcmp(e->d_name, Target)) {
         archivePath = child;
         return true;
      }
      else if (FindUniqueFolder(child, Target))
         return true;
      }
   return false;
}
