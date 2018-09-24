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
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "common.h"
#include "sf.h"
#include "colors.h"

int c;
int * pc = &c;

int abortinput = 0;

extern char fileMenuText[74];
extern char filePosText[58];
extern char regexinput[1024];
extern FILE *file;
extern int topline;
extern char fileName[512];
extern int displaysize;
extern int totallines;
extern int viewmode;

extern int wrap;
extern int wrapmode;

void show_file_find(int charcase)
{
  int regexcase;
  int result;
  char inputmessage[32];
  char errormessage[1024];
  if (charcase){
    regexcase = 0;
    strcpy(inputmessage, "Match Case - Enter string:");
  } else {
    regexcase = REG_ICASE;
    strcpy(inputmessage, "Ignore Case - Enter string:");
  }
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, inputmessage);
  curs_set(TRUE);
  move(0, strlen(inputmessage) + 1);
  curs_set(FALSE);
  if (readline(regexinput, 1024, regexinput) == -1 ){
    abortinput = 1;
  } else {
    result = findInFile(fileName, regexinput, regexcase);
    if ( result > 0 ){
      topline = result;
      displayFile(fileName, topline);
    } else if ( result == -2 ){
      // Not a feature in DF-EDIT 2.3d, but a nice to have
      sprintf(errormessage, "No further references to '%s' found.", regexinput);
      topLineMessage(errormessage);
    }
  }
}

int show_file_find_case_input()
{
  int result;
  move(0,0);
  clrtoeol();
  // printMenu(0,0,"!Ignore-case !Case-sensitive !Regular-expression (enter = I)");
  printMenu(0,0,"!Ignore-case !Case-sensitive (enter = I)");
  while(1)
    {
    findCaseLoop:
      *pc = getch();
      switch(*pc)
        {
        case 10:
        case 'i':
          result = 0;
          break;
        case 'c':
          result = 1;
          break;
        // case 'r':
        //   result = 2;
        //   break;
        case 27:
          result = -1;
          break;
        default:
          goto findCaseLoop;
        }
      break;
    }
  return(result);
}

void show_file_position_input(int currentpos)
{
  char newpos[11];
  viewmode = 2;
  move(0,0);
  clrtoeol();
  printMenu(0,0,filePosText);
  curs_set(TRUE);
  move(0,52);
  readline(newpos, 11, "");
  curs_set(FALSE);
  if (check_first_char(newpos, "+")){
    memmove(newpos, newpos+1, strlen(newpos));
    if (check_numbers_only(newpos)){
      topline = topline + atoi(newpos);
    }
  } else if (check_first_char(newpos, "-")) {
    memmove(newpos, newpos+1, strlen(newpos));
    if (check_numbers_only(newpos)){
      topline = topline - atoi(newpos);
    }
  } else {
    if (check_numbers_only(newpos)){
      topline = atoi(newpos);
    }
  }
  printMenu(0, 0, fileMenuText);
}

void show_file_inputs()
{
  int e = 0;
  printMenu(0, 0, fileMenuText);
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'f':
          e = show_file_find_case_input();
          if (e != -1){
            show_file_find(e);
          } else {
            abortinput = 0;
          }
          printMenu(0, 0, fileMenuText);
          break;
        case 'h':
          showManPage("sf");
          refreshScreen();
          break;
        case 'p':
          show_file_position_input(topline);
          if (topline > totallines + 1){
            topline = totallines;
          } else if (topline < 1){
            topline = 1;
          }
          displayFile(fileName, topline);
          break;
        case 'q':
          exittoshell();
          break;
        case 'w':
          if (wrap){
            wrap = 0;
          } else {
            wrap = 1;
          }
          displayFile(fileName, topline);
          break;
        case 258: // Down Arrow
          if (topline < totallines + 1){
            topline++;
            displayFile(fileName, topline);
          }
          break;
        case 259: // Up Arrow
          if (topline > 1){
            topline--;
            displayFile(fileName, topline);
          }
          break;
        case 338: // PgDn
        case 265: // F1
          topline = topline + displaysize;
          if (topline > totallines + 1){
            topline = totallines + 1;
          }
          displayFile(fileName, topline);
          break;
        case 339: // PgUp
        case 266: // F2
          topline = topline - displaysize;
          if (topline < 1){
            topline = 1;
          }
          displayFile(fileName, topline);
          break;
        case 267: // F3
          topline = 1;
          displayFile(fileName, topline);
          break;
        case 268: // F4
          topline = totallines;
          displayFile(fileName, topline);
          break;
        }
    }
}

void show_file_file_input()
{
  char *rewrite;
  move(0,0);
  clrtoeol(); // Probably not needed as this is only ever displayed when launching without a file
  mvprintw(0,0,"Show File - Enter pathname:");
  curs_set(TRUE);
  move(0,28);
  readline(fileName, 512, "");
  curs_set(FALSE);
  if (check_first_char(fileName, "~")){
    rewrite = str_replace(fileName, "~", getenv("HOME"));
    strcpy(fileName, rewrite);
    free(rewrite);
  }
  file_view(fileName);
}
