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
#include <ncurses.h>
#include "colors.h"

int lightColorPair[14];

// int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL = 0;

extern int colormode;

void setColorMode(int mode){
  lightColorPair[0] = 0; // Unused array value
  /*
    Pairs:
    1    : Command Lines
    2    : Information Lines
    3    : Text Input
    4    : Selected Block Lines
    5    : Display Lines
    6    : Danger Lines
    7    : Directory
    8    : Symlink
    9    : Executable
    10   : SUID
    11   : SGID
    12   : Highlight Pair
   */
  switch(mode){
  case 0:
    init_pair(COMMAND_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[COMMAND_PAIR] = 0;
    init_pair(INFO_PAIR, COLOR_GREEN, COLOR_BLACK);
    lightColorPair[INFO_PAIR] = 0;
    init_pair(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[INPUT_PAIR] = 0;
    init_pair(SELECT_PAIR, COLOR_BLUE, COLOR_BLACK);
    lightColorPair[SELECT_PAIR] = 1;
    init_pair(DISPLAY_PAIR, COLOR_CYAN, COLOR_BLACK);
    lightColorPair[DISPLAY_PAIR] = 0;
    init_pair(DANGER_PAIR, COLOR_RED, COLOR_BLACK);
    lightColorPair[DANGER_PAIR] = 1;
    init_pair(DIR_PAIR, COLOR_MAGENTA, COLOR_BLACK);
    lightColorPair[DIR_PAIR] = 1;
    init_pair(SLINK_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[SLINK_PAIR] = 1;
    init_pair(EXE_PAIR, COLOR_YELLOW, COLOR_BLACK);
    lightColorPair[EXE_PAIR] = 1;
    init_pair(SUID_PAIR, COLOR_WHITE, COLOR_RED);
    lightColorPair[SUID_PAIR] = 0;
    init_pair(SGID_PAIR, COLOR_BLACK, COLOR_GREEN);
    lightColorPair[SGID_PAIR] = 1;
    init_pair(HILITE_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[HILITE_PAIR] = 1;
    init_pair(ERROR_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[ERROR_PAIR] = 1;
    break;
  case 1:
    init_pair(COMMAND_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[COMMAND_PAIR] = 0;
    init_pair(INFO_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[INFO_PAIR] = 0;
    init_pair(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[INPUT_PAIR] = 0;
    init_pair(SELECT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[SELECT_PAIR] = 0;
    init_pair(DISPLAY_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[DISPLAY_PAIR] = 0;
    init_pair(DANGER_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[DANGER_PAIR] = 0;
    init_pair(DIR_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[DIR_PAIR] = 1;
    init_pair(SLINK_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[SLINK_PAIR] = 1;
    init_pair(EXE_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[EXE_PAIR] = 1;
    init_pair(SUID_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[SUID_PAIR] = 0;
    init_pair(SGID_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[SGID_PAIR] = 1;
    init_pair(HILITE_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[HILITE_PAIR] = 1;
    init_pair(ERROR_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[ERROR_PAIR] = 1;
    break;
  case 2:
    init_pair(COMMAND_PAIR, COLOR_CYAN, COLOR_BLUE);
    lightColorPair[COMMAND_PAIR] = 0;
    init_pair(INFO_PAIR, COLOR_YELLOW, COLOR_BLUE);
    lightColorPair[INFO_PAIR] = 1;
    init_pair(INPUT_PAIR, COLOR_BLUE, COLOR_WHITE);
    lightColorPair[INPUT_PAIR] = 0;
    init_pair(SELECT_PAIR, COLOR_BLUE, COLOR_WHITE);
    lightColorPair[SELECT_PAIR] = 0;
    init_pair(DISPLAY_PAIR, COLOR_CYAN, COLOR_BLUE);
    lightColorPair[DISPLAY_PAIR] = 0;
    init_pair(DANGER_PAIR, COLOR_RED, COLOR_BLUE);
    lightColorPair[DANGER_PAIR] = 0;
    init_pair(DIR_PAIR, COLOR_MAGENTA, COLOR_BLUE);
    lightColorPair[DIR_PAIR] = 1;
    init_pair(SLINK_PAIR, COLOR_WHITE, COLOR_BLUE);
    lightColorPair[SLINK_PAIR] = 1;
    init_pair(EXE_PAIR, COLOR_YELLOW, COLOR_BLUE);
    lightColorPair[EXE_PAIR] = 1;
    init_pair(SUID_PAIR, COLOR_WHITE, COLOR_RED);
    lightColorPair[SUID_PAIR] = 0;
    init_pair(SGID_PAIR, COLOR_BLACK, COLOR_GREEN);
    lightColorPair[SGID_PAIR] = 1;
    init_pair(HILITE_PAIR, COLOR_YELLOW, COLOR_BLUE);
    lightColorPair[HILITE_PAIR] = 1;
    init_pair(ERROR_PAIR, COLOR_RED, COLOR_BLUE);
    lightColorPair[ERROR_PAIR] = 1;
    break;
  }
}

void setColors(int pair)
{
  attron(COLOR_PAIR(pair));
  if (lightColorPair[pair]){
    attron(A_BOLD);
  } else {
    attroff(A_BOLD);
  }
}
