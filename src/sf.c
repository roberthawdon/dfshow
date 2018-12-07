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
#include <libconfig.h>
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
int longestline = 0;
int longestlongline = 0;
int viewmode = 0;

int tabsize = 8;

int wrap = 0;
int wrapmode = LINE_WRAP;

char fileName[512];

extern FILE *file;

FILE *stream;
char *line = NULL;
wchar_t *longline = NULL;
size_t len = 0;
ssize_t nread;
int count;
int displaycount;
int top, left;
int lasttop;
int i, s;

long int topPos;
long int *filePos;

struct sigaction sa;

extern config_t themeConfig;
extern config_setting_t *root, *setting, *group, *array;

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
  endwin();
  clear();
  refresh();
  initscr();
  displaysize = LINES - 2;
  if (viewmode == 0){
    mvprintw(0,0,"Show File - Enter pathname:");
  } else if (viewmode == 1){
    printMenu(0, 0, fileMenuText);
    loadFile(fileName);
  } else if (viewmode == 2){
    printMenu(0,0,filePosText);
  }
}

int calculateTab(int pos)
{
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
  refreshScreen();
}

int findInFile(const char * currentfile, const char * search, int charcase)
{
  regex_t regex;
  int reti;
  char msgbuf[8192];

  reti = regcomp(&regex, search, charcase);

  if (reti) {
    return(-1);
  }

  fseek(stream, filePos[top], SEEK_SET);
  top = 0;
  count = 0;

  if ( stream ) {
    while (line = read_line(stream) ){
      count++;
      reti = regexec(&regex, line, 0, NULL, 0);
      if (!reti && count > topline) {
        regfree(&regex);
        return(count);
      }
    }
  }

  regfree(&regex);
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

void fileShowStatus()
{
  wchar_t statusText[1024];
  if (wrap){
    swprintf(statusText, 1024, L"File = <%s>  Top = <%i>", fileName, topline);
  } else {
    swprintf(statusText, 1024, L"File = <%s>  Top = <%i:%i>", fileName, topline, leftcol);
  }
  wPrintMenu(LINES - 1, 0, statusText);
}

void updateView()
{
  int longlinelen = 0;
  top = topline;
  left = leftcol;
  len = 0;
  top--;
  left--;
  displaycount = 0;

  clear_workspace();
  setColors(DISPLAY_PAIR);

  fseek(stream, filePos[top], SEEK_SET);
  top = 0;

  line = malloc(sizeof(char) + 1); // Preallocating memory appears to fix a crash on FreeBSD, it might also fix the same issue on macOS

  while ((nread = getline(&line, &len, stream)) != -1) {
    s = 0;
    mbstowcs(longline, line, len);
    longlinelen = wcslen(longline);
    if (displaycount < displaysize){
      for(i = 0; i < longlinelen; i++){
        mvprintw(displaycount + 1, s - left, "%lc", longline[i]);
        // This doesn't increase the max line.
        if (line[i] == '\t'){
          s = s + calculateTab(s);
        } else {
          s++;
        }
        if ( s == COLS + left){
          if ( wrap ) {
            if ( wrapmode != WORD_WRAP ){
              s = 0;
              displaycount++;
            }
          } else {
            break;
          }
        }
      }
      displaycount++;
    } else {
      break;
    }
  }
  attron(A_BOLD);
  mvprintw(displaycount + 1, 0, "*eof");
  attroff(A_BOLD);
  fileShowStatus();
  free(line);
}

void loadFile(const char * currentfile)
{

  len = 0;
  longestline = 0;
  longestlongline = 0;
  viewmode = 1;
  totallines = 0;

  filePos = malloc(sizeof(long int) * 1); // Initial isze of lookup
  filePos[0] = 0;

  stream = fopen(currentfile, "rb");
  if (stream == NULL) {

    return;
    }

  line = malloc(sizeof(char) + 1);
  longline = malloc(sizeof(wchar_t));

  while ((nread = getline(&line, &len, stream)) != -1) {
    totallines++;
    filePos = realloc(filePos, sizeof(long int) * totallines + 1);
    filePos[totallines] = ftell(stream);
    if (nread > longestline){
      longestline = nread;
      longline = realloc(longline, sizeof(wchar_t) * longestline +1);
    }
    mbstowcs(longline, line, len);
    if (wcslen(longline) > longestlongline){
      longestlongline = wcslen(longline);
    }
  }
  free(line);
  updateView();
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
    loadFile(currentfile);
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
  char themeEnv[48];

  config_init(&themeConfig);
  root = config_root_setting(&themeConfig);

  // Check for theme env variable
  if ( getenv("DFS_THEME")) {
    if (themeSelect(getenv("DFS_THEME")) != -1 ){
      colormode = themeSelect(getenv("DFS_THEME"));
    }
  }

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
      printVersion(PROGRAM_NAME);
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
        } else {
          strcpy(themeEnv,"DFS_THEME=");
          strcat(themeEnv,optarg);
          putenv(themeEnv);
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
