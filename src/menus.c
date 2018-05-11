#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include "functions.h"
#include "main.h"
#include "views.h"
#include "vars.h"

int inputmode = 0;
char sortmode[5] = "name";

int c;
int * pc = &c;

char chpwd[1024];
char selfile[1024];

extern results* ob;
extern history* hs;
extern char currentpwd[1024];

extern int historyref;
extern int selected;
extern int topfileref;
extern int totalfilecount;
extern int displaysize;

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
  mvprintw(0, 26, "M");
  attroff(A_BOLD);
  mvprintw(0, 27, "odify,");
  attron(A_BOLD);
  mvprintw(0, 34, "Q");
  attroff(A_BOLD);
  mvprintw(0, 35, "uit,");
  attron(A_BOLD);
  mvprintw(0, 40, "R");
  attroff(A_BOLD);
  mvprintw(0, 41, "ename,");
  attron(A_BOLD);
  mvprintw(0, 48, "S");
  attroff(A_BOLD);
  mvprintw(0, 49, "how");
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

void directory_view_menu_inputs0();

void show_directory_input()
{
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Show Directory - Enter pathname:");
  attron(COLOR_PAIR(3));
  //mvprintw(0, 33, "*.*"); // Placeholder for typed text
  echo();
  curs_set(TRUE);
  move(0,33);
  //mvscanw(0,33,"%s\n",&currentpwd);
  getstr(currentpwd);
  noecho();
  curs_set(FALSE);
  attron(COLOR_PAIR(1));
  if (!check_dir(currentpwd)){
    quit_menu();
  }
  set_history(currentpwd, topfileref, selected);
  topfileref = 0;
  selected = 0;
  chdir(currentpwd);
  ob = get_dir(currentpwd);
  clear_workspace();
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, 0, selected);
  directory_top_menu();
  function_key_menu();
  directory_view_menu_inputs0();
}

void copy_file_input(char *file)
{
  char newfile[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Copy file to:");
  attron(COLOR_PAIR(3));
  // mvprintw(0, 14, "%s", file); // Placeholder
  echo();
  curs_set(TRUE);
  move(0,14);
  getstr(newfile);
  copy_file(file, newfile);
  noecho();
  curs_set(FALSE);
  attron(COLOR_PAIR(1));
  ob = get_dir(currentpwd);
  clear_workspace();
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, 0, selected);
  directory_top_menu();
  function_key_menu();
  directory_view_menu_inputs0();
}

void edit_file_input()
{
  char filepath[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Edit File - Enter pathname:");
  attron(COLOR_PAIR(3));
  // mvprintw(0, 14, "%s", file); // Placeholder
  echo();
  curs_set(TRUE);
  move(0,28);
  getstr(filepath);
  noecho();
  curs_set(FALSE);
  attron(COLOR_PAIR(1));
  SendToEditor(filepath);
}

void rename_file_input(char *file)
{
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Rename file to:");
  attron(COLOR_PAIR(3));
  mvprintw(0, 16, "%s", file); // Placeholder
  attron(COLOR_PAIR(1));
}

void make_directory_input()
{
  char newdir[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Make Directory - Enter pathname:");
  attron(COLOR_PAIR(3));
  //mvprintw(0, 33, "%s", "*.*"); // Placeholder
  echo();
  curs_set(TRUE);
  move (0,33);
  getstr(newdir);
  mk_dir(newdir);
  noecho();
  curs_set(FALSE);
  attron(COLOR_PAIR(1));
  ob = get_dir(currentpwd);
  clear_workspace();
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, 0, selected);
  directory_top_menu();
  function_key_menu();
  directory_view_menu_inputs0();
}

void delete_file_confirm()
{
  move(0,0);
  clrtoeol();
  mvprintw(0,0, "Delete file? (");
  attron(A_BOLD);
  mvprintw(0, 14, "Y");
  attroff(A_BOLD);
  mvprintw(0, 15, "es/");
  attron(A_BOLD);
  mvprintw(0, 18, "N");
  attroff(A_BOLD);
  mvprintw(0, 19, "o)");
}

void sort_view()
{
  move(0, 0);
  clrtoeol();
  mvprintw(0, 0, "Sort list by -");
  attron(A_BOLD);
  mvprintw(0, 15, "D");
  attroff(A_BOLD);
  mvprintw(0, 16, "ate & time,");
  attron(A_BOLD);
  mvprintw(0, 28, "N");
  attroff(A_BOLD);
  mvprintw(0, 29, "ame,");
  attron(A_BOLD);
  mvprintw(0, 34, "S");
  attroff(A_BOLD);
  mvprintw(0, 35, "ize");
}

void delete_file_confirm_input(char *file)
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'y':
          delete_file(file);
          ob = get_dir(currentpwd);
          clear_workspace();
          reorder_ob(ob, sortmode);
          display_dir(currentpwd, ob, topfileref, selected);
          // Not breaking here, intentionally dropping through to the default
        default:
          directory_top_menu();
          directory_view_menu_inputs0();
          break;
        }
    }
}

void sort_view_inputs()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'n':
          clear_workspace();
          strcpy(sortmode, "name");
          reorder_ob(ob, sortmode);
          display_dir(currentpwd, ob, topfileref, selected);
          directory_top_menu();
          directory_view_menu_inputs0();
          break;
        case 'd':
          clear_workspace();
          strcpy(sortmode, "date");
          reorder_ob(ob, sortmode);
          display_dir(currentpwd, ob, topfileref, selected);
          directory_top_menu();
          directory_view_menu_inputs0();
          break;
        case 's':
          clear_workspace();
          strcpy(sortmode, "size");
          reorder_ob(ob, sortmode);
          display_dir(currentpwd, ob, topfileref, selected);
          directory_top_menu();
          directory_view_menu_inputs0();
          break;
        case 27: // ESC Key
          directory_top_menu();
          directory_view_menu_inputs0();
          break;
        }
    }
}

void show_directory_inputs()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
          // case 10: // Enter key
            //   directory_view(currentpwd);
          //   break;
        case 27: // ESC Key
          directory_top_menu();
          directory_view_menu_inputs0();
          break;
        }
    }
}

void directory_view_menu_inputs1()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'm':
          make_directory_input();
          break;
        case 'r':
          LaunchShell();
          directory_top_menu();
          function_key_menu();
          display_dir(currentpwd, ob, topfileref, selected);
          directory_view_menu_inputs0();
          break;
        case 'e':
          edit_file_input();
          directory_top_menu();
          function_key_menu();
          display_dir(currentpwd, ob, topfileref, selected);
          directory_view_menu_inputs0();
          break;
        case 'q':
          quit_menu();
          break;
        case 's':
          show_directory_input();
          show_directory_inputs();
          break;
          /* default:
             mvprintw(LINES-2, 1, "Character pressed is = %3d Hopefully it can be printed as '%c'", c, c);
             refresh(); */
        }
    }
}

void directory_view_menu_inputs0()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'c':
          strcpy(selfile, currentpwd);
          if (!check_last_char(selfile, "/")){
            strcat(selfile, "/");
          }
          strcat(selfile, ob[selected].name);
          if (!check_dir(selfile)){
            copy_file_input(selfile);
          }
          break;
        case 'd':
          strcpy(selfile, currentpwd);
          if (!check_last_char(selfile, "/")){
            strcat(selfile, "/");
          }
          strcat(selfile, ob[selected].name);
          if (!check_dir(selfile)){
            delete_file_confirm();
            delete_file_confirm_input(selfile);
          }
          break;
        case 'e':
          strcpy(chpwd, currentpwd);
          if (!check_last_char(chpwd, "/")){
            strcat(chpwd, "/");
          }
          strcat(chpwd, ob[selected].name);
          //mvprintw(0, 66, "%s", chpwd);
          //break;
          if (!check_dir(chpwd)){
            SendToEditor(chpwd);
            directory_top_menu();
            function_key_menu();
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 'h':
          break;
        case 'm':
          break;
        case 'q':
          if (historyref > 1){
            strcpy(chpwd, hs[historyref - 2].path);
            selected = hs[historyref - 1].selected;
            topfileref = hs[historyref - 1].topfileref;
            historyref--;
            if (check_dir(chpwd)){
              strcpy(currentpwd, chpwd);
              chdir(currentpwd);
              ob = get_dir(currentpwd);
              clear_workspace();
              reorder_ob(ob, sortmode);
              display_dir(currentpwd, ob, topfileref, selected);
            }
            break;
          } else {
            historyref--;
            quit_menu();
          }
          break;
        case'r':
          strcpy(selfile, currentpwd);
          if (!check_last_char(selfile, "/")){
            strcat(selfile, "/");
          }
          strcat(selfile, ob[selected].name);
          rename_file_input(selfile);
          break;
        case 's':
          strcpy(chpwd, currentpwd);
          if (!check_last_char(chpwd, "/")){
            strcat(chpwd, "/");
          }
          strcat(chpwd, ob[selected].name);
          //mvprintw(0, 66, "%s", chpwd);
          //break;
          if (check_dir(chpwd)){
            set_history(chpwd, topfileref, selected);
            topfileref = 0;
            selected = 0;
            strcpy(currentpwd, chpwd);
            chdir(currentpwd);
            ob = get_dir(currentpwd);
            clear_workspace();
            reorder_ob(ob, sortmode);
            display_dir(currentpwd, ob, topfileref, selected);
          } else {
            SendToPager(chpwd);
            directory_top_menu();
            function_key_menu();
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 27:
          inputmode = 1;
          directory_change_menu();
          directory_view_menu_inputs1();
          break;
        case 10: // Enter - Falls through
        case 258: // Down Arrow
          if (selected < (totalfilecount - 1)) {
            selected++;
            if (selected > ((topfileref + displaysize) - 1)){
              topfileref++;
              clear_workspace();
            }
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 259: // Up Arrow
          if (selected > 0){
            selected--;
            if (selected < topfileref){
              topfileref--;
              clear_workspace();
            }
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 260: // Left Arrow
          break;
        case 261: // Right Arrow
          break;
        case 338: // PgDn - Drop through
        case 265: // F1
          if (selected < totalfilecount) {
            clear_workspace();
            topfileref = topfileref + displaysize;
            if (topfileref > (totalfilecount - displaysize)){
              topfileref = totalfilecount - displaysize;
            }
            selected = selected + displaysize;
            if (selected > totalfilecount - 1){
              selected = totalfilecount - 1;
            }
          }
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 339: // PgUp - Drop through
        case 266: // F2
          if (selected > 0){
            clear_workspace();
            topfileref = topfileref - displaysize;
            if (topfileref < 0){
              topfileref = 0;
            }
            selected = selected - displaysize;
            if (selected < 0){
              selected = 0;
            }
          }
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 267: // F3
          clear_workspace();
          selected = 0;
          topfileref =0;
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 268: // F4
          clear_workspace();
          selected = totalfilecount - 1;
          topfileref = totalfilecount - displaysize;
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 269: // F5
          ob = get_dir(currentpwd);
          clear_workspace();
          reorder_ob(ob, sortmode);
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 270: // F6
          break;
        case 271: // F7
          break;
        case 272: // F8
          break;
        case 273: // F9
          sort_view();
          sort_view_inputs();
          break;
        case 262: // Home
          selected = topfileref;
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 360: // End
          selected = topfileref + (displaysize - 1);
          display_dir(currentpwd, ob, topfileref, selected);
          break;
          // default:
          //     mvprintw(LINES-2, 1, "Character pressed is = %3d Hopefully it can be printed as '%c'", c, c);
          //     refresh();
        }
      //mvprintw(LINES-3, 1, "%i",totalfilecount);
      //mvprintw(LINES-2, 1, "%i",selected);
      //refresh();
    }
}
void directory_change_menu_inputs()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'm':
          make_directory_input();
          break;
        case 'r':
          LaunchShell();
          directory_change_menu();
          //directory_top_menu();
          //function_key_menu();
          //display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 'e':
          edit_file_input();
          directory_change_menu();
          break;
        case 'q':
          exittoshell();
          refresh();
          break;
        case 's':
          // directory_view(); // TODO: Ask which directory to show, this is a temporary placeholder
          show_directory_input();
          show_directory_inputs();
          break;
          /* case 27: // Pressing escape here didn't actually do anything in DF-EDIT 2.3d
             directory_view();
             break; */
        }
    }
}
