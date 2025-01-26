/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2025  Robert Ian Hawdon

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
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <libconfig.h>
#include <wchar.h>
#include <libintl.h>
#include <math.h>
#include "display.h"
#include "menu.h"
#include "settings.h"
#include "colors.h"
#include "input.h"
#include "common.h"
#include "banned.h"
#include "i18n.h"

int settingsPos = 0;
int settingsBinPos = -1;
int settingsFreePos = -1;

menuButton *settingButtons;

extern int * pc;

extern char globalConfLocation[4096];
extern char homeConfLocation[4096];

extern int viewMode;

extern MEVENT event;

void updateSetting(settingIndex **settings, int index, int type, int intSetting)
{
  // To-Do, do a verification on the type

  (*settings)[index].intSetting = intSetting;
}

void readShowConfig(const char * confFile);
void readSfConfig(const char * confFile);

int generateShowSettingsVars();
int generateSfSettingsVars();

void applyShowSettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount);
void applySfSettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount);

void saveShowConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex);
void saveSfConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex);

void settingsAction(char *action, char *application, wchar_t *settingsMenuLabel, settingIndex **settings, t1CharValues **charValues, t2BinValues **binValues, int totalCharItems, int totalBinItems, int totalItems, const char * confFile)
{

  if ( !strcmp(action, "generate" ) ){
    #ifdef APPLICATION_SHOW
      if (!strcmp(application, "show")) {
        generateShowSettingsVars();
      }
    #endif
    #ifdef APPLICATION_SF
      if (!strcmp(application, "sf")) {
        generateSfSettingsVars();
      }
    #endif
  } else if ( !strcmp(action, "apply" ) ) {
    #ifdef APPLICATION_SHOW
      if (!strcmp(application, "show")) {
        applyShowSettings(settings, charValues, totalItems, totalCharItems);
      }
    #endif
    #ifdef APPLICATION_SF
      if (!strcmp(application, "sf")) {
        applySfSettings(settings, charValues, totalItems, totalCharItems);
      }
    #endif
  } else if ( !strcmp(action, "save" ) ) {
    #ifdef APPLICATION_SHOW
      if (!strcmp(application, "show")) {
        saveShowConfig(confFile, settings, charValues, binValues, totalItems, totalCharItems, totalBinItems);
      }
    #endif
    #ifdef APPLICATION_SF
      if (!strcmp(application, "sf")) {
        saveSfConfig(confFile, settings, charValues, binValues, totalItems, totalCharItems, totalBinItems);
      }
    #endif
  } else if ( !strcmp(action, "read" ) ) {
    #ifdef APPLICATION_SHOW
      if (!strcmp(application, "show") || !strcmp(application, "all")) {
        readShowConfig(confFile);
      }
    #endif
    #ifdef APPLICATION_SF
      if (!strcmp(application, "sf") || !strcmp(application, "all")) {
        readSfConfig(confFile);
      }
    #endif
  }
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

void importSetting(settingIndex **settings, int *items, char *refLabel, char *textLabel, int type, int storeType, char *charSetting, int intSetting, int maxValue, int invert)
{
  settingIndex *tmp;
  int currentItem = *items;
  wchar_t *wideTextLabel;

  wideTextLabel = malloc(sizeof(wchar_t) * (strlen(textLabel) + 1));
  mbstowcs(wideTextLabel, textLabel, (strlen(textLabel) + 1));

  if (*items == 0){
    tmp = malloc(sizeof(settingIndex) * 2);
  } else {
    tmp = realloc(*settings, (currentItem + 1) * (sizeof(settingIndex) + 1));
  }

  if (tmp){
    *settings = tmp;
  }

  (*settings)[currentItem].type = type;
  (*settings)[currentItem].storeType = storeType;
  snprintf((*settings)[currentItem].refLabel, 16, "%s", refLabel);
  swprintf((*settings)[currentItem].textLabel, 64, L"%ls", wideTextLabel);
  free(wideTextLabel);
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

void adjustBinSetting(settingIndex **settings, t2BinValues **values, char *refLabel, int maxValue)
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

int settingButtonAction(const char * refLabel, settingIndex **settings, int menuItems){
  int output = -1;
  int newPos = -1;
  int oldPos = -1;
  int i;

  // endwin();
  oldPos = settingsPos;

  for (i = 0; i < menuItems; i++){
    // printf("%i of %i: %s (target: %s)\n", i, menuItems, (*settings)[i].refLabel, refLabel);
    if (!strcmp((*settings)[i].refLabel, refLabel)){
        newPos = i;
        if (newPos == oldPos){
          switch((*settings)[i].type){
            case SETTING_BOOL:
              output = 32;
              break;
            case SETTING_SELECT:
              output = 261;
              break;
            case SETTING_FREE:
              output = 32;
              break;
            default:
              break;
          }
        } else {
          settingsPos = newPos;
        }
        return output;
      }
  }

  return output;
}

void settingsMenuView(wchar_t *settingsMenuLabel, int settingsMenuSize, menuDef *settingsMenu, menuButton *settingsMenuButtons, settingIndex **settings, t1CharValues **charValues, t2BinValues **binValues, int totalCharItems, int totalBinItems, int totalItems, char *application)
{
  viewMode = 3;
  int count = 0;
  int x = 2;
  int y = 3;
  int markedCount, sortmodeCount, timestyleCount, ownerCount;
  int sortmodeInt, timestyleInt;
  int e;
  int b;
  char charTempValue[1024];

  clear();
  wPrintMenu(0,0,settingsMenuLabel);

  if (settingButtons){
    free(settingButtons);
  }

  settingButtons = malloc(sizeof(menuButton) * totalItems);

  while(1)
    {
      for (count = 0; count < totalItems; count++){
      // printSetting(2 + count, 3, settings, charValues, binValues, count, totalCharItems, totalBinItems, settings[count]->type, settings[count]->invert);
      printSetting(2 + count, 3, settings, charValues, binValues, count, totalCharItems, totalBinItems, (*settings)[count].type, (*settings)[count].invert);
      b = wcslen((*settings)[count].textLabel);
      snprintf(settingButtons[count].refLabel, 16, "%s", (*settings)[count].refLabel);
      settingButtons[count].topX = 3;
      settingButtons[count].bottomX = 7 + b; // Including check mark
      settingButtons[count].topY = settingButtons[count].bottomY = count + 2;
      }

      move(x + settingsPos, y + 1);
      *pc = getch10th();
      loop:
      if (getmouse(&event) == OK) {
        if (event.bstate & BUTTON1_PRESSED){
          if (event.y == 0){
            // Setting key based on click
            *pc = menuHotkeyLookup(settingsMenu, (menuButtonLookup(settingsMenuButtons, settingsMenuSize, event.x, event.y, 0, 0, true)), settingsMenuSize);
            goto loop;
          } else {
            // To Do
            *pc = settingButtonAction(menuButtonLookup(settingButtons, totalItems, event.x, event.y, 0, 0, false), settings, totalItems);
            goto loop;
          }
        } else if (event.bstate & BUTTON5_PRESSED){
          goto down;
        } else if (event.bstate & BUTTON4_PRESSED){
          goto up;
        }
      } else if (*pc == menuHotkeyLookup(settingsMenu, "s_quit", settingsMenuSize)){
        curs_set(FALSE);
        settingsAction("apply", application, NULL, settings, charValues, NULL, totalCharItems, 0, totalItems, NULL);
        // free(settings);
        return;
      } else if (*pc == menuHotkeyLookup(settingsMenu, "s_revert", settingsMenuSize)){
        // free(settings);
        settingsAction("read", application, NULL, NULL, NULL, NULL, 0, 0, 0, globalConfLocation);
        settingsAction("read", application, NULL, NULL, NULL, NULL, 0, 0, 0, homeConfLocation);
        settingsAction("generate", application, NULL, NULL, NULL, NULL, 0, 0, 0, NULL);
      } else if (*pc == menuHotkeyLookup(settingsMenu, "s_save", settingsMenuSize)){
        settingsAction("apply", application, NULL, settings, charValues, NULL, totalCharItems, 0, totalItems, NULL);
        if (access(dirFromPath(homeConfLocation), W_OK) != 0) {
          createParentDirs(homeConfLocation);
        }
        settingsAction("save", application, NULL, settings, charValues, binValues, totalCharItems, totalBinItems, totalItems, homeConfLocation);
        curs_set(FALSE);
        topLineMessage(_("Settings saved."));
        curs_set(TRUE);
        wPrintMenu(0,0,settingsMenuLabel);
      } else if (*pc == 258 || *pc == 10){
        down:
        if (settingsPos < (totalItems -1 )){
          settingsBinPos = -1;
          settingsPos++;
        } else {
          settingsPos = totalItems - 1;
        }
      } else if (*pc == 32 || *pc == 260 || *pc == 261){
        // Adjust
        if ((*settings)[settingsPos].type == SETTING_BOOL){
          if ((*settings)[settingsPos].intSetting > 0){
            updateSetting(settings, settingsPos, SETTING_BOOL, 0);
          } else {
            updateSetting(settings, settingsPos, SETTING_BOOL, 1);
          }
        } else if ((*settings)[settingsPos].type == SETTING_SELECT){
          if (*pc == 32 || *pc == 261){
            if ((*settings)[settingsPos].intSetting < ((*settings)[settingsPos].maxValue) - 1){
              updateSetting(settings, settingsPos, SETTING_SELECT, ((*settings)[settingsPos].intSetting) + 1);
            } else {
              updateSetting(settings, settingsPos, SETTING_SELECT, 0);
            }
          } else {
            if ((*settings)[settingsPos].intSetting > 0){
              updateSetting(settings, settingsPos, SETTING_SELECT, ((*settings)[settingsPos].intSetting) - 1);
            } else {
              updateSetting(settings, settingsPos, SETTING_SELECT, ((*settings)[settingsPos].maxValue - 1));
            }
          }
        } else if ((*settings)[settingsPos].type == SETTING_MULTI){
          if (*pc == 261 && (settingsBinPos < ((*settings)[settingsPos].maxValue -1))){
            settingsBinPos++;
          } else if (*pc == 260 && (settingsBinPos > -1)){
            settingsBinPos--;
          } else if (*pc == 32 && (settingsBinPos > -1)){
            // Not fond of this, but it should work
            if (!strcmp((*settings)[settingsPos].refLabel, "owner")){
              adjustBinSetting(settings, binValues, "owner", totalBinItems);
            }
          }
        } else if ((*settings)[settingsPos].type == SETTING_FREE){
          if (*pc == 32 || *pc == 261) {
            settingsFreePos = 0;
            move(x + settingsPos, y + wcslen((*settings)[settingsPos].textLabel) + 6);
            e = readline(charTempValue, 1024, (*settings)[settingsPos].charSetting);
            if (strcmp(charTempValue, "")){
              free((*settings)[settingsPos].charSetting);
              (*settings)[settingsPos].charSetting = malloc(sizeof(char) * (strlen(charTempValue) + 1));
              snprintf((*settings)[settingsPos].charSetting, (strlen(charTempValue) + 1), "%s", charTempValue);
            }
            move(x + settingsPos, 0);
            clrtoeol();
            settingsFreePos = -1;
          }
        }
      } else if (*pc == 259){
        up:
        if (settingsPos > 0){
          settingsBinPos = -1;
          settingsPos--;
        } else {
          settingsPos = 0;
        }
      }
    }

}


