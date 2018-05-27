#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
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

char ownerinput[256];
char groupinput[256];
char uids[24];
char gids[24];

int s;
char *buf;

extern results* ob;
extern history* hs;
extern char currentpwd[1024];

extern int historyref;
extern int selected;
extern int topfileref;
extern int totalfilecount;
extern int displaysize;
extern int showhidden;

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
  mvprintw(0, 21, "idden,");
  attron(A_BOLD);
  mvprintw(0, 28, "M");
  attroff(A_BOLD);
  mvprintw(0, 29, "odify,");
  attron(A_BOLD);
  mvprintw(0, 36, "Q");
  attroff(A_BOLD);
  mvprintw(0, 37, "uit,");
  attron(A_BOLD);
  mvprintw(0, 42, "R");
  attroff(A_BOLD);
  mvprintw(0, 43, "ename,");
  attron(A_BOLD);
  mvprintw(0, 50, "S");
  attroff(A_BOLD);
  mvprintw(0, 51, "how");
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

void modify_key_menu()
{
  move(0, 0);
  clrtoeol();
  mvprintw(0, 0, "Modify:");
  attron(A_BOLD);
  mvprintw(0, 8, "O");
  attroff(A_BOLD);
  mvprintw(0, 9, "wner/Group,");
  attron(A_BOLD);
  mvprintw(0, 21, "P");
  attroff(A_BOLD);
  mvprintw(0, 22, "ermissions");
}

void directory_view_menu_inputs0(); // Needed to allow menu inputs to switch between each other

void show_directory_input()
{
  char oldpwd[1024];
  strcpy(oldpwd, currentpwd);
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Show Directory - Enter pathname:");
  curs_set(TRUE);
  move(0,33);
  readline(currentpwd, 1024, oldpwd);
  curs_set(FALSE);
  if (!check_dir(currentpwd)){
    quit_menu();
  }
  if (strcmp(oldpwd,currentpwd)){
    set_history(currentpwd, topfileref, selected);
  }
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
  curs_set(TRUE);
  move(0,14);
  readline(newfile, 1024, file);
  copy_file(file, newfile);
  curs_set(FALSE);
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
  curs_set(TRUE);
  move(0,28);
  readline(filepath, 1024, "");
  curs_set(FALSE);
  SendToEditor(filepath);
}

void rename_file_input(char *file)
{
  char dest[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Rename file to:");
  // attron(COLOR_PAIR(3));
  // mvprintw(0, 16, "%s", file); // Placeholder
  // attron(COLOR_PAIR(1));
  curs_set(TRUE);
  move(0,16);
  readline(dest, 1024, file);
  curs_set(FALSE);
  RenameObject(file, dest);

}

void make_directory_input()
{
  char newdir[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Make Directory - Enter pathname:");
  curs_set(TRUE);
  move (0,33);
  if (!check_last_char(currentpwd, "/")){
    strcat(currentpwd, "/");
  }
  readline(newdir, 1024, currentpwd);
  mk_dir(newdir);
  curs_set(FALSE);
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

void modify_group_input()
{
  char ofile[1024];

  struct group grp;
  struct group *gresult;
  size_t bufsize;
  char errortxt[256];

  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Set Group:");
  curs_set(TRUE);
  move(0,11);
  readline(groupinput, 256, "");
  curs_set(FALSE);

  bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize == -1)          /* Value was indeterminate */
    {
      bufsize = 16384;        /* Should be more than enough */
    }

  buf = malloc(bufsize);
  if (buf == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  s = getgrnam_r(groupinput, &grp, buf, bufsize, &gresult);
  free(buf);
  if (gresult == NULL){
    if (s == 0){
      move(0,0);
      clrtoeol();
      mvprintw(0,0,"Invalid group: %s", groupinput);
    }
  } else {
    sprintf(gids, "%d", gresult->gr_gid);

    strcpy(ofile, currentpwd);
    if (!check_last_char(ofile, "/")){
      strcat(ofile, "/");
    }
    strcat(ofile, ob[selected].name);

    if (UpdateOwnerGroup(ofile, uids, gids) == -1) {
      move(0,0);
      clrtoeol();
      mvprintw(0,0,"Error: %s", strerror(errno));
    } else{
      ob = get_dir(currentpwd);
      clear_workspace();
      reorder_ob(ob, sortmode);
      display_dir(currentpwd, ob, topfileref, selected);

      directory_top_menu();
      directory_view_menu_inputs0();
    }
  }
}

void modify_owner_input()
{
  struct passwd pwd;
  struct passwd *presult;
  size_t bufsize;

  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Set Owner:");
  curs_set(TRUE);
  move(0,11);
  readline(ownerinput, 256, "");
  curs_set(FALSE);

  bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize == -1)          /* Value was indeterminate */
    {
      bufsize = 16384;        /* Should be more than enough */
    }

  buf = malloc(bufsize);
  if (buf == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  s = getpwnam_r(ownerinput, &pwd, buf, bufsize, &presult);
  free(buf);
  if (presult == NULL){
    if (s == 0){
      move(0,0);
      clrtoeol();
      mvprintw(0,0,"Invalid user: %s", ownerinput);
    }
  } else {
    sprintf(uids, "%d", presult->pw_uid);
    modify_group_input();
  }

  // if (strcmp(ownerinput,"")){
  // } else {
  //   directory_top_menu();
  //   directory_view_menu_inputs0();
  // }
}

void modify_permissions_input()
{
  int newperm;
  char perms[4];
  char *ptr;
  char pfile[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Modify Permissions:");
  curs_set(TRUE);
  move(0,20);
  readline(perms, 5, "");
  curs_set(FALSE);

  newperm = strtol(perms, &ptr, 8); // Convert string to Octal and then store it as an int. Yay, numbers.

  strcpy(pfile, currentpwd);
  if (!check_last_char(pfile, "/")){
    strcat(pfile, "/");
  }
  strcat(pfile, ob[selected].name);
  chmod(pfile, newperm);


  ob = get_dir(currentpwd);
  clear_workspace();
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, topfileref, selected);

  directory_top_menu();
  directory_view_menu_inputs0();

}

void modify_key_menu_inputs()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'o':
          modify_owner_input();
          break;
        case 'p':
          modify_permissions_input();
          break;
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
        case 27:
          inputmode = 0; // Don't think this does anything
          directory_top_menu();
          directory_view_menu_inputs0();
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
          if (showhidden == 0) {
            showhidden = 1;
          } else {
            showhidden = 0;
          }
          ob = get_dir(currentpwd);
          clear_workspace();
          reorder_ob(ob, sortmode);
          // Selecting top item to avoid buffer underflows
          selected = 0;
          topfileref = 0;
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 'm':
          modify_key_menu();
          modify_key_menu_inputs();
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
