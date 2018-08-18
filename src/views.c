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
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include "show.h"
#include "views.h"
#include "functions.h"
#include "menus.h"
#include "colors.h"

results *ob;

extern int topfileref;

extern char fileMenuText[256];
extern char globalMenuText[256];
extern char functionMenuText[256];

extern char sortmode[5];

extern char *objectWild;

int directory_view(char * currentpwd)
{
  objectWild = objectFromPath(currentpwd);
  if ( strchr(objectWild, MULTICHAR) || strchr(objectWild, ONECHAR)){
    strcpy(currentpwd, dirFromPath(currentpwd));
  } else {
    strcpy(objectWild, "");
  }

  topfileref = 0;
  clear();
  setColors(COMMAND_PAIR);

  // directory_top_menu();

  printMenu(0, 0, fileMenuText);

  set_history(currentpwd, "", "", 0, 0);
  ob = get_dir(currentpwd);
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, topfileref, 0);

  // function_key_menu();

  printMenu(LINES-1, 0, functionMenuText);

  refresh();

  directory_view_menu_inputs();

  free(ob); //freeing memory
  return 0;
}

int global_menu()
{
  clear();

  printMenu(0, 0, globalMenuText);

  refresh();

  global_menu_inputs();

  return 0;
}

void clear_workspace()
{
  size_t line_count = 1;
  for (line_count = 1; line_count < (LINES - 1);)
    {
      move (line_count,0);
      clrtoeol();
      line_count++;
    }
}
