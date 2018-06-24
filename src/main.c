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

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include "functions.h"
#include "views.h"
#include "menus.h"

char currentpwd[1024];

char fileMenuText[256];
char globalMenuText[256];
char functionMenuText[256];
char modifyMenuText[256];
char sortMenuText[256];

int viewMode = 0;

extern results* ob;
extern int topfileref;
extern int selected;
extern int totalfilecount;
extern char sortmode[5];
extern int showhidden;

struct sigaction sa;

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
         {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long(argc, argv, "aA", long_options, &option_index);

      if ( c == -1 ){
        break;
      }

    switch(c){
    case 'A':
      // Dropthourgh
    case 'a':
      showhidden = 1;
      break;
    default:
      // abort();
      exit(2);
    }
  }




  // Writing Menus
  strcpy(fileMenuText, "!Copy, !Delete, !Edit, !Hidden, !Modify, !Quit, !Rename, !Show");
  strcpy(globalMenuText, "!Run command, !Edit file, !Help, !Make dir, !Quit, !Show dir");
  strcpy(functionMenuText, "<F1>-Down <F2>-Up <F3>-Top <F4>-Bottom <F5>-Refresh <F6>-Mark/Unmark <F7>-All <F8>-None <F9>-Sort");
  strcpy(modifyMenuText, "Modify: !Owner/Group, !Permissions");
  strcpy(sortMenuText, "Sort list by - !Date & time, !Name, !Size");

  set_escdelay(10);

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
