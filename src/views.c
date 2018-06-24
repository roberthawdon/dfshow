#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "main.h"
#include "views.h"
#include "functions.h"
#include "menus.h"

results *ob;

extern int topfileref;

extern char fileMenuText[256];
extern char globalMenuText[256];
extern char functionMenuText[256];

extern char sortmode[5];

int directory_view(char * currentpwd)
{
  topfileref = 0;
  clear();
  attron(COLOR_PAIR(1));

  // directory_top_menu();

  printMenu(0, 0, fileMenuText);

  set_history(currentpwd, 0, 0);
  ob = get_dir(currentpwd);
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, topfileref, 0);

  // function_key_menu();

  printMenu(LINES-1, 0, functionMenuText);

  refresh();

  directory_view_menu_inputs0();

  free(ob); //freeing memory
  return 0;
}

int quit_menu()
{
  clear();

  printMenu(0, 0, globalMenuText);

  refresh();

  directory_change_menu_inputs();

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
