#include <ncurses.h>
#include <unistd.h>
#include "main.h"
#include "views.h"

int c;
int * pc = &c;

void directory_top_menu()
{
  move(0, 0);
  clrtoeol();
  attron(A_BOLD);
  mvprintw(0, 0, "C");
  attroff(A_BOLD);
  mvprintw(0, 1, "opy,");
  attron(A_BOLD);
  mvprintw(0, 6, "M");
  attroff(A_BOLD);
  mvprintw(0, 7, "ove,");
  attron(A_BOLD);
  mvprintw(0, 6, "D");
  attroff(A_BOLD);
  mvprintw(0, 7, "elete,");
  attron(A_BOLD);
  mvprintw(0, 14, "E");
  attroff(A_BOLD);
  mvprintw(0, 15, "dit,");
  attron(A_BOLD);
  mvprintw(0, 20, "H");
  attroff(A_BOLD);
  mvprintw(0, 21, "elp,");
  attron(A_BOLD);
  mvprintw(0, 26, "Q");
  attroff(A_BOLD);
  mvprintw(0, 27, "uit,");
  attron(A_BOLD);
  mvprintw(0, 32, "R");
  attroff(A_BOLD);
  mvprintw(0, 33, "ename,");
  attron(A_BOLD);
  mvprintw(0, 40, "S");
  attroff(A_BOLD);
  mvprintw(0, 41, "how");
}

void directory_change_menu()
{
  move(0, 0);
  clrtoeol();
  attron(A_BOLD);
  mvprintw(0, 0, "C");
  attroff(A_BOLD);
  mvprintw(0, 1, "hange dir,");
  attron(A_BOLD);
  mvprintw(0, 12, "R");
  attroff(A_BOLD);
  mvprintw(0, 13, "un command,");
  attron(A_BOLD);
  mvprintw(0, 25, "E");
  attroff(A_BOLD);
  mvprintw(0, 26, "dit file,");
  attron(A_BOLD);
  mvprintw(0, 36, "H");
  attroff(A_BOLD);
  mvprintw(0, 37, "elp,");
  attron(A_BOLD);
  mvprintw(0, 42, "M");
  attroff(A_BOLD);
  mvprintw(0, 43, "ake dir,");
  attron(A_BOLD);
  mvprintw(0, 52, "Q");
  attroff(A_BOLD);
  mvprintw(0, 53, "uit,");
  attron(A_BOLD);
  mvprintw(0, 58, "S");
  attroff(A_BOLD);
  mvprintw(0, 59, "how dir");
}

void function_key_menu()
{
  move(LINES-1, 0);
  clrtoeol();
  attron(A_BOLD);
  mvprintw(LINES-1, 0, "F1");
  attroff(A_BOLD);
  mvprintw(LINES-1, 2, "-Down");
  attron(A_BOLD);
  mvprintw(LINES-1, 8, "F2");
  attroff(A_BOLD);
  mvprintw(LINES-1, 10, "-Up");
  attron(A_BOLD);
  mvprintw(LINES-1, 14, "F3");
  attroff(A_BOLD);
  mvprintw(LINES-1, 16, "-Top");
  attron(A_BOLD);
  mvprintw(LINES-1, 21, "F4");
  attroff(A_BOLD);
  mvprintw(LINES-1, 23, "-Bottom");
  attron(A_BOLD);
  mvprintw(LINES-1, 31, "F5");
  attroff(A_BOLD);
  mvprintw(LINES-1, 33, "-Refresh");
  attron(A_BOLD);
  mvprintw(LINES-1, 42, "F6");
  attroff(A_BOLD);
  mvprintw(LINES-1, 44, "-Mark/Unmark");
  attron(A_BOLD);
  mvprintw(LINES-1, 57, "F7");
  attroff(A_BOLD);
  mvprintw(LINES-1, 59, "-All");
  attron(A_BOLD);
  mvprintw(LINES-1, 64, "F8");
  attroff(A_BOLD);
  mvprintw(LINES-1, 66, "-None");
  attron(A_BOLD);
  mvprintw(LINES-1, 72, "F9");
  attroff(A_BOLD);
  mvprintw(LINES-1, 74, "-Sort");
}

void directory_view_menu_inputs0()
{
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

void directory_change_menu_inputs()
{
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
