/*
  DF-SHOW - A clone of 'SHOW' directory browser from DF-EDIT by Larry Kroeker
  Copyright (C) 2018  Robert Ian Hawdon

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
char themeEnv[48];

char fileMenuText[256];
char globalMenuText[256];
char functionMenuText[256];
char functionMenuTextShort[256];
char functionMenuTextLong[256];
char modifyMenuText[256];
char sortMenuText[256];

int viewMode = 0;

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
int markedauto = 0;
int useEnvPager = 0;

int messageBreak = 0;

char *objectWild;

results *ob;

extern history *hs;
extern int topfileref;
extern int selected;
extern int totalfilecount;
extern char sortmode[5];
extern int showhidden;

struct sigaction sa;

int setMarked(char* markedinput);
int checkStyle(char* styleinput);

void readConfig()
{
  config_t cfg;
  config_setting_t *root, *setting, *group, *array; //probably don't need the array, but it may be used in the future.
  char themeName[24];
  char markedParam[8];
  config_init(&cfg);
  if (config_read_file(&cfg, GLOBAL_CONF)){
    // Deal with the globals first
    group = config_lookup(&cfg, "common");
    if (group){
      setting = config_setting_get_member(group, "theme");
      if (setting){
        strcpy(themeName, config_setting_get_string(setting));
        strcpy(themeEnv,"DFS_THEME=");
        strcat(themeEnv,themeName);
        putenv(themeEnv);
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

  printMenu(0, 0, fileMenuText);

  set_history(currentpwd, "", "", 0, 0);
  free(ob);
  ob = get_dir(currentpwd);
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, topfileref, 0);

  // function_key_menu();

  printMenu(LINES-1, 0, functionMenuText);

  refresh();

  directory_view_menu_inputs();

  free(ob); //freeing memory
  return 0;
}

int global_menu()
{
  clear();

  printMenu(0, 0, globalMenuText);

  refresh();

  global_menu_inputs();

  return 0;
}

int setMarked(char* markedinput)
{
  int status = -1;
  if (!strcmp(markedinput, "always")){
    markedinfo = 1;
    markedauto = 0;
    status = 0;
  } else if (!strcmp(markedinput, "never")){
    markedinfo = 0;
    markedauto = 0;
    status = 0;
  } else if (!strcmp(markedinput, "auto")){
    markedinfo = 0;
    markedauto = 1;
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

int themeSelect(char* themeinput){
  if (!strcmp(themeinput, "default")){
    colormode = 0;
  } else if (!strcmp(themeinput, "monochrome")){
    colormode = 1;
  } else if (!strcmp(themeinput, "nt")){
    colormode = 2;
  } else {
    colormode = -1;
  }
  return colormode;
}

void refreshScreen()
{
  endwin();
  clear();
  refresh();
  initscr();
  //mvprintw(0,0,"%d:%d", LINES, COLS);
  if (COLS < 89){
    strcpy(functionMenuText, functionMenuTextShort);
  } else {
    strcpy(functionMenuText, functionMenuTextLong);
  }
  switch(viewMode)
    {
    case 0:
      printMenu(0, 0, fileMenuText);
      printMenu(LINES-1, 0, functionMenuText);
      resizeDisplayDir(ob);
      break;
    case 1:
      printMenu(0, 0, globalMenuText);
      printMenu(LINES-1, 0, functionMenuText);
      resizeDisplayDir(ob);
      break;
    case 2:
      printMenu(0, 0, modifyMenuText);
      printMenu(LINES-1, 0, functionMenuText);
      resizeDisplayDir(ob);
      break;
    case 3:
      printMenu(0, 0, sortMenuText);
      printMenu(LINES-1, 0, functionMenuText);
      resizeDisplayDir(ob);
      break;
    case 4:
      printMenu(0, 0, globalMenuText);
      break;
    }
}

void sigwinchHandle(int sig){
  refreshScreen();
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
                               valid themes.\n\
      --monochrome             compatability mode for monochrome displays\n\
                               (deprecated)\n\
      --no-danger              turns off danger colors when running with\n\
                               elevated privileges\n\
      --marked=[MARKED]        shows information about marked objects. See\n\
                               MARKED section below for valid options\n\
      --no-sf                  does not display files in sf\n"), stdout);
  fputs (("\n\
The THEME argument can be:\n\
               default:    original theme\n\
               monochrome: comaptability mode for monochrome displays\n\
               nt:         a theme that closer resembles win32 versions of\n\
                           DF-EDIT\n"), stdout);
  fputs (("\n\
The MARKED argument can be: always; never; auto.\n"), stdout);
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

  // Check if we're root to display danger
  if (uid == 0 || euid == 0){
    danger = 1;
  }

  // Read the config

  readConfig();

  // Check for theme env variable
  if ( getenv("DFS_THEME")) {
    if (themeSelect(getenv("DFS_THEME")) != -1 ){
      colormode = themeSelect(getenv("DFS_THEME"));
    }
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
         {"monochrome",     no_argument,       0, GETOPT_MONOCHROME_CHAR},
         {"no-danger",      no_argument,       0, GETOPT_NODANGER_CHAR},
         {"color",          optional_argument, 0, GETOPT_COLOR_CHAR},
         {"theme",          optional_argument, 0, GETOPT_THEME_CHAR},
         {"marked",         optional_argument, 0, GETOPT_MARKED_CHAR},
         {"no-sf",          no_argument,       0, GETOPT_ENVPAGER_CHAR},
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
        if (themeSelect(optarg) == -1 ){
          printf("%s: invalid argument '%s' for 'theme'\n", argv[0], optarg);
          fputs (("\
Valid arguments are:\n\
  - default\n\
  - monochrome\n\
  - nt\n"), stdout);
          printf("Try '%s --help' for more information.\n", argv[0]);
          exit(2);
        } else {
          strcpy(themeEnv,"DFS_THEME=");
          strcat(themeEnv,optarg);
          putenv(themeEnv);
        }
      } else {
        colormode = 0;
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
    case GETOPT_MONOCHROME_CHAR:
      colormode = 1;
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
  strcpy(fileMenuText, "!Copy, !Delete, !Edit, !Hidden, !Modify, !Quit, !Rename, !Show, h!Unt, e!Xec");
  strcpy(globalMenuText, "c!Olors, !Run command, !Edit file, !Help, !Make dir, !Quit, !Show dir");
  strcpy(functionMenuTextShort, "<F1>-Down <F2>-Up <F3>-Top <F4>-Bottom <F5>-Refresh <F6>-Mark/Unmark <F7>-All <F8>-None <F9>-Sort");
  strcpy(functionMenuTextLong, "<F1>-Down <F2>-Up <F3>-Top <F4>-Bottom <F5>-Refresh <F6>-Mark/Unmark <F7>-All <F8>-None <F9>-Sort <F10>-Block");
  strcpy(modifyMenuText, "Modify: !Owner/Group, !Permissions");
  strcpy(sortMenuText, "Sort list by - !Date & time, !Name, !Size");

  set_escdelay(10);
  //ESCDELAY = 10;

  setlocale(LC_ALL, "");

  initscr();

  // Decide which function menu needs initially printing
  if (COLS < 89){
    strcpy(functionMenuText, functionMenuTextShort);
  } else {
    strcpy(functionMenuText, functionMenuTextLong);
  }

  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigwinchHandle;
  sigaction(SIGWINCH, &sa, NULL);

  start_color();
  cbreak(); //Added for new method
  setColorMode(colormode);
  bkgd(COLOR_PAIR(DISPLAY_PAIR));
  cbreak();
  // nodelay(stdscr, TRUE);
  noecho();
  curs_set(FALSE); // Hide Curser (Will want to bring it back later)
  keypad(stdscr, TRUE);



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
    global_menu();
  }
  testSlash:
  if (check_last_char(currentpwd, "/") && strcmp(currentpwd, "/")){
    currentpwd[strlen(currentpwd) - 1] = '\0';
    goto testSlash;
  }
  directory_view(currentpwd);
  return 0;
}
