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
#include <ctype.h>
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

int readline(char *buffer, int buflen, char *oldbuf)
/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
{
  int old_curs = curs_set(1);
  int pos;
  int len;
  int oldlen;
  int x, y, c;
  int status = 0;

  oldlen = strlen(oldbuf);
  setColors(INPUT_PAIR);
  // attron(COLOR_PAIR(INPUT_PAIR));

  pos = oldlen;
  len = oldlen;

  getyx(stdscr, y, x);

  strcpy(buffer, oldbuf);

  for (;;) {

    buffer[len] = ' ';
    mvaddnstr(y, x, buffer, len+1); // Prints buffer on screen
    move(y, x+pos); //
    c = getch();

    if (c == KEY_ENTER || c == '\n' || c == '\r') {
      // attron(COLOR_PAIR(COMMAND_PAIR));
      setColors(COMMAND_PAIR);
      break;
    } else if (isprint(c)) {
      if (pos < buflen-1) {
        memmove(buffer+pos+1, buffer+pos, len-pos);
        buffer[pos++] = c;
        len += 1;
      } else {
        beep();
      }
    } else if (c == KEY_LEFT) {
      if (pos > 0) pos -= 1; else beep();
    } else if (c == KEY_RIGHT) {
      if (pos < len) pos += 1; else beep();
    } else if ((c == KEY_BACKSPACE) || (c == 127)) {
      if (pos > 0) {
        memmove(buffer+pos-1, buffer+pos, len-pos);
        pos -= 1;
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == KEY_DC) {
      if (pos < len) {
        memmove(buffer+pos, buffer+pos+1, len-pos-1);
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == 270) {
      // F6 deletes to the end of line. Recently discovered in DF-EDIT 2.3d hidden in program documentation (2018-08-18)
      if ( pos < len ) {
        memmove(buffer+pos, buffer+pos+(len-pos), 0);
        len = pos;
        clrtoeol();
      }
    } else if (c == 27) {
      //pos = oldlen;
      //len = oldlen;
      //strcpy(buffer, oldbuf); //abort
      status = -1;
      pos = 0;
      len = 0;
      strcpy(buffer, ""); //abort by blanking
      // attron(COLOR_PAIR(COMMAND_PAIR));
      setColors(COMMAND_PAIR);
      break;
    } else {
      beep();
    }
  }
  buffer[len] = '\0';
  if (old_curs != ERR) curs_set(old_curs);
  return(status);
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
