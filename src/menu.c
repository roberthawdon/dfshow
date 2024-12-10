/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2024  Robert Ian Hawdon

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
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <libintl.h>
#include "colors.h"
#include "common.h"
#include "menu.h"
#include "banned.h"
#include "i18n.h"

int topMenuStart = 0;
int bottomMenuStart = 0;
bool topMenu = true;
bool bottomMenu = true;
wchar_t *topMenuBuffer;
wchar_t *bottomMenuBuffer;

extern int * pc;
extern MEVENT event;

int dynamicMenuLabel(wchar_t **label, const char *str)
{
  int length = 0;

  length = strlen(str);
  length++;

  *label = malloc(sizeof(wchar_t) * length);
  mbstowcs(*label, str, length);

  return(length);
}

int cmp_menu_ref(const void *lhs, const void *rhs)
{

  menuDef *dforderA = (menuDef *)lhs;
  menuDef *dforderB = (menuDef *)rhs;

  return strcoll(dforderA->refLabel, dforderB->refLabel);

}

void updateMenuItem(menuDef **dfMenu, int *menuSize, char* refLabel, char* displayLabel){
  // To Do
  int i;
  wchar_t *wideDisplayLabel;

  wideDisplayLabel = malloc(sizeof(wchar_t) * (strlen(displayLabel) + 1 ));
  mbstowcs(wideDisplayLabel, displayLabel, (strlen(displayLabel) + 1));

  for(i = 0; i < *menuSize; i++){
    if (!strcmp(((*dfMenu)[i].refLabel), refLabel)){
      swprintf((*dfMenu)[i].displayLabel, 32, L"%ls", wideDisplayLabel);
      break;
    }
  }
  free(wideDisplayLabel);
  return;
}

void addMenuItem(menuDef **dfMenu, int *pos, char* refLabel, char* displayLabel, int defaultHotKey, bool sort){

  int menuPos = *pos;
  int charCount = 0;
  int i;
  menuDef *tmp;
  wchar_t *wideLabel;
  bool hotKeyFound = false;

  if (menuPos == 0){
    tmp = malloc(sizeof(menuDef) * 2);
  } else {
    tmp = realloc(*dfMenu, (menuPos + 1) * (sizeof(menuDef) + 1) );
  }
  if (tmp){
    *dfMenu = tmp;
  }

  wideLabel = malloc(sizeof(wchar_t) * (strlen(displayLabel) + 1));
  mbstowcs(wideLabel, displayLabel, (strlen(displayLabel) + 1));

  snprintf((*dfMenu)[menuPos].refLabel, 16, "%s", refLabel);
  swprintf((*dfMenu)[menuPos].displayLabel, 32, L"%ls", wideLabel);
  (*dfMenu)[menuPos].hotKey = defaultHotKey; // Set the hotkey to the one we've provided first

  for (i = 0; i < wcslen(wideLabel); i++)
    {
      if ( wideLabel[i] == '!' || wideLabel[i] == '<' || wideLabel[i] == '>' || wideLabel[i] == '\\') {
        if (wideLabel[i] == '!' && !hotKeyFound){
          (*dfMenu)[menuPos].hotKey = tolower(wideLabel[i + 1]);
          hotKeyFound = true;
        }
        i++;
        charCount = charCount + wcwidth(wideLabel[i]);
      } else {
        charCount = charCount + wcwidth(wideLabel[i]);
      }
    }
  free(wideLabel);
  (*dfMenu)[menuPos].displayLabelSize = charCount;

  if (sort){
      qsort((*dfMenu), menuPos + 1, sizeof(menuDef), cmp_menu_ref);
  }

  ++*pos;

}

char * menuButtonLookup(menuButton *dfButtons, int size, int xpos, int ypos, int xdelta, int ydelta, bool allowEsc){
  static char outputRef[16];
  int i;
  int topX = -1;
  int bottomX = -1;
  int topY = -1;
  int bottomY = -1;

  snprintf(outputRef, 16, "%s", "\0");

  for (i = 0; i < size; i++){
    topX = dfButtons[i].topX + xdelta;
    bottomX = dfButtons[i].bottomX + xdelta;
    topY = dfButtons[i].topY + ydelta;
    bottomY = dfButtons[i].bottomY + ydelta;
    if ((xpos >= topX && xpos <= bottomX) && (ypos >= topY && ypos <= bottomY)){
        snprintf(outputRef, 16, "%s", dfButtons[i].refLabel);
        return outputRef;
    }
  }
  if (allowEsc) {
    if (xpos > topX && xpos > bottomX) {
      snprintf(outputRef, 16, "escChar");
      return outputRef;
    }
  }
  return outputRef;
}

wchar_t * genMenuDisplayLabel(char* preMenu, menuDef* dfMenu, int size, char* postMenu, int comma, menuButton **dfButtons){
  wchar_t * output;
  int gapSize = 0;
  int currentLen = 0;
  int i;
  int c;
  int startPos = -1;
  int currentPosLen = 0;
  wchar_t *widePreMenu;
  wchar_t *widePostMenu;
  menuButton *tmp;

  tmp = malloc(sizeof(menuButton) * size + 1);

  if (tmp){
    *dfButtons = tmp;
  }

  widePreMenu = malloc(sizeof(wchar_t) * (strlen(preMenu) + 1));
  mbstowcs(widePreMenu, preMenu, (strlen(preMenu) + 1));

  widePostMenu = malloc(sizeof(wchar_t) * (strlen(postMenu) + 1));
  mbstowcs(widePostMenu, postMenu, (strlen(postMenu) + 1));

  // output = malloc(sizeof(wchar_t) * ( wcslen(widePreMenu) + 2));
  if (wcscmp(widePreMenu, L"")){
    // wcscpy(output, widePreMenu);
    // wcscat(output, L" ");
    // startPos = startPos + setDynamicWChar(&output, L"%ls ", widePreMenu);
    c = setDynamicWChar(&output, L"%ls ", widePreMenu);
  } else {
    // wcscpy(output, L"\0");
    c = setDynamicWChar(&output, L"\0");
    // startPos = startPos + setDynamicWChar(&output, L"\0");
  }
  free(widePreMenu);
  startPos = startPos + c;
  for (i = 0; i < size ; i++){
   startPos = startPos + gapSize;
   // output = realloc(output, ((i + 1) * sizeof(dfMenu[i].displayLabel) + wcslen(output) + 1) * sizeof(wchar_t) );
   dfMenu[i].startPos = startPos;
   snprintf((*dfButtons)[i].refLabel, 16, "%s", dfMenu[i].refLabel);
   (*dfButtons)[i].topX = startPos;
   currentPosLen = gapSize = 0;
   // startPos = startPos + setDynamicWChar(&output, L"%ls%ls", output, dfMenu[i].displayLabel);
   if (comma == 1){
     gapSize = 2;
   } else if (comma == -1) {
     gapSize = 0;
   } else {
     gapSize = 1;
   }
   if ( i == 0 ){
     currentLen = currentLen + dfMenu[i].displayLabelSize;
     if ( currentLen - 1 < COLS){
       // wcscat(output, dfMenu[i].displayLabel);
       c = setDynamicWChar(&output, L"%ls%ls", output, dfMenu[i].displayLabel);
     } else if ( currentLen +1 > COLS && i == 0){
       // wcscat(output, L"");
     }
   } else {
     currentLen = currentLen + dfMenu[i].displayLabelSize + gapSize;
     if (currentLen - 1 < COLS){
       if (comma == 1){
         // wcscat(output, L", ");
         c = setDynamicWChar(&output, L"%ls%ls", output, L", ");
       } else if (comma == 0) {
         // wcscat(output, L" ");
         c = setDynamicWChar(&output, L"%ls%ls", output, L" ");
       }
       // wcscat(output, dfMenu[i].displayLabel);
         c = setDynamicWChar(&output, L"%ls%ls", output, dfMenu[i].displayLabel);
     }
   }
   // currentPosLen = currentPosLen + dfMenu[i].displayLabelSize;
   currentPosLen = currentPosLen + wPrintMenu(-1, -1, dfMenu[i].displayLabel);
   startPos = startPos + currentPosLen;
   (*dfButtons)[i].bottomX = startPos - 1;
   (*dfButtons)[i].topY = (*dfButtons)[i].bottomY = 0;
  }
  // output = realloc(output, (sizeof(wchar_t) * (wcslen(output) + wcslen(widePostMenu) + 2) ));
  if (wcscmp(widePostMenu, L"")){
    // wcscat(output, L" ");
    // wcscat(output, widePostMenu);
    c = setDynamicWChar(&output, L"%ls %ls", output, widePostMenu);
  } else {
    // wcscat(output, L"\0");
    c = setDynamicWChar(&output, L"%ls\0", output);
  }

  // endwin();
  // for (i = 0; i < size; i++){
  //   printf("%s:\nTX: %i, TY: %i\nBX: %i, BY: %i\n\n", (*dfButtons)[i].refLabel, (*dfButtons)[i].topX, (*dfButtons)[i].topY, (*dfButtons)[i].bottomX, (*dfButtons)[i].bottomY);
  // }
  // exit(123);

  free(widePostMenu);
  return output;
}

int menuHotkeyLookup(menuDef* dfMenu, char* refLabel, int size){
  int i;
  int r = -1;
  if (!strcmp(refLabel, "escChar")){
    // Instantly return escape
    return 27;
  }
  for (i = 0; i < size; i++){
    if (!strcmp(dfMenu[i].refLabel, refLabel)){
      r = dfMenu[i].hotKey;
    }
  }
  return r;
}

int altHotkey(int key)
{
  int alt;
  if ((key < 123) && (key > 96)){
    alt = key - 32;
  } else if ((key < 91) && (key > 64)){
    alt = key + 32;
  } else {
    alt = -1;
  }
  return(alt);
}

int wPrintMenu(int line, int col, wchar_t *menustring)
{
  int i, len, charcount, pad, returnChars;
  bool writeOut = true;
  returnChars = 0;
  charcount = 0;
  move(line, col);
  clrtoeol();
  len = wcslen(menustring);
  if ( line < 0 || col < 0 ){
    writeOut = false;
  }
  if ( line == 0 ){
    setDynamicWChar(&topMenuBuffer, L"%ls", menustring);
    topMenu = true;
    topMenuStart = col;
  } else {
    setDynamicWChar(&bottomMenuBuffer, L"%ls", menustring);
    bottomMenu = true;
    bottomMenuStart = col;
  }
  setColors(COMMAND_PAIR);
  for (i = 0; i < len; i++)
    {
      if ( menustring[i] == '!' ) {
        setColors(HILITE_PAIR);
        i++;
        if (writeOut) {
          mvprintw(line, col + charcount, "%lc", menustring[i]);
        }
        setColors(COMMAND_PAIR);
        charcount = charcount + wcwidth(menustring[i]);
      } else if ( menustring[i] == '<' ) {
        setColors(HILITE_PAIR);
        i++;
        if (writeOut) {
          mvprintw(line, col + charcount, "%lc", menustring[i]);
        }
        charcount = charcount + wcwidth(menustring[i]);
      } else if ( menustring[i] == '>' ) {
        setColors(COMMAND_PAIR);
        if (i < (len - 1)){
          i++;
          if (writeOut) {
            mvprintw(line, col + charcount, "%lc", menustring[i]);
          }
          charcount = charcount + wcwidth(menustring[i]);
        }
      } else if ( menustring[i] == '\\' ) {
        i++;
        if (writeOut) {
          mvprintw(line, col + charcount, "%lc", menustring[i]);
        }
        charcount = charcount + wcwidth(menustring[i]);
      } else {
        if (writeOut) {
          mvprintw(line, col + charcount, "%lc", menustring[i]);
        }
        charcount = charcount + wcwidth(menustring[i]);
      }
    }
  returnChars = charcount;
  pad = COLS - charcount;
  for (i = 0; i < pad; i++)
    {
      if (writeOut) {
        mvprintw(line, col + charcount, " ");
      }
      charcount++;
    }
  // endwin();
  // printf("Rawlen: %li, Printlen: %i\n", wcslen(menustring), returnChars);
  return(returnChars);
}

int printMenu(int line, int col, const char *menustring)
{
  // Small wrapper to seemlessly forward calls to the wide char version
  wchar_t *wMenuString;
  int returnChars = 0;
  wMenuString = malloc(sizeof(wchar_t) * (strlen(menustring) + 1));
  swprintf(wMenuString, strlen(menustring) + 1, L"%s", menustring);
  returnChars = wPrintMenu(line, col, wMenuString);
  free(wMenuString);
  return(returnChars);
}

int commonConfirmMenu(int line, int col, const char *label, bool multi){
  int i, c, b;
  int output = -1;
  char *optionLabels;
  int optionLabelsLen = 2; // Initial bracket and null terminator
  char *option[4];
  int hotkey[4];
  int options = 4;
  menuButton *optionButton;

  b = 0;

  setDynamicChar(&option[0], _("!Yes"));
  setDynamicChar(&option[1], _("!No"));
  setDynamicChar(&option[2], _("!All"));
  setDynamicChar(&option[3], _("!Stop"));

  for ( i = 0; i < 4; i++ ){
    for (c = 0; c < strlen(option[i]); c++){
      if (option[i][c] == '!'){
        hotkey[i] = tolower(option[i][c + 1]);
      }
    }
  }

  if (!multi){
    options = 2;
  }

  optionButton = malloc(sizeof(menuButton) * options);

  optionLabelsLen = strlen(label) + 3;

  b = printMenu(-1, -1, label) + 2;

  for (i = 0; i < options; i++){
     optionLabelsLen = optionLabelsLen + strlen(option[i]) + 1;
     snprintf(optionButton[i].refLabel, 16, "button%d", i);
     optionButton[i].topX = b;
     b = b + strlen(option[i]) - 1;
     optionButton[i].bottomX = b - 1;
     optionButton[i].topY = optionButton[i].bottomY = line;
     b++;
  }

  optionLabels = malloc(sizeof(char) * optionLabelsLen);

  snprintf(optionLabels, optionLabelsLen, "%s (", label); // Label and Inital Bracket

  for (i = 0; i < options; i++){
    snprintf(optionLabels, optionLabelsLen, "%s%s", optionLabels, option[i]);
    if ( i < (options - 1) ){
      snprintf(optionLabels, optionLabelsLen, "%s%s", optionLabels, "/");
    }
  }

  snprintf(optionLabels, optionLabelsLen, "%s)", optionLabels); // Closing Bracket

  printMenu(line, col, optionLabels);

  free(optionLabels);

  while(1)
    {
      *pc = getch10th();
      loop:
      if (getmouse(&event) == OK) {
        if (event.bstate & BUTTON1_PRESSED){
          if (event.y == line){
            if (!strcmp(menuButtonLookup(optionButton, options, event.x, event.y, line, col, true), "button0")){
              *pc = hotkey[0];
            } else if (!strcmp(menuButtonLookup(optionButton, options, event.x, event.y, line, col, true), "button1")){
              *pc = hotkey[1];
            } else if (!strcmp(menuButtonLookup(optionButton, options, event.x, event.y, line, col, true), "button2")){
              *pc = hotkey[2];
            } else if (!strcmp(menuButtonLookup(optionButton, options, event.x, event.y, line, col, true), "button3")){
              *pc = hotkey[3];
            } else {
              *pc = 27;
            }
            goto loop;
          }
        }
      }
      if (*pc == KEY_MOUSE) {
        // Ignore mouse input conflicting with menu functions.
        continue;
      } else if (*pc == hotkey[0]){
        // Yes
        output = 0;
      } else if (*pc == hotkey[1]){
        // No
        output = 1;
      } else if (*pc == hotkey[2]){
        // All
        if (multi){
          output = 2;
        } else {
          output = -1;
        }
      } else if (*pc == hotkey[3]){
        // Stop
        if (multi){
          output = 3;
        } else {
          output = -1;
        }
      } else {
        output = -1;
      }
      break;
    }

  return output;
}
