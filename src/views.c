#include <ncurses.h>
#include <unistd.h>
#include "main.h"
#include "views.h"
#include "functions.h"
#include "menus.h"

int directory_view()
{
  char currentpwd[1024];
  getcwd(currentpwd, sizeof(currentpwd));
  clear();
  attron(COLOR_PAIR(1));

  directory_top_menu();

  attron(COLOR_PAIR(2));
  mvprintw(2, 2, "%s", currentpwd);
  mvprintw(3, 2, "14 Objects   42563 Used 10930239 Available"); // Placeholder for PWD info
  mvprintw(4, 4, "---Attrs--- -Owner & Group-  -Size- --Date & Time-- ----Name----"); // Header
  attron(COLOR_PAIR(1));

  function_key_menu();

  refresh();

  directory_view_menu_inputs0();

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
