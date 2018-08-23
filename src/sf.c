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
int displaysize;
int topline = 1;

char fileName[512];

extern FILE *file;

void fileShowStatus(const char * currentfile, int top)
{
  char statusText[512];
  sprintf(statusText, "File = <%s>  Top = <%i>", currentfile, top);
  printMenu(LINES - 1, 0, statusText);
}

void displayFile(const char * currentfile, int top)
{
  unsigned char line[8192];
  int count = 0;
  int displaycount = 0;
  //mvprintw(0, 66, "%i", top);
  top--;
  file=fopen(currentfile,"rb");
  clear_workspace();
  setColors(DISPLAY_PAIR);
  if (file != NULL )
    {
      while (fgets(line, sizeof line, file) != NULL) /* read a line */
        {
          // This logic converts Windows/Dos line endings to Unix
          if (line && (2 <= strlen(line)))
            {
              size_t size = strcspn(line, "\r\n");
              line[size] = 0;
            }
          if ((count == top + displaycount) && (displaycount < displaysize))
            {
              //use line or in a function return it
              //in case of a return first close the file with "fclose(file);"
              mvprintw(displaycount + 1, 0, "%s" , line);
              displaycount++;
              count++;
            } else {
            count++;
          }
        }
    }
  fileShowStatus(currentfile, topline);
  fclose(file);
}

void file_view(char * currentfile)
{
  char notFoundMessage[512];
  clear();
  setColors(COMMAND_PAIR);

  printMenu(0, 0, fileMenuText);

  displaysize = LINES - 2;

  refresh();

  if ( check_file(currentfile) ){
    displayFile(currentfile, topline);
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
    strcpy(fileName, argv[optind]);
    file_view(fileName);
  }

  exittoshell();
  return 0;
}
