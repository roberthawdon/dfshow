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

int c;
int * pc = &c;

void directory_view();
void quit_menu();
int exittoshell();

void directory_view()
{
  clear();
  attron(COLOR_PAIR(1));
  mvprintw(0, 0, "Copy, Move, Delete, Edit, Help, Quit, Rename, Show"); // Placehoder for top bar
  attron(COLOR_PAIR(2));
  mvprintw(2, 2, "/home/ian/projects/dfshow"); // Placeholder for PWD
  mvprintw(3, 2, "14 Objects   42563 Used 10930239 Available"); // Placeholder for PWD info
  mvprintw(4, 4, "-Attrs-     -Owner & Group-  -Size- --Date & Time-- ----Name----"); // Header
  attron(COLOR_PAIR(1));
  mvprintw(LINES-1, 0, "F1-Down F2-Up F3-Top F4-Bottom F5-Refresh F6-Mark/Unmark F7-All F8-None F9-Sort"); // Placeholder for bottom bar
  refresh();

  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'q':
          quit_menu();
          break;
          /* default:
             mvprintw(LINES-2, 1, "Character pressed is = %3d Hopefully it can be printed as '%c'", c, c);
             refresh(); */
        }
    }

}

void quit_menu()
{
  clear();
  mvprintw(0, 0, "Change dir, Command, Edit file, Help, Make dir, Quit, Show dir"); // Placehoder for top bar
  refresh();

  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'q':
          exittoshell();
          refresh();
          break;
        case 's':
          directory_view(); // TODO: Ask which directory to show, this is a temporary placeholder
          break;
          /* case 27: // Pressing escape here didn't actually do anything in DF-EDIT 2.3b
          directory_view();
          break; */
        }
    }
}

int exittoshell()
{
  clear();
  endwin();
  exit(0);
  return 0;
}

int main()
{

  initscr();
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  cbreak();
  // nodelay(stdscr, TRUE);
  noecho();
  curs_set(FALSE); // Hide Curser (Will want to bring it back later)
  keypad(stdscr, TRUE);

  directory_view();
}
