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
#include <unistd.h>
#include <ncurses.h>
#include <locale.h>
#include <getopt.h>
#include <string.h>
#include "config.h"
#include "colors.h"
#include "common.h"
#include "sfmenus.h"
#include "sf.h"

char fileMenuText[256];

int colormode = 0;
int messageBreak = 0;

extern FILE *file;

void fileShowStatus(const char * currentfile, int top)
{
  char statusText[512];
  sprintf(statusText, "File = <%s>  Top = <%i>", currentfile, top);
  printMenu(LINES - 1, 0, statusText);
}

void readFile(const char * currentfile)
{
  file=fopen(currentfile,"r+");
}

void file_view(char * currentfile)
{
  char notFoundMessage[512];
  clear();
  setColors(COMMAND_PAIR);

  printMenu(0, 0, fileMenuText);

  refresh();

  if ( check_file(currentfile) ){
    readFile(currentfile);
    fileShowStatus(currentfile, 1);
    show_file_inputs();
  } else {
    sprintf(notFoundMessage, "File [%s] does not exist", currentfile);
    topLineMessage(notFoundMessage);
  }
  // sleep(10); // No function, so we'll pause for 10 seconds to display our menu

  return;
}

int main(int argc, char *argv[])
{
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
         {"help",           no_argument,       0, GETOPT_HELP_CHAR},
         {"version",        no_argument,       0, GETOPT_VERSION_CHAR},
         {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long(argc, argv, "", long_options, &option_index);

      if ( c == -1 ){
        break;
      }

    switch(c){
    case GETOPT_HELP_CHAR:
      // printHelp(argv[0]);
      exit(0);
      break;
    case GETOPT_VERSION_CHAR:
      printVersion(argv[0]);
      exit(0);
      break;
    default:
      // abort();
      exit(2);
    }
  }

  // Writing Menus
  strcpy(fileMenuText, "<F1>-Down, <F2>-Up, <F3>-Top, <F4>-Bottom, !Find, !Help, !Position, !Quit");

  setlocale(LC_ALL, "");

  initscr();

  start_color();
  cbreak();
  setColorMode(colormode);
  bkgd(COLOR_PAIR(DISPLAY_PAIR));
  cbreak();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);

  if (optind < argc){
    file_view(argv[optind]);
  }

  exittoshell();
  return 0;
}
