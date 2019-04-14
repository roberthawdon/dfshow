/*
  DF-SHOW - A clone of 'SHOW' directory browser from DF-EDIT by Larry Kroeker
  Copyright (C) 2018-2019  Robert Ian Hawdon

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
#include "config.h"
#include "showfunctions.h"
#include "showmenus.h"
#include "colors.h"
#include "common.h"
#include "show.h"

char currentpwd[1024];

int viewMode = 0;
int resized = 0;

char sortmode[5] = "name";
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


int plugins = 0; // Not yet implemented

int enterAsShow = 0;

int messageBreak = 0;

int showProcesses;

char *objectWild;

results *ob;

extern int settingsPos;
extern int settingsBinPos;

extern menuDef *settingsMenu;
extern int settingsMenuSize;
extern wchar_t *settingsMenuLabel;

extern int * pc;

extern history *hs;
extern int topfileref;
extern int selected;
extern int totalfilecount;
extern char sortmode[5];
extern int showhidden;

extern char globalConfLocation[128];
extern char homeConfLocation[128];

extern char themeName[128];

struct sigaction sa;

extern int exitCode;
extern int enableCtrlC;

extern wchar_t *globalMenuLabel;
extern wchar_t *fileMenuLabel;
extern wchar_t *functionMenuLabel;
extern wchar_t *modifyMenuLabel;
extern wchar_t *sortMenuLabel;
extern wchar_t *linkMenuLabel;

int setMarked(char* markedinput);
int checkStyle(char* styleinput);

void readConfig(const char * confFile)
{
  config_t cfg;
  config_setting_t *root, *setting, *group, *array; //probably don't need the array, but it may be used in the future.
  char markedParam[8];
  config_init(&cfg);
  if (config_read_file(&cfg, confFile)){
    // Deal with the globals first
    group = config_lookup(&cfg, "common");
    if (group){
      setting = config_setting_get_member(group, "theme");
      if (setting){
        if (!getenv("DFS_THEME_OVERRIDE")){
          strcpy(themeName, config_setting_get_string(setting));
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
        strcpy(markedParam, config_setting_get_string(setting));
        setMarked(markedParam);
      }
      // Check Sort
      setting = config_setting_get_member(group, "sortmode");
      if (setting){
        strcpy(sortmode, config_setting_get_string(setting));
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
        strcpy(timestyle, config_setting_get_string(setting));
        if(!checkStyle(timestyle)){
          strcpy(timestyle, "locale");
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
      // Check Enter As Show
      setting = config_setting_get_member(group, "show-on-enter");
      if (setting){
        if (config_setting_get_int(setting)){
          enterAsShow = 1;
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

void saveConfig(const char * confFile, settingIndex **settings, int items)
{
  config_t cfg;
  config_setting_t *root, *setting, *group, array;
  int i, v;

  config_init(&cfg);

  //Might need to "if" this
  config_read_file(&cfg, confFile);
  root = config_root_setting(&cfg);

  //config_setting_remove(root, PROGRAM_NAME);

  group = config_setting_get_member(root, PROGRAM_NAME);

  if (!group){
    group = config_setting_add(root, PROGRAM_NAME, CONFIG_TYPE_GROUP);
  }

  for (i = 0; i < items; i++){
    if ((*settings)[i].type == 0){
      config_setting_remove(group, (*settings)[i].refLabel);
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
      } else if (!strcmp((*settings)[i].refLabel, "show-on-enter")){
        config_setting_set_int(setting, enterAsShow);
      }
    }
  }

  config_write_file(&cfg, confFile);

  config_destroy(&cfg);
}

void applySettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount)
{
  int i, j;
  // do stuff
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
    } else if (!strcmp((*settings)[i].refLabel, "show-on-enter")){
      enterAsShow = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "marked")){
      markedinfo = (*settings)[i].intSetting;
    } else if (!strcmp((*settings)[i].refLabel, "sortmode")){
      for (j = 0; j < valuesCount; j++){
        if (!strcmp((*values)[j].refLabel, "sortmode") && ((*values)[j].index == (*settings)[i].intSetting)){
          strcpy(sortmode, (*values)[j].value);
        }
      }
    } else if (!strcmp((*settings)[i].refLabel, "timestyle")){
      for (j = 0; j < valuesCount; j++){
        if (!strcmp((*values)[j].refLabel, "timestyle") && ((*values)[j].index == (*settings)[i].intSetting)){
          strcpy(timestyle, (*values)[j].value);
        }
      }
    } else if (!strcmp((*settings)[i].refLabel, "owner")){
      ogavis = (*settings)[i].intSetting;
    }
  }
}

void settingsMenuView(){
  uid_t uid=getuid(), euid=geteuid();
  int items, count = 0;
  int x = 2;
  int y = 3;
  settingIndex *settingIndex;
  // type1SValue *tmpValues, *noValue, *markedValue, *sortmodeValue, *timestyleValue;
  t1CharValues *charValues;
  t2BinValues *binValues;
  int markedCount, sortmodeCount, timestyleCount, ownerCount;
  int charValuesCount;
  int binValuesCount;
  int sortmodeInt, timestyleInt;

 reloadSettings:

  items = charValuesCount = binValuesCount = markedCount = sortmodeCount = timestyleCount = ownerCount = 0;

  clear();
  wPrintMenu(0,0,settingsMenuLabel);
  // mvprintw(2, 10, "SHOW Settings Menu");

  addT1CharValue(&charValues, &charValuesCount, &markedCount, "marked", "never");
  addT1CharValue(&charValues, &charValuesCount, &markedCount, "marked", "always");
  addT1CharValue(&charValues, &charValuesCount, &markedCount, "marked", "auto");

  addT1CharValue(&charValues, &charValuesCount, &sortmodeCount, "sortmode", "name");
  addT1CharValue(&charValues, &charValuesCount, &sortmodeCount, "sortmode", "date");
  addT1CharValue(&charValues, &charValuesCount, &sortmodeCount, "sortmode", "size");
  addT1CharValue(&charValues, &charValuesCount, &sortmodeCount, "sortmode", "unsorted");

  addT1CharValue(&charValues, &charValuesCount, &timestyleCount, "timestyle", "locale");
  addT1CharValue(&charValues, &charValuesCount, &timestyleCount, "timestyle", "iso");
  addT1CharValue(&charValues, &charValuesCount, &timestyleCount, "timestyle", "long-iso");
  addT1CharValue(&charValues, &charValuesCount, &timestyleCount, "timestyle", "full-iso");

  addT2BinValue(&binValues, &binValuesCount, &ownerCount, "owner", "owner", 1);
  addT2BinValue(&binValues, &binValuesCount, &ownerCount, "owner", "group", 0);
  addT2BinValue(&binValues, &binValuesCount, &ownerCount, "owner", "author", 0);

  sortmodeInt = textValueLookup(&charValues, &charValuesCount, "sortmode", sortmode);
  timestyleInt = textValueLookup(&charValues, &charValuesCount, "timestyle", timestyle);

  importSetting(&settingIndex, &items, "color",          L"Display file colors", 0, filecolors, -1, 0);
  importSetting(&settingIndex, &items, "marked",         L"Show marked file info", 1, markedinfo, markedCount, 0);
  importSetting(&settingIndex, &items, "sortmode",       L"Sorting mode", 1, sortmodeInt, sortmodeCount, 0);
  importSetting(&settingIndex, &items, "reverse",        L"Reverse sorting order", 0, reverse, -1, 0);
  importSetting(&settingIndex, &items, "timestyle",      L"Time style", 1, timestyleInt, timestyleCount, 0);
  importSetting(&settingIndex, &items, "hidden",         L"Show hidden files", 0, showhidden, -1, 0);
  importSetting(&settingIndex, &items, "ignore-backups", L"Hide backup files", 0, showbackup, -1, 1);
  importSetting(&settingIndex, &items, "no-sf",          L"Use 3rd party pager over SF", 0, useEnvPager, -1, 0);
  if (uid == 0 || euid == 0){
    importSetting(&settingIndex, &items, "no-danger",      L"Hide danger lines as root", 0, danger, -1, 1);
  }
  importSetting(&settingIndex, &items, "si",             L"Use SI units", 0, si, -1, 0);
  importSetting(&settingIndex, &items, "human-readable", L"Human readable sizes", 0, human, -1, 0);
  importSetting(&settingIndex, &items, "show-on-enter",  L"Enter key acts like Show", 0, enterAsShow, -1, 0);
  importSetting(&settingIndex, &items, "owner",          L"Owner Column", 2, ogavis, ownerCount, 0);

  populateBool(&binValues, "owner", ogavis, binValuesCount);

  curs_set(TRUE);

  while(1)
    {
      for (count = 0; count < items; count++){
        printSetting(2 + count, 3, &settingIndex, &charValues, &binValues, count, charValuesCount, binValuesCount, settingIndex[count].type, settingIndex[count].invert);
      }

      move(x + settingsPos, y + 1);
      *pc = getch10th();
      if (*pc == menuHotkeyLookup(settingsMenu, "s_quit", settingsMenuSize)){
        curs_set(FALSE);
        applySettings(&settingIndex, &charValues, items, charValuesCount);
        free(settingIndex);
        return;
      } else if (*pc == menuHotkeyLookup(settingsMenu, "s_revert", settingsMenuSize)){
        free(settingIndex);
        goto reloadSettings;
      } else if (*pc == menuHotkeyLookup(settingsMenu, "s_save", settingsMenuSize)){
        applySettings(&settingIndex, &charValues, items, charValuesCount);
        saveConfig(homeConfLocation, &settingIndex, items);
      } else if (*pc == 258 || *pc == 10){
        if (settingsPos < (items -1 )){
          settingsBinPos = -1;
          settingsPos++;
        }
      } else if (*pc == 32 || *pc == 260 || *pc == 261){
        // Adjust
        if (settingIndex[settingsPos].type == 0){
          if (settingIndex[settingsPos].intSetting > 0){
            updateSetting(&settingIndex, settingsPos, 0, 0);
          } else {
            updateSetting(&settingIndex, settingsPos, 0, 1);
          }
        } else if (settingIndex[settingsPos].type == 1){
          if (*pc == 32 || *pc == 261){
            if (settingIndex[settingsPos].intSetting < (settingIndex[settingsPos].maxValue) - 1){
              updateSetting(&settingIndex, settingsPos, 1, (settingIndex[settingsPos].intSetting) + 1);
            } else {
              updateSetting(&settingIndex, settingsPos, 1, 0);
            }
          } else {
            if (settingIndex[settingsPos].intSetting > 0){
              updateSetting(&settingIndex, settingsPos, 1, (settingIndex[settingsPos].intSetting) - 1);
            } else {
              updateSetting(&settingIndex, settingsPos, 1, (settingIndex[settingsPos].maxValue - 1));
            }
          }
        } else if (settingIndex[settingsPos].type == 2){
          if (*pc == 261 && (settingsBinPos < (settingIndex[settingsPos].maxValue -1))){
            settingsBinPos++;
          } else if (*pc == 260 && (settingsBinPos > -1)){
            settingsBinPos--;
          } else if (*pc == 32 && (settingsBinPos > -1)){
            // Not fond of this, but it should work
            if (!strcmp(settingIndex[settingsPos].refLabel, "owner")){
              adjustBinSetting(&settingIndex, &binValues, "owner", &ogavis, binValuesCount);
            }
          }
        }
      } else if (*pc == 259){
        if (settingsPos > 0){
          settingsBinPos = -1;
          settingsPos--;
        }
      }
    }
}

int directory_view(char * currentpwd)
{
  objectWild = objectFromPath(currentpwd);
  if ( strchr(objectWild, MULTICHAR) || strchr(objectWild, ONECHAR)){
    strcpy(currentpwd, dirFromPath(currentpwd));
  } else {
    strcpy(objectWild, "");
  }

  topfileref = 0;
  clear();
  setColors(COMMAND_PAIR);

  // directory_top_menu();

  wPrintMenu(0, 0, fileMenuLabel);

  set_history(currentpwd, "", "", 0, 0);
  free(ob);
  ob = get_dir(currentpwd);
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, topfileref, 0);

  // function_key_menu();

  //printMenu(LINES-1, 0, functionMenuText);
  wPrintMenu(LINES-1, 0, functionMenuLabel);

  refresh();

  directory_view_menu_inputs();

  free(ob); //freeing memory
  return 0;
}

int global_menu()
{
  clear();

  // printMenu(0, 0, globalMenuText);

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
    // } else if (!strcmp(colorinput, "auto")){
    //   filecolors = 0; // Need to make this autodetect
    //   return 1;
  } else if (!strcmp(colorinput, "never")){
    filecolors = 0;
    return 1;
  } else {
    return 0;
  }
}

void refreshScreen()
{
  endwin();
  clear();
  refresh();
  initscr();
  //mvprintw(0,0,"%d:%d", LINES, COLS);
  unloadMenuLabels();
  refreshMenuLabels();
  switch(viewMode)
    {
    case 0:
      wPrintMenu(0, 0, fileMenuLabel);
      wPrintMenu(LINES-1, 0, functionMenuLabel);
      resizeDisplayDir(ob);
      break;
    case 1:
      wPrintMenu(0,0,globalMenuLabel);
      wPrintMenu(LINES-1, 0, functionMenuLabel);
      resizeDisplayDir(ob);
      break;
    case 2:
      wPrintMenu(0, 0, modifyMenuLabel);
      wPrintMenu(LINES-1, 0, functionMenuLabel);
      resizeDisplayDir(ob);
      break;
    case 3:
      wPrintMenu(0, 0, sortMenuLabel);
      wPrintMenu(LINES-1, 0, functionMenuLabel);
      resizeDisplayDir(ob);
      break;
    case 4:
      wPrintMenu(0,0,globalMenuLabel);
      break;
    case 5:
      wPrintMenu(0, 0, linkMenuLabel);
      wPrintMenu(LINES-1, 0, functionMenuLabel);
      resizeDisplayDir(ob);
      break;
    }
}

void sigwinchHandle(int sig){
  resized = 1;
  // refreshScreen();
}

void printHelp(char* programName){
  printf (("Usage: %s [OPTION]... [FILE]...\n"), programName);
  fputs ((PROGRAM_DESC), stdout);
  fputs (("\n\
Sorts objects alphabetically if -St is not set.\n\
"), stdout);
  fputs (("\n\
Options shared with ls:\n\
  -a, --all                    do not ignore entries starting with .\n\
      --author                 prints the author of each file\n\
  -B, --ignore-backups         do not list implied entries ending with ~\n\
      --color[=WHEN]           colorize the output, see the color section below\n\
  -f                           do not sort, enables -aU\n\
      --full-time              display time as full-iso format\n\
  -g                           only show group\n\
  -G, --no-group               do not show group\n\
  -h, --human-readable         print sizes like 1K 234M 2G etc.\n\
      --si                     as above, but use powers of 1000 not 1024\n\
  -r, --reverse                reverse order while sorting\n\
  -S                           sort file by size, largest first\n\
      --time-style=TIME_STYLE  time/date format, see TIME_STYLE section below\n\
  -t                           sort by modification time, newest first\n\
  -U                           do not sort; lists objects in directory order\n\
      --help                   displays help message, then exits\n\
      --version                displays version, then exits\n"), stdout);
  fputs (("\n\
The TIME_STYLE argument can be: full-iso; long-iso; iso; locale.\n"), stdout);
  fputs (("\n\
Using color to highlight file attributes is disabled by default and with\n\
--color=never. With --color or --color=always this function is enabled.\n"), stdout);
  fputs (("\n\
Options specific to show:\n\
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
      --edit-themes            launchs directly into the theme editor\n"), stdout);
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

int main(int argc, char *argv[])
{
  uid_t uid=getuid(), euid=geteuid();
  int c;

  showProcesses = checkRunningEnv() + 1;

  // Set Config locations
  setConfLocations();

  // Check if we're root to display danger
  if (uid == 0 || euid == 0){
    danger = 1;
  }

  // Read the config

  readConfig(globalConfLocation);
  readConfig(homeConfLocation);

  // Check for theme env variable
  if ( getenv("DFS_THEME")) {
    strcpy(themeName, getenv("DFS_THEME"));
  }

  // Getting arguments
  while (1)
    {
      static struct option long_options[] =
        {
         {"all",            no_argument,       0, 'a'},
         {"almost-all",     no_argument,       0, 'A'},
         {"author",         no_argument,       0, GETOPT_AUTHOR_CHAR},
         {"ignore-backups", no_argument,       0, 'B'},
         {"human-readable", no_argument,       0, 'h'},
         {"no-group",       no_argument,       0, 'G'},
         {"reverse",        no_argument,       0, 'r'},
         {"time-style",     required_argument, 0, GETOPT_TIMESTYLE_CHAR},
         {"si",             no_argument,       0, GETOPT_SI_CHAR},
         {"help",           no_argument,       0, GETOPT_HELP_CHAR},
         {"version",        no_argument,       0, GETOPT_VERSION_CHAR},
         {"no-danger",      no_argument,       0, GETOPT_NODANGER_CHAR},
         {"color",          optional_argument, 0, GETOPT_COLOR_CHAR},
         {"theme",          optional_argument, 0, GETOPT_THEME_CHAR},
         {"marked",         optional_argument, 0, GETOPT_MARKED_CHAR},
         {"no-sf",          no_argument,       0, GETOPT_ENVPAGER_CHAR},
         {"show-on-enter",  no_argument,       0, GETOPT_SHOWONENTER_CHAR},
         {"running",        no_argument,       0, GETOPT_SHOWRUNNING_CHAR},
         {"full-time",      no_argument,       0, GETOPT_FULLTIME_CHAR},
         {"edit-themes",    no_argument,       0, GETOPT_THEMEEDIT_CHAR},
         {"settings-menu",   no_argument,       0, GETOPT_OPTIONSMENU_CHAR},
         {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long(argc, argv, "aABfgGhrStU", long_options, &option_index);

      if ( c == -1 ){
        break;
      }

    switch(c){
    case 'A':
      // Dropthourgh
    case 'a':
      showhidden = 1;
      break;
    case GETOPT_AUTHOR_CHAR:
      ogavis = ogavis + 4;
      break;
    case 'B':
      showbackup = 0;
      break;
    case GETOPT_COLOR_CHAR:
      if (optarg){
        if (!setColor(optarg)){
          printf("%s: invalid argument '%s' for 'color'\n", argv[0], optarg);
          fputs (("\
Valid arguments are:\n\
  - always\n\
  - never\n"), stdout);
          printf("Try '%s --help' for more information.\n", argv[0]);
          exit(2);
        }
      } else {
        filecolors = 1;
        }
      break;
    case GETOPT_THEME_CHAR:
      if (optarg){
        if (strcmp(optarg, "\0")){
          strcpy(themeName, optarg);
          setenv("DFS_THEME_OVERRIDE", "TRUE", 1);
        }
      } else {
        printf("%s: The following themes are available:\n", argv[0]);
        listThemes();
        exit(2);
      }
      break;
    case 'f':
      strcpy(sortmode, "none"); // This can be set to anything non valid
      showhidden = 1;
      break;
    case 'S':
      strcpy(sortmode, "size");
      break;
    case GETOPT_TIMESTYLE_CHAR:
      strcpy(timestyle, optarg);
      if (!checkStyle(timestyle)){
        printf("%s: invalid argument '%s' for 'time style'\n", argv[0], timestyle);
        fputs (("\
Valid arguments are:\n\
  - full-iso\n\
  - long-iso\n\
  - iso\n\
  - locale\n"), stdout);
        printf("Try '%s --help' for more information.\n", argv[0]);
        exit(2);
      }
      break;
    case GETOPT_FULLTIME_CHAR:
      strcpy(timestyle, "full-iso");
      break;
    case 't':
      strcpy(sortmode, "date");
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
    case GETOPT_SI_CHAR:
      human = 1;
      si = 1;
      break;
    case 'r':
      reverse = 1;
      break;
    case 'U':
      strcpy(sortmode, "none"); // Again, invalid
      break;
    case GETOPT_NODANGER_CHAR:
      danger = 0;
      break;
    case GETOPT_MARKED_CHAR:
      if (optarg){
        if ( setMarked(optarg) == -1 ){
          printf("%s: invalid argument '%s' for 'marked'\n", argv[0], optarg);
          fputs (("\
Valid arguments are:\n\
  - always\n\
  - never\n\
  - auto\n"), stdout);
          printf("Try '%s --help' for more information.\n", argv[0]);
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
        printf("There are currently %i running parent show application(s).\n\nUse 'exit' to return to Show.\n", checkRunningEnv());
        exit(0);
      } else {
        printf("There are no parent show applications currently running.\n");
        exit(0);
      }
      break;
    case GETOPT_THEMEEDIT_CHAR:
      launchThemeEditor = 1;
      break;
    case GETOPT_OPTIONSMENU_CHAR:
      launchSettingsMenu = 1;
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

  generateDefaultMenus();
  set_escdelay(10);
  //ESCDELAY = 10;

  setlocale(LC_ALL, "");

  initscr();
  refreshMenuLabels();

  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigwinchHandle;
  sigaction(SIGWINCH, &sa, NULL);
  if (!enableCtrlC){
    signal(SIGINT, sigintHandle);
  }

  start_color();
  cbreak(); //Added for new method
  setDefaultTheme();
  loadAppTheme(themeName);
  bkgd(COLOR_PAIR(DISPLAY_PAIR));
  cbreak();
  // nodelay(stdscr, TRUE);
  noecho();
  curs_set(FALSE); // Hide Curser (Will want to bring it back later)
  keypad(stdscr, TRUE);


  if (launchThemeEditor == 1){
    themeBuilder();
    theme_menu_inputs();
    exittoshell();
  } else if (launchSettingsMenu == 1) {
    settingsMenuView();
    exittoshell();
  } else {
    // Remaining arguments passed as working directory
    if (optind < argc){
      if (!check_first_char(argv[optind], "/")){
        // If the path given doesn't start with a / then assume we're dealing with a relative path.
        getcwd(currentpwd, sizeof(currentpwd));
        sprintf(currentpwd, "%s/%s", currentpwd, argv[optind]);
      } else {
        strcpy(currentpwd, argv[optind]);
      }
      chdir(currentpwd);
    } else {
      getcwd(currentpwd, sizeof(currentpwd));
    }

    if (!check_dir(currentpwd)){
      //strcpy(currentpwd, "/"); // If dir doesn't exist, default to root
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
