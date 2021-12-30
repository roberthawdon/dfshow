/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2021  Robert Ian Hawdon

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
#include "colors.h"
#include "menu.h"
#include "banned.h"

int cmp_menu_ref(const void *lhs, const void *rhs)
{

  menuDef *dforderA = (menuDef *)lhs;
  menuDef *dforderB = (menuDef *)rhs;

  return strcoll(dforderA->refLabel, dforderB->refLabel);

}

void updateMenuItem(menuDef **dfMenu, int *menuSize, char* refLabel, wchar_t* displayLabel){
  // To Do
  int i;
  for(i = 0; i < *menuSize; i++){
    if (!strcmp(((*dfMenu)[i].refLabel), refLabel)){
      swprintf((*dfMenu)[i].displayLabel, 32, L"%ls", displayLabel);
      break;
    }
  }
  return;
}

void addMenuItem(menuDef **dfMenu, int *pos, char* refLabel, wchar_t* displayLabel, int hotKey){

  int menuPos = *pos;
  int charCount = 0;
  int i;
  menuDef *tmp;

  if (menuPos == 0){
    tmp = malloc(sizeof(menuDef) * 2);
  } else {
    tmp = realloc(*dfMenu, (menuPos + 1) * (sizeof(menuDef) + 1) );
  }
  if (tmp){
    *dfMenu = tmp;
  }

  snprintf((*dfMenu)[menuPos].refLabel, 16, "%s", refLabel);
  swprintf((*dfMenu)[menuPos].displayLabel, 32, L"%ls", displayLabel);
  (*dfMenu)[menuPos].hotKey = hotKey;

  for (i = 0; i < wcslen(displayLabel); i++)
    {
      if ( displayLabel[i] == '!' || displayLabel[i] == '<' || displayLabel[i] == '>' || displayLabel[i] == '\\') {
        i++;
        charCount++;
      } else {
        charCount++;
      }
    }
  (*dfMenu)[menuPos].displayLabelSize = charCount;

  qsort((*dfMenu), menuPos + 1, sizeof(menuDef), cmp_menu_ref);

  ++*pos;

}

wchar_t * genMenuDisplayLabel(wchar_t* preMenu, menuDef* dfMenu, int size, wchar_t* postMenu, int comma){
  wchar_t * output;
  int gapSize;
  int currentLen = 0;
  int i;

  output = malloc(sizeof(wchar_t) * ( wcslen(preMenu) + 2));
  if (wcscmp(preMenu, L"")){
    wcscpy(output, preMenu);
    wcscat(output, L" ");
  } else {
    wcscpy(output, L"\0");
  }
  for (i = 0; i < size ; i++){
    output = realloc(output, ((i + 1) * sizeof(dfMenu[i].displayLabel) + wcslen(output) + 1) * sizeof(wchar_t) );
   if ( i == 0 ){
     currentLen = currentLen + dfMenu[i].displayLabelSize;
     if ( currentLen - 1 < COLS){
       wcscat(output, dfMenu[i].displayLabel);
     } else if ( currentLen +1 > COLS && i == 0){
       wcscat(output, L"");
     }
   } else {
     if (comma == 1){
       gapSize = 2;
     } else if (comma == -1) {
       gapSize = 0;
     } else {
       gapSize = 1;
     }
     currentLen = currentLen + dfMenu[i].displayLabelSize + gapSize;
     if (currentLen - 1 < COLS){
       if (comma == 1){
         wcscat(output, L", ");
       } else if (comma == 0) {
         wcscat(output, L" ");
       }
       wcscat(output, dfMenu[i].displayLabel);
     }
   }
  }
  output = realloc(output, (sizeof(wchar_t) * (wcslen(output) + wcslen(postMenu) + 2) ));
  if (wcscmp(postMenu, L"")){
    wcscat(output, L" ");
    wcscat(output, postMenu);
  } else {
    wcscat(output, L"\0");
  }
  return output;
}

int menuHotkeyLookup(menuDef* dfMenu, char* refLabel, int size){
  int i;
  int r = -1;
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
  returnChars = 0;
  charcount = 0;
  move(line, col);
  clrtoeol();
  len = wcslen(menustring);
  setColors(COMMAND_PAIR);
  for (i = 0; i < len; i++)
    {
      if ( menustring[i] == '!' ) {
        setColors(HILITE_PAIR);
        i++;
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        setColors(COMMAND_PAIR);
        charcount++;
      } else if ( menustring[i] == '<' ) {
        setColors(HILITE_PAIR);
        i++;
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        charcount++;
      } else if ( menustring[i] == '>' ) {
        setColors(COMMAND_PAIR);
        if (i < (len - 1)){
          i++;
          mvprintw(line, col + charcount, "%lc", menustring[i]);
          charcount++;
        }
      } else if ( menustring[i] == '\\' ) {
        i++;
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        charcount++;
      } else {
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        charcount++;
      }
    }
  returnChars = charcount;
  pad = COLS - charcount;
  for (i = 0; i < pad; i++)
    {
      mvprintw(line, col + charcount, " ");
      charcount++;
    }
  return(returnChars);
}

int printMenu(int line, int col, char *menustring)
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
