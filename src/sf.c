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
#include "sf.h"
#include "sffunctions.h"
#include "i18n.h"


char regexinput[1024];

int colormode = 0;
int messageBreak = 0;
int displaysize;
int topline = 1;
int leftcol = 1;
int totallines = 0;
int longestline = 0;
int longestlongline = 0;
int viewmode = 0;

int tabsize = 8;

int wrap = 0;
int wrapmode = LINE_WRAP;

int launchSettingsMenu = 0;

char fileName[4096];

int resized = 0;

extern FILE *file;

extern int exitCode;
extern int enableCtrlC;

FILE *stream;
char *line = NULL;
wchar_t *longline = NULL;
size_t len = 0;
ssize_t nread;
int count;
int displaycount;
int top, left;
int lasttop;
int i, s;

long int topPos;
long int *filePos;

struct sigaction sa;

extern int * pc;

extern int settingsPos;
extern int settingsBinPos;

extern char globalConfLocation[4096];
extern char homeConfLocation[4096];

extern char themeName[256];

extern wchar_t *fileMenuLabel;

extern menuDef *settingsMenu;
extern int settingsMenuSize;
extern wchar_t *settingsMenuLabel;

void readConfig(const char * confFile)
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
    }
    // Now for program specific
    group = config_lookup(&cfg, PROGRAM_NAME);
    if (group){
      // Check Wrap
      setting = config_setting_get_member(group, "wrap");
      if (setting){
        if (config_setting_get_int(setting)){
          wrap = 1;
        }
      }
    }
  }
}

void refreshScreen()
{
  endwin();
  clear();
  refresh();
  // newterm(NULL, stderr, stdin); 
  // initscr();
  displaysize = LINES - 2;
  unloadMenuLabels();
  refreshMenuLabels();
  if (viewmode == 0){
    mvprintw(0,0,_("Show File - Enter pathname:"));
  } else if (viewmode > 0){
    wPrintMenu(0, 0, fileMenuLabel);
    loadFile(fileName);
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

void sigwinchHandle(int sig)
{
  // refreshScreen();
  resized = 1;
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

void printHelp(char* programName)
{
  printf (("Usage: %s [OPTION]... [FILE]...\n"), programName);
  fputs ((PROGRAM_DESC), stdout);
  fputs (("\n\
Options:\n\
  -w, --wrap                   turn line wrapping on\n\
      --theme=[THEME]          color themes, see the THEME section below for\n\
                               valid themes.\n\
      --settings-menu          launch settings menu\n\
      --help                   displays help message, then exits\n\
      --version                displays version, then exits\n"), stdout);
  fputs (("\n\
The THEME argument can be:\n"), stdout);
  listThemes();
  printf ("\nPlease report any bugs to: <%s>\n", PACKAGE_BUGREPORT);
}

void fileShowStatus()
{
  char *statusText;
  if (wrap){
    setDynamicChar(&statusText, _("File = <%s>  Top = <%i>"), fileName, topline);
  } else {
    setDynamicChar(&statusText, _("File = <%s>  Top = <%i:%i>"), fileName, topline, leftcol);
  }
  printMenu(LINES - 1, 0, statusText);
  free(statusText);
}

void updateView()
{
  int longlinelen = 0;
  top = topline;
  left = leftcol;
  len = 0;
  top--;
  left--;
  displaycount = 0;

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
      for(i = 0; i < longlinelen; i++){
        mvprintw(displaycount + 1, s - left, "%lc", longline[i]);
        // This doesn't increase the max line.
        if (line[i] == '\t'){
          s = s + calculateTab(s);
        } else {
          s++;
        }
        if ( s == COLS + left){
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
}

void loadFile(const char * currentfile)
{

  len = 0;
  longestline = 0;
  longestlongline = 0;
  viewmode = 1;
  totallines = 0;

  filePos = malloc(sizeof(long int) * 1); // Initial isze of lookup
  filePos[0] = 0;

  stream = fopen(currentfile, "rb");
  if (stream == NULL) {

    return;
    }

  line = malloc(sizeof(char) + 1);
  longline = malloc(sizeof(wchar_t));

  while ((nread = getline(&line, &len, stream)) != -1) {
    totallines++;
    filePos = realloc(filePos, sizeof(long int) * totallines + 1);
    filePos[totallines] = ftell(stream);
    if (nread > longestline){
      longestline = nread;
      longline = realloc(longline, sizeof(wchar_t) * longestline +1);
    }
    mbstowcs(longline, line, len);
    if (wcslen(longline) > longestlongline){
      longestlongline = wcslen(longline);
    }
  }
  free(line);
  updateView();
}

void file_view(char * currentfile)
{
  char *notFoundMessage;
  clear();
  setColors(COMMAND_PAIR);


  displaysize = LINES - 2;

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
  // sleep(10); // No function, so we'll pause for 10 seconds to display our menu

  return;
}

void saveConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex)
{
  config_t cfg;
  config_setting_t *root, *setting, *group;
  int i;

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

      if (!strcmp((*settings)[i].refLabel, "wrap")){
        config_setting_set_int(setting, wrap);
      }
    } else if ((*settings)[i].type == SETTING_SELECT){
      // None of those in SF (yet?)
    } else if ((*settings)[i].type == SETTING_MULTI){
      // None of those in SF (yet?)
    }
  }

  config_write_file(&cfg, confFile);

  config_destroy(&cfg);

}

void applySettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount)
{
  int i;
  for (i = 0; i < items; i++){
    if (!strcmp((*settings)[i].refLabel, "wrap")){
      wrap = (*settings)[i].intSetting;
    }
  }
}

void settingsMenuView()
{
  int items, count = 0;
  int x = 2;
  int y = 3;
  settingIndex *settingIndex;
  t1CharValues *charValues;
  t2BinValues *binValues;
  int charValuesCount;
  int binValuesCount;

 reloadSettings:

  items = charValuesCount = binValuesCount = 0;

  clear();
  wPrintMenu(0,0,settingsMenuLabel);

  importSetting(&settingIndex, &items, "wrap", _("Enable text wrapping"), SETTING_BOOL, NULL, wrap, -1, 0);

  while(1)
    {
      // if (settingsBinPos < 0){
      //   curs_set(TRUE);
      // } else {
      //   curs_set(FALSE);
      // }
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
        if (access(dirFromPath(homeConfLocation), W_OK) != 0) {
          createParentDirs(homeConfLocation);
        }
        saveConfig(homeConfLocation, &settingIndex, &charValues, &binValues, items, charValuesCount, binValuesCount);
        // Future task: ensure saving actually worked
        curs_set(FALSE);
        topLineMessage(_("Settings saved."));
        curs_set(TRUE);
        wPrintMenu(0,0,settingsMenuLabel);
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

void freeSettingVars()
{
  return;
}

int main(int argc, char *argv[])
{
  int c;

  initI18n();
  
  setConfLocations();

  // Read the config

  readConfig(globalConfLocation);
  readConfig(homeConfLocation);

  // Check for theme env variable
  if ( getenv("DFS_THEME")) {
    snprintf(themeName, 256, "%s", getenv("DFS_THEME"));
  }

  while (1)
    {
      static struct option long_options[] =
        {
         {"wrap",           no_argument,       0, 'w'},
         {"help",           no_argument,       0, GETOPT_HELP_CHAR},
         {"version",        no_argument,       0, GETOPT_VERSION_CHAR},
         {"theme",          optional_argument, 0, GETOPT_THEME_CHAR},
         {"settings-menu",  no_argument,       0, GETOPT_OPTIONSMENU_CHAR},
         {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long(argc, argv, "w", long_options, &option_index);

      if ( c == -1 ){
        break;
      }

    switch(c){
    case 'w':
      wrap = 1;
      break;
    case GETOPT_HELP_CHAR:
      printHelp(argv[0]);
      exit(0);
      break;
    case GETOPT_VERSION_CHAR:
      printVersion(PROGRAM_NAME);
      exit(0);
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
    case GETOPT_OPTIONSMENU_CHAR:
      launchSettingsMenu = 1;
      break;
    default:
      // abort();
      exit(2);
    }
  }

  generateDefaultMenus();

  set_escdelay(10);
  //ESCDELAY = 10;

  // Blank out regexinput

  //strcpy(regexinput, "");
  regexinput[0]=0;

  newterm(NULL, stderr, stdin); 
  // initscr();

  refreshMenuLabels();

  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigwinchHandle;
  sigaction(SIGWINCH, &sa, NULL);

  if (!enableCtrlC){
    signal(SIGINT, sigintHandle);
  }

  start_color();
  cbreak();
  setDefaultTheme();
  loadAppTheme(themeName);
  bkgd(COLOR_PAIR(DISPLAY_PAIR));
  cbreak();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);

  if (launchSettingsMenu == 1) {
    settingsMenuView();
  } else {
    if (optind < argc){
      snprintf(fileName, 4096, "%s", argv[optind]);
      file_view(fileName);
    } else {
      show_file_file_input();
    }
  }

  exittoshell();
  return 0;
}
