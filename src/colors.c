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

int lightColorPair[13];

// int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL = 0;

extern int colormode;

void setColorMode(int mode){
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
    lightColorPair[0] = 0;
    init_pair(INFO_PAIR, COLOR_GREEN, COLOR_BLACK);
    lightColorPair[1] = 0;
    init_pair(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[2] = 0;
    init_pair(SELECT_PAIR, COLOR_BLUE, COLOR_BLACK);
    lightColorPair[3] = 1;
    init_pair(DISPLAY_PAIR, COLOR_CYAN, COLOR_BLACK);
    lightColorPair[4] = 0;
    init_pair(DANGER_PAIR, COLOR_RED, COLOR_BLACK);
    lightColorPair[5] = 1;
    init_pair(DIR_PAIR, COLOR_MAGENTA, COLOR_BLACK);
    lightColorPair[6] = 1;
    init_pair(SLINK_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[7] = 1;
    init_pair(EXE_PAIR, COLOR_YELLOW, COLOR_BLACK);
    lightColorPair[8] = 1;
    init_pair(SUID_PAIR, COLOR_WHITE, COLOR_RED);
    lightColorPair[9] = 0;
    init_pair(SGID_PAIR, COLOR_BLACK, COLOR_GREEN);
    lightColorPair[10] = 1;
    init_pair(HILITE_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[11] = 1;
    init_pair(ERROR_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[12] = 1;
    break;
  case 1:
    init_pair(COMMAND_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[0] = 0;
    init_pair(INFO_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[1] = 0;
    init_pair(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[2] = 0;
    init_pair(SELECT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[3] = 0;
    init_pair(DISPLAY_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[4] = 0;
    init_pair(DANGER_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[5] = 0;
    init_pair(DIR_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[6] = 1;
    init_pair(SLINK_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[7] = 1;
    init_pair(EXE_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[8] = 1;
    init_pair(SUID_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[9] = 0;
    init_pair(SGID_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[10] = 1;
    init_pair(HILITE_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[11] = 1;
    init_pair(ERROR_PAIR, COLOR_WHITE, COLOR_BLACK);
    lightColorPair[12] = 1;
    // dirL, slinkL, exeL, sgidL, hiliteL = 1;
    break;
  case 2:
    init_pair(COMMAND_PAIR, COLOR_CYAN, COLOR_BLUE);
    lightColorPair[0] = 0;
    init_pair(INFO_PAIR, COLOR_YELLOW, COLOR_BLUE);
    lightColorPair[1] = 1;
    init_pair(INPUT_PAIR, COLOR_BLUE, COLOR_WHITE);
    lightColorPair[2] = 0;
    init_pair(SELECT_PAIR, COLOR_BLUE, COLOR_WHITE);
    lightColorPair[3] = 0;
    init_pair(DISPLAY_PAIR, COLOR_CYAN, COLOR_BLUE);
    lightColorPair[4] = 0;
    init_pair(DANGER_PAIR, COLOR_RED, COLOR_BLUE);
    lightColorPair[5] = 0;
    init_pair(DIR_PAIR, COLOR_MAGENTA, COLOR_BLUE);
    lightColorPair[6] = 1;
    init_pair(SLINK_PAIR, COLOR_WHITE, COLOR_BLUE);
    lightColorPair[7] = 1;
    init_pair(EXE_PAIR, COLOR_YELLOW, COLOR_BLUE);
    lightColorPair[8] = 1;
    init_pair(SUID_PAIR, COLOR_WHITE, COLOR_RED);
    lightColorPair[9] = 0;
    init_pair(SGID_PAIR, COLOR_BLACK, COLOR_GREEN);
    lightColorPair[10] = 1;
    init_pair(HILITE_PAIR, COLOR_YELLOW, COLOR_BLUE);
    lightColorPair[11] = 1;
    init_pair(ERROR_PAIR, COLOR_RED, COLOR_BLUE);
    lightColorPair[12] = 1;
    // infoL, dirL, slinkL, exeL, sgidL, hiliteL = 1;
    break;
  }
}

void setColors(int pair)
{
  attron(COLOR_PAIR(pair));
  if (lightColorPair[pair - 1]){
    attron(A_BOLD);
  } else {
    attroff(A_BOLD);
  }
}
