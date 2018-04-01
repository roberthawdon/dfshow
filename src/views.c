#include <ncurses.h>
#include <unistd.h>
#include "main.h"
#include "views.h"
#include "functions.h"
#include "menus.h"

void directory_view()
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

  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'q':
          quit_menu();
          break;
        case 27:
          directory_change_menu();
          break;
          /* default:
             mvprintw(LINES-2, 1, "Character pressed is = %3d Hopefully it can be printed as '%c'", c, c);
             refresh(); */
        }
    }

}

void quit_menu()
{
  clear();

  directory_change_menu();

  refresh();

  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'q':
          exittoshell();
          refresh();
          break;
        case 's':
          directory_view(); // TODO: Ask which directory to show, this is a temporary placeholder
          break;
          /* case 27: // Pressing escape here didn't actually do anything in DF-EDIT 2.3b
          directory_view();
          break; */
        }
    }
}
