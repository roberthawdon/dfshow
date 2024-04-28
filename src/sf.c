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


extern char regexinput[1024];

int colormode = 0;
int messageBreak = 0;

extern MEVENT event;

extern char *programName;

extern int displaysize;
extern int topline;
extern int leftcol;
extern int totallines;
extern int longestline;
extern int longestlongline;
extern int viewmode;

extern int tabsize;

extern int wrap;
extern int wrapmode;

int launchSettingsMenu = 0;

extern char fileName[4096];

int resized = 0;

extern settingIndex *settingIndexSf;
extern t1CharValues *charValuesSf;
extern t2BinValues *binValuesSf;
extern int totalCharItemsSf;
extern int totalBinItemsSf;

extern FILE *file;

extern int enableCtrlC;

extern FILE *stream;
extern char *line;
extern wchar_t *longline;
extern size_t len;
extern ssize_t nread;
extern int count;
extern int displaycount;
extern int top, left;
extern int lasttop;
extern int i, s;

long int topPos;
extern long int *filePos;

struct sigaction sa;

extern char globalConfLocation[4096];
extern char homeConfLocation[4096];

extern char themeName[256];

extern menuDef *sfSettingsMenu;
extern int sfSettingsMenuSize;
extern wchar_t *sfSettingsMenuLabel;

void sigwinchHandle(int sig)
{
  // refreshScreenSf();
  resized = 1;
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

int main(int argc, char *argv[])
{
  int c;

  initI18n();

  setDynamicChar(&programName, "%s", PROGRAM_NAME);

  setConfLocations();

  // Read the config

  settingsAction("read", "sf", NULL, NULL, NULL, NULL, 0, 0, 0, globalConfLocation);
  settingsAction("read", "sf", NULL, NULL, NULL, NULL, 0, 0, 0, homeConfLocation);
  // readSfConfig(globalConfLocation);
  // readSfConfig(homeConfLocation);

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

  generateDefaultSfMenus();

  set_escdelay(10);
  //ESCDELAY = 10;

  // Blank out regexinput

  //strcpy(regexinput, "");
  regexinput[0]=0;

  newterm(NULL, stderr, stdin); 
  // initscr();

  refreshSfMenuLabels();

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

  // Enable mouse events
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  mouseinterval(0);

  if (launchSettingsMenu == 1) {
    settingsMenuView(sfSettingsMenuLabel, sfSettingsMenuSize, sfSettingsMenu, &settingIndexSf, &charValuesSf, &binValuesSf, totalCharItemsSf, totalBinItemsSf, generateSfSettingsVars(), "sf");
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

