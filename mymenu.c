/*
 * mymenu.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include "mymenu.h"

// --- cMyMenuRecordings -------------------------------------------------------

#if APIVERSNUM > 20101
cString cMyMenuRecordings::path;
cString cMyMenuRecordings::fileName;
#endif

cMyMenuRecordings::cMyMenuRecordings(const char *Base, int Level, bool OpenSubMenus, bool Show)
:cOsdMenu(Base ? Base : tr("HDD-Archive"), 9, 6, 6), show(Show)
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
#if APIVERSNUM > 20101
   else if (OpenSubMenus && (cReplayControl::LastReplayed() || *path || *fileName)) {
      if (!*path || Level < strcountchr(path, FOLDERDELIMCHAR)) {
         if (Open(true))
            return;
      }
   }
#else
    else if (OpenSubMenus && cReplayControl::LastReplayed() && Open(true))
       return;
#endif
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
      else
         NewHelpKeys = 2;
   }
   if (NewHelpKeys != helpKeys) {
      switch (NewHelpKeys) {
         case 0: SetHelp(NULL); break;
         case 1: SetHelp(tr("Button$Open"), NULL, NULL, tr("Button$Edit")); break;
         case 2: SetHelp(RecordingCommands.Count() ? tr("Commands") : tr("Button$Play"), tr("Button$Archive"), tr("Button$Delete"), tr("Button$Info"));
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
      if ((!base || (strstr(recording->Name(), base) == recording->Name() && recording->Name()[strlen(base)] == FOLDERDELIMCHAR))) {
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

eOSState cMyMenuRecordings::Archive(void)
{
   return osContinue;
}

eOSState cMyMenuRecordings::Delete(void)
{
   if (HasSubMenu() || Count() == 0)
      return osContinue;
   cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
   const char *archiveId = cArchive::GetArchiveId(ri->Recording());
   if (strcmp(archiveId, "")) {
      Skins.Message(mtError, tr("Cannot delete archived recording!"));
      return osContinue;
   }
   if (ri && !ri->IsDirectory()) {
      if (Interface->Confirm(tr("Delete recording?"))) {
         cRecordControl *rc = cRecordControls::GetRecordControl(ri->Recording()->FileName());
         if (rc) {
            if (Interface->Confirm(tr("Timer still recording - really delete?"))) {
               cTimer *timer = rc->Timer();
               if (timer) {
                  timer->Skip();
                  cRecordControls::Process(time(NULL));
                  if (timer->IsSingleEvent()) {
                     isyslog("deleting timer %s", *timer->ToDescr());
                     Timers.Del(timer);
                  }
                  Timers.SetModified();
               }
            }
            else
               return osContinue;
         }
         cRecording *recording = ri->Recording();
         cString FileName = recording->FileName();
#if APIVERSNUM > 20101
         if (RecordingsHandler.GetUsage(FileName)) {
#else
	    if (cCutter::Active(ri->Recording()->FileName())) {
#endif
            if (Interface->Confirm(tr("Recording is being edited - really delete?"))) {
#if APIVERSNUM > 20101
               RecordingsHandler.Del(FileName);
#else
	       cCutter::Stop();
#endif
               recording = Recordings.GetByName(FileName); // RecordingsHandler.Del() might have deleted it if it was the edited version
               // we continue with the code below even if recording is NULL,
               // in order to have the menu updated etc.
            }
            else
               return osContinue;
         }
         if (cReplayControl::NowReplaying() && strcmp(cReplayControl::NowReplaying(), FileName) == 0)
            cControl::Shutdown();
         if (!recording || recording->Delete()) {
            cReplayControl::ClearLastReplayed(FileName);
            Recordings.DelByName(FileName);
            cOsdMenu::Del(Current());
            SetHelpKeys();
            cVideoDiskUsage::ForceCheck();
            Display();
            if (!Count())
               return osBack;
         }
         else
            Skins.Message(mtError, tr("Error while deleting recording!"));
      }
   }
   return osContinue;
}

eOSState cMyMenuRecordings::Info(void)
{
   if (HasSubMenu() || Count() == 0)
      return osContinue;
#if APIVERSNUM > 20101
   if (cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current())) {
      if (ri->IsDirectory())
         return AddSubMenu(new cMenuPathEdit(cString(ri->Recording()->Name(), strchrn(ri->Recording()->Name(), FOLDERDELIMCHAR, ri->Level() + 1))));
      else
         return AddSubMenu(new cMenuRecording(ri->Recording(), true));
   }
#else
   cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
   if (ri && !ri->IsDirectory() && ri->Recording()->Info()->Title())
      return AddSubMenu(new cMenuRecording(ri->Recording(), true));
#endif
   return osContinue;
}

eOSState cMyMenuRecordings::Commands(eKeys Key)
{
   if (HasSubMenu() || Count() == 0)
      return osContinue;
   cMyMenuRecordingItem *ri = (cMyMenuRecordingItem *)Get(Current());
   if (ri && !ri->IsDirectory()) {
      cMenuCommands *menu;
      eOSState state = AddSubMenu(menu = new cMenuCommands(tr("Recording commands"), &RecordingCommands, cString::sprintf("\"%s\"", *strescape(ri->Recording()->FileName(), "\\\"$"))));
      if (Key != kNone)
         state = menu->ProcessKey(Key);
      return state;
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
            return Play();
         case kRed:
            return (helpKeys > 1 && RecordingCommands.Count()) ? Commands() : Play();
         case kGreen:
            return Archive();
         case kYellow:
            return Delete();
         case kInfo:
         case kBlue:
            return Info();
         case k0:
            return Sort();
         case k1...k9:
            return Commands(Key);
         case kNone:
            if (Recordings.StateChanged(recordingsState))
               Set(true);
            break;
         default: break;
      }
   }
#if APIVERSNUM > 20101
   else if (state == osUser1) {
      // a recording or path was renamed, so let's refresh the menu
      CloseSubMenu(false);
      if (base)
         return state; // closes all recording menus except for the top one
      Set(); // this is the top level menu, so we refresh it...
      Open(true); // ...and open any necessary submenus to show the new name
      Display();
      path = NULL;
      fileName = NULL;
      }
#endif
   if (Key == kYellow && HadSubMenu && !HasSubMenu()) {
      // the last recording in a subdirectory was deleted, so let's go back up
      cOsdMenu::Del(Current());
      if (!Count())
         return osBack;
      Display();
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
