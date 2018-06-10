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
#include "functions.h"
#include "vars.h"
#include "views.h"

char currentpwd[1024];

char fileMenuText[256];
char globalMenuText[256];
char functionMenuText[256];
char modifyMenuText[256];
char sortMenuText[256];


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

  // Writing Menus
  strcpy(fileMenuText, "!Copy, !Delete, !Edit, !Hidden, !Modify, !Quit, !Rename, !Show");
  strcpy(globalMenuText, "!Change dir, !Run command, !Edit file, !Help, !Make dir, !Quit, !Show dir");
  strcpy(functionMenuText, "<F1>-Down <F2>-Up <F3>-Top <F4>-Bottom <F5>-Refresh <F6>-Mark/Unmark <F7>-All <F8>-None <F9>-Sort");
  strcpy(modifyMenuText, "Modify: !Owner/Group, !Permissions");
  strcpy(sortMenuText, "Sort list by - !Date & time, !Name, !Size");

  set_escdelay(10);

  setlocale(LC_ALL, "");

  initscr();
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

  if ( argc < 2 ){
    getcwd(currentpwd, sizeof(currentpwd));
  } else {
    strcpy(currentpwd, argv[1]);
    chdir(currentpwd);
  }
  if (!check_dir(currentpwd)){
    //strcpy(currentpwd, "/"); // If dir doesn't exist, default to root
    quit_menu();
  }

  directory_view(currentpwd);
  return 0;
}
