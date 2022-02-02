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
#include "sf.h"
#include "colors.h"
#include "input.h"
#include "i18n.h"
#include "banned.h"

int c;
int * pc = &c;

int abortinput = 0;

bool findSet = false;

menuDef *fileMenu;
int fileMenuSize = 0;
wchar_t *fileMenuLabel;

menuDef *caseMenu;
int caseMenuSize = 0;
wchar_t *caseMenuLabel;

extern menuDef *settingsMenu;
extern int settingsMenuSize;
extern wchar_t *settingsMenuLabel;

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

void generateDefaultMenus(){
  // File Menu
  addMenuItem(&fileMenu, &fileMenuSize, "f_01", _("<F1>-Down"), 265);
  addMenuItem(&fileMenu, &fileMenuSize, "f_02", _("<F2>-Up"), 266);
  addMenuItem(&fileMenu, &fileMenuSize, "f_03", _("<F3>-Top"), 267);
  addMenuItem(&fileMenu, &fileMenuSize, "f_04", _("<F4>-Bottom"), 268);
  addMenuItem(&fileMenu, &fileMenuSize, "f_config", _("!Config"), 'c');
  addMenuItem(&fileMenu, &fileMenuSize, "f_find", _("!Find"), 'f');
  addMenuItem(&fileMenu, &fileMenuSize, "f_help", _("!Help"), 'h');
  addMenuItem(&fileMenu, &fileMenuSize, "f_position", _("!Position"), 'p');
  addMenuItem(&fileMenu, &fileMenuSize, "f_quit", _("!Quit"), 'q');
  if (wrap){
    addMenuItem(&fileMenu, &fileMenuSize, "f_wrap", _("!Wrap-off"), 'w');
  } else {
    addMenuItem(&fileMenu, &fileMenuSize, "f_wrap", _("!Wrap-on"), 'w');
  }

  // Case Menu
  addMenuItem(&caseMenu, &caseMenuSize, "c1_ignore", _("!Ignore-case"), 'i');
  addMenuItem(&caseMenu, &caseMenuSize, "c2_sensitive", _("!Case-sensitive"), 'c');

  // Setings Menu
  addMenuItem(&settingsMenu, &settingsMenuSize, "s_quit", _("!Quit"), 'q');
  addMenuItem(&settingsMenu, &settingsMenuSize, "s_revert", _("!Revert"), 'r');
  addMenuItem(&settingsMenu, &settingsMenuSize, "s_save", _("!Save"), 's');
}

void refreshMenuLabels(){
  fileMenuLabel     = genMenuDisplayLabel(L"", fileMenu, fileMenuSize, L"", 1);
  caseMenuLabel     = genMenuDisplayLabel(L"", caseMenu, caseMenuSize, L"(enter = I)", 0);
  settingsMenuLabel = genMenuDisplayLabel(L"SF Settings Menu -", settingsMenu, settingsMenuSize, L"", 1);
}

void unloadMenuLabels(){
  free(fileMenuLabel);
  free(caseMenuLabel);
  free(settingsMenuLabel);
}

void show_file_find(bool charcase, bool useLast)
{
  int regexcase;
  int result;
  int curPos = 0;
  char inputmessage[32];
  char *errormessage;
  if (!useLast){
    if (charcase){
      regexcase = 0;
      snprintf(inputmessage, 32, "Match Case - Enter string:");
    } else {
      regexcase = REG_ICASE;
      snprintf(inputmessage, 32, "Ignore Case - Enter string:");
    }
    move(0,0);
    clrtoeol();
    curPos = (printMenu(0, 0, inputmessage) + 1);
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
      setDynamicChar(&errormessage, "No further references to '%s' found.", regexinput);
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
  setDynamicChar(&filePosText, "Position relative (<+num> || <-num>) or absolute (<num>):");
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
  wPrintMenu(0, 0, fileMenuLabel);
}

void show_file_inputs()
{
  int e = 0;
  wPrintMenu(0, 0, fileMenuLabel);
  while(1)
    {
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(fileMenu,"f_find", fileMenuSize)){
        e = show_file_find_case_input();
        if (e != -1){
          show_file_find(e, false);
        } else {
          abortinput = 0;
        }
        wPrintMenu(0, 0, fileMenuLabel);
      } else if (*pc == 6){
        if (findSet){
          show_file_find(false, true);
        }
        wPrintMenu(0, 0, fileMenuLabel);
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_help", fileMenuSize)){
        showManPage("sf");
        wPrintMenu(0, 0, fileMenuLabel);
        refreshScreen();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_position", fileMenuSize)){
        show_file_position_input(topline);
        if (topline > totallines + 1){
          topline = totallines + 1;
        } else if (topline < 1){
          topline = 1;
        }
        updateView();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_config", fileMenuSize)){
        settingsMenuView();
        wPrintMenu(0, 0, fileMenuLabel);
        if(wrap){
          leftcol = 1;
        }
        refreshScreen();
        // updateView();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_quit", fileMenuSize)){
        free(longline);
        free(filePos);
        fclose(stream);
        exittoshell();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_wrap", fileMenuSize)){
        if (wrap){
          updateMenuItem(&fileMenu, &fileMenuSize, "f_wrap", L"!Wrap-on");
          wrap = 0;
        } else {
          updateMenuItem(&fileMenu, &fileMenuSize, "f_wrap", L"!Wrap-off");
          leftcol = 1;
          wrap = 1;
        }
        unloadMenuLabels();
        refreshMenuLabels();
        wPrintMenu(0,0,fileMenuLabel);
        updateView();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_01", fileMenuSize) || *pc == 338){
        topline = topline + displaysize;
        if (topline > totallines + 1){
          topline = totallines + 1;
        }
        updateView();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_02", fileMenuSize) || *pc == 339){
        topline = topline - displaysize;
        if (topline < 1){
          topline = 1;
        }
        updateView();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_03", fileMenuSize)){
        topline = 1;
        updateView();
      } else if (*pc == menuHotkeyLookup(fileMenu, "f_04", fileMenuSize)){
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
  curPos = (printMenu(0,0,"Show File - Enter pathname:") + 1);
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
