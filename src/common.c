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
#include <string.h>
#include "colors.h"
#include "config.h"

extern int * pc;

extern char fileMenuText[256];

int exittoshell()
{
  clear();
  endwin();
  exit(0);
  return 0;
}

void printVersion(char* programName){
  printf (("%s %s\n"), programName, VERSION);
  fputs (("\
Copyright (C) 2018 Robert Ian Hawdon\n\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n\
This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n\
are welcome to redistribute it under certain conditions.\n"), stdout);
}

void printMenu(int line, int col, char *menustring)
{
  int i, len, charcount;
  charcount = 0;
  move(line, col);
  clrtoeol();
  len = strlen(menustring);
  setColors(COMMAND_PAIR);
  for (i = 0; i < len; i++)
     {
      if ( menustring[i] == '!' ) {
          i++;
          setColors(HILITE_PAIR);
          mvprintw(line, col + charcount, "%c", menustring[i]);
          setColors(COMMAND_PAIR);
          charcount++;
      } else if ( menustring[i] == '<' ) {
          i++;
          setColors(HILITE_PAIR);
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
      } else if ( menustring[i] == '>' ) {
          i++;
          setColors(COMMAND_PAIR);
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
      } else if ( menustring[i] == '\\' ) {
          i++;
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
      } else {
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
        }
    }
}

void topLineMessage(const char *message){
  move(0,0);
  clrtoeol();
  setColors(ERROR_PAIR);
  mvprintw(0,0, "%s", message);
  setColors(COMMAND_PAIR);
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        default: // Where's the "any" key?
          printMenu(0, 0, fileMenuText);
          return;
          //directory_view_menu_inputs();
          break;
        }
    }
}

