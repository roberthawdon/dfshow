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
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <libconfig.h>
#include <wchar.h>
#include <math.h>
#include "menu.h"
#include "settings.h"
#include "colors.h"
#include "input.h"
#include "banned.h"

menuDef *settingsMenu;
int settingsMenuSize = 0;
wchar_t *settingsMenuLabel;

int settingsPos = 0;
int settingsBinPos = -1;
int settingsFreePos = -1;

void updateSetting(settingIndex **settings, int index, int type, int intSetting)
{
  // To-Do, do a verification on the type

  (*settings)[index].intSetting = intSetting;
}

void addT1CharValue(t1CharValues **values, int *totalItems, int *maxItem, char *refLabel, char *value)
{
  t1CharValues *tmp;
  int i = *totalItems, j, k;

  if (i == 0){
    tmp = malloc(sizeof(t1CharValues) * 2);
    j = -1;
  } else {
    tmp = realloc(*values, (i + 1) * (sizeof(t1CharValues) + 1));
  }

  if (tmp){
    *values = tmp;
  }

  for (k = 0; k < i; k++){
    if (strcmp((*values)[k].refLabel, refLabel)){
      j = -1;
    } else {
      j = (*values)[k].index;
    }
  }

  j++;
  (*values)[i].index = j;
  snprintf((*values)[i].refLabel, 16, "%s", refLabel);
  snprintf((*values)[i].value, 16, "%s", value);

  ++*totalItems;
  ++*maxItem;

}

void addT2BinValue(t2BinValues **values, int *totalItems, int *maxItem, char *refLabel, char *settingLabel, int reset)
{
  t2BinValues *tmp;
  int value;
  int i = *totalItems, j;

  if (i == 0){
    tmp = malloc(sizeof(t2BinValues) * 2);
    j = -1;
  } else {
    tmp = realloc(*values, (i + 1) * (sizeof(t2BinValues) + 1));
  }

  if (tmp){
    *values = tmp;
  }

  if (reset == 1){
    j = -1;
  } else {
    j = ((*values))[i - 1].index;
  }

  value = pow(2, *maxItem);

  j++;
  ((*values))[i].index = j;
  snprintf((*values)[i].refLabel, 16, "%s", refLabel);
  snprintf((*values)[i].settingLabel, 16, "%s", settingLabel);
  ((*values))[i].value = value;
  ((*values))[i].boolVal = 0;

  ++*totalItems;
  ++*maxItem;

}

void importSetting(settingIndex **settings, int *items, char *refLabel, wchar_t *textLabel, int type, char *charSetting, int intSetting, int maxValue, int invert)
{
  settingIndex *tmp;
  int currentItem = *items;

  if (*items == 0){
    tmp = malloc(sizeof(settingIndex) * 2);
  } else {
    tmp = realloc(*settings, (currentItem + 1) * (sizeof(settingIndex) + 1));
  }

  if (tmp){
    *settings = tmp;
  }

  (*settings)[currentItem].type = type;
  snprintf((*settings)[currentItem].refLabel, 16, "%s", refLabel);
  swprintf((*settings)[currentItem].textLabel, 32, L"%ls", textLabel);
  (*settings)[currentItem].intSetting = intSetting;
  (*settings)[currentItem].maxValue = maxValue;
  (*settings)[currentItem].invert = invert;

  if (charSetting){
    (*settings)[currentItem].charSetting = malloc(sizeof(char) * (strlen(charSetting) + 1));
    snprintf((*settings)[currentItem].charSetting, (strlen(charSetting) + 1), "%s", charSetting);
  }

  ++*items;
}

int intSettingValue(int *setting, int newValue){
  if (newValue > -1){
    *setting = newValue;
  }
  return *setting;
}

void populateBool(t2BinValues **values, char *refLabel, int setting, int maxValue)
{
  int i;

  for (i = maxValue - 1; i > -1 ; i--){
    if (!strcmp((*values)[i].refLabel, refLabel)){
      if (setting - ((*values))[i].value > - 1){
        ((*values))[i].boolVal = 1;
        setting = setting - ((*values))[i].value;
      }
    }
  }
}

void adjustBinSetting(settingIndex **settings, t2BinValues **values, char *refLabel, int *setting, int maxValue)
{
  int i;

  for (i = 0; i < maxValue + 1; i++){
    if (!strcmp((*values)[i].refLabel, refLabel) && ((*values)[i].index == settingsBinPos)){
      if ((*values)[i].boolVal > 0){
        (*settings)[settingsPos].intSetting = (*settings)[settingsPos].intSetting - (*values)[i].value;
        (*values)[i].boolVal = 0;
      } else {
        (*settings)[settingsPos].intSetting = (*settings)[settingsPos].intSetting + (*values)[i].value;
        (*values)[i].boolVal = 1;
      }
    }
  }
}

void printSetting(int line, int col, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int index, int charIndex, int binIndex, int type, int invert)
{

  int settingWork, b, c, i, v;
  int labelLen = 0, valueLen = 0, itemAdjust = 0;
  char refLabel[16];

  labelLen = wcslen((*settings)[index].textLabel) + 2;
  snprintf(refLabel, 16, "%s", (*settings)[index].refLabel);

  for (i = 0; i < charIndex; i++){
    if (!strcmp((*values)[i].refLabel, refLabel) && ((*values)[i].index) == 0){
      v = i;
    }
  }

  for (c = 0; c < binIndex; c++){
    if (!strcmp((*bins)[c].refLabel, refLabel) && ((*bins)[c].index) == 0){
      b = c;
    }
  }

  if (type == 0 ){
    if (invert == 1){
      if ((*settings)[index].intSetting > 0){
        settingWork = 0;
      } else {
        settingWork = 1;
      }
    } else {
      settingWork = (*settings)[index].intSetting;
    }
    setColors(HILITE_PAIR);
    mvprintw(line, col, "[");
    if (settingsPos == index){
      attron(A_REVERSE);
    }
    if (settingWork == 0){
      mvprintw(line, col + 1, " ");
    } else {
      mvprintw(line, col + 1, "*");
    }
    attroff(A_REVERSE);
    mvprintw(line, col + 2, "]");
    setColors(COMMAND_PAIR);
    mvprintw(line, col + 4, "%ls", (*settings)[index].textLabel);
  } else if (type == 1){
    setColors(HILITE_PAIR);
    mvprintw(line, col, "<");
    if (settingsPos == index){
      attron(A_REVERSE);
    }
    if ((*settings)[index].maxValue > 0) {
      mvprintw(line, col + 1, "-");
    } else {
      mvprintw(line, col + 1, "?");
    }
    attroff(A_REVERSE);
    mvprintw(line, col + 2, ">");
    setColors(COMMAND_PAIR);
    mvprintw(line, col + 4, "%ls:", (*settings)[index].textLabel);
    for(i = 0; i < ((*settings)[index].maxValue); i++){
      //Temp Test
      valueLen = strlen((*values)[i + v].value) + 3;
      if (i == (*settings)[index].intSetting){
        setColors(HILITE_PAIR);
      } else {
        setColors(COMMAND_PAIR);
      }
      mvprintw(line, (col + 4 + labelLen + itemAdjust), "<%s>", (*values)[i + v].value);
      itemAdjust = itemAdjust + valueLen;
    }
  } else if (type == 2){
    setColors(HILITE_PAIR);
    mvprintw(line, col, "<");
    if (settingsPos == index && settingsBinPos < 0){
      attron(A_REVERSE);
    }
    if ((*settings)[index].maxValue > 0) {
      mvprintw(line, col + 1, " ");
    } else {
      mvprintw(line, col + 1, "?");
    }
    attroff(A_REVERSE);
    mvprintw(line, col + 2, ">");
    setColors(COMMAND_PAIR);
    mvprintw(line, col + 4, "%ls:", (*settings)[index].textLabel);
    for(i = 0; i < ((*settings)[index].maxValue); i++){
      valueLen = strlen((*bins)[i + b].settingLabel) + 3;
      if ((*bins)[i + b].boolVal == 1){
        setColors(HILITE_PAIR);
      }
      if (settingsBinPos == (i + b) ){
        attron(A_REVERSE);
      }
      mvprintw(line, (col + 4 + labelLen + itemAdjust), "<%s>", (*bins)[i + b].settingLabel);
      attroff(A_REVERSE);
      setColors(COMMAND_PAIR);
      itemAdjust = itemAdjust + valueLen;
    }
  } else if (type == 3){
    // To Do: Add Free Text logic
    setColors(HILITE_PAIR);
    mvprintw(line, col, " ");
    if (settingsPos == index && settingsFreePos < 0){
      attron(A_REVERSE);
    }
    mvprintw(line, col + 1, "-");
    attroff(A_REVERSE);
    mvprintw(line, col + 2, ">");
    setColors(COMMAND_PAIR);
    mvprintw(line, col + 4, "%ls:", (*settings)[index].textLabel);
    setColors(HILITE_PAIR);
    move(line, (col + 4 + labelLen + itemAdjust));
    mvprintw(line, (col + 4 + labelLen + itemAdjust), "%s", (*settings)[index].charSetting); // To Do
    setColors(COMMAND_PAIR);
  }
}

int textValueLookup(t1CharValues **values, int *items, char *refLabel, char *value)
{
  int i;

  for (i = 0; i < *items; i++){
    if (!strcmp((*values)[i].value, value) && !strcmp((*values)[i].refLabel, refLabel)){
      return (*values)[i].index;
    }
  }

  return -1;
}
