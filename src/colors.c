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
#include "common.h"
#include "colors.h"

int lightColorPair[15];
// int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL = 0;

extern int colormode;

void setColorMode(int mode){
  use_default_colors();
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
    init_pair(COMMAND_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[COMMAND_PAIR] = 0;
    init_pair(INFO_PAIR, COLOR_GREEN, DEFAULT_COLOR);
    lightColorPair[INFO_PAIR] = 0;
    init_pair(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[INPUT_PAIR] = 0;
    init_pair(SELECT_PAIR, BRIGHT_BLUE, DEFAULT_COLOR);
    lightColorPair[SELECT_PAIR] = 0;
    init_pair(DISPLAY_PAIR, COLOR_CYAN, DEFAULT_COLOR);
    lightColorPair[DISPLAY_PAIR] = 0;
    init_pair(DANGER_PAIR, BRIGHT_RED, DEFAULT_COLOR);
    lightColorPair[DANGER_PAIR] = 0;
    init_pair(DIR_PAIR, BRIGHT_MAGENTA, DEFAULT_COLOR);
    lightColorPair[DIR_PAIR] = 0;
    init_pair(SLINK_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[SLINK_PAIR] = 1;
    init_pair(EXE_PAIR, BRIGHT_YELLOW, DEFAULT_COLOR);
    lightColorPair[EXE_PAIR] = 0;
    init_pair(SUID_PAIR, DEFAULT_COLOR, COLOR_RED);
    lightColorPair[SUID_PAIR] = 0;
    init_pair(SGID_PAIR, BRIGHT_BLACK, COLOR_GREEN);
    lightColorPair[SGID_PAIR] = 0;
    init_pair(HILITE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[HILITE_PAIR] = 1;
    init_pair(ERROR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[ERROR_PAIR] = 1;
    init_pair(HEADING_PAIR, COLOR_GREEN, DEFAULT_COLOR);
    lightColorPair[HEADING_PAIR] = 0;
    init_pair(DEADLINK_PAIR, BRIGHT_RED, DEFAULT_COLOR);
    lightColorPair[DEADLINK_PAIR] = 0;
    break;
  case 1:
    init_pair(COMMAND_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[COMMAND_PAIR] = 0;
    init_pair(INFO_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[INFO_PAIR] = 0;
    init_pair(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[INPUT_PAIR] = 0;
    init_pair(SELECT_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[SELECT_PAIR] = 0;
    init_pair(DISPLAY_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[DISPLAY_PAIR] = 0;
    init_pair(DANGER_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[DANGER_PAIR] = 0;
    init_pair(DIR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[DIR_PAIR] = 1;
    init_pair(SLINK_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[SLINK_PAIR] = 1;
    init_pair(EXE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[EXE_PAIR] = 1;
    init_pair(SUID_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[SUID_PAIR] = 0;
    init_pair(SGID_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[SGID_PAIR] = 1;
    init_pair(HILITE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[HILITE_PAIR] = 1;
    init_pair(ERROR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[ERROR_PAIR] = 1;
    init_pair(HEADING_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
    lightColorPair[HEADING_PAIR] = 0;
    init_pair(DEADLINK_PAIR, COLOR_BLACK, COLOR_WHITE);
    lightColorPair[DEADLINK_PAIR] = 0;
    break;
  case 2:
    init_pair(COMMAND_PAIR, BRIGHT_CYAN, COLOR_BLUE);
    lightColorPair[COMMAND_PAIR] = 0;
    init_pair(INFO_PAIR, BRIGHT_WHITE, COLOR_BLUE);
    lightColorPair[INFO_PAIR] = 0;
    init_pair(INPUT_PAIR, COLOR_BLUE, COLOR_WHITE);
    lightColorPair[INPUT_PAIR] = 0;
    init_pair(SELECT_PAIR, COLOR_BLUE, COLOR_WHITE);
    lightColorPair[SELECT_PAIR] = 0;
    init_pair(DISPLAY_PAIR, BRIGHT_CYAN, COLOR_BLUE);
    lightColorPair[DISPLAY_PAIR] = 0;
    init_pair(DANGER_PAIR, BRIGHT_RED, COLOR_BLUE);
    lightColorPair[DANGER_PAIR] = 0;
    init_pair(DIR_PAIR, BRIGHT_MAGENTA, COLOR_BLUE);
    lightColorPair[DIR_PAIR] = 0;
    init_pair(SLINK_PAIR, BRIGHT_WHITE, COLOR_BLUE);
    lightColorPair[SLINK_PAIR] = 0;
    init_pair(EXE_PAIR, BRIGHT_YELLOW, COLOR_BLUE);
    lightColorPair[EXE_PAIR] = 0;
    init_pair(SUID_PAIR, COLOR_WHITE, COLOR_RED);
    lightColorPair[SUID_PAIR] = 0;
    init_pair(SGID_PAIR, BRIGHT_BLACK, COLOR_GREEN);
    lightColorPair[SGID_PAIR] = 0;
    init_pair(HILITE_PAIR, BRIGHT_YELLOW, COLOR_BLUE);
    lightColorPair[HILITE_PAIR] = 0;
    init_pair(ERROR_PAIR, BRIGHT_RED, COLOR_BLUE);
    lightColorPair[ERROR_PAIR] = 0;
    init_pair(HEADING_PAIR, BRIGHT_YELLOW, COLOR_BLUE);
    lightColorPair[HEADING_PAIR] = 0;
    init_pair(DEADLINK_PAIR, BRIGHT_RED, COLOR_BLUE);
    lightColorPair[DEADLINK_PAIR] = 0;
    break;
  }
  init_pair(COLORMENU_PAIR_0, COLOR_BLACK, COLOR_WHITE);
  init_pair(COLORMENU_PAIR_1, COLOR_RED, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_2, COLOR_GREEN, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_3, COLOR_YELLOW, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_4, COLOR_BLUE, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_5, COLOR_MAGENTA, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_6, COLOR_CYAN, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_7, COLOR_WHITE, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_8, BRIGHT_BLACK, COLOR_WHITE);
  init_pair(COLORMENU_PAIR_9, BRIGHT_RED, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_A, BRIGHT_GREEN, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_B, BRIGHT_YELLOW, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_C, BRIGHT_BLUE, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_D, BRIGHT_MAGENTA, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_E, BRIGHT_CYAN, DEFAULT_COLOR);
  init_pair(COLORMENU_PAIR_F, BRIGHT_WHITE, DEFAULT_COLOR);

  init_pair(DEFAULT_COLOR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR);
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

void themeBuilder()
{
  clear();
  printMenu(0, 0, "Color number, !Help, !Load, !Quit, !Save");

  setColors(COMMAND_PAIR);
  mvprintw(2, 4, "Command lines");
  setColors(DISPLAY_PAIR);
  mvprintw(3, 4, "Display lines");
  setColors(ERROR_PAIR);
  mvprintw(4, 4, "Error messages");
  setColors(INFO_PAIR);
  mvprintw(5, 4, "Information lines");
  setColors(DANGER_PAIR);
  mvprintw(6, 4, "Danger lines");
  setColors(SELECT_PAIR);
  mvprintw(7, 4, "Selected block lines");
  setColors(HILITE_PAIR);
  mvprintw(8, 4, "Highlight");

  attron(COLOR_PAIR(DEFAULT_COLOR_PAIR));
  mvprintw(2, 45, "!-Default");
  attron(COLOR_PAIR(COLORMENU_PAIR_0));
  mvprintw(3, 45, "0-Black");
  attron(COLOR_PAIR(COLORMENU_PAIR_1));
  mvprintw(4, 45, "1-Red");
  attron(COLOR_PAIR(COLORMENU_PAIR_2));
  mvprintw(5, 45, "2-Green");
  attron(COLOR_PAIR(COLORMENU_PAIR_3));
  mvprintw(6, 45, "3-Yellow");
  attron(COLOR_PAIR(COLORMENU_PAIR_4));
  mvprintw(7, 45, "4-Blue");
  attron(COLOR_PAIR(COLORMENU_PAIR_5));
  mvprintw(8, 45, "5-Magenta");
  attron(COLOR_PAIR(COLORMENU_PAIR_6));
  mvprintw(9, 45, "6-Cyan");
  attron(COLOR_PAIR(COLORMENU_PAIR_7));
  mvprintw(10, 45, "7-White");
  attron(COLOR_PAIR(COLORMENU_PAIR_8));
  mvprintw(11, 45, "8-Bright Black");
  attron(COLOR_PAIR(COLORMENU_PAIR_9));
  mvprintw(12, 45, "9-Bright Red");
  attron(COLOR_PAIR(COLORMENU_PAIR_A));
  mvprintw(13, 45, "A-Bright Green");
  attron(COLOR_PAIR(COLORMENU_PAIR_B));
  mvprintw(14, 45, "B-Bright Yellow");
  attron(COLOR_PAIR(COLORMENU_PAIR_C));
  mvprintw(15, 45, "C-Bright Blue");
  attron(COLOR_PAIR(COLORMENU_PAIR_D));
  mvprintw(16, 45, "D-Bright Magenta");
  attron(COLOR_PAIR(COLORMENU_PAIR_E));
  mvprintw(17, 45, "E-Bright Cyan");
  attron(COLOR_PAIR(COLORMENU_PAIR_F));
  mvprintw(18, 45, "F-Bright White");

  setColors(ERROR_PAIR);
  mvprintw(21, 22, "Select 0 to F for desired foreground color");
  mvprintw(22, 22, "Use alt-0 to alt-F for background color");

}
