#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "main.h"
#include "views.h"
#include "functions.h"
#include "menus.h"
#include "vars.h"

results *ob;
extern int topfileref;

int directory_view(char * currentpwd)
{
  topfileref = 0;
  clear();
  attron(COLOR_PAIR(1));

  directory_top_menu();

  set_history(currentpwd);
  ob = get_dir(currentpwd);
  reorder_ob(ob, "name");
  display_dir(currentpwd, ob, topfileref, 0);

  function_key_menu();

  refresh();

  switch(inputmode)
    {
    case 0:
      directory_view_menu_inputs0();
      break;
    case 1:
      directory_view_menu_inputs1();
      break;
    }

  free(ob); //freeing memory
  return 0;
}

int quit_menu()
{
  clear();

  directory_change_menu();

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
