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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <libintl.h>
#include <locale.h>
#include "menu.h"
#include "display.h"
#include "settings.h"
#include "common.h"
#include "colors.h"
#include "input.h"
#include "i18n.h"
#include "sffunctions.h"
#include "banned.h"

bool findSet = false;

menuDef *sfFileMenu;
int sfFileMenuSize = 0;
wchar_t *sfFileMenuLabel;

menuDef *caseMenu;
int caseMenuSize = 0;
wchar_t *caseMenuLabel;

extern bool parentShow;

extern int c;
extern int * pc;

extern int abortinput;

extern settingIndex *settingIndexSf;
extern t1CharValues *charValuesSf;
extern t2BinValues *binValuesSf;
extern int totalCharItemsSf;
extern int totalBinItemsSf;

menuDef *sfSettingsMenu;
int sfSettingsMenuSize;
wchar_t *sfSettingsMenuLabel;

extern char regexinput[1024];
extern FILE *file;
extern int topline;
extern int leftcol;
extern char fileName[4096];
extern int displaysize;
extern int totallines;
extern int longestlongline;
extern int viewmode;

extern int wrap;
extern int wrapmode;

extern FILE *stream;
extern char *line;

extern long int *filePos;
extern wchar_t *longline;

void generateDefaultSfMenus(){
  // File Menu
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_01", _("<F1>-Down"), 265);
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_02", _("<F2>-Up"), 266);
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_03", _("<F3>-Top"), 267);
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_04", _("<F4>-Bottom"), 268);
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_config", _("!Config"), 'c');
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_find", _("!Find"), 'f');
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_help", _("!Help"), 'h');
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_position", _("!Position"), 'p');
  addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_quit", _("!Quit"), 'q');
  if (wrap){
    addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_wrap", _("!Wrap-off"), 'w');
  } else {
    addMenuItem(&sfFileMenu, &sfFileMenuSize, "f_wrap", _("!Wrap-on"), 'w');
  }

  // Case Menu
  addMenuItem(&caseMenu, &caseMenuSize, "c1_ignore", _("!Ignore-case"), 'i');
  addMenuItem(&caseMenu, &caseMenuSize, "c2_sensitive", _("!Case-sensitive"), 'c');

  // Setings Menu
  addMenuItem(&sfSettingsMenu, &sfSettingsMenuSize, "s_quit", _("!Quit"), 'q');
  addMenuItem(&sfSettingsMenu, &sfSettingsMenuSize, "s_revert", _("!Revert"), 'r');
  addMenuItem(&sfSettingsMenu, &sfSettingsMenuSize, "s_save", _("!Save"), 's');
}

void refreshSfMenuLabels(){
  sfFileMenuLabel     = genMenuDisplayLabel("", sfFileMenu, sfFileMenuSize, "", 1);
  caseMenuLabel     = genMenuDisplayLabel("", caseMenu, caseMenuSize, _("(enter = I)"), 0);
  sfSettingsMenuLabel = genMenuDisplayLabel(_("SF Settings Menu -"), sfSettingsMenu, sfSettingsMenuSize, "", 1);
}

void unloadSfMenuLabels(){
  free(sfFileMenuLabel);
  free(caseMenuLabel);
  free(sfSettingsMenuLabel);
}

void show_file_find(bool charcase, bool useLast)
{
  int regexcase;
  int result;
  int curPos = 0;
  char *inputmessage;
  char *errormessage;
  if (!useLast){
    if (charcase){
      regexcase = 0;
      setDynamicChar(&inputmessage, _("Match Case - Enter string:"));
    } else {
      regexcase = REG_ICASE;
      setDynamicChar(&inputmessage, _("Ignore Case - Enter string:"));
    }
    move(0,0);
    clrtoeol();
    curPos = (printMenu(0, 0, inputmessage) + 1);
    free(inputmessage);
    curs_set(TRUE);
    move(0, curPos);
    curs_set(FALSE);
    if (readline(regexinput, 1024, regexinput) == -1 ){
      abortinput = 1;
    }
  }
  if (abortinput != 1) {
    findSet = true;
    result = findInFile(fileName, regexinput, regexcase);
    if ( result > 0 ){
      topline = result;
      updateView();
    } else if ( result == -2 ){
      // Not a feature in DF-EDIT 2.3d, but a nice to have
      setDynamicChar(&errormessage, _("No further references to '%s' found."), regexinput);
      topLineMessage(errormessage);
      free(errormessage);
    }
  }
}

int show_file_find_case_input()
{
  int result;
  move(0,0);
  clrtoeol();
  wPrintMenu(0,0,caseMenuLabel);
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(caseMenu, "c1_ignore", caseMenuSize) || *pc == 10){
        result = 0;
        break;
      } else if (*pc == menuHotkeyLookup(caseMenu, "c2_sensitive", caseMenuSize)){
        result = 1;
        break;
      } else if (*pc == 27){
        // ESC
        result = -1;
        break;
      }
    }
  return(result);
}

void show_file_position_input(int currentpos)
{
  char newpos[11];
  char *filePosText;
  int status;
  int curPos = 0;
  // Fun fact, in DF-EDIT 2.3d, the following text input typoed "absolute" as "absolue", this typo also exists in the Windows version from 1997 (2.3d-76), however, the 1986 documentation correctly writes it as "absolute".
  setDynamicChar(&filePosText, _("Position relative (<+num> || <-num>) or absolute (<num>):"));
  viewmode = 2;
  move(0,0);
  clrtoeol();
  curPos = (printMenu(0,0,filePosText) + 1);
  free(filePosText);
  curs_set(TRUE);
  move(0,curPos);
  status = readline(newpos, 11, ""); // DF-EDIT defaulted to 0, but it also defaulted to overtype mode, so for ease of use, we'll leave the default blank.
  curs_set(FALSE);
  if ((status != -1) && (strcmp(newpos,"") != 0)){
    if (check_first_char(newpos, "+")){
      memmove(newpos, newpos+1, strlen(newpos));
      if (check_numbers_only(newpos)){
        topline = topline + atoi(newpos);
      }
    } else if (check_first_char(newpos, "-")) {
      memmove(newpos, newpos+1, strlen(newpos));
      if (check_numbers_only(newpos)){
        topline = topline - atoi(newpos);
      }
    } else {
      if (check_numbers_only(newpos)){
        topline = atoi(newpos);
      }
    }
  }
  wPrintMenu(0, 0, sfFileMenuLabel);
}

void show_file_inputs()
{
  int e = 0;
  wPrintMenu(0, 0, sfFileMenuLabel);
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(sfFileMenu,"f_find", sfFileMenuSize)){
        e = show_file_find_case_input();
        if (e != -1){
          show_file_find(e, false);
        } else {
          abortinput = 0;
        }
        wPrintMenu(0, 0, sfFileMenuLabel);
      } else if (*pc == 6){
        if (findSet){
          show_file_find(false, true);
        }
        wPrintMenu(0, 0, sfFileMenuLabel);
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_help", sfFileMenuSize)){
        showManPage("sf");
        wPrintMenu(0, 0, sfFileMenuLabel);
        refreshScreenSf();
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_position", sfFileMenuSize)){
        show_file_position_input(topline);
        if (topline > totallines + 1){
          topline = totallines + 1;
        } else if (topline < 1){
          topline = 1;
        }
        updateView();
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_config", sfFileMenuSize)){
        settingsMenuView(sfSettingsMenuLabel, sfSettingsMenuSize, sfSettingsMenu, &settingIndexSf, &charValuesSf, &binValuesSf, totalCharItemsSf, totalBinItemsSf, generateSfSettingsVars(), "sf");
        wPrintMenu(0, 0, sfFileMenuLabel);
        if(wrap){
          leftcol = 1;
        }
        refreshScreenSf();
        // updateView();
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_quit", sfFileMenuSize)){
        free(longline);
        free(filePos);
        fclose(stream);
	if (!parentShow){
          exittoshell();
	} else {
          return;
	}
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_wrap", sfFileMenuSize)){
        if (wrap){
          updateMenuItem(&sfFileMenu, &sfFileMenuSize, "f_wrap", _("!Wrap-on"));
          wrap = 0;
        } else {
          updateMenuItem(&sfFileMenu, &sfFileMenuSize, "f_wrap", _("!Wrap-off"));
          leftcol = 1;
          wrap = 1;
        }
        unloadSfMenuLabels();
        refreshSfMenuLabels();
        wPrintMenu(0,0,sfFileMenuLabel);
        updateView();
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_01", sfFileMenuSize) || *pc == 338){
        topline = topline + displaysize;
        if (topline > totallines + 1){
          topline = totallines + 1;
        }
        updateView();
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_02", sfFileMenuSize) || *pc == 339){
        topline = topline - displaysize;
        if (topline < 1){
          topline = 1;
        }
        updateView();
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_03", sfFileMenuSize)){
        topline = 1;
        updateView();
      } else if (*pc == menuHotkeyLookup(sfFileMenu, "f_04", sfFileMenuSize)){
        topline = totallines + 1; // Show EOF
        updateView();
      } else if (*pc == 258){
        // Down Arrow
        if (topline < totallines + 1){
          topline++;
          //loadFile(fileName);
          updateView();
        }
      } else if (*pc == 259){
        // Up Arrow
        if (topline > 1){
          topline--;
          updateView();
        }
      } else if (*pc == 260){
        // Left Arrow
        if ((leftcol > 1) && (wrap != 1)){
          leftcol--;
          updateView();
        }
      } else if (*pc == 261){
        // Right Arrow
        if ((leftcol < longestlongline) && (wrap != 1)){
          leftcol++;
          updateView();
        }
      } else if (*pc == 262){
        // Home
        // Let's not disable this key when Wrapping is on, just in case.
        leftcol = 1;
        updateView();
      } else if (*pc == 360){
        // End
        if (wrap != 1){
          leftcol = longestlongline;
          updateView();
        }
      }
   }
}

void show_file_file_input()
{
  char *rewrite;
  int curPos = 0;
  move(0,0);
  clrtoeol(); // Probably not needed as this is only ever displayed when launching without a file
  curPos = (printMenu(0,0,_("Show File - Enter pathname:")) + 1);
  curs_set(TRUE);
  move(0,curPos);
  readline(fileName, 4096, "");
  curs_set(FALSE);
  if (check_first_char(fileName, "~")){
    rewrite = str_replace(fileName, "~", getenv("HOME"));
    memcpy(fileName, rewrite, (strlen(rewrite) + 1));
    free(rewrite);
  }
  file_view(fileName);
}
