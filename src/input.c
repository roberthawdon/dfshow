/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2023  Robert Ian Hawdon

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
#include <wchar.h>
#include <wctype.h>
#include "colors.h"
#include "banned.h"

int wReadLine(wchar_t *buffer, int buflen, wchar_t *oldbuf)
/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
{
  int old_curs = curs_set(1);
  int pos;
  int len;
  int over = 0;
  int oldlen;
  int x, y;
  int i;
  int status = 0;
  wint_t c;

  oldlen = wcslen(oldbuf);

  pos = oldlen;
  len = oldlen;

  getyx(stdscr, y, x);

  wcscpy(buffer, oldbuf);

  for (;;) {

    buffer[len] = ' ';
    for (i = 0; i < len; i++){
      setColors(INPUT_PAIR);
      mvprintw(y, x + i, "%lc", buffer[i + over]); // Prints buffer on screen
      setColors(COMMAND_PAIR);
      mvprintw(y, x + i + 1, "%lc", ' '); // Ensuring the last cursor char is blanked
    }
    if (len < 1){
      setColors(COMMAND_PAIR);
      mvprintw(y, x, "%lc", ' '); // Blacking if there's nothing in the buffer
    }
    if ((x + pos) < COLS){
      move(y, x+pos); //
      over = 0; // Just to be sure
    } else {
      move(y, COLS - 1);
      over = (x+pos) - COLS;
    }
    get_wch(&c);

    if (c == KEY_ENTER || c == '\n' || c == '\r') {
      // Enter Key
      setColors(COMMAND_PAIR);
      break;
    } else if (c == KEY_LEFT) {
      // Left Key
      if (pos > 0) pos -= 1; else beep();
    } else if (c == KEY_RIGHT) {
      // Right Key
      if (pos < len) pos += 1; else beep();
    } else if ((c == KEY_BACKSPACE) || (c == 127)) {
      // Backspace Key
      if (pos > 0) {
        wmemmove(buffer+pos-1, buffer+pos, len-pos);
        pos -= 1;
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == KEY_DC) {
      // Delete Key
      if (pos < len) {
        wmemmove(buffer+pos, buffer+pos+1, len-pos-1);
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == 262) {
      // Home Key
      if (pos > 0) pos = 0; else beep();
    } else if (c == 360) {
      // End Key
      if (pos < len) pos = len; else beep();
    } else if (c == 270) {
      // F6 Key
      // F6 deletes to the end of line. Recently discovered in DF-EDIT 2.3d hidden in program documentation (2018-08-18)
      if ( pos < len ) {
        wmemmove(buffer+pos, buffer+pos+(len-pos), 0);
        len = pos;
        clrtoeol();
      }
    } else if (c == 27) {
      // ESC Key
      status = -1;
      pos = 0;
      len = 0;
      wcscpy(buffer, L""); //abort by blanking
      setColors(COMMAND_PAIR);
      break;
    } else if ((c == KEY_UP) || (c == KEY_DOWN) || (c == KEY_IC) || (c == 265) || (c == 266) || (c == 267) || (c == 268) || (c == 269) || (c == 270) || (c == 271) || (c == 272) || (c == 273) || (c == 274) || (c == 275) || (c == 276) || (c == 338) || (c == 339)) {
      // Ignore navigation and function keys.
      continue;
    } else if (iswprint(c)) {
      // Anything else that can be printed.
      if (pos < buflen-1) {
        wmemmove(buffer+pos+1, buffer+pos, len-pos);
        buffer[pos++] = c;
        len += 1;
      } else {
        beep();
      }
    } else if (c == 0) {
      // Do nothing
    } else {
      beep();
    }
  }
  buffer[len] = '\0';
  if (old_curs != ERR) curs_set(old_curs);
  return(status);
}

int readline(char *buffer, int buflen, char *oldbuf)
{

  // Small wrapper to seemlessly forward calls to the wide char version
  wchar_t *wBuffer, *wOldBuf;
  int status;
  wBuffer = malloc(sizeof(wchar_t) * buflen);
  wOldBuf = malloc(sizeof(wchar_t) * (strlen(oldbuf) + 1));
  swprintf(wOldBuf, strlen(oldbuf) + 1, L"%s", oldbuf);
  status = wReadLine(wBuffer, buflen, wOldBuf);
  snprintf(buffer, buflen, "%ls", wBuffer);
  free(wBuffer);
  free(wOldBuf);
  return(status);
}
