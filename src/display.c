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
#include <string.h>
#include <wchar.h>
#include "colors.h"
#include "banned.h"

extern int * pc;
extern MEVENT event;

void wPrintLine(int line, int col, wchar_t *textString){
  int i;
  move(line,col);
  clrtoeol();
  for ( i = 0; i < wcslen(textString) ; i++){
    mvprintw(line, col + i, "%lc", textString[i]);
    if ( (col + i) == COLS ){
      break;
    }
  }
}

void printLine(int line, int col, char *textString){
  // Small wrapper to seemlessly forward calls to the wide char version
  wchar_t *wTextString;
  wTextString = malloc( sizeof ( wchar_t ) * (strlen(textString) + 1));
  swprintf(wTextString, strlen(textString) + 1, L"%s", textString);
  wPrintLine(line, col, wTextString);
  free(wTextString);
}


void topLineMessage(const char *message){
  bool secondClick = false; // Hacky Hack McHacknessness!
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
        case -1:
          break;
        case KEY_MOUSE:
          // And here comes the second part of the hack!
          if (secondClick){
            return;
            break;
          } else {
            secondClick = true;
            break;
          }
        default: // Where's the "any" key?
          return;
          break;
        }
    }
}

void clear_workspace()
{
  size_t line_count = 1;
  for (line_count = 1; line_count < (LINES - 1);)
    {
      move (line_count,0);
      clrtoeol();
      line_count++;
    }
}
