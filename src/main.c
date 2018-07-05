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
#include "config.h"
#include "functions.h"
#include "views.h"
#include "menus.h"
#include "main.h"

char currentpwd[1024];

char fileMenuText[256];
char globalMenuText[256];
char functionMenuText[256];
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

extern results* ob;
extern int topfileref;
extern int selected;
extern int totalfilecount;
extern char sortmode[5];
extern int showhidden;

struct sigaction sa;

int checkStyle(char* styleinput)
{
  if ( (strcmp(styleinput, "long-iso") && (strcmp(styleinput, "full-iso") && strcmp(styleinput, "iso") && strcmp(styleinput, "locale") ))){
      return 0;
    } else {
      return 1;
    }
}

void refreshScreen()
{
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
  endwin();
  clear();
  refresh();
  initscr();
  //mvprintw(0,0,"%d:%d", LINES, COLS);
  refreshScreen();
}


int exittoshell()
{
  history *hs;
  clear();
  endwin();
  free(hs);
  exit(0);
  return 0;
}

void printHelp(char* programName){
  printf (("Usage: %s [OPTION]... [FILE]...\n"), programName);
  fputs (("\n\
DF-SHOW: An interactive directory/file browser written for Unix-like systems.\n\
Based on the SHOW application from the PC-DOS DF-EDIT suite by Larry Kroeker.\n"), stdout);
  fputs (("\n\
Sorts objects alphabetically if -St is not set.\n\
"), stdout);
  fputs (("\n\
Application Options:\n\
  -a, --all                    do not ignore entries starting with .\n\
      --author                 prints the author of each file\n\
  -B, --ignore-backups         do not list implied entries ending with ~\n\
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
      --help                   displays this help message, then exits\n\
      --version                displays version, then exits\n"), stdout);
  fputs (("\n\
The TIME_STYLE arguement can be: full-iso; long-iso; iso; locale.\n"), stdout);
  fputs (("\n\
Exit status:\n\
 0  if OK,\n\
 1  if minor problems (e.g., cannot access subdirectory),\n\
 2  if major problems (e.g., cannot access command-line arguement).\n"), stdout);
  printf ("\nPlease report any bugs to: <%s>\n", PACKAGE_BUGREPORT);
}

void printVersion(char* programName){
  printf (("%s %s\n"), programName, VERSION);
  fputs (("\
Copyright (C) 2018 Robert Ian Hawdon\n\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n\
This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n\
are welcome to redistribute it under certain conditions.\n"), stdout);
}

int main(int argc, char *argv[])
{

  int c;

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
    case GETOPT_HELP_CHAR:
      printHelp(argv[0]);
      exit(0);
      break;
    case GETOPT_VERSION_CHAR:
      printVersion(argv[0]);
      exit(0);
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
  strcpy(fileMenuText, "!Copy, !Delete, !Edit, !Hidden, !Modify, !Quit, !Rename, !Show");
  strcpy(globalMenuText, "!Run command, !Edit file, !Help, !Make dir, !Quit, !Show dir");
  strcpy(functionMenuText, "<F1>-Down <F2>-Up <F3>-Top <F4>-Bottom <F5>-Refresh <F6>-Mark/Unmark <F7>-All <F8>-None <F9>-Sort");
  strcpy(modifyMenuText, "Modify: !Owner/Group, !Permissions");
  strcpy(sortMenuText, "Sort list by - !Date & time, !Name, !Size");

  // set_escdelay(10);
  ESCDELAY = 10;

  setlocale(LC_ALL, "");

  initscr();


  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigwinchHandle;
  sigaction(SIGWINCH, &sa, NULL);

  start_color();
  cbreak(); //Added for new method
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
  init_pair(4, COLOR_BLUE, COLOR_BLACK);
  init_pair(5, COLOR_RED, COLOR_BLACK);
  cbreak();
  // nodelay(stdscr, TRUE);
  noecho();
  curs_set(FALSE); // Hide Curser (Will want to bring it back later)
  keypad(stdscr, TRUE);



  // Remaining arguments passed as working directory
  if (optind < argc){
    strcpy(currentpwd, argv[optind]);
    chdir(currentpwd);
  } else {
    getcwd(currentpwd, sizeof(currentpwd));
  }

  if (!check_dir(currentpwd)){
    //strcpy(currentpwd, "/"); // If dir doesn't exist, default to root
    quit_menu();
  }

  directory_view(currentpwd);
  return 0;
}
