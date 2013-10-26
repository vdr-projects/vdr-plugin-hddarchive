/*
 * mymenu.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include "mymenu.h"

// --- cMyMenuRecordings -------------------------------------------------------

cMyMenuRecordings::cMyMenuRecordings(const char *Base, int Level, bool OpenSubMenus, bool Show)
: cOsdMenu(Base ? Base : tr("HDD-Archive"), 9, 6, 6), show(Show)
{
   SetMenuCategory(mcRecording);
   base = Base ? strdup(Base) : NULL;
   level = Setup.RecordingDirs ? Level : -1;
   Recordings.StateChanged(recordingsState); // just to get the current state
   helpKeys = -1;
   if (show)
      Display(); // this keeps the higher level menus from showing up briefly when pressing 'Back' during replay
   Set();
   if (Current() < 0)
      SetCurrent(First());
   else if (OpenSubMenus && cMyReplayControl::LastReplayed() && Open(true))
      return;
   if (show)
      Display();
   SetHelpKeys();
}

cMyMenuRecordings::~cMyMenuRecordings()
{
   helpKeys = -1;
   free(base);
}

void cMyMenuRecordings::SetHelpKeys(void)
{
   cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
   int NewHelpKeys = 0;
   if (ri) {
      if (ri->IsDirectory())
         NewHelpKeys = 1;
      else {
         NewHelpKeys = 2;
      if (ri->Recording()->Info()->Title())
         NewHelpKeys = 3;
      }
   }
   if (NewHelpKeys != helpKeys) {
      switch (NewHelpKeys) {
         case 0:  SetHelp(NULL);
                  break;
         case 1:  SetHelp(tr("Button$Open"));
                  break;
         case 2:
         case 3:  SetHelp(tr("Button$Play"), tr("Button$Rewind"), tr("Button$Archive"), NewHelpKeys == 3 ? tr("Button$Info") : NULL);
         default: ;
      }
      helpKeys = NewHelpKeys;
   }
}

void cMyMenuRecordings::Set(bool Refresh)
{
   const char *CurrentRecording = cMyReplayControl::LastReplayed();
   cMyMenuRecordingItem *LastItem = NULL;
   cThreadLock RecordingsLock(&Recordings);

   if (Refresh) {
      if (cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current()))
         CurrentRecording = ri->Recording()->FileName();
   }
   Clear();
   GetRecordingsSortMode(DirectoryName());
   Recordings.Sort();
   for (cRecording *recording = Recordings.First(); recording; recording = Recordings.Next(recording)) {
      if (!base || (strstr(recording->Name(), base) == recording->Name() && recording->Name()[strlen(base)] == FOLDERDELIMCHAR)) {
         cMyMenuRecordingItem *Item = new cMyMenuRecordingItem(recording, level);
         cMyMenuRecordingItem *LastDir = NULL;
         if (Item->IsDirectory()) {
            // Sorting may ignore non-alphanumeric characters, so we need to explicitly handle directories in case they only differ in such characters:
            for (cMyMenuRecordingItem *p = LastItem; p; p = dynamic_cast<cMyMenuRecordingItem *>(p->Prev())) {
               if (p->Name() && strcmp(p->Name(), Item->Name()) == 0) {
                  LastDir = p;
                  break;
               }
            }
         }
         if (*Item->Text() && !LastDir) {
            Add(Item);
            LastItem = Item;
            if (Item->IsDirectory())
               LastDir = Item;
         }
         else
            delete Item;
         if (LastItem || LastDir) {
            if (CurrentRecording && strcmp(CurrentRecording, recording->FileName()) == 0)
               SetCurrent(LastDir ? LastDir : LastItem);
         }
         if (LastDir)
            LastDir->IncrementCounter(recording->IsNew());
      }
   }
   if (Refresh && show)
      Display();
}

cString cMyMenuRecordings::DirectoryName(void)
{
#if APIVERSNUM > 20101
   cString d(cVideoDirectory::Name());
#else
   cString d(VideoDirectory);
#endif
   if (base) {
      char *s = ExchangeChars(strdup(base), true);
      d = AddDirectory(d, s);
      free(s);
   }
   return d;
}

bool cMyMenuRecordings::Open(bool OpenSubMenus)
{
   cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
   if (ri && ri->IsDirectory()) {
      const char *t = ri->Name();
      cString buffer;
      if (base) {
         buffer = cString::sprintf("%s~%s", base, t);
         t = buffer;
      }
      AddSubMenu(new cMyMenuRecordings(t, level + 1, OpenSubMenus));
      return true;
   }
   return false;
}

bool cMyMenuRecordings::Prepare(const cRecording *Recording)
{
   const char *archiveId = cArchive::GetArchiveId(Recording);
   if (strcmp(archiveId, "")) {
      char *msg = NULL;
      asprintf(&msg, tr("Please attach archive-hdd %s"), archiveId);
      if (!Interface->Confirm(msg)) {
         free(msg);
         return false;
      }
      free(msg);
      if (!cArchive::MountArchive()) {
         Skins.Message(mtError, tr("Could not mount archive-hdd!"));
         return false;
      }
      if (!cArchive::LinkArchive(Recording)) {
         Skins.Message(mtError, tr("Recording not found!"));
         cArchive::UnmountArchive();
         return false;
      }
   }
   return true;
}

eOSState cMyMenuRecordings::Play(void)
{
   cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
   if (ri) {
      if (ri->IsDirectory())
         Open();
      else
         return Play(ri->Recording(), true);
   }
   return osContinue;
}

eOSState cMyMenuRecordings::Play(const cRecording *Recording, bool IsPluginReplay)
{
   if (Prepare(Recording)) {
         cMyReplayControl::SetRecording(Recording->FileName());
         cControl::Launch(new cMyReplayControl(IsPluginReplay));
         return osEnd;
   }
   return osContinue;
}

eOSState cMyMenuRecordings::Rewind(void)
{
   if (HasSubMenu() || Count() == 0)
      return osContinue;
   cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
   if (ri && !ri->IsDirectory()) {
      cDevice::PrimaryDevice()->StopReplay(); // must do this first to be able to rewind the currently replayed recording
      cResumeFile ResumeFile(ri->Recording()->FileName(), ri->Recording()->IsPesRecording());
      ResumeFile.Delete();
      return Play();
   }
   return osContinue;
}

eOSState cMyMenuRecordings::Archive(void)
{
   return osContinue;
}

eOSState cMyMenuRecordings::Info(void)
{
   if (HasSubMenu() || Count() == 0)
      return osContinue;
   if (cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current())) {
      if (ri->IsDirectory())
         return AddSubMenu(new cMenuPathEdit(cString(ri->Recording()->Name(), strchrn(ri->Recording()->Name(), FOLDERDELIMCHAR, ri->Level() + 1))));
      else
         return AddSubMenu(new cMenuRecording(ri->Recording(), true));
   }
   return osContinue;
}

eOSState cMyMenuRecordings::Sort(void)
{
   if (HasSubMenu())
      return osContinue;
   IncRecordingsSortMode(DirectoryName());
   Set(true);
   return osContinue;
}

eOSState cMyMenuRecordings::ProcessKey(eKeys Key)
{
   bool HadSubMenu = HasSubMenu();
   eOSState state = cOsdMenu::ProcessKey(Key);
   if (state == osUnknown) {
      switch (Key) {
         case kPlayPause:
         case kPlay:
         case kOk:
         case kRed:
            return Play();
         case kGreen:
            return Rewind();
         case kYellow:
            return Archive();
         case kInfo:
         case kBlue:
            return Info();
         case k0:
            return Sort();
         case kNone:
            if (Recordings.StateChanged(recordingsState))
               Set(true);
            break;
         default: break;
      }
   }
   if (!HasSubMenu()) {
      if (Key != kNone)
         SetHelpKeys();
   }
   return state;
}

// --- cMyMenuRecordingItem ----------------------------------------------------

cMyMenuRecordingItem::cMyMenuRecordingItem(cRecording *Recording, int Level) : recording(Recording), level(Level)
{
   name = NULL;
   totalEntries = newEntries = 0;
   // replace newindicator with '#' for archives
   if (strcmp(cArchive::GetArchiveId(Recording), "") && (level < 0 || level == recording->HierarchyLevels())) {
      char *text = strdup(recording->Title('\t', true, level));
      int nipos = strlen(strrchr(recording->Title('\t', true, level), '\t') + 1);
      text[strlen(recording->Title('\t', true, level)) - nipos - 2] = '#';
      SetText(text); 
      free(text);
   }
   else
      SetText(recording->Title('\t', true, level));
   if (*Text() == '\t')
      name = strdup(Text() + 2); // 'Text() + 2' to skip the two '\t'
}

cMyMenuRecordingItem::~cMyMenuRecordingItem()
{
   free(name);
}

void cMyMenuRecordingItem::IncrementCounter(bool New)
{
   totalEntries++;
   if (New)
      newEntries++;
   SetText(cString::sprintf("%d\t\t%d\t%s", totalEntries, newEntries, name));
}

void cMyMenuRecordingItem::SetMenuItem(cSkinDisplayMenu *DisplayMenu, int Index, bool Current, bool Selectable)
{
   if (!DisplayMenu->SetItemRecording(recording, Index, Current, Selectable, level, totalEntries, newEntries))
      DisplayMenu->SetItem(Text(), Index, Current, Selectable);
}

// --- cMyReplayControl --------------------------------------------------------

cMyReplayControl::cMyReplayControl(bool IsPluginReplay)
: isPluginReplay(IsPluginReplay)
{
}

cMyReplayControl::~cMyReplayControl()
{
   cReplayControl::Stop();
   cArchive::UnlinkArchive(GetRecording());
   cArchive::UnmountArchive();
   if (isPluginReplay)
      cRemote::CallPlugin("hddarchive");
}
