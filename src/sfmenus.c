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
#include "common.h"
#include "sf.h"
#include "colors.h"

int c;
int * pc = &c;

extern char fileMenuText[256];
extern FILE *file;
extern int topline;
extern char fileName[512];
extern int displaysize;
extern int totallines;

void show_file_inputs()
{
  printMenu(0, 0, fileMenuText);
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'f':
          break;
        case 'h':
          break;
        case 'p':
          break;
        case 'q':
          exittoshell();
          break;
        case 258: // Down Arrow
          if (topline < totallines){
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
          if (topline > totallines){
            topline = totallines;
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
