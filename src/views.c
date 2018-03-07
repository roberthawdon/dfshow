#include <ncurses.h>
#include <unistd.h>
#include "main.h"
#include "views.h"
#include "functions.h"

void directory_view()
{
  char currentpwd[1024];
  getcwd(currentpwd, sizeof(currentpwd));
  clear();
  attron(COLOR_PAIR(1));
  mvprintw(0, 0, "Copy, Move, Delete, Edit, Help, Quit, Rename, Show"); // Placehoder for top bar
  attron(COLOR_PAIR(2));
  mvprintw(2, 2, "%s", currentpwd);
  mvprintw(3, 2, "14 Objects   42563 Used 10930239 Available"); // Placeholder for PWD info
  mvprintw(4, 4, "-Attrs-     -Owner & Group-  -Size- --Date & Time-- ----Name----"); // Header
  attron(COLOR_PAIR(1));
  mvprintw(LINES-1, 0, "F1-Down F2-Up F3-Top F4-Bottom F5-Refresh F6-Mark/Unmark F7-All F8-None F9-Sort"); // Placeholder for bottom bar
  refresh();

  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'q':
          quit_menu();
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
  mvprintw(0, 0, "Change dir, Command, Edit file, Help, Make dir, Quit, Show dir"); // Placehoder for top bar
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
