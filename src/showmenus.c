/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2022  Robert Ian Hawdon

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <regex.h>
#include <time.h>
#include <utime.h>
#include "menu.h"
#include "display.h"
#include "settings.h"
#include "common.h"
#include "showfunctions.h"
#include "show.h"
#include "colors.h"
#include "input.h"
#include "banned.h"

int c;
int * pc = &c;

char chpwd[4096];
char selfile[4096];

char ownerinput[256];
char groupinput[256];
char uids[24];
char gids[24];
char currentfilename[512];

int s;
char *buf;
char *rewrite;

int blockstart = -1;
int blockend = -1;

int abortinput = 0;

struct utimbuf touchDate;
time_t touchTime;

extern char *errmessage;

extern results* ob;
extern history* hs;
extern char currentpwd[4096];

extern int historyref;
extern int sessionhistory;
extern int selected;
extern int topfileref;
extern int lineStart;
extern int bottomFileRef;
extern int visibleObjects;
extern int hpos;
extern int maxdisplaywidth;
extern int totalfilecount;
extern int displaysize;
extern int displaycount;
extern int showhidden;
extern int markall;
extern int viewMode;
extern int markedinfo;
extern int automark;
extern int skipToFirstFile;

extern int skippable;

extern int plugins;

extern int enterAsShow;

extern int reverse;

extern int invalidstart;

extern struct sigaction sa;

extern char sortmode[5];

extern int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL;

extern char *objectWild;

extern int resized;

extern int dirAbort;

extern int exitCode;

extern xattrList *xa;
extern int xattrPos;

extern int visibleOffset;

extern entryLines *el;

extern int listLen;

menuDef *globalMenu;
int globalMenuSize = 0;
wchar_t *globalMenuLabel;

menuDef *fileMenu;
int fileMenuSize = 0;
wchar_t *fileMenuLabel;

menuDef *functionMenu;
int functionMenuSize = 0;
wchar_t *functionMenuLabel;

menuDef *modifyMenu;
int modifyMenuSize = 0;
wchar_t *modifyMenuLabel;

menuDef *sortMenu;
int sortMenuSize = 0;
wchar_t *sortMenuLabel;

menuDef *linkMenu;
int linkMenuSize = 0;
wchar_t *linkMenuLabel;

menuDef *linkLocationMenu;
int linkLocationMenuSize = 0;
wchar_t *linkLocationMenuLabel;

menuDef *touchMenu;
int touchMenuSize = 0;
wchar_t *touchMenuLabel;

menuDef *touchDateConfirmMenu;
int touchDateConfirmMenuSize = 0;
wchar_t *touchDateConfirmMenuLabel;

extern menuDef *colorMenu;
extern int colorMenuSize;
extern wchar_t *colorMenuLabel;

extern menuDef *settingsMenu;
extern int settingsMenuSize;
extern wchar_t *settingsMenuLabel;

void modify_owner_input();

void generateDefaultMenus(){
  // Global Menu
  addMenuItem(&globalMenu, &globalMenuSize, "g_colors", L"c!Olors", 'o');
  addMenuItem(&globalMenu, &globalMenuSize, "g_config", L"!Config", 'c');
  addMenuItem(&globalMenu, &globalMenuSize, "g_run", L"!Run", 'r');
  addMenuItem(&globalMenu, &globalMenuSize, "g_edit", L"!Edit file", 'e');
  addMenuItem(&globalMenu, &globalMenuSize, "g_help", L"!Help", 'h');
  addMenuItem(&globalMenu, &globalMenuSize, "g_mkdir", L"!Make dir", 'm');
  addMenuItem(&globalMenu, &globalMenuSize, "g_quit", L"!Quit", 'q');
  addMenuItem(&globalMenu, &globalMenuSize, "g_show", L"!Show dir", 's');
  addMenuItem(&globalMenu, &globalMenuSize, "g_touch", L"!Touch file", 't');

  // File Menu
  addMenuItem(&fileMenu, &fileMenuSize, "f_copy", L"!Copy", 'c');
  addMenuItem(&fileMenu, &fileMenuSize, "f_delete", L"!Delete", 'd');
  addMenuItem(&fileMenu, &fileMenuSize, "f_edit", L"!Edit", 'e');
  addMenuItem(&fileMenu, &fileMenuSize, "f_hidden", L"!Hidden", 'h');
  addMenuItem(&fileMenu, &fileMenuSize, "f_link", L"!Link", 'l');
  addMenuItem(&fileMenu, &fileMenuSize, "f_modify", L"!Modify", 'm');
  if (plugins){
    addMenuItem(&fileMenu, &fileMenuSize, "f_plugin", L"!Plugin", 'p');
  }
  addMenuItem(&fileMenu, &fileMenuSize, "f_quit", L"!Quit", 'q');
  addMenuItem(&fileMenu, &fileMenuSize, "f_rename", L"!Rename", 'r');
  addMenuItem(&fileMenu, &fileMenuSize, "f_show", L"!Show", 's');
  addMenuItem(&fileMenu, &fileMenuSize, "f_touch", L"!Touch", 't');
  addMenuItem(&fileMenu, &fileMenuSize, "f_uhunt", L"h!Unt", 'u');
  addMenuItem(&fileMenu, &fileMenuSize, "f_xexec", L"e!Xec", 'x');

  // Function Menu
  addMenuItem(&functionMenu, &functionMenuSize, "f_01", L"<F1>-Down", 265);
  addMenuItem(&functionMenu, &functionMenuSize, "f_02", L"<F2>-Up", 266);
  addMenuItem(&functionMenu, &functionMenuSize, "f_03", L"<F3>-Top", 267);
  addMenuItem(&functionMenu, &functionMenuSize, "f_04", L"<F4>-Bottom", 268);
  addMenuItem(&functionMenu, &functionMenuSize, "f_05", L"<F5>-Refresh", 269);
  addMenuItem(&functionMenu, &functionMenuSize, "f_06", L"<F6>-Mark/Unmark", 270);
  addMenuItem(&functionMenu, &functionMenuSize, "f_07", L"<F7>-All", 271);
  addMenuItem(&functionMenu, &functionMenuSize, "f_08", L"<F8>-None", 272);
  addMenuItem(&functionMenu, &functionMenuSize, "f_09", L"<F9>-Sort", 273);
  addMenuItem(&functionMenu, &functionMenuSize, "f_10", L"<F10>-Block", 274);

  // Modify Menu
  addMenuItem(&modifyMenu, &modifyMenuSize, "m_owner", L"!Owner/Group", 'o');
  addMenuItem(&modifyMenu, &modifyMenuSize, "m_perms", L"!Permissions", 'p');

  // Sort Menu
  addMenuItem(&sortMenu, &sortMenuSize, "s_date", L"!Date & time", 'd');
  addMenuItem(&sortMenu, &sortMenuSize, "s_name", L"!Name", 'n');
  addMenuItem(&sortMenu, &sortMenuSize, "s_size", L"!Size", 's');

  // Link Menu
  addMenuItem(&linkMenu, &linkMenuSize, "l_hard", L"!Hard", 'h');
  addMenuItem(&linkMenu, &linkMenuSize, "l_symbolic", L"!Symbolic", 's');

  // Link Location Menu
  addMenuItem(&linkLocationMenu, &linkLocationMenuSize, "l_absolute", L"!Absolute", 'a');
  addMenuItem(&linkLocationMenu, &linkLocationMenuSize, "l_relative", L"!Relative", 'r');

  // Touch Menu
  addMenuItem(&touchMenu, &touchMenuSize, "t_accessed", L"!Accessed", 'a');
  addMenuItem(&touchMenu, &touchMenuSize, "t_both", L"!Both", 'b');
  addMenuItem(&touchMenu, &touchMenuSize, "t_modified", L"!Modified", 'm');

  // Touch Set Date Confirm
  addMenuItem(&touchDateConfirmMenu, &touchDateConfirmMenuSize, "t_1", L"!Yes/", 'y');
  addMenuItem(&touchDateConfirmMenu, &touchDateConfirmMenuSize, "t_2", L"!No", 'n');

  // Color Menu
  addMenuItem(&colorMenu, &colorMenuSize, "c_color", L"Color number", -1);
  addMenuItem(&colorMenu, &colorMenuSize, "c_load", L"!Load", 'l');
  addMenuItem(&colorMenu, &colorMenuSize, "c_quit", L"!Quit", 'q');
  addMenuItem(&colorMenu, &colorMenuSize, "c_save", L"!Save", 's');
  addMenuItem(&colorMenu, &colorMenuSize, "c_toggle", L"!Toggle", 't');
  addMenuItem(&colorMenu, &colorMenuSize, "c_use", L"!Use", 'u');

  // Setings Menu
  addMenuItem(&settingsMenu, &settingsMenuSize, "s_quit", L"!Quit", 'q');
  addMenuItem(&settingsMenu, &settingsMenuSize, "s_revert", L"!Revert", 'r');
  addMenuItem(&settingsMenu, &settingsMenuSize, "s_save", L"!Save", 's');
}

void refreshMenuLabels(){
  globalMenuLabel = genMenuDisplayLabel(L"", globalMenu, globalMenuSize, L"", 1);
  fileMenuLabel = genMenuDisplayLabel(L"", fileMenu, fileMenuSize, L"", 1);
  functionMenuLabel = genMenuDisplayLabel(L"", functionMenu, functionMenuSize, L"", 0);
  modifyMenuLabel = genMenuDisplayLabel(L"Modify -", modifyMenu, modifyMenuSize, L"", 1);
  sortMenuLabel = genMenuDisplayLabel(L"Sort list by -", sortMenu, sortMenuSize, L"", 1);
  linkMenuLabel = genMenuDisplayLabel(L"Link Type -", linkMenu, linkMenuSize, L"(enter = S)", 1);
  linkLocationMenuLabel = genMenuDisplayLabel(L"Link Location -", linkLocationMenu, linkLocationMenuSize, L"(enter = R)", 1);
  touchMenuLabel = genMenuDisplayLabel(L"Set Time -", touchMenu, touchMenuSize, L"(enter = B)", 1);
  touchDateConfirmMenuLabel = genMenuDisplayLabel(L"Set Time?", touchDateConfirmMenu, touchDateConfirmMenuSize, L"(enter = N)", -1);
  colorMenuLabel = genMenuDisplayLabel(L"", colorMenu, colorMenuSize, L"", 1);
  settingsMenuLabel = genMenuDisplayLabel(L"SHOW Settings Menu -", settingsMenu, settingsMenuSize, L"", 1);
}

void unloadMenuLabels(){
  free(globalMenuLabel);
  free(fileMenuLabel);
  free(functionMenuLabel);
  free(modifyMenuLabel);
  free(sortMenuLabel);
  free(linkLocationMenuLabel);
  free(touchMenuLabel);
  free(touchDateConfirmMenuLabel);
  free(colorMenuLabel);
  free(settingsMenuLabel);
}

int sanitizeTopFileRef(int topfileref)
{
  if (((topfileref + displaysize) < totalfilecount + 1 ) && ((selected) > topfileref + 1) && (selected < (topfileref + displaysize))) {
    // If we're already good, we don't need to adjust the topfileref
  } else if ( (topfileref > totalfilecount) ){
    // If the top file ref exceeds the number of files, we'll want to do something about that
    topfileref = totalfilecount - (displaysize);
  } else if ( (selected - topfileref) < 0 ) {
    // We certainly don't want the top file ref in the negatives
    topfileref = 0;
  } else if ((topfileref + displaysize) > totalfilecount){
    // If we end up with the top file ref in a position where the list of files ends before the end of the screen, we need to sort that too.
    topfileref = totalfilecount - (displaysize);
  }
  if ( (selected - topfileref ) > displaysize ){
    // We don't want the selected item off the bottom of the screen
    topfileref = selected - displaysize + 1;
  }
  if ( selected == 0 ) {
    // Just in case we're thrust to the top of the list - like when in a hidden directory and hidden files are switched off
    topfileref = 0;
  }
  if (topfileref < 0){
    // Likewise, we don't want the topfileref < 0 here either
    topfileref = 0;
  }
  if ( totalfilecount < displaysize ){
    // Finally, to override all of the above, if we have less files than display, reset top file ref to 0.
    topfileref = 0;
  }

  return topfileref;
}

void refreshDirectory(char *sortmode, int origlineStart, int origselected, int destructive)
{
  char currentselectname[512];
  int i;
 handleMissingDir:
  if (check_dir(currentpwd)){
    if (invalidstart) {
      currentselectname[0]=0;
      exitCode = 0;
      invalidstart = 0;
    } else {
      if (destructive == 2){
        memcpy(currentselectname, currentfilename, 512);
      } else {
        snprintf(currentselectname, 512, "%s", ob[origselected].name);
      }
    }
    if (destructive != -1){
      freeResults(ob, totalfilecount);
      freeXAttrs(xa, xattrPos);
      ob = get_dir(currentpwd);
      clear_workspace();
      reorder_ob(ob, sortmode);
      generateEntryLineIndex(ob);
    }
    if (destructive > 0){
      i = findResultByName(ob, currentselectname);
      if (i != -1){
        selected = i;
      } else {
        if (selected > totalfilecount - 1){
          selected = totalfilecount - 1;
        } else {
          selected = origselected;
        }
      }
    } else {
      selected = findResultByName(ob, currentselectname);

    }
    if (destructive == -2){
      if (skipToFirstFile == 1 && skippable == 1){
        selected = 2;
      } else {
        selected = 0;
      }
    }
  } else {
    if (historyref > 1){
      memcpy(currentpwd, hs[historyref - 2].path, strlen(hs[historyref - 2].path));
      objectWild = hs[historyref - 2].objectWild;
      historyref--;
      chdir(currentpwd);
      goto handleMissingDir;
    } else {
      global_menu();
    }
  }
  if (dirAbort == 1){
    lineStart = hs[historyref].lineStart;
    selected = hs[historyref].selected;
    dirAbort = 0;
  }
  if (selected == -1){
    if (skipToFirstFile == 1 && skippable == 1){
      selected = 2;
    } else {
      selected = 0;
    }
  }
  display_dir(currentpwd, ob);
}

void directory_view_menu_inputs(); // Needed to allow menu inputs to switch between each other
void global_menu_inputs();

void show_directory_input()
{
  char *oldpwd = malloc(sizeof(char) * (strlen(currentpwd) + 1));
  char *direrror = malloc(sizeof(char) + 1);
  int curPos;

  memcpy(oldpwd, currentpwd, (strlen(currentpwd) + 1));
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Show Directory - Enter pathname:") + 1);
  curs_set(TRUE);
  move(0,curPos);
  readline(currentpwd, 4096, oldpwd);
  curs_set(FALSE);
  testSlash:
  if (check_last_char(currentpwd, "/") && strcmp(currentpwd, "/")){
    currentpwd[strlen(currentpwd) - 1] = '\0';
    goto testSlash;
  }
  if ((strcmp(currentpwd, oldpwd) && strcmp(currentpwd, "")) || !historyref){
    objectWild = objectFromPath(currentpwd);
    if ( strchr(objectWild, MULTICHAR) || strchr(objectWild, ONECHAR)){
      snprintf(currentpwd, 4096, "%s", dirFromPath(currentpwd));
    } else {
      objectWild[0]=0;
    }

    if (check_first_char(currentpwd, "~")){
      rewrite = str_replace(currentpwd, "~", getenv("HOME"));
      memcpy(currentpwd, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
    if (check_object(currentpwd) == 1){
      if ( invalidstart ){
        set_history(currentpwd, "", "", 0, 0);
      } else {
        set_history(currentpwd, objectWild, ob[selected].name, lineStart, selected);
      }
      lineStart = 0;
      selected = 0;
      chdir(currentpwd);
      refreshDirectory(sortmode, 0, selected, -2);
    } else {
      setDynamicChar(&direrror, "The location %s cannot be opened or is not a directory\n", currentpwd);
      memcpy(currentpwd, oldpwd, (strlen(oldpwd) + 1));
      topLineMessage(direrror);
    }
  } else {
    memcpy(currentpwd, oldpwd, (strlen(oldpwd) + 1)); // Copying old value back if the input was aborted
  }
  free(direrror);
  free(oldpwd);
  if (historyref > 0){
    directory_view_menu_inputs();
  } else {
    global_menu_inputs();
  }
}


int replace_file_confirm_input(char *filename)
{
  char *message = malloc(sizeof(char) * 1);
  setDynamicChar(&message, "Replace file [<%s>]? (!Yes/!No)", filename);
  printMenu(0,0, message);
  free(message);
  while(1)
    {
      *pc = getch10th();
      switch(*pc)
        {
        case 'y':
          return 1;
          break;
        default:
          return 0;
          break;
        }
    }
}

void copy_file_input(char *file, mode_t mode)
{
  // YUCK, repetition, this needs sorting
  char newfile[4096];
  int e;
  int curPos = 0;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Copy file to:") + 1);
  curs_set(TRUE);
  move(0,curPos);
  readline(newfile, 4096, file);
  curs_set(FALSE);
  // If the two values don't match, we want to do the copy
  if ( strcmp(newfile, file) && strcmp(newfile, "")) {
    if (check_first_char(newfile, "~")){
      rewrite = str_replace(newfile, "~", getenv("HOME"));
      memcpy(newfile, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
  copyFile:
    if (access(dirFromPath(newfile), W_OK) == 0){
      if ( check_file(newfile) )
        {
          if ( replace_file_confirm_input(newfile) )
            {
              copy_file(file, newfile, mode);
              refreshDirectory(sortmode, 0, selected, 0);
            }
        } else {
        copy_file(file, newfile, mode);
        refreshDirectory(sortmode, 0, selected, 0);
      }
    } else {
      if (errno == ENOENT){
        e = createParentsInput(dirFromPath(newfile));
        if (e == 1){
          createParentDirs(newfile);
          goto copyFile;
        } else {
          setDynamicChar(&errmessage, "Error: %s", strerror(errno));
          topLineMessage(errmessage);
          free(errmessage);
        }
      } else {
        setDynamicChar(&errmessage, "Error: %s", strerror(errno));
        topLineMessage(errmessage);
        free(errmessage);
      }
    }
  }
  directory_view_menu_inputs();
}

void copy_multi_file_input(results* ob, char *input)
{
  int i, e;
  int curPos = 0;
  char dest[4096];
  char destfile[4096];
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Copy multiple files to:") + 1);
  curs_set(TRUE);
  move(0, curPos);
  readline(dest, 4096, input);
  curs_set(FALSE);
  if ( strcmp(dest, input) && strcmp(dest, "")) {
    if (check_first_char(dest, "~")){
      rewrite = str_replace(dest, "~", getenv("HOME"));
      memcpy(dest, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
  copyMultiFile:
    if ( check_dir(dest) ){
      for (i = 0; i < totalfilecount; i++)
        {
          if ( *ob[i].marked )
            {
              memcpy(selfile, currentpwd, 4096);
              if (!check_last_char(selfile, "/")){
                snprintf(selfile + strlen(selfile), 4096, "%s", "/");
              }
              snprintf(selfile + strlen(selfile), 4096, "%s", ob[i].name);
              memcpy(destfile, dest, 4096);
              if (!check_last_char(destfile, "/")){
                snprintf(destfile + strlen(destfile), 4096, "%s", "/");
              }
              snprintf(destfile + strlen(destfile), 4096, "%s", ob[i].name);
              if ( check_file(destfile) )
                {
                  if ( replace_file_confirm_input(destfile) )
                    {
                      copy_file(selfile, destfile, ob[i].mode);
                    }
                } else {
                copy_file(selfile, destfile, ob[i].mode);
              }
            }
        }
      refreshDirectory(sortmode, 0, selected, 0);
    } else {
      e = createParentsInput(dest);
      if (e == 1){
        createParentDirs(dest);
        mk_dir(dest); // Needed as the final element is omitted by the above
        goto copyMultiFile;
      } else {
        topLineMessage("Error: Directory Not Found.");
      }
    }
  }
  directory_view_menu_inputs();
}

void rename_multi_file_input(results* ob, char *input)
{
  int i, e;
  int curPos = 0;
  char dest[4096];
  char destfile[4096];
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Rename multiple files to:") + 1);
  curs_set(TRUE);
  move(0, curPos);
  readline(dest, 4096, input);
  curs_set(FALSE);
  if (strcmp(dest, input) && strcmp(dest, "")){
    if (check_first_char(dest, "~")){
      rewrite = str_replace(dest, "~", getenv("HOME"));
      memcpy(dest, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
  renameMultiFile:
    if ( check_dir(dest) ){
      for (i = 0; i < totalfilecount; i++)
        {
          if ( *ob[i].marked )
            {
              memcpy(selfile, currentpwd, 4096);
              if (!check_last_char(selfile, "/")){
                snprintf(selfile + strlen(selfile), 4096, "%s", "/");
              }
              snprintf(selfile + strlen(selfile), 4096, "%s", ob[i].name);
              memcpy(destfile, dest, 4096);
              if (!check_last_char(destfile, "/")){
                snprintf(destfile + strlen(destfile), 4096, "%s", "/");
              }
              snprintf(destfile + strlen(destfile), 4096, "%s", ob[i].name);
              if ( check_file(destfile) )
                {
                  if ( replace_file_confirm_input(destfile) )
                    {
                      RenameObject(selfile, destfile);
                    }
                } else {
                RenameObject(selfile, destfile);
              }
            }
        }
    } else {
      e = createParentsInput(dest);
      if (e == 1){
        createParentDirs(dest);
        mk_dir(dest); // Needed as the final element is omitted by the above
        goto renameMultiFile;
      } else {
        topLineMessage("Error: Directory Not Found.");
      }
    }
    refreshDirectory(sortmode, 0, selected, 1);
  }

  directory_view_menu_inputs();
}

void edit_file_input()
{
  char filepath[4096];
  int curPos = 0;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Edit File - Enter pathname:") + 1);
  curs_set(TRUE);
  move(0,curPos);
  readline(filepath, 4096, "");
  curs_set(FALSE);
  SendToEditor(filepath);
  refreshDirectory(sortmode, lineStart, selected, 1);
}

void rename_file_input(char *file)
{
  // YUCK, repetition, this needs sorting
  char dest[4096];
  int curPos = 0;
  int e;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Rename file to:") + 1);
  curs_set(TRUE);
  move(0,curPos);
  readline(dest, 4096, file);
  curs_set(FALSE);
  if (strcmp(dest, file) && strcmp(dest, "")){
    if (check_first_char(dest, "~")){
      rewrite = str_replace(dest, "~", getenv("HOME"));
      memcpy(dest, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
  renameFile:
    if (access(dirFromPath(dest), W_OK) == 0){
      if ( check_file(dest) )
        {
          if ( replace_file_confirm_input(dest) )
            {
              RenameObject(file, dest);
              snprintf(currentfilename, 512, "%s", objectFromPath(dest));
              refreshDirectory(sortmode, 0, selected, 2);
            }
        } else {
        RenameObject(file, dest);
        snprintf(currentfilename, 512, "%s", objectFromPath(dest));
        refreshDirectory(sortmode, 0, selected, 2);
      }
    } else {
      if (errno == ENOENT){
        e = createParentsInput(dirFromPath(dest));
        if (e == 1){
          createParentDirs(dest);
          goto renameFile;
        } else {
          setDynamicChar(&errmessage, "Error: %s", strerror(errno));
          topLineMessage(errmessage);
          free(errmessage);
        }
      } else {
        setDynamicChar(&errmessage, "Error: %s", strerror(errno));
        topLineMessage(errmessage);
        free(errmessage);
      }
    }
    refreshDirectory(sortmode, 0, selected, 0);
  }
  directory_view_menu_inputs();
}

void make_directory_input()
{
  char *tmpPwd;
  char newdir[4096];
  int curPos = 0;
  int e;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Make Directory - Enter pathname:") + 1);
  move (0,curPos);
  setDynamicChar(&tmpPwd, "%s/", currentpwd);
  readline(newdir, 4096, tmpPwd);
  free(tmpPwd);
  if (strcmp(newdir, currentpwd) && strcmp(newdir, "")){
    if (check_first_char(newdir, "~")){
      rewrite = str_replace(newdir, "~", getenv("HOME"));
      memcpy(newdir, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }
  makeDir:
    if (access(dirFromPath(newdir), W_OK) == 0){
      mk_dir(newdir);
    } else {
      if (errno == ENOENT){
        e = createParentsInput(dirFromPath(newdir));
        if (e == 1){
          createParentDirs(newdir);
          goto makeDir;
        } else {
          setDynamicChar(&errmessage, "Error: %s", strerror(errno));
          topLineMessage(errmessage);
          free(errmessage);
        }
      } else {
        setDynamicChar(&errmessage, "Error: %s", strerror(errno));
        topLineMessage(errmessage);
        free(errmessage);
      }
    }
  testSlash:
    if (check_last_char(currentpwd, "/") && strcmp(currentpwd, "/")){
      currentpwd[strlen(currentpwd) - 1] = '\0';
      goto testSlash;
    }
    refreshDirectory(sortmode, 0, selected, 0);
  }
  directory_view_menu_inputs();
}

time_t touchTimeInput(int type)
{
  char menuTitle[32];
  char charTime[64];
  struct tm tmp, localTmp;
  time_t newTime, tmpTime;
  int curPos = 0;
  if (type == 1){
    snprintf(menuTitle, 32, "Set Access Time:");
  } else if (type == 2){
    snprintf(menuTitle, 32, "Set Modified Time:");
  } else {
    snprintf(menuTitle, 32, "Set Time:");
  }
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0,0,menuTitle) + 1);
  move(0, curPos);
  if (readline(charTime, 64, "") != -1){
    time(&tmpTime);
    gmtime_r(&tmpTime, &localTmp);
    if (strptime(charTime, "%Y-%m-%d %H:%M:%S", &tmp) != NULL){
      tmp.tm_isdst = -1;
      newTime = mktime(&tmp);
    } else if (strptime(charTime, "%H:%M:%S", &tmp) != NULL){
      tmp.tm_year = localTmp.tm_year;
      tmp.tm_mon = localTmp.tm_mon;
      tmp.tm_mday = localTmp.tm_mday;
      tmp.tm_wday = localTmp.tm_wday;
      tmp.tm_yday = localTmp.tm_yday;
      tmp.tm_isdst = -1;
      newTime = mktime(&tmp);
    } else if (!strcmp(charTime, "")){
      time(&newTime);
    } else {
      abortinput = 1;
      topLineMessage("Error parsing time");
      time(&newTime);
    }
  } else {
    abortinput = 1;
    time(&newTime);
  }
  return(newTime);
}

int touchType()
{
  int result = 0;
  wPrintMenu(0,0,touchMenuLabel);
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(touchMenu, "t_accessed", touchMenuSize)){
        result = 1;
        break;
      } else if (*pc == menuHotkeyLookup(touchMenu, "t_both", touchMenuSize) || *pc == 10){
        result = 0;
        break;
      } else if (*pc == menuHotkeyLookup(touchMenu, "t_modified", touchMenuSize)){
        result = 2;
        break;
      } else if (*pc == 27){
        // ESC Key
        directory_view_menu_inputs();
      }
    }
  return(result);
}

void touch_file_input()
{
  char *tmpPwd;
  char touchFile[4096];
  FILE* touchFileObject;
  int setDateFlag = -1;
  int e;
  int curPos = 0;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Touch File - Enter pathname:") + 1);
  move (0, curPos);
  setDynamicChar(&tmpPwd, "%s/", currentpwd);
  if (readline(touchFile, 4096, tmpPwd) != -1){
    wPrintMenu(0,0,touchDateConfirmMenuLabel);
    *pc = getch10th();
    if (*pc == menuHotkeyLookup(touchDateConfirmMenu, "t_1", touchDateConfirmMenuSize)){
      setDateFlag = touchType();
      touchTime = touchTimeInput(setDateFlag);
    }
    if (strcmp(touchFile, currentpwd) && strcmp(touchFile, "")){
      if (check_first_char(touchFile, "~")){
        rewrite = str_replace(touchFile, "~", getenv("HOME"));
        memcpy(touchFile, rewrite, (strlen(rewrite) + 1));
        free(rewrite);
      }
    }
  touchFile:
    if (access(dirFromPath(touchFile), W_OK) == 0) {
      if (check_object(touchFile) == 0){
        touchFileObject = fopen(touchFile, "w");
        fclose(touchFileObject);
        if (setDateFlag != -1){
          if (setDateFlag == 0){
            touchDate.actime = touchDate.modtime = touchTime;
          } else if ( setDateFlag == 1 ){
            touchDate.actime = touchTime;
            time(&touchDate.modtime);
          } else if ( setDateFlag == 2 ){
            time(&touchDate.actime);
            touchDate.modtime = touchTime;
          }
          utime(touchFile, &touchDate);
        }
      }
    } else {
      if (errno == ENOENT){
        e = createParentsInput(dirFromPath(touchFile));
        if (e == 1){
          createParentDirs(touchFile);
          goto touchFile;
        } else {
          setDynamicChar(&errmessage, "Error: %s", strerror(errno));
          topLineMessage(errmessage);
          free(errmessage);
        }
      } else {
        setDynamicChar(&errmessage, "Error: %s", strerror(errno));
        topLineMessage(errmessage);
        free(errmessage);
      }
    }
  testSlash:
    if (check_last_char(currentpwd, "/") && strcmp(currentpwd, "/")){
      currentpwd[strlen(currentpwd) - 1] = '\0';
      goto testSlash;
    }
    refreshDirectory(sortmode, 0, selected, 0);
  }
  free(tmpPwd);
  directory_view_menu_inputs();
}

char * execute_argument_input(const char *exec)
{
  char *strout;
  char *message;
  int curPos = 0;
  strout = malloc(sizeof(char) * 1024);
  message = malloc(sizeof(char) * 1);
  setDynamicChar(&message, "Args to pass to %s:", exec);
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, message) + 1);
  free(message);
  curs_set(TRUE);
  move(0, curPos);
  if (readline(strout, 1024, "") == -1){
    abortinput = 1;
  }
  curs_set(FALSE);
  return strout;
}

int huntCaseSelectInput()
{
  int result = 0;
  char *message;
  setDynamicChar(&message, "Case Sensitive, !Yes/!No/<ESC> (enter = no)");
  printMenu(0,0, message);
  while(1)
    {
    huntCaseLoop:
      *pc = getch10th();
      switch(*pc)
        {
        case 'y':
          result = 1;
          break;
	case 10:
        case 'n':
          result = 0;
          break;
        case 27:
          result = -1;
          break;
        default:
          goto huntCaseLoop;
        }
      break;
    }
  free(message);
  return(result);
}

void huntInput(int selected, int charcase)
{
  int regexcase;
  int i;
  int curPos = 0;
  char regexinput[4096];
  char *inputmessage;
  if (charcase){
    regexcase = 0;
    setDynamicChar(&inputmessage, "Match Case - Enter string:");
  } else {
    regexcase = REG_ICASE;
    setDynamicChar(&inputmessage, "Ignore Case - Enter string:");
  }
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, inputmessage) + 1);
  move(0, curPos);
  free(inputmessage);
  if (readline(regexinput, 4096, "") == -1) {
    abortinput = 1;
  } else {
    if (CheckMarked(ob) < 1){
      memcpy(chpwd, currentpwd, 4096);
      if (!check_last_char(chpwd, "/")){
        snprintf(chpwd + strlen(chpwd), 4096, "%s", "/");
      }
      snprintf(chpwd + strlen(chpwd), 4096, "%s", ob[selected].name);
      if (huntFile(chpwd, regexinput, regexcase)){
        *ob[selected].marked = 1;
      }
    } else {
      for (i = 0; i < totalfilecount; i++){
        if ( *ob[i].marked ){
          memcpy(chpwd, currentpwd, 4096);
          if (!check_last_char(chpwd, "/")){
            snprintf(chpwd + strlen(chpwd), 4096, "%s", "/");
          }
          snprintf(chpwd + strlen(chpwd), 4096, "%s", ob[i].name);
          if (huntFile(chpwd, regexinput, regexcase)){
            *ob[i].marked = 1;
          } else {
            *ob[i].marked = 0;
          }
          clear_workspace();
        }
      }
    }
  }
}

void delete_file_confirm_input(char *file)
{
  printMenu(0,0, "Delete file? (!Yes/!No)");
  while(1)
    {
      *pc = getch10th();
      switch(*pc)
        {
        case 'y':
          delete_file(file);
          refreshDirectory(sortmode, lineStart, selected, 1);
          // Not breaking here, intentionally dropping through to the default
        default:
          directory_view_menu_inputs();
          break;
        }
    }
}

void delete_directory_confirm_input(char *directory)
{
  int e;
  printMenu(0,0, "Delete directory? (!Yes/!No)");
  while(1)
    {
      *pc = getch10th();
      switch(*pc)
        {
        case 'y':
          e = rmdir(directory);
          if (e != 0){
            setDynamicChar(&errmessage, "Error: %s", strerror(errno));
            topLineMessage(errmessage);
            free(errmessage);
          }
          refreshDirectory(sortmode, lineStart, selected, 1);
          // Not breaking here, intentionally dropping through to the default
        default:
          directory_view_menu_inputs();
          break;
        }
    }
}

void delete_multi_file_confirm_input(results* ob)
{
  int i, k;
  int allflag = 0;
  int abortflag = 0;
  char *message;

  for (i = 0; i < totalfilecount; i++)
    {
      if ( *ob[i].marked && !abortflag )
        {
          memcpy(selfile, currentpwd, 4096);
          if (!check_last_char(selfile, "/")){
            snprintf(selfile + strlen(selfile), 4096, "%s", "/");
          }
          snprintf(selfile + strlen(selfile), 4096, "%s", ob[i].name);
          if ( allflag )
            {
              delete_file(selfile);
            } else {
            setDynamicChar(&message, "Delete file [<%s>]? (!Yes/!No/!All/!Stop)", selfile);
            printMenu(0,0, message);
            free(message);
            k = 1;
            while(k)
              {
                *pc = getch10th();
                switch(*pc)
                  {
                  case 'y':
                    delete_file(selfile);
                    k = 0;
                    break;
                  case 'a':
                    allflag = 1;
                    k = 0;
                    delete_file(selfile);
                    break;
                  case 's':
                    abortflag = 1;
                    k = 0;
                    break;
                  case 'n':
                    k = 0;
                    break;
                  }
              }
          }
        }
    }
}

void sort_view_inputs()
{
  viewMode = 3;
  while(1)
    {
      wPrintMenu(0,0,sortMenuLabel);
      *pc = getch10th();
      if (*pc == 27){
        // ESC Key
        directory_view_menu_inputs();
      } else if (*pc == menuHotkeyLookup(sortMenu, "s_name", sortMenuSize)){
        snprintf(sortmode, 5, "name");
        reverse = 0;
      } else if (*pc == menuHotkeyLookup(sortMenu, "s_date", sortMenuSize)){
        snprintf(sortmode, 5, "date");
        reverse = 0;
      } else if (*pc == menuHotkeyLookup(sortMenu, "s_size", sortMenuSize)){
        snprintf(sortmode, 5, "size");
        reverse = 0;
      } else if (*pc == altHotkey(menuHotkeyLookup(sortMenu, "s_name", sortMenuSize))){
        snprintf(sortmode, 5, "name");
        reverse = 1;
      } else if (*pc == altHotkey(menuHotkeyLookup(sortMenu, "s_date", sortMenuSize))){
        snprintf(sortmode, 5, "date");
        reverse = 1;
      } else if (*pc == altHotkey(menuHotkeyLookup(sortMenu, "s_size", sortMenuSize))){
        snprintf(sortmode, 5, "size");
        reverse = 1;
      }
      refreshDirectory(sortmode, lineStart, selected, 0);
      directory_view_menu_inputs();
    }
}

void modify_group_input()
{
  char *ofile;

  struct group grp;
  struct group *gresult;
  size_t bufsize;
  int i, status;
  char *message;
  int curPos = 0;

  message = malloc(sizeof(char) * 1);

 groupInputLoop:
  move(0,0);
  clrtoeol();
  setDynamicChar(&message, "Set Group (%s):", ownerinput);
  curPos = (printMenu(0, 0, message) + 1);
  free(message);
  curs_set(TRUE);
  move(0,curPos);
  status = readline(groupinput, 256, "");
  curs_set(FALSE);

  if (status != -1){
    if (!strcmp(groupinput, "")){
      snprintf(groupinput, 256, "%s", ownerinput);
    }
    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)          /* Value was indeterminate */
      {
        bufsize = 16384;        /* Should be more than enough */
      }

    buf = malloc(bufsize);
    if (buf == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    s = getgrnam_r(groupinput, &grp, buf, bufsize, &gresult);
    free(buf);
    if (gresult == NULL){
      if (s == 0){
        setDynamicChar(&errmessage, "Invalid group: %s", groupinput);
        topLineMessage(errmessage);
        free(errmessage);
        goto groupInputLoop;
      }
    } else {
      snprintf(gids, 24, "%d", gresult->gr_gid);

      if ( (CheckMarked(ob) > 0) ){
        for (i = 0; i < totalfilecount; i++)
          {
            if ( *ob[i].marked )
              {
                ofile = malloc(sizeof(char) * (strlen(currentpwd) + strlen(ob[i].name) + 2));
                snprintf(ofile, (strlen(currentpwd) + strlen(ob[i].name) + 2), "%s", currentpwd);
                if (!check_last_char(ofile, "/")){
                  snprintf(ofile + strlen(ofile), (strlen(currentpwd) + strlen(ob[i].name) + 2), "%s", "/");
                }
                snprintf(ofile + strlen(ofile), (strlen(currentpwd) + strlen(ob[i].name) + 2), "%s", ob[i].name);
                UpdateOwnerGroup(ofile, uids, gids);
                free(ofile);
              }
          }
      } else {
        ofile = malloc(sizeof(char) * (strlen(currentpwd) + strlen(ob[selected].name) + 2));
        snprintf(ofile, (strlen(currentpwd) + strlen(ob[selected].name) + 2), "%s", currentpwd);
        if (!check_last_char(ofile, "/")){
          snprintf(ofile + strlen(ofile), (strlen(currentpwd) + strlen(ob[selected].name) + 2), "%s", "/");
        }
        snprintf(ofile + strlen(ofile), (strlen(currentpwd) + strlen(ob[selected].name) + 2), "%s", ob[selected].name);

        if (UpdateOwnerGroup(ofile, uids, gids) == -1) {
          setDynamicChar(&errmessage, "Error: %s", strerror(errno));
          topLineMessage(errmessage);
          free(errmessage);
        }
        free(ofile);
      }
      refreshDirectory(sortmode, lineStart, selected, 0);

      directory_view_menu_inputs();
    }
  } else {
    modify_owner_input();
  }
}

void modify_owner_input()
{
  struct passwd pwd;
  struct passwd *presult;
  size_t bufsize;
  int status;
  int curPos = 0;

 ownerInputLoop:
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Set Owner:") + 1);
  curs_set(TRUE);
  move(0,curPos);
  status = readline(ownerinput, 256, "");
  curs_set(FALSE);

  if (status != -1 ){
    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)          /* Value was indeterminate */
      {
        bufsize = 16384;        /* Should be more than enough */
      }

    buf = malloc(bufsize);
    if (buf == NULL) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }

    s = getpwnam_r(ownerinput, &pwd, buf, bufsize, &presult);
    free(buf);
    if (presult == NULL){
      if (s == 0){
        setDynamicChar(&errmessage, "Invalid user: %s", ownerinput);
        topLineMessage(errmessage);
        free(errmessage);
        goto ownerInputLoop;
      }
    } else {
      snprintf(uids, 24, "%d", presult->pw_uid);
      modify_group_input();
    }
  } else {
    directory_view_menu_inputs();
  }
}

void modify_permissions_input()
{
  int newperm, i, status;
  char perms[5];
  char *ptr;
  char *pfile;
  int curPos;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0, 0, "Modify Permissions:") + 1);
  curs_set(TRUE);
  move(0,curPos);
  status = readline(perms, 5, "");
  curs_set(FALSE);

  if (status != -1 ){
    newperm = strtol(perms, &ptr, 8); // Convert string to Octal and then store it as an int. Yay, numbers.

    if ( (CheckMarked(ob) > 0) ) {
      for (i = 0; i < totalfilecount; i++)
        {
          if ( *ob[i].marked )
            {
              pfile = malloc(sizeof(char) * (strlen(currentpwd) + strlen(ob[i].name) + 2));
              snprintf(pfile, (strlen(currentpwd) + strlen(ob[i].name) + 2), "%s", currentpwd);
              if (!check_last_char(pfile, "/")){
                snprintf(pfile + strlen(pfile), (strlen(currentpwd) + strlen(ob[i].name) + 2), "%s", "/");
              }
              snprintf(pfile + strlen(pfile), (strlen(currentpwd) + strlen(ob[i].name) + 2), "%s", ob[i].name);
              chmod(pfile, newperm);
              free(pfile);
            }
        }
    } else {
      pfile = malloc(sizeof(char) * (strlen(currentpwd) + strlen(ob[selected].name) + 2));
      snprintf(pfile, (strlen(currentpwd) + strlen(ob[selected].name) + 2), "%s", currentpwd);
      if (!check_last_char(pfile, "/")){
        snprintf(pfile + strlen(pfile), (strlen(currentpwd) + strlen(ob[selected].name) + 2), "%s", "/");
      }
      snprintf(pfile + strlen(pfile), (strlen(currentpwd) + strlen(ob[selected].name) + 2), "%s", ob[selected].name);
      chmod(pfile, newperm);
      free(pfile);
    }
    refreshDirectory(sortmode, lineStart, selected, 0);
  }

  directory_view_menu_inputs();

}

int symLinkLocation()
{
  int result = 0;
  wPrintMenu(0,0,linkLocationMenuLabel);
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(linkLocationMenu, "l_absolute", linkLocationMenuSize)){
        result = 0;
        break;
      } else if (*pc == menuHotkeyLookup(linkLocationMenu, "l_relative", linkLocationMenuSize) || *pc == 10){
        result = 1;
        break;
      } else if (*pc == 27){
        // ESC Key
        directory_view_menu_inputs();
      }
    }
  return(result);
}

void linktext_input(char *file, int symbolic)
{
  char inputmessage[32];
  char typeText[9];
  char target[4096];
  int relative, e;
  char *relativeFile;
  int curPos = 0;
  memcpy(target, currentpwd, 4096);
  if (!check_last_char(target, "/")){
    snprintf(target + strlen(target), 4096, "/");
  }
  if (symbolic){
    snprintf(typeText, 9, "Symbolic");
  } else {
    snprintf(typeText, 9, "Hard");
  }
  snprintf(inputmessage, 32, "%s link to:", typeText);
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0,0,inputmessage) + 1);
  move(0, curPos);
  if (readline(target, 4096, target) != -1){

    // Check for ~ that needs replacing with home directory
    if (check_first_char(file, "~")){
      rewrite = str_replace(file, "~", getenv("HOME"));
      memcpy(file, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }

    if (check_first_char(target, "~")){
      rewrite = str_replace(target, "~", getenv("HOME"));
      memcpy(target, rewrite, (strlen(rewrite) + 1));
      free(rewrite);
    }

    makeSymlink:
    if (access(dirFromPath(target), W_OK) == 0){
      if (check_file(target)){
        topLineMessage("Error: File exists.");
      } else {
        if (symbolic){
          relative = symLinkLocation();
          if (relative){
            // Do a thing
            relativeFile = getRelativePath(file, target);
            symlink(relativeFile, target);
            free(relativeFile);
          } else {
            symlink(file, target);
          }
        } else {
          link(file, target);
        }
        refreshDirectory(sortmode, 0, selected, 0);
      }
    } else {
      if (errno == ENOENT){
        e = createParentsInput(dirFromPath(target));
        if (e == 1){
          createParentDirs(target);
          goto makeSymlink;
        } else {
          setDynamicChar(&errmessage, "Error: %s", strerror(errno));
          topLineMessage(errmessage);
          free(errmessage);
        }
      } else {
        setDynamicChar(&errmessage, "Error: %s", strerror(errno));
        topLineMessage(errmessage);
        free(errmessage);
      }
    }
  }
  directory_view_menu_inputs();
}

void link_key_menu_inputs()
{
  viewMode = 5;
  wPrintMenu(0,0,linkMenuLabel);
  memcpy(selfile, currentpwd, 4096);
  if (!check_last_char(selfile, "/")){
    snprintf(selfile + strlen(selfile), 4096, "%s", "/");
  }
  snprintf(selfile + strlen(selfile), 4096, "%s", ob[selected].name);
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(linkMenu, "l_hard", linkMenuSize)){
        if (!check_dir(selfile)){
          linktext_input(selfile, 0);
        } else {
          topLineMessage("Error: Selected object is a directory.");
          directory_view_menu_inputs();
        }
      } else if (*pc == menuHotkeyLookup(linkMenu, "l_symbolic", linkMenuSize) || *pc == 10){
        linktext_input(selfile, 1);
        directory_view_menu_inputs();
      } else if (*pc == 27){
        // ESC Key
        directory_view_menu_inputs();
      }
    }
}

void modify_key_menu_inputs()
{
  viewMode = 2;
  wPrintMenu(0,0,modifyMenuLabel);
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(modifyMenu, "m_owner", modifyMenuSize)){
        modify_owner_input();
      } else if (*pc == menuHotkeyLookup(modifyMenu, "m_perms", modifyMenuSize)){
        modify_permissions_input();
      } else if (*pc == 27){
        // ESC Key
        directory_view_menu_inputs();
      }
    }
}

void directory_view_menu_inputs()
{
  int e, i = 0;
  char *updir;
  char *execArgs;
  viewMode = 0;
  while(1)
    {
      wPrintMenu(0, 0, fileMenuLabel);
      wPrintMenu(LINES-1, 0, functionMenuLabel);
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(fileMenu, "f_copy", fileMenuSize)){
        if ( (CheckMarked(ob) > 0) ) {
          copy_multi_file_input(ob, currentpwd);
        } else {
          memcpy(selfile, currentpwd, 4096);
          if (!check_last_char(selfile, "/")){
            snprintf(selfile + strlen(selfile), 4096, "%s", "/");
          }
          snprintf(selfile + strlen(selfile), 4096, "%s", ob[selected].name);
          if (!check_dir(selfile)){
            copy_file_input(selfile, ob[selected].mode);
          }
        }
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_delete", fileMenuSize)){
        if ( (CheckMarked(ob) > 0) ) {
          delete_multi_file_confirm_input(ob);
          refreshDirectory(sortmode, lineStart, selected, 1);
          directory_view_menu_inputs();
        } else {
          memcpy(selfile, currentpwd, 4096);
          if (!check_last_char(selfile, "/")){
            snprintf(selfile + strlen(selfile), 4096, "%s", "/");
          }
          snprintf(selfile + strlen(selfile), 4096, "%s", ob[selected].name);
          if (!check_dir(selfile) || (strcmp(ob[selected].slink, ""))){
            delete_file_confirm_input(selfile);
          } else if (check_dir(selfile)){
            delete_directory_confirm_input(selfile);
          }
        }
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_edit", fileMenuSize)){
        memcpy(chpwd, currentpwd, 4096);
        if (!check_last_char(chpwd, "/")){
          snprintf(chpwd + strlen(chpwd), 4096, "%s", "/");
        }
        snprintf(chpwd + strlen(chpwd), 4096, "%s", ob[selected].name);
        if (!check_dir(chpwd)){
          SendToEditor(chpwd);
          refreshDirectory(sortmode, lineStart, selected, 1);
        }
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_hidden", fileMenuSize)){
        snprintf(currentfilename, 512, "%s", ob[selected].name);
        if (showhidden == 0) {
          showhidden = 1;
        } else {
          showhidden = 0;
        }
        freeResults(ob, totalfilecount);
        freeXAttrs(xa, xattrPos);
        ob = get_dir(currentpwd);
        clear_workspace();
        reorder_ob(ob, sortmode);
        generateEntryLineIndex(ob);
        selected = findResultByName(ob, currentfilename);
        if (selected == -1){
          if (skipToFirstFile == 1 && skippable == 1){
            selected = 2;
          } else {
            selected = 0;
          }
        }
        refreshDirectory(sortmode, lineStart, selected, 0);
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_link", fileMenuSize)){
        if ( !(CheckMarked(ob) > 0) ) {
          link_key_menu_inputs();
        } else {
          topLineMessage("Error: Links can only be made against single files.");
        }
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_modify", fileMenuSize)){
        //printMenu(0, 0, modifyMenuText);
        modify_key_menu_inputs();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_quit", fileMenuSize)){
      handleMissingDir:
          if (historyref > 1){
            snprintf(chpwd, 4096, "%s", hs[historyref - 2].path);
            objectWild = hs[historyref - 2].objectWild;
            historyref--;
            if (check_dir(chpwd)){
              memcpy(currentpwd, chpwd, 4096);
              chdir(currentpwd);
              freeResults(ob, totalfilecount);
              freeXAttrs(xa, xattrPos);
              ob = get_dir(currentpwd);
              reorder_ob(ob, sortmode);
              generateEntryLineIndex(ob);
              selected = findResultByName(ob, hs[historyref].name);
              if (selected == -1){
                if (skipToFirstFile == 1 && skippable == 1){
                  selected = 2;
                } else {
                  selected = 0;
                }
              }
              lineStart = hs[historyref].lineStart;
              clear_workspace();
              display_dir(currentpwd, ob);
            } else {
              // Skip removed directories
              historyref--;
              goto handleMissingDir;
            }
          } else {
            historyref = 0; // Reset historyref here. A hacky workaround due to the value occasionally dipping to minus numbers.
            global_menu();
          }
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_rename", fileMenuSize)){
        if ( (CheckMarked(ob) > 0) ) {
          rename_multi_file_input(ob, currentpwd);
        } else {
          memcpy(selfile, currentpwd, 4096);
          if (!check_last_char(chpwd, "/")){
            snprintf(selfile + strlen(selfile), 4096, "%s", "/");
          }
          snprintf(selfile + strlen(selfile), 4096, "%s", ob[selected].name);
          rename_file_input(selfile);
        }
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_show", fileMenuSize)){
        showCommand:
          memcpy(chpwd, currentpwd, 4096);
          if (!check_last_char(chpwd, "/")){
            snprintf(chpwd + strlen(chpwd), 4096, "%s", "/");
          }
          snprintf(chpwd + strlen(chpwd), 4096, "%s", ob[selected].name);
          if (!strcmp(ob[selected].name, "..")) {
            if (strcmp(currentpwd, "/")){
              updir = dirFromPath(currentpwd);
              memcpy(chpwd, updir, (strlen(updir) + 1));
              free(updir);
              objectWild = "";
              testSlash:
              if (check_last_char(chpwd, "/") && strcmp(chpwd, "/")){
                chpwd[strlen(chpwd) - 1] = '\0';
                goto testSlash;
              }
              set_history(chpwd, objectWild, ob[selected].name, lineStart, selected);
              lineStart = 0;
              selected = 0;
              memcpy(currentpwd, chpwd, 4096);
              chdir(currentpwd);
              refreshDirectory(sortmode, lineStart, selected, -2);
            }
          } else if (!strcmp(ob[selected].name, ".")) {
            // Do nothing
          } else {
            if (check_dir(chpwd)){
              objectWild = "";
              set_history(chpwd, objectWild, ob[selected].name, lineStart, selected);
              lineStart = 0;
              selected = 0;
              memcpy(currentpwd, chpwd, 4096);
              chdir(currentpwd);
              refreshDirectory(sortmode, lineStart, selected, -2);
            } else {
              e = SendToPager(chpwd);
              refreshDirectory(sortmode, lineStart, selected, 1);
              // display_dir(currentpwd, ob);
            }
          }
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_touch", fileMenuSize)){
        e = touchType();
        // Add what to do with result.
        if (e > -1){
          touchTime = touchTimeInput(e);
          if (abortinput == 0) {
            if (e == 0){
              touchDate.actime = touchDate.modtime = touchTime;
            } else if ( e == 1 ){
              touchDate.actime = touchTime;
              touchDate.modtime = ob[selected].date;
            } else if ( e == 2 ){
              touchDate.actime = ob[selected].adate;
              touchDate.modtime = touchTime;
            }
            if ((CheckMarked(ob) > 0)){
              for (i = 0; i < totalfilecount; i++){
                if (*ob[i].marked){
                  utime(ob[i].name, &touchDate);
                }
              }
            } else {
              utime(ob[selected].name, &touchDate);
            }
          } else {
            abortinput = 0;
          }
        }
        refreshDirectory(sortmode, lineStart, selected, 0);
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_uhunt", fileMenuSize)){
        e = huntCaseSelectInput();
        if (e != -1){
          huntInput(selected, e);
        }
        abortinput = 0;
        display_dir(currentpwd, ob);
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_xexec", fileMenuSize)){
        memcpy(chpwd, currentpwd, 4096);
        if (!check_last_char(chpwd, "/")){
          snprintf(chpwd + strlen(chpwd), 4096, "%s", "/");
        }
        snprintf(chpwd + strlen(chpwd), 4096, "%s", ob[selected].name);
        if (check_exec(chpwd)){
          execArgs = execute_argument_input(ob[selected].name);
          if (!abortinput){
            LaunchExecutable(chpwd, execArgs);
            free(execArgs);
          }
          abortinput = 0;
          display_dir(currentpwd, ob);
        } else {
          topLineMessage("Error: Permission denied");
        }
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_01", functionMenuSize) || *pc == 338){
        if (selected < (totalfilecount - 1) ) {
          clear_workspace();
          for (i = lineStart; i < listLen; i++){
            if (el[i].fileRef == selected) {
              if ((i + displaysize) < listLen){
                selected = el[i + displaysize].fileRef;
              } else {
                selected = totalfilecount - 1;
              }
              break;
            }
          }
          if (listLen < displaysize){
            lineStart = 0;
          } else {
            lineStart = lineStart + displaysize;
            if (lineStart > (listLen - displaysize)){
              lineStart = listLen - displaysize;
            }
          }
        }
        display_dir(currentpwd, ob);
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_02", functionMenuSize) || *pc == 339){
        if (selected > 0){
          clear_workspace();
          lineStart = lineStart - displaysize;
          if (lineStart < 0){
            lineStart = 0;
          }
          for (i = 0; i < listLen; i++){
            if ((el[i].fileRef == selected) && (el[i].entryLineType == ET_OBJECT)){
              if ((i - displaysize) < 0) {
                selected = 0;
              } else {
                selected = el[i - displaysize].fileRef;
                if (el[i - displaysize].entryLineType != ET_OBJECT){
                  selected++;
                }
              }
            }
          }
        }
        display_dir(currentpwd, ob);
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_03", functionMenuSize)){
        clear_workspace();
        selected = 0;
        lineStart = 0;
        display_dir(currentpwd, ob);
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_04", functionMenuSize)){
        clear_workspace();
        selected = totalfilecount - 1;
        if ((totalfilecount - 1 ) > bottomFileRef){
          lineStart = listLen - displaysize;
        } else {
          lineStart = 0;
        }
        display_dir(currentpwd, ob);
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_05", functionMenuSize)){
        refreshDirectory(sortmode, lineStart, selected, 0);
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_06", functionMenuSize)){
        memcpy(selfile, currentpwd, 4096);
        if (!check_last_char(chpwd, "/")){
          snprintf(selfile + strlen(selfile), 4096, "%s", "/");
        }
        snprintf(selfile + strlen(selfile), 4096, "%s", ob[selected].name);
        if (!check_dir(selfile)){
          if ( *ob[selected].marked ){
            *ob[selected].marked = 0;
            clear_workspace();
          } else {
            *ob[selected].marked = 1;
          }
          if (selected < (totalfilecount - 1)) {
            selected++;
            if (selected > bottomFileRef){
              lineStart = lineStartFromBottomFileRef(selected, el, listLen, displaysize);
              if (markedinfo == 2 && automark == 0){
                lineStart++;
              }
            }
          } else {
            if (markedinfo == 2 && automark == 0){
              lineStart++;
            }
          }
          clear_workspace();
          display_dir(currentpwd, ob);
        }
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_07", functionMenuSize)){
        markall = 1;
        freeResults(ob, totalfilecount);
        freeXAttrs(xa, xattrPos);
        ob = get_dir(currentpwd);
        markall = 0; // Leaving this set as 1 keeps things marked even after refresh. This is bad
        clear_workspace();
        reorder_ob(ob, sortmode);
        generateEntryLineIndex(ob);
        display_dir(currentpwd, ob);
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_08", functionMenuSize)){
        markall = 0;
        freeResults(ob, totalfilecount);
        freeXAttrs(xa, xattrPos);
        ob = get_dir(currentpwd);
        clear_workspace();
        reorder_ob(ob, sortmode);
        generateEntryLineIndex(ob);
        display_dir(currentpwd, ob);
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_09", functionMenuSize)){
        sort_view_inputs();
      } else if (*pc == menuHotkeyLookup(functionMenu, "f_10", functionMenuSize)){
          memcpy(selfile, currentpwd, 4096);
          if (!check_last_char(chpwd, "/")){
            snprintf(selfile + strlen(selfile), 4096, "%s", "/");
          }
          snprintf(selfile + strlen(selfile), 4096, "%s", ob[selected].name);
          if ( *ob[selected].marked == 0 ){
            if ( blockstart == -1 ){
              blockstart = selected;
              if (!check_dir(selfile)){
                *ob[selected].marked = 1;
              }
              if (selected < (totalfilecount - 1)) {
                selected++;
                if (selected > bottomFileRef){
                  lineStart = lineStartFromBottomFileRef(selected, el, listLen, displaysize);
                  if (markedinfo == 2 && automark == 0){
                    lineStart++;
                  }
                }
              } else {
                if (markedinfo == 2 && automark == 0){
                  lineStart++;
                }
              }
              clear_workspace();
              display_dir(currentpwd, ob);
            } else {
              blockend = selected;
              if (blockstart > blockend){
                // While we're still on the second item, let's flip them around if the second selected file is higher up the list.
                blockend = blockstart;
                blockstart = selected;
              }
              for(; blockstart < blockend + 1; blockstart++){
                memcpy(selfile, currentpwd, 4096);
                if (!check_last_char(selfile, "/")){
                  snprintf(selfile + strlen(selfile), 4096, "%s", "/");
                }
                snprintf(selfile + strlen(selfile), 4096, "%s", ob[blockstart].name);
                if (!check_dir(selfile)){
                  *ob[blockstart].marked = 1;
                }
              }
              blockstart = blockend = -1;
              display_dir(currentpwd, ob);
              }
            }
      } else if (*pc == 10){
        // Enter Key
        if (enterAsShow){
          goto showCommand;
        } else {
          goto moveDown;
        }
      } else if (*pc == 27){
        // Esc Key
        global_menu_inputs();
      } else if (*pc == 258){
        // Down Arrow
      moveDown:
        if (selected < (totalfilecount - 1)) {
          if (selected != bottomFileRef){
            selected++;
          } else {
            if (el[lineStart + displaysize].fileRef == el[lineStart + displaysize - 1].fileRef){
              lineStart++;
            } else {
              selected++;
              lineStart++;
            }
          }
          clear_workspace();
          display_dir(currentpwd, ob);
        }
      } else if (*pc == 259){
        // Up Arrow
        if (selected > 0){
          //topfileref is correct here.
          if (selected == topfileref){
            if (el[lineStart - 1].entryLineType != ET_OBJECT){
              lineStart--;
            } else {
              selected--;
              lineStart--;
            }
          } else {
            selected--;
          }
          clear_workspace();
          display_dir(currentpwd, ob);
        }
      } else if (*pc == 260){
        // Left Arrow
        if (hpos > 0){
          hpos--;
          clear_workspace();
          display_dir(currentpwd, ob);
        }
      } else if (*pc == 261){
        // Right Arrow
        if (hpos < (maxdisplaywidth - COLS)){
          hpos++;
          clear_workspace();
          display_dir(currentpwd, ob);
        }
      } else if (*pc == 262){
        // Home Key
        selected = el[lineStart].fileRef;
        display_dir(currentpwd, ob);
      } else if (*pc == 360){
        // End Key
        // selected = topfileref + (displaycount - 1);
        selected = bottomFileRef;
        display_dir(currentpwd, ob);
      // } else if (*pc == 276){
      //   // F12 Key
      }
    }
}
void global_menu_inputs()
{
  wPrintMenu(0,0,globalMenuLabel);
  if (historyref == 0){
    viewMode = 4;
  } else {
    viewMode = 1;
  }
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(globalMenu, "g_colors", globalMenuSize)){
        themeBuilder();
        theme_menu_inputs();
        if (historyref == 0){
          clear();
          global_menu_inputs();
        } else {
          refreshDirectory(sortmode, lineStart, selected, 0);
          wPrintMenu(LINES-1, 0, functionMenuLabel);
          global_menu_inputs();
        }
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_config", globalMenuSize)) {
        settingsMenuView();
        if (historyref == 0){
          clear();
          global_menu_inputs();
        } else {
          refreshDirectory(sortmode, lineStart, selected, 0);
          wPrintMenu(LINES-1, 0, functionMenuLabel);
          global_menu_inputs();
        }
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_run", globalMenuSize)) {
        LaunchShell();
        if (historyref == 0){
          wPrintMenu(0,0,globalMenuLabel);
        } else {
          refreshDirectory(sortmode, lineStart, selected, 1);
          directory_view_menu_inputs();
        }
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_edit", globalMenuSize)) {
        edit_file_input();
        if (historyref == 0){
          wPrintMenu(0,0,globalMenuLabel);
        } else {
          refreshDirectory(sortmode, lineStart, selected, 1);
          directory_view_menu_inputs();
        }
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_help", globalMenuSize)) {
        showManPage("show");
        refreshScreen();
        if (historyref == 0){
          wPrintMenu(0,0,globalMenuLabel);
        } else {
          directory_view_menu_inputs();
        }
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_mkdir", globalMenuSize)) {
        make_directory_input();
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_quit", globalMenuSize)) {
        if (historyref == 0){
          freeHistory(hs, sessionhistory);
          exittoshell();
          refresh();
        } else {
          historyref = 0;
          global_menu();
        }
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_show", globalMenuSize)) {
        show_directory_input();
      } else if (*pc == menuHotkeyLookup(globalMenu, "g_touch", globalMenuSize)) {
        touch_file_input();
      } else if (*pc == 27) {
        if (historyref != 0){
          directory_view_menu_inputs();
        }
      }
    }
}
