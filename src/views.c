#include <ncurses.h>
#include <unistd.h>
#include "main.h"
#include "views.h"
#include "functions.h"
#include "menus.h"
#include "vars.h"

int directory_view()
{
  char currentpwd[1024];
  getcwd(currentpwd, sizeof(currentpwd));
  clear();
  attron(COLOR_PAIR(1));

  directory_top_menu();

  list_dir(currentpwd);

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
