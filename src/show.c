#include <ncurses.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  initscr();
  noecho();
  curs_set(FALSE);

  mvprintw(0, 0, "Copy, Move, Delete, Edit, Help, Quit, Rename, Show");
  mvprintw(2, 2, "/home/ian/projects/dfshow"); // Placeholder for PWD
  mvprintw(3, 2, "14 Objects   42563 Used 10930239 Available"); // Placeholder for PWD info
  mvprintw(4, 4, "-Attrs-     -Owner & Group-  -Size- --Date & Time-- ----Name----"); // Header
  refresh();

  sleep(10);

  endwin();
}
