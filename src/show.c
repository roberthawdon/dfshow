/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2023  Robert Ian Hawdon

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
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <libconfig.h>
#include <ctype.h>
#include <math.h>
#include <libintl.h>
#include "config.h"
#include "showfunctions.h"
#include "showmenus.h"
#include "sfmenus.h"
#include "colors.h"
#include "menu.h"
#include "display.h"
#include "settings.h"
#include "common.h"
#include "show.h"
#include "input.h"
#include "banned.h"
#include "i18n.h"

char * visualPath;
char * pagerPath;

char currentpwd[4096];

int resized = 0;

char sortmode[9] = "name";
char timestyle[9] = "locale";
int reverse = 0;
int human = 0;
int si = 0;
int ogavis = 3;
int ogapad = 1;
int showbackup = 1;
int colormode = 0;
int danger = 0;
int invalidstart = 0;
int filecolors = 0;
int markedinfo = 0;
int useEnvPager = 0;
int launchThemeEditor = 0;
int launchSettingsMenu = 0;
int oneLine = 0;
int skipToFirstFile = 0;
bool currentDirOnly = false;
bool useDefinedEditor = 0;
bool useDefinedPager = 0;

bool dirOnly = 0;
bool scaleSize = 0;

int plugins = 0; // Not yet implemented

int enterAsShow = 0;

int messageBreak = 0;

int showProcesses;

int showContext = 0;

int showSizeBlocks = 0;

int showXAttrs = 0;

int showAcls = 0; // Might end up not implementing this.

bool showInodes = false;

bool numericIds = false;

#ifdef HAVE_GNU_BLOCKSIZE
int block_size = 1024;
#else
int block_size = 512;
#endif

char *objectWild;

char block_unit[4] = "\0\0\0\0";

results *ob;

int segOrder[10] = {COL_MARK, COL_INODE, COL_SIZEBLOCKS, COL_ATTR, COL_HLINK, COL_OWNER, COL_CONTEXT, COL_SIZE, COL_DATE, COL_NAME};
// int segOrder[10] = {COL_MARK, COL_NAME, COL_SIZE, COL_DATE, COL_ATTR}; // Emulating NET-DF-EDIT's XENIX layout

settingIndex *settingIndexShow;
t1CharValues *charValuesShow;
t2BinValues *binValuesShow;
int totalCharItemsShow;
int totalBinItemsShow;

extern int viewMode;

extern int skippable;

extern int settingsPos;
extern int settingsBinPos;
extern int settingsFreePos;

extern menuDef *showSettingsMenu;
extern int showSettingsMenuSize;
extern wchar_t *showSettingsMenuLabel;

extern int * pc;

extern history *hs;
extern int topfileref;
extern int lineStart;
extern int selected;
extern int totalfilecount;
extern int showhidden;

extern char globalConfLocation[4096];
extern char homeConfLocation[4096];

extern char themeName[256];

struct sigaction sa;

extern int exitCode;
extern int enableCtrlC;

extern wchar_t *globalMenuLabel;
extern wchar_t *showFileMenuLabel;
extern wchar_t *functionMenuLabel;
extern wchar_t *modifyMenuLabel;
extern wchar_t *sortMenuLabel;
extern wchar_t *linkMenuLabel;

extern xattrList *xa;
extern int xattrPos;

extern bool topMenu;
extern bool bottomMenu;
extern wchar_t *topMenuBuffer;
extern wchar_t *bottomMenuBuffer;

int setMarked(char* markedinput);
int checkStyle(char* styleinput);

void readShowConfig(const char * confFile)
{
  config_t cfg;
  config_setting_t *setting, *group, *array;
  char markedParam[8];
  int i;
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
    }
    // Now for program specific
    group = config_lookup(&cfg, PROGRAM_NAME);
    if (group){
      // Check File Colour
      setting = config_setting_get_member(group, "color");
      if (setting){
        if (config_setting_get_int(setting)){
          filecolors = 1;
        }
      }
      // Check Marked
      setting = config_setting_get_member(group, "marked");
      if (setting){
        snprintf(markedParam, 8, "%s", config_setting_get_string(setting));
        setMarked(markedParam);
      }
      // Check Sort
      setting = config_setting_get_member(group, "sortmode");
      if (setting){
        snprintf(sortmode, 9, "%s", config_setting_get_string(setting));
      }
      // Check Reverse
      setting = config_setting_get_member(group, "reverse");
      if (setting){
        if (config_setting_get_int(setting)){
          reverse = 1;
        }
      }
      // Check Timestyle
      setting = config_setting_get_member(group, "timestyle");
      if (setting){
        snprintf(timestyle, 9, "%s", config_setting_get_string(setting));
        if(!checkStyle(timestyle)){
          snprintf(timestyle, 9, "locale");
        }
      }
      // Check Hidden
      setting = config_setting_get_member(group, "hidden");
      if (setting){
        if (config_setting_get_int(setting)){
          showhidden = 1;
        }
      }
      // Check Ignore Backups
      setting = config_setting_get_member(group, "ignore-backups");
      if (setting){
        if (config_setting_get_int(setting)){
          showbackup = 0;
        }
      }
      // Check No SF
      setting = config_setting_get_member(group, "no-sf");
      if (setting){
        if (config_setting_get_int(setting)){
          useEnvPager = 1;
        }
      }
      // Check No Danger
      setting = config_setting_get_member(group, "no-danger");
      if (setting){
        if (config_setting_get_int(setting)){
          danger = 0;
        }
      }
      // Check SI
      setting = config_setting_get_member(group, "si");
      if (setting){
        if (config_setting_get_int(setting)){
          si = 1;
        }
      }
      // Check Human Readable
      setting = config_setting_get_member(group, "human-readable");
      if (setting){
        if (config_setting_get_int(setting)){
          human = 1;
        }
      }
      // Check Show Inodes
      setting = config_setting_get_member(group, "showInodes");
      if (setting){
        if (config_setting_get_int(setting)){
          showInodes = true;
        }
      }
      // Check Numeric Ids
      setting = config_setting_get_member(group, "numericIds");
      if (setting){
        if (config_setting_get_int(setting)){
          numericIds = true;
        }
      }
      // Check Enter As Show
      setting = config_setting_get_member(group, "show-on-enter");
      if (setting){
        if (config_setting_get_int(setting)){
          enterAsShow = 1;
        }
      }
      // Check Security Context
      setting = config_setting_get_member(group, "context");
      if (setting){
        if (config_setting_get_int(setting)){
          showContext = 1;
        }
      }
      // Check Skip To First Object
      setting = config_setting_get_member(group, "skip-to-first");
      if (setting){
        if (config_setting_get_int(setting)){
          skipToFirstFile = 1;
        }
      }
      // Check Showing XAttrs
      setting = config_setting_get_member(group, "showXAttrs");
      if (setting){
        if (config_setting_get_int(setting)){
          showXAttrs = 1;
        }
      }
      // Check Showing Only Current Directory
      setting = config_setting_get_member(group, "directory");
      if (setting){
        if (config_setting_get_int(setting)){
          currentDirOnly = 1;
        }
      }
      // Check Showing Only Directories
      setting = config_setting_get_member(group, "only-dirs");
      if (setting){
        if (config_setting_get_int(setting)){
          dirOnly = 1;
        }
      }
      // Check Show Size In Blocks
      setting = config_setting_get_member(group, "sizeblocks");
      if (setting){
        if (config_setting_get_int(setting)){
          dirOnly = 1;
        }
      }
      // Check use defined editor
      setting = config_setting_get_member(group, "defined-editor");
      if (setting){
        if (config_setting_get_int(setting)){
          useDefinedEditor = 1;
        }
      }
      // Check visualPath
      setting = config_setting_get_member(group, "visualPath");
      if (setting){
        if (visualPath){
          free(visualPath);
        }
        visualPath = calloc(strlen(config_setting_get_string(setting)) + 1, sizeof(char));
        snprintf(visualPath, (strlen(config_setting_get_string(setting)) + 1), "%s", config_setting_get_string(setting));
      }
      // Check use defined pager
      setting = config_setting_get_member(group, "defined-pager");
      if (setting){
        if (config_setting_get_int(setting)){
          useDefinedPager = 1;
        }
      }
      // Check pagerPath
      setting = config_setting_get_member(group, "pagerPath");
      if (setting){
        if (pagerPath){
          free(pagerPath);
        }
        pagerPath = calloc(strlen(config_setting_get_string(setting)) + 1, sizeof(char));
        snprintf(pagerPath, (strlen(config_setting_get_string(setting)) + 1), "%s", config_setting_get_string(setting));
      }
      // Check Layout
      array = config_setting_get_member(group, "layout");
      if (array){
        for (i = 0; i < 8; i++){
          if (config_setting_get_elem(array, i) != NULL){
            segOrder[i] = config_setting_get_int_elem(array, i);
          } else {
            segOrder[i] = -1;
          }
        }
      }
    }
    // Check owner column
    group = config_lookup(&cfg, "show.owner");
    if (group){
      ogavis = 0;
      setting = config_setting_get_member(group, "owner");
      if (setting){
        if (config_setting_get_int(setting)){
          ogavis = ogavis + 1;
        }
      }
      setting = config_setting_get_member(group, "group");
      if (setting){
        if (config_setting_get_int(setting)){
          ogavis = ogavis + 2;
        }
      }
      setting = config_setting_get_member(group, "author");
      if (setting){
        if (config_setting_get_int(setting)){
          ogavis = ogavis + 4;
        }
      }
    }
  };
  config_destroy(&cfg);
}

void saveShowConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex)
{
  config_t cfg;
  config_setting_t *root, *setting, *group, *subgroup;
  int i, v;

  config_init(&cfg);

  config_read_file(&cfg, confFile);
  root = config_root_setting(&cfg);

  group = config_setting_get_member(root, PROGRAM_NAME);

  if (!group){
    group = config_setting_add(root, PROGRAM_NAME, CONFIG_TYPE_GROUP);
  }

  for (i = 0; i < items; i++){
    config_setting_remove(group, (*settings)[i].refLabel);
    if ((*settings)[i].type == SETTING_BOOL){
      setting = config_setting_add(group, (*settings)[i].refLabel, CONFIG_TYPE_INT);

      if (!strcmp((*settings)[i].refLabel, "color")){
        config_setting_set_int(setting, filecolors);
      } else if (!strcmp((*settings)[i].refLabel, "reverse")){
        config_setting_set_int(setting, reverse);
      } else if (!strcmp((*settings)[i].refLabel, "hidden")){
        config_setting_set_int(setting, showhidden);
      } else if (!strcmp((*settings)[i].refLabel, "ignore-backups")){
        config_setting_set_int(setting, !showbackup);
      } else if (!strcmp((*settings)[i].refLabel, "no-sf")){
        config_setting_set_int(setting, useEnvPager);
      } else if (!strcmp((*settings)[i].refLabel, "no-danger")){
        config_setting_set_int(setting, !danger);
      } else if (!strcmp((*settings)[i].refLabel, "si")){
        config_setting_set_int(setting, si);
      } else if (!strcmp((*settings)[i].refLabel, "human-readable")){
        config_setting_set_int(setting, human);
      } else if (!strcmp((*settings)[i].refLabel, "showInodes")){
        config_setting_set_int(setting, showInodes);
      } else if (!strcmp((*settings)[i].refLabel, "numericIds")){
        config_setting_set_int(setting, numericIds);
      } else if (!strcmp((*settings)[i].refLabel, "show-on-enter")){
        config_setting_set_int(setting, enterAsShow);
      } else if (!strcmp((*settings)[i].refLabel, "context")){
        config_setting_set_int(setting, showContext);
      } else if (!strcmp((*settings)[i].refLabel, "skip-to-first")){
        config_setting_set_int(setting, skipToFirstFile);
      } else if (!strcmp((*settings)[i].refLabel, "showXAttrs")){
        config_setting_set_int(setting, showXAttrs);
      } else if (!strcmp((*settings)[i].refLabel, "directory")){
        config_setting_set_int(setting, currentDirOnly);
      } else if (!strcmp((*settings)[i].refLabel, "only-dirs")){
        config_setting_set_int(setting, dirOnly);
      } else if (!strcmp((*settings)[i].refLabel, "sizeblocks")){
        config_setting_set_int(setting, showSizeBlocks);
      } else if (!strcmp((*settings)[i].refLabel, "defined-editor")){
        config_setting_set_int(setting, useDefinedEditor);
      } else if (!strcmp((*settings)[i].refLabel, "defined-pager")){
        config_setting_set_int(setting, useDefinedPager);
      }
    } else if ((*settings)[i].type == SETTING_SELECT){
      //
      setting = config_setting_add(group, (*settings)[i].refLabel, CONFIG_TYPE_STRING);
      if (!strcmp((*settings)[i].refLabel, "marked")){
        for(v = 0; v < charIndex; v++){
          if (!strcmp((*values)[v].refLabel, "marked") && ((*settings)[i].intSetting == (*values)[v].index)){
            config_setting_set_string(setting, (*values)[v].value);
          }
        }
      } else if (!strcmp((*settings)[i].refLabel, "sortmode")){
        config_setting_set_string(setting, sortmode);
      } else if (!strcmp((*settings)[i].refLabel, "timestyle")){
        config_setting_set_string(setting, timestyle);
      }
    } else if ((*settings)[i].type == SETTING_MULTI){
      if (!strcmp((*settings)[i].refLabel, "owner")){
        subgroup = config_setting_add(group, "owner", CONFIG_TYPE_GROUP);
        for (v = 0; v < binIndex; v++){
          setting = config_setting_add(subgroup, (*bins)[v].settingLabel, CONFIG_TYPE_INT);
          config_setting_set_int(setting, (*bins)[v].boolVal);
        }
      }
    } else if ((*settings)[i].type == SETTING_FREE){
      setting = config_setting_add(group, (*settings)[i].refLabel, CONFIG_TYPE_STRING);
      if (!strcmp((*settings)[i].refLabel, "visualPath")){
        config_setting_set_string(setting, (*settings)[i].charSetting);
      } else if (!strcmp((*settings)[i].refLabel, "pagerPath")){
        config_setting_set_string(setting, (*settings)[i].charSetting);
      }
    }
  }

  config_write_file(&cfg, confFile);

  config_destroy(&cfg);
}

void applyShowSettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount)
{
  int i, j;
  for (i = 0; i < items; i++){
    if (!strcmp((*settings)[i].refLabel, "color")){
      filecolors = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "reverse")){
      reverse = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "hidden")){
      showhidden = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "ignore-backups")){
      showbackup = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "no-sf")){
      useEnvPager = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "no-danger")){
      danger = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "si")){
      si = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "human-readable")){
      human = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "showInodes")){
      showInodes = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "numericIds")){
      numericIds = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "show-on-enter")){
      enterAsShow = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "marked")){
      markedinfo = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "context")){
      showContext = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "skip-to-first")){
      skipToFirstFile = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "sortmode")){
      for (j = 0; j < valuesCount; j++){
        if (!strcmp((*values)[j].refLabel, "sortmode") && ((*values)[j].index == (*settings)[i].intSetting)){
          snprintf(sortmode, 9, "%s", (*values)[j].value);
        }
      }
    } else if (!strcmp((*settings)[i].refLabel, "timestyle")){
      for (j = 0; j < valuesCount; j++){
        if (!strcmp((*values)[j].refLabel, "timestyle") && ((*values)[j].index == (*settings)[i].intSetting)){
          snprintf(timestyle, 9, "%s", (*values)[j].value);
        }
      }
    } else if (!strcmp((*settings)[i].refLabel, "owner")){
      ogavis = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "showXAttrs")){
      showXAttrs = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "directory")){
      currentDirOnly = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "only-dirs")){
      dirOnly = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "sizeblocks")){
      showSizeBlocks = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "defined-editor")){
      useDefinedEditor = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "visualPath")){
      free(visualPath);
      visualPath = calloc((strlen((*settings)[i].charSetting) + 1), sizeof(char));
      snprintf(visualPath, (strlen((*settings)[i].charSetting) + 1), "%s", (*settings)[i].charSetting);
    } else if (!strcmp((*settings)[i].refLabel, "defined-pager")){
      useDefinedPager = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "pagerPath")){
      free(pagerPath);
      pagerPath = calloc((strlen((*settings)[i].charSetting) + 1), sizeof(char));
      snprintf(pagerPath, (strlen((*settings)[i].charSetting) + 1), "%s", (*settings)[i].charSetting);
    }
  }
}

int generateShowSettingsVars()
{
  uid_t uid=getuid(), euid=geteuid();
  int items = 0;
  int markedCount = 0, sortmodeCount = 0, timestyleCount = 0, ownerCount = 0;
  int sortmodeInt = 0, timestyleInt = 0;
  int charValuesCount = 0;
  int binValuesCount = 0;

  addT1CharValue(&charValuesShow, &charValuesCount, &markedCount, "marked", _("never"));
  addT1CharValue(&charValuesShow, &charValuesCount, &markedCount, "marked", _("always"));
  addT1CharValue(&charValuesShow, &charValuesCount, &markedCount, "marked", _("auto"));

  addT1CharValue(&charValuesShow, &charValuesCount, &sortmodeCount, "sortmode", _("name"));
  addT1CharValue(&charValuesShow, &charValuesCount, &sortmodeCount, "sortmode", _("date"));
  addT1CharValue(&charValuesShow, &charValuesCount, &sortmodeCount, "sortmode", _("size"));
  addT1CharValue(&charValuesShow, &charValuesCount, &sortmodeCount, "sortmode", _("unsorted"));

  addT1CharValue(&charValuesShow, &charValuesCount, &timestyleCount, "timestyle", _("locale"));
  addT1CharValue(&charValuesShow, &charValuesCount, &timestyleCount, "timestyle", _("iso"));
  addT1CharValue(&charValuesShow, &charValuesCount, &timestyleCount, "timestyle", _("long-iso"));
  addT1CharValue(&charValuesShow, &charValuesCount, &timestyleCount, "timestyle", _("full-iso"));

  addT2BinValue(&binValuesShow, &binValuesCount, &ownerCount, "owner", "owner", 1);
  addT2BinValue(&binValuesShow, &binValuesCount, &ownerCount, "owner", "group", 0);
  addT2BinValue(&binValuesShow, &binValuesCount, &ownerCount, "owner", "author", 0);

  sortmodeInt = textValueLookup(&charValuesShow, &charValuesCount, "sortmode", sortmode);
  timestyleInt = textValueLookup(&charValuesShow, &charValuesCount, "timestyle", timestyle);

  importSetting(&settingIndexShow, &items, "color",          _("Display file colors"), SETTING_BOOL, NULL, filecolors, -1, 0);
  importSetting(&settingIndexShow, &items, "marked",         _("Show marked file info"), SETTING_SELECT, NULL, markedinfo, markedCount, 0);
  importSetting(&settingIndexShow, &items, "sortmode",       _("Sorting mode"), SETTING_SELECT, NULL, sortmodeInt, sortmodeCount, 0);
  importSetting(&settingIndexShow, &items, "reverse",        _("Reverse sorting order"), SETTING_BOOL, NULL, reverse, -1, 0);
  importSetting(&settingIndexShow, &items, "timestyle",      _("Time style"), SETTING_SELECT, NULL, timestyleInt, timestyleCount, 0);
  importSetting(&settingIndexShow, &items, "hidden",         _("Show hidden files"), SETTING_BOOL, NULL, showhidden, -1, 0);
  importSetting(&settingIndexShow, &items, "ignore-backups", _("Hide backup files"), SETTING_BOOL, NULL, showbackup, -1, 1);
  importSetting(&settingIndexShow, &items, "no-sf",          _("Use 3rd party pager over SF"), SETTING_BOOL, NULL, useEnvPager, -1, 0);
  if (uid == 0 || euid == 0){
    importSetting(&settingIndexShow, &items, "no-danger",      _("Hide danger lines as root"), SETTING_BOOL, NULL, danger, -1, 1);
  }
  importSetting(&settingIndexShow, &items, "si",             _("Use SI units"), SETTING_BOOL, NULL, si, -1, 0);
  importSetting(&settingIndexShow, &items, "human-readable", _("Human readable sizes"), SETTING_BOOL, NULL, human, -1, 0);
  importSetting(&settingIndexShow, &items, "showInodes",     _("Show Inode"), SETTING_BOOL, NULL, showInodes, -1, 0);
  importSetting(&settingIndexShow, &items, "numericIds",     _("Use numeric UID and GIDs"), SETTING_BOOL, NULL, numericIds, -1, 0);
  importSetting(&settingIndexShow, &items, "show-on-enter",  _("Enter key acts like Show"), SETTING_BOOL, NULL, enterAsShow, -1, 0);
  importSetting(&settingIndexShow, &items, "owner",          _("Owner Column"), SETTING_MULTI, NULL, ogavis, ownerCount, 0);
  importSetting(&settingIndexShow, &items, "context",        _("Show security context of files"), SETTING_BOOL, NULL, showContext, -1, 0);
  importSetting(&settingIndexShow, &items, "skip-to-first",  _("Skip to the first object"), SETTING_BOOL, NULL, skipToFirstFile, -1, 0);
#ifdef HAVE_ACL_TYPE_EXTENDED
  importSetting(&settingIndexShow, &items, "showXAttrs",     _("Display extended attribute keys and sizes"), SETTING_BOOL, NULL, showXAttrs, -1, 0);
#endif
  importSetting(&settingIndexShow, &items, "directory",      _("Display only current directory"), SETTING_BOOL, NULL, currentDirOnly, -1, 0);
  importSetting(&settingIndexShow, &items, "only-dirs",      _("Display only directories"), SETTING_BOOL, NULL, dirOnly, -1, 0);
  importSetting(&settingIndexShow, &items, "sizeblocks",     _("Show allocated size in blocks"), SETTING_BOOL, NULL, showSizeBlocks, -1, 0);
  importSetting(&settingIndexShow, &items, "defined-editor", _("Override default editor"), SETTING_BOOL, NULL, useDefinedEditor, -1, 0);
  importSetting(&settingIndexShow, &items, "visualPath",     _("Editor utility program command"), SETTING_FREE, visualPath, -1, -1, 0);
  importSetting(&settingIndexShow, &items, "defined-pager",  _("Override default pager"), SETTING_BOOL, NULL, useDefinedPager, -1, 0);
  importSetting(&settingIndexShow, &items, "pagerPath",      _("Pager utility program command"), SETTING_FREE, pagerPath, -1, -1, 0);

  populateBool(&binValuesShow, "owner", ogavis, binValuesCount);

  totalBinItemsShow = binValuesCount;
  totalCharItemsShow = charValuesCount;
  return items;
}

int directory_view(char * currentpwd)
{
  objectWild = objectFromPath(currentpwd);
  if ( strchr(objectWild, MULTICHAR) || strchr(objectWild, ONECHAR)){
    snprintf(currentpwd, strlen(dirFromPath(currentpwd)), "%s", dirFromPath(currentpwd));
  } else {
    objectWild[0]=0;
  }

  lineStart = 0;
  clear();
  setColors(COMMAND_PAIR);


  wPrintMenu(0, 0, showFileMenuLabel);

  set_history(currentpwd, "", "", 0, 0);
  freeResults(ob, totalfilecount);
  freeXAttrs(xa, xattrPos);
  ob = get_dir(currentpwd);
  reorder_ob(ob, sortmode);
  generateEntryLineIndex(ob);

  if (skipToFirstFile == 1 && skippable == 1){
    selected = 2;
  } else {
    selected = 0;
  }

  display_dir(currentpwd, ob);

  refresh();

  directory_view_menu_inputs();

  freeResults(ob, totalfilecount); //freeing memory
  freeXAttrs(xa, xattrPos);
  return 0;
}

int global_menu()
{
  clear();
  refresh();
  global_menu_inputs();
  return 0;
}

int setMarked(char* markedinput)
{
  int status = -1;
  if (!strcmp(markedinput, "always")){
    markedinfo = 1;
    status = 0;
  } else if (!strcmp(markedinput, "never")){
    markedinfo = 0;
    status = 0;
  } else if (!strcmp(markedinput, "auto")){
    markedinfo = 2;
    status = 0;
  }
  return(status);
}

int checkStyle(char* styleinput)
{
  if ( (strcmp(styleinput, "long-iso") && (strcmp(styleinput, "full-iso") && strcmp(styleinput, "iso") && strcmp(styleinput, "locale") ))){
      return 0;
    } else {
      return 1;
    }
}

int setColor(char* colorinput)
{
  if (!strcmp(colorinput, "always")){
    filecolors = 1;
    return 1;
  } else if (!strcmp(colorinput, "never")){
    filecolors = 0;
    return 1;
  } else {
    return 0;
  }
}

int setBlockSize(const char * arg){
    size_t numarg;
    int returnCode = 0;
    int multiplier = 1;
    int power = 1;
    int powerUnit = 1024;

    int i1 = 1;
    signed char c1;
    signed char e1 = 0;
    signed char e2 = 0;

    if ( isdigit(arg[0]) ){
      sscanf(arg, "%d%1c%1c%1c", &i1, &c1, &e1, &e2);
    } else {
      sscanf(arg, "%1c%1c%1c", &c1, &e1, &e2);
    }

    if ( e1 == 66 || e1 == 98){
      powerUnit = 1000;
    } else if ( e1 != 0){
      returnCode = 1;
    }

    if ( e2 != 0 ){
      returnCode = 1;
    }

    // Deal with humans using lowercase
    if ((c1 < 123) && (c1 > 96)){
      c1 = c1 - 32;
    }

    block_unit[0] = c1;
    block_unit[1] = e1;

    switch (c1){
      case 'K':
        power = 1;
        break;
      case 'M':
        power = 2;
        break;
      case 'G':
        power = 3;
        break;
      case 'T':
        power = 4;
        break;
      case 'P':
        power = 5;
        break;
      case 'E':
        power = 6;
        break;
      case 'Z':
        power = 7;
        break;
      case 'Y':
        power = 8;
        break;
      case 'R':
        power = 9;
        break;
      case 'Q':
        power = 10;
        break;
      case '\0':
        power = 0;
        break;
      default:
        returnCode = 1;
        break;
    }

    multiplier = pow(powerUnit, power);

    numarg = i1 * multiplier;
    if (returnCode != 1){
      if (numarg < 1){
        returnCode = 1;
      } else {
        block_size = numarg;
        returnCode = 0;
      }
    }

    return returnCode;
}

void refreshScreenShow()
{
  // unloadShowMenuLabels();
  // refreshShowMenuLabels();
  switch(viewMode)
    {
    case 0: // Directory View
      resizeDisplayDir(ob);
      wPrintMenu(0, 0, topMenuBuffer);
      wPrintMenu(LINES-1, 0, bottomMenuBuffer);
      break;
    case 1: // Global Menu View
      wPrintMenu(0, 0, topMenuBuffer);
      break;
    case 2: // Colors View
      themeBuilder();
      break;
    case 3: // Settings View (currently broken)
      wPrintMenu(0, 0, topMenuBuffer);
      break;
    case 4: // SF View
      wPrintMenu(0, 0, topMenuBuffer);
      wPrintMenu(LINES-1, 0, bottomMenuBuffer);
      break;
    default: // Fallback
      resizeDisplayDir(ob);
      wPrintMenu(0, 0, topMenuBuffer);
      wPrintMenu(LINES-1, 0, bottomMenuBuffer);
      break;
    }
}

void sigwinchHandle(int sig){
  resized = 1;
}

void printHelp(char* programName){
  printf (("Usage: %s [OPTION]... [FILE]...\n"), programName);
  fputs ((PROGRAM_DESC), stdout);
  fputs (("\n\
Sorts objects alphabetically if -St is not set.\n\
"), stdout);
  fputs (("\n\
Options shared with ls:\n"), stdout);
#ifdef HAVE_ACL_TYPE_EXTENDED
  fputs(("  -@                           display extended attribute keys and sizes\n"), stdout);
#endif
  fputs(("  -a, --all                    do not ignore entries starting with .\n\
      --author                 prints the author of each file\n\
      --block-size=SIZE        scale sizes by SIZE, for example '--block-size=M'\n\
                                 see SIZE format below\n\
  -B, --ignore-backups         do not list implied entries ending with ~\n\
      --color[=WHEN]           colorize the output, see the color section below\n\
  -d, --directory              list the directory only, not the contents\n\
  -f                           do not sort, enables -aU\n\
      --full-time              display time as full-iso format\n\
  -g                           only show group\n\
  -G, --no-group               do not show group\n\
  -h, --human-readable         print sizes like 1K 234M 2G etc.\n\
  -i, --inode                  print index number of each file\n\
      --si                     as above, but use powers of 1000 not 1024\n\
  -n, --numeric-uid-gid        show numeric values for user and group IDs\n\
  -r, --reverse                reverse order while sorting\n\
  -s, --size                   display the allocated size of files, in blocks\n\
  -S                           sort file by size, largest first\n\
      --time-style=TIME_STYLE  time/date format, see TIME_STYLE section below\n\
  -t                           sort by modification time, newest first\n\
  -U                           do not sort; lists objects in directory order\n\
  -Z, --context                show security context of each file, if any\n\
  -1                           only show file name, one per line\n\
      --help                   displays help message, then exits\n\
      --version                displays version, then exits\n"), stdout);
  fputs (("\n\
The SIZE agrument is an integer and optional unit (for example: 10K = 10*1024).\n\
Units are K,M,G,T,P,E,Z,Y,R,Q (powers of 1024) or KB,MB, etc. (powers of 1000).\n"), stdout);
  fputs (("\n\
The TIME_STYLE argument can be: full-iso; long-iso; iso; locale.\n"), stdout);
  fputs (("\n\
Using color to highlight file attributes is disabled by default and with\n\
--color=never. With --color or --color=always this function is enabled.\n"), stdout);
  fputs (("\n\
Options specific to show:\n\
      --directories-only       show only directories\n\
      --theme=[THEME]          color themes, see the THEME section below for\n\
                                 valid themes\n\
      --no-danger              turns off danger colors when running with\n\
                                 elevated privileges\n\
      --marked=[MARKED]        shows information about marked objects. See\n\
                                 MARKED section below for valid options\n\
      --no-sf                  does not display files in sf\n\
      --show-on-enter          repurposes the Enter key to launch the show\n\
                                 command\n\
      --running                display number of parent show processes\n\
      --settings-menu          launch settings menu\n\
      --edit-themes            launchs directly into the theme editor\n\
      --skip-to-first          skips navigation items if at the top of list\n"), stdout);
  fputs (("\n\
The THEME argument can be:\n"), stdout);
  listThemes();
  fputs (("\n\
Exit status:\n\
 0  if OK,\n\
 1  if minor problems (e.g., cannot access subdirectory),\n\
 2  if major problems (e.g., cannot access command-line arguement).\n"), stdout);
  printf ("\nPlease report any bugs to: <%s>\n", PACKAGE_BUGREPORT);
}

// void freeSettingVars()
// {
//   free(visualPath);
//   return;
// }

int main(int argc, char *argv[])
{
  uid_t uid=getuid(), euid=geteuid();
  int c;
  char * tmpPwd;
  char options[22];

  initI18n();

#ifdef HAVE_ACL_TYPE_EXTENDED
  snprintf(options, 22, "%s", "@aABdfgGhilnrsStUZ1");
#else
  snprintf(options, 22, "%s", "aABdfgGhilnrsStUZ1");
#endif

  // Setting the default editor
#ifdef HAVE_NANO
  visualPath = calloc(5, sizeof(char));
  snprintf(visualPath, 5, "nano");
#elif HAVE_VIM
  visualPath = calloc(4, sizeof(char));
  snprintf(visualPath, 4, "vim");
#elif HAVE_VI
  visualPath = calloc(3, sizeof(char));
  snprintf(visualPath, 3, "vi");
#elif HAVE_EMACS
  visualPath = calloc(6, sizeof(char));
  snprintf(visualPath, 6, "emacs");
#elif HAVE_JED
  visualPath = calloc(4, sizeof(char));
  snprintf(visualPath, 4, "jed");
#else
  visualPath = calloc(3, sizeof(char));
  snprintf(visualPath, 3, "vi");
#endif

  // Setting the default pager
#ifdef HAVE_LESS
  pagerPath = calloc(5, sizeof(char));
  snprintf(pagerPath, 5, "less");
#elif HAVE_MORE
  pagerPath = calloc(5, sizeof(char));
  snprintf(pagerPath, 5, "more");
#else
  pagerPath = calloc(5, sizeof(char));
  snprintf(pagerPath, 5, "more");
#endif

  showProcesses = checkRunningEnv() + 1;

  // Set Config locations
  setConfLocations();

  // Check if we're root to display danger
  if (uid == 0 || euid == 0){
    danger = 1;
  }

  // Read the config

  
  settingsAction("read", "all", NULL, NULL, NULL, NULL, 0, 0, 0, globalConfLocation);
  settingsAction("read", "all", NULL, NULL, NULL, NULL, 0, 0, 0, homeConfLocation);
  // readShowConfig(globalConfLocation);
  // readShowConfig(homeConfLocation);

  // Check for theme env variable
  if ( getenv("DFS_THEME")) {
    snprintf(themeName, 256, "%s", getenv("DFS_THEME"));
  }

  // Getting arguments
  while (1)
    {
      static struct option long_options[] =
        {
         {"all",              no_argument,       0, 'a'},
         {"almost-all",       no_argument,       0, 'A'},
         {"author",           no_argument,       0, GETOPT_AUTHOR_CHAR},
         {"block-size",       required_argument, 0, GETOPT_BLOCKSIZE_CHAR},
         {"ignore-backups",   no_argument,       0, 'B'},
         {"directory",        no_argument,       0, 'd'},
         {"directories-only", no_argument,       0, GETOPT_DIRONLY_CHAR},
         {"human-readable",   no_argument,       0, 'h'},
         {"inode",            no_argument,       0, 'i'},
         {"no-group",         no_argument,       0, 'G'},
         {"numeric-uid-gid",  no_argument,       0, 'n'},
         {"reverse",          no_argument,       0, 'r'},
         {"size",             no_argument,       0, 's'},
         {"time-style",       required_argument, 0, GETOPT_TIMESTYLE_CHAR},
         {"si",               no_argument,       0, GETOPT_SI_CHAR},
         {"help",             no_argument,       0, GETOPT_HELP_CHAR},
         {"version",          no_argument,       0, GETOPT_VERSION_CHAR},
         {"no-danger",        no_argument,       0, GETOPT_NODANGER_CHAR},
         {"color",            optional_argument, 0, GETOPT_COLOR_CHAR},
         {"theme",            optional_argument, 0, GETOPT_THEME_CHAR},
         {"marked",           optional_argument, 0, GETOPT_MARKED_CHAR},
         {"no-sf",            no_argument,       0, GETOPT_ENVPAGER_CHAR},
         {"show-on-enter",    no_argument,       0, GETOPT_SHOWONENTER_CHAR},
         {"running",          no_argument,       0, GETOPT_SHOWRUNNING_CHAR},
         {"full-time",        no_argument,       0, GETOPT_FULLTIME_CHAR},
         {"edit-themes",      no_argument,       0, GETOPT_THEMEEDIT_CHAR},
         {"settings-menu",    no_argument,       0, GETOPT_OPTIONSMENU_CHAR},
         {"contect",          no_argument,       0, 'Z'},
         {"skip-to-first",    no_argument,       0, GETOPT_SKIPTOFIRST_CHAR},
         {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long(argc, argv, options, long_options, &option_index);

      if ( c == -1 ){
        break;
      }

    switch(c){
    case 'A':
      // Dropthrough
    case 'a':
      showhidden = 1;
      break;
    case GETOPT_AUTHOR_CHAR:
      ogavis = ogavis + 4;
      break;
    case GETOPT_BLOCKSIZE_CHAR:
      if (setBlockSize(optarg)){
          printf(_("%s: invalid argument '%s' for 'block-size'\n"), argv[0], optarg);
          exit(2);
      };
      scaleSize = 1;
      break;
    case 'B':
      showbackup = 0;
      break;
    case GETOPT_COLOR_CHAR:
      if (optarg){
        if (!setColor(optarg)){
          printf(_("%s: invalid argument '%s' for 'color'\n"), argv[0], optarg);
          fputs ((_("\
Valid arguments are:\n\
  - always\n\
  - never\n")), stdout);
          printf(_("Try '%s --help' for more information.\n"), argv[0]);
          exit(2);
        }
      } else {
        filecolors = 1;
        }
      break;
    case GETOPT_THEME_CHAR:
      if (optarg){
        if (strcmp(optarg, "\0")){
          snprintf(themeName, 256, "%s", optarg);
          setenv("DFS_THEME_OVERRIDE", "TRUE", 1);
        }
      } else {
        printf(_("%s: The following themes are available:\n"), argv[0]);
        listThemes();
        exit(2);
      }
      break;
    case 'd':
      currentDirOnly = true;
      break;
    case GETOPT_DIRONLY_CHAR:
      dirOnly = 1;
      break;
    case 'f':
      snprintf(sortmode, 9, "unsorted"); // This needs to be set to "unsorted" to allow the settings menu to render correctly.
      showhidden = 1;
      break;
    case 'l':
      break; // Allows for accidental -l from `ls` muscle memory commands. Does nothing.
    case 'S':
      snprintf(sortmode, 9, "size");
      break;
    case 's':
      showSizeBlocks = 1;
      break;
    case GETOPT_TIMESTYLE_CHAR:
      snprintf(timestyle, 9, "%s", optarg);
      if (!checkStyle(timestyle)){
        printf(_("%s: invalid argument '%s' for 'time style'\n"), argv[0], timestyle);
        fputs ((_("\
Valid arguments are:\n\
  - full-iso\n\
  - long-iso\n\
  - iso\n\
  - locale\n")), stdout);
        printf(_("Try '%s --help' for more information.\n"), argv[0]);
        exit(2);
      }
      break;
    case GETOPT_FULLTIME_CHAR:
      snprintf(timestyle, 9, "full-iso");
      break;
    case 't':
      snprintf(sortmode, 9, "date");
      break;
    case 'g':
      ogavis--;
      break;
    case 'G':
      ogavis = ogavis - 2;
      break;
    case 'h':
      human = 1;
      break;
    case 'i':
      showInodes = true;
      break;
    case GETOPT_SI_CHAR:
      human = 1;
      si = 1;
      break;
    case 'n':
      numericIds = true;
      break;
    case 'r':
      reverse = 1;
      break;
    case 'U':
      snprintf(sortmode, 9, "unsorted");
      break;
    case GETOPT_NODANGER_CHAR:
      danger = 0;
      break;
    case GETOPT_MARKED_CHAR:
      if (optarg){
        if ( setMarked(optarg) == -1 ){
          printf(_("%s: invalid argument '%s' for 'marked'\n"), argv[0], optarg);
          fputs ((_("\
Valid arguments are:\n\
  - always\n\
  - never\n\
  - auto\n")), stdout);
          printf(_("Try '%s --help' for more information.\n"), argv[0]);
          exit(2);
        }
      }
      break;
    case GETOPT_HELP_CHAR:
      printHelp(argv[0]);
      exit(0);
      break;
    case GETOPT_VERSION_CHAR:
      printVersion(PROGRAM_NAME);
      exit(0);
      break;
    case GETOPT_ENVPAGER_CHAR:
      useEnvPager = 1;
      break;
    case GETOPT_SHOWONENTER_CHAR:
      enterAsShow = 1;
      break;
    case GETOPT_SHOWRUNNING_CHAR:
      if (checkRunningEnv() > 0){
        printf(_("There are currently %i running parent show application(s).\n\nUse 'exit' to return to Show.\n"), checkRunningEnv());
        exit(0);
      } else {
        printf(_("There are no parent show applications currently running.\n"));
        exit(0);
      }
      break;
    case GETOPT_THEMEEDIT_CHAR:
      launchThemeEditor = 1;
      break;
    case GETOPT_OPTIONSMENU_CHAR:
      launchSettingsMenu = 1;
      break;
    case 'Z':
      showContext = 1;
      break;
    case '1':
      oneLine = 1;
      break;
    case GETOPT_SKIPTOFIRST_CHAR:
      skipToFirstFile = 1;
      break;
    case '@':
      showXAttrs = 1;
      break;
    default:
      // abort();
      exit(2);
    }
  }


  // If all author is also requested, the padding needs adjusting.
  if ( ogavis == 7 ){
    ogapad = 2;
  }


  // Writing Menus

  generateDefaultShowMenus();
  generateDefaultSfMenus();

  set_escdelay(10);

  newterm(NULL, stderr, stdin); 
  refreshShowMenuLabels();
  refreshSfMenuLabels();

  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigwinchHandle;
  sigaction(SIGWINCH, &sa, NULL);
  if (!enableCtrlC){
    signal(SIGINT, sigintHandle);
  }

  start_color();
  setDefaultTheme();
  loadAppTheme(themeName);
  bkgd(COLOR_PAIR(DISPLAY_PAIR));
  cbreak();
  noecho();
  curs_set(FALSE); // Hide Curser (Will want to bring it back later)
  keypad(stdscr, TRUE);

  generateShowSettingsVars();

  if (launchThemeEditor == 1){
    themeBuilder();
    theme_menu_inputs();
    exittoshell();
  } else if (launchSettingsMenu == 1) {
    settingsMenuView(showSettingsMenuLabel, showSettingsMenuSize, showSettingsMenu, &settingIndexShow, &charValuesShow, &binValuesShow, totalCharItemsShow, totalBinItemsShow, generateShowSettingsVars(), "show");
    exittoshell();
  } else {
    // Remaining arguments passed as working directory
    if (optind < argc){
      if (!check_first_char(argv[optind], "/")){
        // If the path given doesn't start with a / then assume we're dealing with a relative path.
        tmpPwd = malloc(sizeof(currentpwd));
        getcwd(tmpPwd, sizeof(currentpwd));
        snprintf(currentpwd, 4096, "%s/%s", tmpPwd, argv[optind]);
        free(tmpPwd);
      } else {
        snprintf(currentpwd, 4096, "%s", argv[optind]);
      }
      chdir(currentpwd);
    } else {
      getcwd(currentpwd, sizeof(currentpwd));
    }

    if (!check_dir(currentpwd)){
      invalidstart = 1;
      exitCode = 1;
      global_menu();
    }
  testSlash:
    if (check_last_char(currentpwd, "/") && strcmp(currentpwd, "/")){
      currentpwd[strlen(currentpwd) - 1] = '\0';
      goto testSlash;
    }
    directory_view(currentpwd);
  }
  return exitCode;
}
