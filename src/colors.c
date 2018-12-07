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
#include <string.h>
#include <libconfig.h>
#include "common.h"
#include "colors.h"

int lightColorPair[256];
// int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL = 0;

int colorThemePos = 0;
int totalItemCount = 7;

int selectedItem;

int bgToggle = 0;

colorPairs colors[256];

char fgbgLabel[11];

extern int colormode;
extern int c;
extern int * pc;

int itemLookup(int menuPos){
  switch(menuPos){
  case 0:
    selectedItem = COMMAND_PAIR;
    break;
  case 1:
    selectedItem = DISPLAY_PAIR;
    break;
  case 2:
    selectedItem = ERROR_PAIR;
    break;
  case 3:
    selectedItem = INFO_PAIR;
    break;
  case 4:
    selectedItem = HEADING_PAIR;
    break;
  case 5:
    selectedItem = DANGER_PAIR;
    break;
  case 6:
    selectedItem = SELECT_PAIR;
    break;
  case 7:
    selectedItem = HILITE_PAIR;
    break;
  default:
    selectedItem = -1;
    break;
  }
  return(selectedItem);
}

void updateColorPair(int code, int location){
  int colorCode = -1;
  int colorBold = 0;
  switch(code){
  case 0:
    colorCode = COLOR_BLACK;
    break;
  case 1:
    colorCode = COLOR_RED;
    break;
  case 2:
    colorCode = COLOR_GREEN;
    break;
  case 3:
    colorCode = COLOR_YELLOW;
    break;
  case 4:
    colorCode = COLOR_BLUE;
    break;
  case 5:
    colorCode = COLOR_MAGENTA;
    break;
  case 6:
    colorCode = COLOR_CYAN;
    break;
  case 7:
    colorCode = COLOR_WHITE;
    break;
  case 8:
    if (COLORS > 8){
      colorCode = BRIGHT_BLACK;
    } else {
      colorCode = COLOR_BLACK;
      colorBold = 1;
    }
    break;
  case 9:
    if (COLORS > 8){
      colorCode = BRIGHT_RED;
    } else {
      colorCode = COLOR_RED;
      colorBold = 1;
    }
    break;
  case 10:
    if (COLORS > 8){
      colorCode = BRIGHT_GREEN;
    } else {
      colorCode = COLOR_GREEN;
      colorBold = 1;
    }
    break;
  case 11:
    if (COLORS > 8){
      colorCode = BRIGHT_YELLOW;
    } else {
      colorCode = COLOR_YELLOW;
      colorBold = 1;
    }
    break;
  case 12:
    if (COLORS > 8){
      colorCode = BRIGHT_BLUE;
    } else {
      colorCode = COLOR_BLUE;
      colorBold = 1;
    }
    break;
  case 13:
    if (COLORS > 8){
      colorCode = BRIGHT_MAGENTA;
    } else {
      colorCode = COLOR_MAGENTA;
      colorBold = 1;
    }
    break;
  case 14:
    if (COLORS > 8){
      colorCode = BRIGHT_CYAN;
    } else {
      colorCode = COLOR_CYAN;
      colorBold = 1;
    }
    break;
  case 15:
    if (COLORS > 8){
      colorCode = BRIGHT_WHITE;
    } else {
      colorCode = COLOR_WHITE;
      colorBold = 1;
    }
    break;
  case -1:
    colorCode = DEFAULT_COLOR;
    break;
  case -2:
    colorCode = DEFAULT_COLOR;
    colorBold = 1;
    break;
  }
  if (location == 0) {
    colors[itemLookup(colorThemePos)].foreground = colorCode;
  } else {
    colors[itemLookup(colorThemePos)].background = colorCode;
  }
  colors[itemLookup(colorThemePos)].bold = colorBold;
}

void theme_menu_inputs()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case '!':
          updateColorPair(-1, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '?':
          updateColorPair(-2, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '0':
          updateColorPair(COLOR_BLACK, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '1':
          updateColorPair(COLOR_RED, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '2':
          updateColorPair(COLOR_GREEN, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '3':
          updateColorPair(COLOR_YELLOW, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '4':
          updateColorPair(COLOR_BLUE, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '5':
          updateColorPair(COLOR_MAGENTA, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '6':
          updateColorPair(COLOR_CYAN, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '7':
          updateColorPair(COLOR_WHITE, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '8':
          updateColorPair(BRIGHT_BLACK, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case '9':
          updateColorPair(BRIGHT_RED, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case 'a':
          updateColorPair(BRIGHT_GREEN, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case 'b':
          updateColorPair(BRIGHT_YELLOW, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case 'c':
          updateColorPair(BRIGHT_BLUE, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case 'd':
          updateColorPair(BRIGHT_MAGENTA, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case 'e':
          updateColorPair(BRIGHT_CYAN, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case 'f':
          updateColorPair(BRIGHT_WHITE, bgToggle);
          refreshColors();
          themeBuilder();
          break;
        case 'h':
          break;
        case 'l':
          break;
        case 'q':
          curs_set(FALSE);
          exittoshell();
          break;
        case 's':
          break;
        case 't':
          if (bgToggle == 0){
            bgToggle = 1;
          } else {
            bgToggle = 0;
          }
          themeBuilder();
          break;
        case 10: // Enter - Falls through
        case 258: // Down Arrow
          if (colorThemePos < totalItemCount){
            colorThemePos++;
            themeBuilder();
          }
          break;
        case 259: // Up Arrow
          if (colorThemePos > 0) {
            colorThemePos--;
            themeBuilder();
          }
          break;
        case 260: // Left Arrow
          break;
        case 261: // Right Arrow
          break;
          // default:
          //     mvprintw(LINES-2, 1, "Character pressed is = %d Hopefully it can be printed as '%c'", c, c);
          //     refresh();
        }
    }
}

void setColorPairs(int pair, int foreground, int background, int bold){
  if (COLORS < 9){
    checkColor:
    if ( foreground > 8 ){
      foreground = foreground - 8;
      bold = 1;
      goto checkColor;
    }
    if ( background > 8 ){
      background = background - 8;
      goto checkColor;
    }
  }
  colors[pair].foreground = foreground;
  colors[pair].background = background;
  colors[pair].bold = bold;
}

void refreshColors(){
  int i;
  for ( i = 0; i < 256; i++ ){
    init_pair(i, colors[i].foreground, colors[i].background);
    lightColorPair[i] = colors[i].bold;
  }
}

void setColorMode(int mode){
  use_default_colors();
  lightColorPair[0] = 0; // Unused array value
  switch(mode){
  case 0:
    setColorPairs(COMMAND_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
    setColorPairs(INFO_PAIR, COLOR_GREEN, DEFAULT_COLOR, 0);
    setColorPairs(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE, 0);
    setColorPairs(SELECT_PAIR, BRIGHT_BLUE, DEFAULT_COLOR, 0);
    setColorPairs(DISPLAY_PAIR, COLOR_CYAN, DEFAULT_COLOR, 0);
    setColorPairs(DANGER_PAIR, BRIGHT_RED, DEFAULT_COLOR, 0);
    setColorPairs(DIR_PAIR, BRIGHT_MAGENTA, DEFAULT_COLOR, 0);
    setColorPairs(SLINK_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(EXE_PAIR, BRIGHT_YELLOW, DEFAULT_COLOR, 0);
    setColorPairs(SUID_PAIR, DEFAULT_COLOR, COLOR_RED, 0);
    setColorPairs(SGID_PAIR, BRIGHT_BLACK, COLOR_GREEN, 0);
    setColorPairs(HILITE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(ERROR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(HEADING_PAIR, COLOR_GREEN, DEFAULT_COLOR, 0);
    setColorPairs(DEADLINK_PAIR, BRIGHT_RED, DEFAULT_COLOR, 0);
    break;
  case 1:
    setColorPairs(COMMAND_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
    setColorPairs(INFO_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
    setColorPairs(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE, 0);
    setColorPairs(SELECT_PAIR, COLOR_BLACK, COLOR_WHITE, 0);
    setColorPairs(DISPLAY_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
    setColorPairs(DANGER_PAIR, COLOR_BLACK, COLOR_WHITE, 0);
    setColorPairs(DIR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(SLINK_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(EXE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(SUID_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
    setColorPairs(SGID_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(HILITE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(ERROR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
    setColorPairs(HEADING_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
    setColorPairs(DEADLINK_PAIR, COLOR_BLACK, COLOR_WHITE, 0);
    break;
  case 2:
    setColorPairs(COMMAND_PAIR, BRIGHT_CYAN, COLOR_BLUE, 0);
    setColorPairs(INFO_PAIR, BRIGHT_WHITE, COLOR_BLUE, 0);
    setColorPairs(INPUT_PAIR, COLOR_BLUE, COLOR_WHITE, 0);
    setColorPairs(SELECT_PAIR, COLOR_BLUE, COLOR_WHITE, 0);
    setColorPairs(DISPLAY_PAIR, BRIGHT_CYAN, COLOR_BLUE, 0);
    setColorPairs(DANGER_PAIR, BRIGHT_RED, COLOR_BLUE, 0);
    setColorPairs(DIR_PAIR, BRIGHT_MAGENTA, COLOR_BLUE, 0);
    setColorPairs(SLINK_PAIR, BRIGHT_WHITE, COLOR_BLUE, 0);
    setColorPairs(EXE_PAIR, BRIGHT_YELLOW, COLOR_BLUE, 0);
    setColorPairs(SUID_PAIR, COLOR_WHITE, COLOR_RED, 0);
    setColorPairs(SGID_PAIR, BRIGHT_BLACK, COLOR_GREEN, 0);
    setColorPairs(HILITE_PAIR, BRIGHT_YELLOW, COLOR_BLUE, 0);
    setColorPairs(ERROR_PAIR, BRIGHT_RED, COLOR_BLUE, 0);
    setColorPairs(HEADING_PAIR, BRIGHT_YELLOW, COLOR_BLUE, 0);
    setColorPairs(DEADLINK_PAIR, BRIGHT_RED, COLOR_BLUE, 0);
    break;
  }
  setColorPairs(COLORMENU_PAIR_0, COLOR_BLACK, COLOR_WHITE, 0);
  setColorPairs(COLORMENU_PAIR_1, COLOR_RED, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_2, COLOR_GREEN, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_3, COLOR_YELLOW, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_4, COLOR_BLUE, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_5, COLOR_MAGENTA, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_6, COLOR_CYAN, DEFAULT_COLOR, 0);
  setColorPairs(COLORMENU_PAIR_7, COLOR_WHITE, DEFAULT_COLOR, 0);
  if (COLORS > 8){
    setColorPairs(COLORMENU_PAIR_8, BRIGHT_BLACK, COLOR_WHITE, 0);
    setColorPairs(COLORMENU_PAIR_9, BRIGHT_RED, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_A, BRIGHT_GREEN, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_B, BRIGHT_YELLOW, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_C, BRIGHT_BLUE, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_D, BRIGHT_MAGENTA, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_E, BRIGHT_CYAN, DEFAULT_COLOR, 0);
    setColorPairs(COLORMENU_PAIR_F, BRIGHT_WHITE, DEFAULT_COLOR, 0);
  } else {
    setColorPairs(COLORMENU_PAIR_8, COLOR_BLACK, COLOR_WHITE, 1);
    setColorPairs(COLORMENU_PAIR_9, COLOR_RED, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_A, COLOR_GREEN, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_B, COLOR_YELLOW, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_C, COLOR_BLUE, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_D, COLOR_MAGENTA, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_E, COLOR_CYAN, DEFAULT_COLOR, 1);
    setColorPairs(COLORMENU_PAIR_F, COLOR_WHITE, DEFAULT_COLOR, 1);
  }

  setColorPairs(DEFAULT_BOLD_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
  setColorPairs(DEFAULT_COLOR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);

  refreshColors();

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
  if (bgToggle){
    strcpy(fgbgLabel, "background");
  } else {
    strcpy(fgbgLabel, "foreground");
  }
  printMenu(0, 0, "Color number, !Help, !Load, !Quit, !Save, !Toggle");

  setColors(COMMAND_PAIR);
  mvprintw(2, 4, "Command lines");
  setColors(DISPLAY_PAIR);
  mvprintw(3, 4, "Display lines");
  setColors(ERROR_PAIR);
  mvprintw(4, 4, "Error messages");
  setColors(INFO_PAIR);
  mvprintw(5, 4, "Information lines");
  setColors(HEADING_PAIR);
  mvprintw(6, 4, "Heading lines");
  setColors(DANGER_PAIR);
  mvprintw(7, 4, "Danger lines");
  setColors(SELECT_PAIR);
  mvprintw(8, 4, "Selected block lines");
  setColors(HILITE_PAIR);
  mvprintw(9, 4, "Highlight");

  setColors(DEFAULT_COLOR_PAIR);
  mvprintw(2, 45, "!-Default      ");
  setColors(DEFAULT_BOLD_PAIR);
  mvprintw(3, 45, "?-Default Bold ");
  setColors(COLORMENU_PAIR_0);
  mvprintw(4, 45, "0-Black        ");
  setColors(COLORMENU_PAIR_1);
  mvprintw(5, 45, "1-Red          ");
  setColors(COLORMENU_PAIR_2);
  mvprintw(6, 45, "2-Green        ");
  setColors(COLORMENU_PAIR_3);
  mvprintw(7, 45, "3-Brown        ");
  setColors(COLORMENU_PAIR_4);
  mvprintw(8, 45, "4-Blue         ");
  setColors(COLORMENU_PAIR_5);
  mvprintw(9, 45, "5-Magenta      ");
  setColors(COLORMENU_PAIR_6);
  mvprintw(10, 45, "6-Cyan         ");
  setColors(COLORMENU_PAIR_7);
  mvprintw(11, 45, "7-Light Gray   ");
  setColors(COLORMENU_PAIR_8);
  mvprintw(12, 45, "8-Dark Gray    ");
  setColors(COLORMENU_PAIR_9);
  mvprintw(13, 45, "9-Light Red    ");
  setColors(COLORMENU_PAIR_A);
  mvprintw(14, 45, "A-Light Green  ");
  setColors(COLORMENU_PAIR_B);
  mvprintw(15, 45, "B-Yellow       ");
  setColors(COLORMENU_PAIR_C);
  mvprintw(16, 45, "C-Light Blue   ");
  setColors(COLORMENU_PAIR_D);
  mvprintw(17, 45, "D-Light Magenta");
  setColors(COLORMENU_PAIR_E);
  mvprintw(18, 45, "E-Light Cyan   ");
  setColors(COLORMENU_PAIR_F);
  mvprintw(19, 45, "F-White        ");

  setColors(DEFAULT_BOLD_PAIR);
  mvprintw(22, 22, "Select 0 to F for desired %s color", fgbgLabel);

  curs_set(TRUE);
  move(colorThemePos + 2, 1);

  theme_menu_inputs();

}
