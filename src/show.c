#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

int c;
int * pc = &c;

void directory_view();
void quit_menu();
int exittoshell();

void directory_view()
{
  clear();
  mvprintw(0, 0, "Copy, Move, Delete, Edit, Help, Quit, Rename, Show"); // Placehoder for top bar
  mvprintw(2, 2, "/home/ian/projects/dfshow"); // Placeholder for PWD
  mvprintw(3, 2, "14 Objects   42563 Used 10930239 Available"); // Placeholder for PWD info
  mvprintw(4, 4, "-Attrs-     -Owner & Group-  -Size- --Date & Time-- ----Name----"); // Header
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
        case 27:
          directory_view();
          break;
        }
    }
}

int exittoshell()
{
  clear();
  endwin();
  exit(0);
  return 0;
}

int main()
{

  initscr();
  cbreak();
  // nodelay(stdscr, TRUE);
  noecho();
  curs_set(FALSE); // Hide Curser (Will want to bring it back later)
  keypad(stdscr, TRUE);

  directory_view();
}
