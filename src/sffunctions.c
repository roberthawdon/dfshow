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
#include <locale.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <regex.h>
#include <wchar.h>
#include <libconfig.h>
#include <libintl.h>
#include "banned.h"
#include "config.h"
#include "colors.h"
#include "settings.h"
#include "common.h"
#include "menu.h"
#include "display.h"
#include "sfmenus.h"
#include "sffunctions.h"
#include "i18n.h"


int topline = 1;
int leftcol = 1;
int totallines = 0;
int longestline = 0;
int longestlongline = 0;

int tabsize = 8;

int sfScrollStep = 4;

int wrap = 0;
int wrapmode = LINE_WRAP;


FILE *stream;
char *line = NULL;
wchar_t *longline = NULL;
size_t len = 0;
ssize_t nread;
int count;
int top, left;
int lasttop;

char regexinput[1024];

long int *filePos;

settingSection *settingSectionsSf;
int settingSectionsSfCount = 0;
settingIndex *settingIndexSf;
t1CharValues *charValuesSf;
t2BinValues *binValuesSf;
int totalCharItemsSf;
int totalBinItemsSf;

int sfMenuItems = 0;

extern char fileName[4096];

extern int displaysize;
extern int displaycount;

extern int i, s;

extern int prevLine;
extern int wrapStartChar;

extern bool lineWrapped;

extern FILE *file;
extern int exitCode;

extern wchar_t *sfFileMenuLabel;
extern char themeName[256];
extern int enableCtrlC;
extern bool enableMouse;

extern int * pc;

extern char globalConfLocation[4096];
extern char homeConfLocation[4096];

extern int settingsPos;
extern int settingsBinPos;

extern menuDef *settingsMenu;
extern int settingsMenuSize;
extern wchar_t *settingsMenuLabel;

extern bool topMenu;
extern bool bottomMenu;
extern wchar_t *topMenuBuffer;
extern wchar_t *bottomMenuBuffer;

extern int viewMode;

void readSfConfig(const char * confFile)
{
  config_t cfg;
  config_setting_t *setting, *group;
  config_init(&cfg);
  if (config_read_file(&cfg, confFile)){
    // Deal with the globals first
    group = config_lookup(&cfg, "common");
    if (group){
      setting = config_setting_get_member(group, "theme");
      if (setting){
        if (!getenv("DFS_THEME_OVERRIDE")){
          snprintf(themeName, 256, "%s", config_setting_get_string(setting));
          setenv("DFS_THEME", themeName, 1);
        }
      }
      setting = config_setting_get_member(group, "sigint");
      if (setting){
        if (config_setting_get_int(setting)){
          enableCtrlC = 1;
        }
      }
      // Are we enabling mouse support?
      setting = config_setting_get_member(group, "enable-mouse");
      if (setting){
        if (config_setting_get_int(setting)){
          enableMouse = true;
        }
      }
    }
    // Now for program specific
    group = config_lookup(&cfg, "sf");
    if (group){
      // Check Wrap
      setting = config_setting_get_member(group, "wrap");
      if (setting){
        if (config_setting_get_int(setting)){
          wrap = 1;
        }
      }
      // Check scrollStep
      setting = config_setting_get_member(group, "scrollStep");
      if (setting){
        if (config_setting_get_string(setting)){
          sfScrollStep = strToInt(config_setting_get_string(setting));
        }
      }
    }
  }
}

void saveSfConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex)
{
  config_t cfg;
  config_setting_t *root, *setting, *group;
  int i;
  int storeType;

  config_init(&cfg);

  config_read_file(&cfg, confFile);
  root = config_root_setting(&cfg);

  // Global Settings
  group = config_setting_get_member(root, "common");

  if (!group){
    group = config_setting_add(root, "common", CONFIG_TYPE_GROUP);
  }

  for (i = 0; i < items; i++){
    if (!strcmp((*settings)[i].sectionRef, "global")){
      config_setting_remove(group, (*settings)[i].refLabel);
      storeType = (*settings)[i].storeType;
      if (storeType == SETTING_STORE_STRING){
        setting = config_setting_add(group, (*settings)[i].refLabel, CONFIG_TYPE_STRING);
      } else if (storeType == SETTING_STORE_GROUP){
        // Groups are handled by subgroups
      } else {
        setting = config_setting_add(group, (*settings)[i].refLabel, CONFIG_TYPE_INT);
      }
      if ((*settings)[i].type == SETTING_BOOL){

        if (!strcmp((*settings)[i].refLabel, "enable-mouse")){
          config_setting_set_int(setting, enableMouse);
        }
      }
    }
  }

  // Sf Settings
  group = config_setting_get_member(root, "sf");

  if (!group){
    group = config_setting_add(root, "sf", CONFIG_TYPE_GROUP);
  }

  for (i = 0; i < items; i++){
    if (strcmp((*settings)[i].sectionRef, "global")){
      config_setting_remove(group, (*settings)[i].refLabel);
      storeType = (*settings)[i].storeType;
      if (storeType == SETTING_STORE_STRING){
        setting = config_setting_add(group, (*settings)[i].refLabel, CONFIG_TYPE_STRING);
      } else if (storeType == SETTING_STORE_GROUP){
        // Groups are handled by subgroups
      } else {
        setting = config_setting_add(group, (*settings)[i].refLabel, CONFIG_TYPE_INT);
      }
      if ((*settings)[i].type == SETTING_BOOL){
        if (!strcmp((*settings)[i].refLabel, "wrap")){
          config_setting_set_int(setting, wrap);
        }
      } else if ((*settings)[i].type == SETTING_SELECT){
        if (!strcmp((*settings)[i].refLabel, "scrollStep")){
          config_setting_set_int(setting, sfScrollStep);
        }
      } else if ((*settings)[i].type == SETTING_MULTI){
        // None of those in SF (yet?)
      }
    }
  }

  config_write_file(&cfg, confFile);

  config_destroy(&cfg);

}

int generateSfSettingsVars()
{
  int items = 0;
  int charValuesCount = 0, scrollStepCount = 0;
  int binValuesCount = 0;

  settingSectionsSfCount = 0;

  addSettingSection(&settingSectionsSf, &settingSectionsSfCount, "global",       _("Global Settings"));
  addSettingSection(&settingSectionsSf, &settingSectionsSfCount, "behavior",     _("Behavior Settings"));

  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "1");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "2");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "3");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "4");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "5");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "6");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "7");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "8");
  addT1CharValue(&charValuesSf, &charValuesCount, &scrollStepCount, "scrollStep", "9");

  importSetting(&settingIndexSf, &items, "global",       "enable-mouse", _("Enable mouse (Requires restart)"), SETTING_BOOL, SETTING_STORE_INT, NULL, enableMouse, -1, 0);
  importSetting(&settingIndexSf, &items, "behavior",     "wrap",         _("Enable text wrapping"), SETTING_BOOL, SETTING_STORE_INT, NULL, wrap, -1, 0);
  importSetting(&settingIndexSf, &items, "behavior",     "scrollStep",   _("Mouse scroll interval size"), SETTING_SELECT, SETTING_STORE_INT, NULL, sfScrollStep - 1, scrollStepCount, 0);

  totalBinItemsSf = binValuesCount;
  totalCharItemsSf = charValuesCount;
  return items;
}

// void freeSettingVars()
// {
//   return;
// }

void refreshScreenSf()
{
  // unloadSfMenuLabels();
  // refreshSfMenuLabels();
  switch(viewMode)
    {
    case 3: // Settings View
      if (topMenuBuffer){
        wPrintMenu(0, 0, topMenuBuffer);
      }
      break;
    case 4: // SF View
      updateView();
      if (topMenuBuffer){
        wPrintMenu(0, 0, topMenuBuffer);
      }
      if (bottomMenuBuffer){
        wPrintMenu(LINES-1, 0, bottomMenuBuffer);
      }
      break;
    default: // Fallback
      if (topMenuBuffer){
        wPrintMenu(0, 0, topMenuBuffer);
      }
      if (bottomMenuBuffer){
        wPrintMenu(LINES-1, 0, bottomMenuBuffer);
      }
      break;
    }
}

int calculateTab(int pos)
{
  int currentpos;
  int result;

  // currentpos = pos + leftcol - 1;
  currentpos = pos;

  while (currentpos > tabsize){
    currentpos = currentpos - tabsize;
  }

  result = tabsize - currentpos;

  if (result <= 0){
    result = tabsize;
  }

  return(result);
}

int findInFile(const char * currentfile, const char * search, int charcase)
{
  regex_t regex;
  int reti;

  reti = regcomp(&regex, search, charcase);

  if (reti) {
    return(-1);
  }

  fseek(stream, filePos[top], SEEK_SET);
  top = 0;
  count = 0;

  if ( stream ) {
    while ((line = read_line(stream) )){
      count++;
      reti = regexec(&regex, line, 0, NULL, 0);
      if (!reti && count > topline) {
        regfree(&regex);
        return(count);
      }
    }
  }

  regfree(&regex);
  return (-2);

}

void fileShowStatus()
{
  char *statusText;
  char wrapStatus[2];
  if (lineWrapped && wrapStartChar != 0){
    wrapStatus[0] = '*';
  } else {
    wrapStatus[0] = '\0';
  }
  if (wrap){
    setDynamicChar(&statusText, _("File = <%s>  Top = <%i%s>"), fileName, topline, wrapStatus);
  } else {
    setDynamicChar(&statusText, _("File = <%s>  Top = <%i:%i>"), fileName, topline, leftcol);
  }
  printMenu(LINES - 1, 0, statusText);
  free(statusText);
}

int findLastLineStart(int longLineLen){
  int c = 0;
  int result = 0;

  while (c < longLineLen){
    result = c;
    c = c + COLS;
  }

  return(result);
}

int updateView()
{
  int longlinelen = 0;
  int startChar = 0;
  int wrapEnd = 0;
  top = topline;
  left = leftcol;
  len = 0;
  top--;
  left--;
  displaycount = 0;
  lineWrapped = false;

  displaysize = LINES - 2;

  clear_workspace();
  setColors(DISPLAY_PAIR);

  fseek(stream, filePos[top], SEEK_SET);
  top = 0;

  line = malloc(sizeof(char) + 1); // Preallocating memory appears to fix a crash on FreeBSD, it might also fix the same issue on macOS

  while ((nread = getline(&line, &len, stream)) != -1) {
    s = 0;
    mbstowcs(longline, line, len);
    longlinelen = wcslen(longline);
    if (displaycount < displaysize){
      if (displaycount == 0){
        if (topline < prevLine){
          wrapStartChar = startChar = findLastLineStart(longlinelen);
          // if (startChar != 0){
          //   lineWrapped = true;
          // }
        } else {
          startChar = wrapStartChar;
        }
      } else {
        startChar = 0;
      }
      if (startChar > 0){
        lineWrapped = true;
      }
      for(i = startChar; i < longlinelen; i++){
        mvprintw(displaycount + 1, s - left, "%lc", longline[i]);
        // This doesn't increase the max line.
        if (line[i] == '\t'){
          s = s + calculateTab(s);
        } else if (line[i] == '\r') {
          continue;
        } else {
          s = s + wcwidth(longline[i]);
        }
        if (longlinelen > COLS){
          if ((longlinelen - startChar) < COLS){
            wrapEnd = 1;
          }
        }
        if ( s == COLS + left){
          if (displaycount == 0){
            lineWrapped = true;
          }
          if ( wrap ) {
            if ( wrapmode != WORD_WRAP ){
              s = 0;
              displaycount++;
            }
          } else {
            break;
          }
        }
      }
      displaycount++;
    } else {
      break;
    }
  }
  attron(A_BOLD);
  mvprintw(displaycount + 1, 0, "*eof");
  attroff(A_BOLD);
  fileShowStatus();
  free(line);
  // free(longline);
  return(wrapEnd);
}

void loadFile(const char * currentfile)
{
  int longlinelen = 0;

  len = 0;
  longestline = 0;
  longestlongline = 0;
  viewMode = 4;
  totallines = 0;

  filePos = malloc(sizeof(long int) + 1); // Initial size of lookup
  filePos[0] = 0;

  stream = fopen(currentfile, "rb");
  if (stream == NULL) {

    return;
    }

  line = calloc(1, sizeof(char));
  longline = calloc(1, sizeof(wchar_t));

  while ((nread = getline(&line, &len, stream)) != -1) {
    totallines++;
    filePos = realloc(filePos, sizeof(long int) * (totallines + 1));
    filePos[totallines] = ftell(stream);
    if (nread > longestline){
      longestline = nread;
      longline = realloc(longline, sizeof(wchar_t) * (longestline + 1));
    }
    mbstowcs(longline, line, len);
    if (longline){
      longlinelen = wcslen(longline);
      if (longlinelen > longestlongline){
        longestlongline = longlinelen;
      }
    }
  }
  free(line);
  updateView();
}

void file_view(char * currentfile)
{
  char *notFoundMessage;

  topline = 1;

  clear();
  wPrintMenu(0, 0, sfFileMenuLabel);
  setColors(COMMAND_PAIR);

  refresh();

  if ( check_file(currentfile) && !check_dir(currentfile)){
    loadFile(currentfile);
    show_file_inputs();
  } else {
    setDynamicChar(&notFoundMessage, _("File [%s] does not exist"), currentfile);
    topLineMessage(notFoundMessage);
    free(notFoundMessage);
    exitCode = 1;
  }

  return;
}

void applySfSettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount)
{
  int i, j;
  for (i = 0; i < items; i++){
    if (!strcmp((*settings)[i].refLabel, "enable-mouse")){
      enableMouse = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "wrap")){
      wrap = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "scrollStep")){
      for (j = 0; j < valuesCount; j++){
        if (!strcmp((*values)[j].refLabel, "scrollStep") && ((*values)[j].index == (*settings)[i].intSetting)){
          sfScrollStep = strToInt((*values)[j].value);
        }
      }
    }
  }
}
