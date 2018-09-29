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
#include <signal.h>
#include <regex.h>
#include <wchar.h>
#include "config.h"
#include "colors.h"
#include "common.h"
#include "sfmenus.h"
#include "sf.h"

char fileMenuText[100];
char filePosText[58];

char regexinput[1024];

int colormode = 0;
int messageBreak = 0;
int displaysize;
int topline = 1;
int leftcol = 1;
int totallines = 0;
int viewmode = 0;

int wrap = 0;
int wrapmode = LINE_WRAP;

char fileName[512];

extern FILE *file;

struct sigaction sa;

void buildMenuText(){
  // Writing Menus
  strcpy(fileMenuText, "<F1>-Down, <F2>-Up, <F3>-Top, <F4>-Bottom, !Find, !Help, !Position, !Quit");
  if (wrap){
    strcat(fileMenuText, ", !Wrap-off");
  } else {
    strcat(fileMenuText, ", !Wrap-on");
  }
  // Fun fact, in DF-EDIT 2.3d, the following text input typoed "absolute" as "absolue", this typo also exists in the Windows version from 1997 (2.3d-76), however, the 1986 documentation correctly writes it as "absolute".
  strcpy(filePosText, "Position relative (<+num> || <-num>) or absolute (<num>):");
}

int themeSelect(char* themeinput){
  if (!strcmp(themeinput, "default")){
    colormode = 0;
  } else if (!strcmp(themeinput, "monochrome")){
    colormode = 1;
  } else if (!strcmp(themeinput, "nt")){
    colormode = 2;
  } else {
    colormode = -1;
  }
  return colormode;
}

void refreshScreen()
{
  if (viewmode == 0){
    mvprintw(0,0,"Show File - Enter pathname:");
  } else if (viewmode == 1){
    printMenu(0, 0, fileMenuText);
    displayFile(fileName);
  } else if (viewmode == 2){
    printMenu(0,0,filePosText);
  }
}

int calculateTab(int pos)
{
  int tabsize = 8;
  int currentpos;
  int result;

  // currentpos = pos + leftcol - 1;
  currentpos = pos;

  while (currentpos > tabsize){
    currentpos = currentpos - tabsize;
  }

  result = tabsize - currentpos;

  if (result <= 0){
    result = tabsize;
  }

  return(result);
}

void sigwinchHandle(int sig)
{
  endwin();
  clear();
  refresh();
  initscr();
  displaysize = LINES - 2;
  refreshScreen();
}

int findInFile(const char * currentfile, const char * search, int charcase)
{
  char *line;
  int count = 0;
  regex_t regex;
  int reti;
  char msgbuf[8192];

  reti = regcomp(&regex, search, charcase);

  if (reti) {
    return(-1);
  }

  file=fopen(currentfile, "rb");

  if ( file ) {
    while (line = read_line(file) ){
      count++;
      reti = regexec(&regex, line, 0, NULL, 0);
      if (!reti && count > topline) {
        fclose(file);
        free(line);
        regfree(&regex);
        return(count);
      }
    }
  }

  free(line);
  regfree(&regex);
  fclose(file);
  return (-2);

}

void printHelp(char* programName)
{
  printf (("Usage: %s [OPTION]... [FILE]...\n"), programName);
  fputs ((PROGRAM_DESC), stdout);
  fputs (("\n\
Options:\n\
  -w, --wrap                   turn line wrapping on\n\
      --theme=[THEME]          color themes, see the THEME section below for\n\
                               valid themes.\n\
      --help                   displays help message, then exits\n\
      --version                displays version, then exits\n"), stdout);
  fputs (("\n\
The THEME argument can be:\n\
               default:    original theme\n\
               monochrome: comaptability mode for monochrome displays\n\
               nt:         a theme that closer resembles win32 versions of\n\
                           DF-EDIT\n"), stdout);
  printf ("\nPlease report any bugs to: <%s>\n", PACKAGE_BUGREPORT);
}

void fileShowStatus(const char * currentfile)
{
  char statusText[512];
  if (wrap){
    sprintf(statusText, "File = <%s>  Top = <%i>", currentfile, topline);
  } else {
    sprintf(statusText, "File = <%s>  Top = <%i:%i>", currentfile, topline, leftcol);
  }
  printMenu(LINES - 1, 0, statusText);
}

void displayFile(const char * currentfile)
{
  wchar_t line[8192];
  int count = 0;
  int displaycount = 0;
  int top, left;
  int i;
  int s;
  top = topline;
  left = leftcol;
  //mvprintw(0, 66, "%i", top);
  viewmode = 1;
  totallines = 0;
  top--;
  left--;
  file=fopen(currentfile,"rb");
  clear_workspace();
  setColors(DISPLAY_PAIR);
  if (file != NULL )
    {
      while (fgetws(line, sizeof line, file) != NULL) /* read a line */
        {
          totallines++;
          s = 0;
          // This logic converts Windows/Dos line endings to Unix
          // if (line && (2 <= wcslen(line)))
          //   {
          //     size_t size = wcscspn(line, L"\r\n");
          //     line[size] = 0;
          //   }
          if ((count == top + displaycount) && (displaycount < displaysize))
            {
              //use line or in a function return it
              //in case of a return first close the file with "fclose(file);"

              //mvprintw(displaycount + 1, 0, "%s" , line);

              for (i = 0; i < wcslen(line); i++){
                mvprintw(displaycount + 1, s - left, "%lc", line[i]);
                if (line[i] == L'\t'){
                  s = s + calculateTab(s);
                } else {
                  s++;
                }
                if ( ( s ) == COLS + left){
                  if ( wrap ){
                    if ( wrapmode != WORD_WRAP ){
                      s = 0;
                      displaycount++;
                      count++;
                    }
                  } else {
                    break;
                  }
                }
              }

              displaycount++;
              count++;
            } else {
            count++;
          }
        }
    }
  attron(A_BOLD);
  mvprintw(displaycount + 1, 0, "*eof");
  attroff(A_BOLD);
  //mvprintw(0,66,"%i",totallines);
  fileShowStatus(currentfile);
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
    displayFile(currentfile);
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
         {"wrap",           no_argument,       0, 'w'},
         {"help",           no_argument,       0, GETOPT_HELP_CHAR},
         {"version",        no_argument,       0, GETOPT_VERSION_CHAR},
         {"theme",          optional_argument, 0, GETOPT_THEME_CHAR},
         {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long(argc, argv, "w", long_options, &option_index);

      if ( c == -1 ){
        break;
      }

    switch(c){
    case 'w':
      wrap = 1;
      break;
    case GETOPT_HELP_CHAR:
      printHelp(argv[0]);
      exit(0);
      break;
    case GETOPT_VERSION_CHAR:
      printVersion(argv[0]);
      exit(0);
      break;
    case GETOPT_THEME_CHAR:
      if (optarg){
        if (themeSelect(optarg) == -1 ){
          printf("%s: invalid argument '%s' for 'theme'\n", argv[0], optarg);
          fputs (("\
Valid arguments are:\n\
  - default\n\
  - monochrome\n\
  - nt\n"), stdout);
          printf("Try '%s --help' for more information.\n", argv[0]);
          exit(2);
        }
      } else {
        colormode = 0;
      }
      break;
    default:
      // abort();
      exit(2);
    }
  }

  buildMenuText();

  set_escdelay(10);
  //ESCDELAY = 10;

  // Blank out regexinput

  strcpy(regexinput, "");

  setlocale(LC_ALL, "");

  initscr();

  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigwinchHandle;
  sigaction(SIGWINCH, &sa, NULL);

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
  } else {
    show_file_file_input();
  }

  exittoshell();
  return 0;
}
