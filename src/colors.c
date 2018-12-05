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

int lightColorPair[256];
// int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL = 0;

int colorThemePos = 0;
int totalItemCount = 6;

int selectedItem;

colorPairs colors[256];

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
    selectedItem = DANGER_PAIR;
    break;
  case 5:
    selectedItem = SELECT_PAIR;
    break;
  case 6:
    selectedItem = HILITE_PAIR;
    break;
  default:
    selectedItem = -1;
    break;
  }
  return(selectedItem);
}

void theme_menu_inputs()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 27: // ALT
          *pc = getch();
          switch(*pc)
            {
            case '!':
              colors[itemLookup(colorThemePos)].background = DEFAULT_COLOR;
              refreshColors();
              themeBuilder();
              break;
            case '?':
              colors[itemLookup(colorThemePos)].background = DEFAULT_COLOR;
              refreshColors();
              themeBuilder();
              break;
            case '0':
              colors[itemLookup(colorThemePos)].background = COLOR_BLACK;
              refreshColors();
              themeBuilder();
              break;
            case '1':
              colors[itemLookup(colorThemePos)].background = COLOR_RED;
              refreshColors();
              themeBuilder();
              break;
            case '2':
              colors[itemLookup(colorThemePos)].background = COLOR_GREEN;
              refreshColors();
              themeBuilder();
              break;
            case '3':
              colors[itemLookup(colorThemePos)].background = COLOR_YELLOW;
              refreshColors();
              themeBuilder();
              break;
            case '4':
              colors[itemLookup(colorThemePos)].background = COLOR_BLUE;
              refreshColors();
              themeBuilder();
              break;
            case '5':
              colors[itemLookup(colorThemePos)].background = COLOR_MAGENTA;
              refreshColors();
              themeBuilder();
              break;
            case '6':
              colors[itemLookup(colorThemePos)].background = COLOR_CYAN;
              refreshColors();
              themeBuilder();
              break;
            case '7':
              colors[itemLookup(colorThemePos)].background = COLOR_WHITE;
              refreshColors();
              themeBuilder();
              break;
            case '8':
              colors[itemLookup(colorThemePos)].background = BRIGHT_BLACK;
              refreshColors();
              themeBuilder();
              break;
            case '9':
              colors[itemLookup(colorThemePos)].background = BRIGHT_RED;
              refreshColors();
              themeBuilder();
              break;
            case 'a':
              colors[itemLookup(colorThemePos)].background = BRIGHT_GREEN;
              refreshColors();
              themeBuilder();
              break;
            case 'b':
              colors[itemLookup(colorThemePos)].background = BRIGHT_YELLOW;
              refreshColors();
              themeBuilder();
              break;
            case 'c':
              colors[itemLookup(colorThemePos)].background = BRIGHT_BLUE;
              refreshColors();
              themeBuilder();
              break;
            case 'd':
              colors[itemLookup(colorThemePos)].background = BRIGHT_MAGENTA;
              refreshColors();
              themeBuilder();
              break;
            case 'e':
              colors[itemLookup(colorThemePos)].background = BRIGHT_CYAN;
              refreshColors();
              themeBuilder();
              break;
            case 'f':
              colors[itemLookup(colorThemePos)].background = BRIGHT_WHITE;
              refreshColors();
              themeBuilder();
              break;
            }
          break;
        case '!':
          colors[itemLookup(colorThemePos)].foreground = DEFAULT_COLOR;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '?':
          colors[itemLookup(colorThemePos)].foreground = DEFAULT_COLOR;
          colors[itemLookup(colorThemePos)].bold = 1;
          refreshColors();
          themeBuilder();
          break;
        case '0':
          colors[itemLookup(colorThemePos)].foreground = COLOR_BLACK;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '1':
          colors[itemLookup(colorThemePos)].foreground = COLOR_RED;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '2':
          colors[itemLookup(colorThemePos)].foreground = COLOR_GREEN;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '3':
          colors[itemLookup(colorThemePos)].foreground = COLOR_YELLOW;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '4':
          colors[itemLookup(colorThemePos)].foreground = COLOR_BLUE;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '5':
          colors[itemLookup(colorThemePos)].foreground = COLOR_MAGENTA;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '6':
          colors[itemLookup(colorThemePos)].foreground = COLOR_CYAN;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '7':
          colors[itemLookup(colorThemePos)].foreground = COLOR_WHITE;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '8':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_BLACK;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case '9':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_RED;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case 'a':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_GREEN;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case 'b':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_YELLOW;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case 'c':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_BLUE;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case 'd':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_MAGENTA;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case 'e':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_CYAN;
          colors[itemLookup(colorThemePos)].bold = 0;
          refreshColors();
          themeBuilder();
          break;
        case 'f':
          colors[itemLookup(colorThemePos)].foreground = BRIGHT_WHITE;
          colors[itemLookup(colorThemePos)].bold = 0;
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
        }
    }
}

void setColorPairs(int pair, int foreground, int background, int bold){
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
    if (COLORS > 8){
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
    } else {
      setColorPairs(COMMAND_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 0);
      setColorPairs(INFO_PAIR, COLOR_GREEN, DEFAULT_COLOR, 0);
      setColorPairs(INPUT_PAIR, COLOR_BLACK, COLOR_WHITE, 0);
      setColorPairs(SELECT_PAIR, COLOR_BLUE, DEFAULT_COLOR, 1);
      setColorPairs(DISPLAY_PAIR, COLOR_CYAN, DEFAULT_COLOR, 0);
      setColorPairs(DANGER_PAIR, COLOR_RED, DEFAULT_COLOR, 1);
      setColorPairs(DIR_PAIR, COLOR_MAGENTA, DEFAULT_COLOR, 1);
      setColorPairs(SLINK_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
      setColorPairs(EXE_PAIR, COLOR_YELLOW, DEFAULT_COLOR, 1);
      setColorPairs(SUID_PAIR, DEFAULT_COLOR, COLOR_RED, 0);
      setColorPairs(SGID_PAIR, COLOR_BLACK, COLOR_GREEN, 1);
      setColorPairs(HILITE_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
      setColorPairs(ERROR_PAIR, DEFAULT_COLOR, DEFAULT_COLOR, 1);
      setColorPairs(HEADING_PAIR, COLOR_GREEN, DEFAULT_COLOR, 0);
      setColorPairs(DEADLINK_PAIR, COLOR_RED, DEFAULT_COLOR, 1);
    }
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

  setColors(DEFAULT_COLOR_PAIR);
  mvprintw(2, 45, "!-Default");
  setColors(DEFAULT_BOLD_PAIR);
  mvprintw(3, 45, "?-Default Bold");
  setColors(COLORMENU_PAIR_0);
  mvprintw(4, 45, "0-Black");
  setColors(COLORMENU_PAIR_1);
  mvprintw(5, 45, "1-Red");
  setColors(COLORMENU_PAIR_2);
  mvprintw(6, 45, "2-Green");
  setColors(COLORMENU_PAIR_3);
  mvprintw(7, 45, "3-Brown");
  setColors(COLORMENU_PAIR_4);
  mvprintw(8, 45, "4-Blue");
  setColors(COLORMENU_PAIR_5);
  mvprintw(9, 45, "5-Magenta");
  setColors(COLORMENU_PAIR_6);
  mvprintw(10, 45, "6-Cyan");
  setColors(COLORMENU_PAIR_7);
  mvprintw(11, 45, "7-Light Gray");
  setColors(COLORMENU_PAIR_8);
  mvprintw(12, 45, "8-Dark Gray");
  setColors(COLORMENU_PAIR_9);
  mvprintw(13, 45, "9-Bright Red");
  setColors(COLORMENU_PAIR_A);
  mvprintw(14, 45, "A-Bright Green");
  setColors(COLORMENU_PAIR_B);
  mvprintw(15, 45, "B-Yellow");
  setColors(COLORMENU_PAIR_C);
  mvprintw(16, 45, "C-Bright Blue");
  setColors(COLORMENU_PAIR_D);
  mvprintw(17, 45, "D-Bright Magenta");
  setColors(COLORMENU_PAIR_E);
  mvprintw(18, 45, "E-Bright Cyan");
  setColors(COLORMENU_PAIR_F);
  mvprintw(19, 45, "F-White");

  setColors(DEFAULT_BOLD_PAIR);
  mvprintw(22, 22, "Select 0 to F for desired foreground color");
  mvprintw(23, 22, "Use alt-0 to alt-F for background color");

  curs_set(TRUE);
  move(colorThemePos + 2, 1);

  theme_menu_inputs();

}
