#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include "functions.h"
#include "main.h"
#include "views.h"

int c;
int * pc = &c;

char chpwd[1024];
char selfile[1024];

char ownerinput[256];
char groupinput[256];
char uids[24];
char gids[24];
char errmessage[256];

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
extern int markall;
extern int viewMode;

extern int reverse;

extern char fileMenuText[256];
extern char globalMenuText[256];
extern char functionMenuText[256];
extern char modifyMenuText[256];
extern char sortMenuText[256];

extern struct sigaction sa;

extern char sortmode[5];

//char testMenu[256];

void topLineMessage(const char *message);

void printMenu(int line, int col, char *menustring)
{
  int i, len, charcount;
  charcount = 0;
  move(line, col);
  clrtoeol();
  len = strlen(menustring);
  for (i = 0; i < len; i++)
     {
      if ( menustring[i] == '!' ) {
          i++;
          attron(A_BOLD);
          mvprintw(line, col + charcount, "%c", menustring[i]);
          attroff(A_BOLD);
          charcount++;
      } else if ( menustring[i] == '<' ) {
          i++;
          attron(A_BOLD);
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
      } else if ( menustring[i] == '>' ) {
          i++;
          attroff(A_BOLD);
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
      } else if ( menustring[i] == '\\' ) {
          i++;
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
      } else {
          mvprintw(line, col + charcount, "%c", menustring[i]);
          charcount++;
        }
    }
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
  if (strcmp(currentpwd, oldpwd) && strcmp(currentpwd, "") || !historyref){
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
  }
  printMenu(0, 0, fileMenuText);
  printMenu(LINES-1, 0, functionMenuText);
  directory_view_menu_inputs0();
}

int replace_file_confirm_input()
{
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'y':
          return 1;
          break;
        default:
          return 0;
          break;
        }
    }
}

void replace_file_confirm(char *filename)
{
  char message[1024];
  sprintf(message, "Replace file [<%s>]? (!Yes/!No)", filename);
  printMenu(0,0, message);
}

void copy_file_input(char *file)
{
  // YUCK, repetition, this needs sorting
  char newfile[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Copy file to:");
  curs_set(TRUE);
  move(0,14);
  readline(newfile, 1024, file);
  curs_set(FALSE);
  // If the two values don't match, we want to do the copy
  if ( strcmp(newfile, file) && strcmp(newfile, "")) {
    if ( check_file(newfile) )
      {
        replace_file_confirm(newfile);
        if ( replace_file_confirm_input() )
          {
            copy_file(file, newfile);
            ob = get_dir(currentpwd);
            clear_workspace();
            reorder_ob(ob, sortmode);
            display_dir(currentpwd, ob, 0, selected);
          }
      } else {
      copy_file(file, newfile);
      ob = get_dir(currentpwd);
      clear_workspace();
      reorder_ob(ob, sortmode);
      display_dir(currentpwd, ob, 0, selected);
    }
  }
  printMenu(0, 0, fileMenuText);
  printMenu(LINES-1, 0, functionMenuText);
  directory_view_menu_inputs0();
}

void copy_multi_file_input(results* ob, char *input)
{
  int i;

  char dest[1024];
  char destfile[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Copy multiple files to:");
  curs_set(TRUE);
  move(0, 24);
  readline(dest, 1024, input);
  curs_set(FALSE);
  if ( strcmp(dest, input) && strcmp(dest, "")) {

    if ( check_dir(dest) ){
      for (i = 0; i < totalfilecount; i++)
        {
          if ( *ob[i].marked )
            {
              strcpy(selfile, currentpwd);
              if (!check_last_char(selfile, "/")){
                strcat(selfile, "/");
              }
              strcat(selfile, ob[i].name);
              strcpy(destfile, dest);
              if (!check_last_char(destfile, "/")){
                strcat(destfile, "/");
              }
              strcat(destfile, ob[i].name);
              if ( check_file(destfile) )
                {
                  replace_file_confirm(destfile);
                  if ( replace_file_confirm_input() )
                    {
                      copy_file(selfile, dest);
                    }
                } else {
                copy_file(selfile, dest);
              }
            }
        }
    } else {
      topLineMessage("Error: Directory Not Found.");
    }
  }
  printMenu(0, 0, fileMenuText);
  directory_view_menu_inputs0();
}

void rename_multi_file_input(results* ob, char *input)
{
  int i;

  char dest[1024];
  char destfile[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Rename multiple files to:");
  curs_set(TRUE);
  move(0, 26);
  readline(dest, 1024, input);
  curs_set(FALSE);
  if (strcmp(dest, input) && strcmp(dest, "")){
    if ( check_dir(dest) ){
      for (i = 0; i < totalfilecount; i++)
        {
          if ( *ob[i].marked )
            {
              strcpy(selfile, currentpwd);
              if (!check_last_char(selfile, "/")){
                strcat(selfile, "/");
              }
              strcat(selfile, ob[i].name);
              strcpy(destfile, dest);
              if (!check_last_char(destfile, "/")){
                strcat(destfile, "/");
              }
              strcat(destfile, ob[i].name);
              if ( check_file(destfile) )
                {
                  replace_file_confirm(destfile);
                  if ( replace_file_confirm_input() )
                    {
                      RenameObject(selfile, destfile);
                    }
                } else {
                RenameObject(selfile, destfile);
              }
            }
        }
    } else {
      topLineMessage("Error: Directory Not Found.");
    }
    ob = get_dir(currentpwd);
    clear_workspace();
    reorder_ob(ob, sortmode);
    display_dir(currentpwd, ob, 0, selected);
  }

  printMenu(0, 0, fileMenuText);
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
  // YUCK, repetition, this needs sorting
  char dest[1024];
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Rename file to:");
  curs_set(TRUE);
  move(0,16);
  readline(dest, 1024, file);
  curs_set(FALSE);
  if (strcmp(dest, file) && strcmp(dest, "")){
    if ( check_file(dest) )
      {
        replace_file_confirm(dest);
        if ( replace_file_confirm_input() )
          {
            RenameObject(file, dest);
            ob = get_dir(currentpwd);
            clear_workspace();
            reorder_ob(ob, sortmode);
            display_dir(currentpwd, ob, 0, selected);
          }
      } else {
      RenameObject(file, dest);
      ob = get_dir(currentpwd);
      clear_workspace();
      reorder_ob(ob, sortmode);
      display_dir(currentpwd, ob, 0, selected);
    }
  }
  printMenu(0, 0, fileMenuText);
  printMenu(LINES-1, 0, functionMenuText);
  directory_view_menu_inputs0();
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
  if (strcmp(newdir, currentpwd) && strcmp(newdir, "")){
    mk_dir(newdir);
    curs_set(FALSE);
    ob = get_dir(currentpwd);
    clear_workspace();
    reorder_ob(ob, sortmode);
    display_dir(currentpwd, ob, 0, selected);
  }
  printMenu(0, 0, fileMenuText);
  printMenu(LINES-1, 0, functionMenuText);
  directory_view_menu_inputs0();
}

void delete_multi_file_confirm(const char *filename)
{
  char message[1024];
  sprintf(message,"Delete file [<%s>]? (!Yes/!No/!All/!Stop)", filename);
  printMenu(0,0, message);
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
          printMenu(0, 0, fileMenuText);
          directory_view_menu_inputs0();
          break;
        }
    }
}

void delete_multi_file_confirm_input(results* ob)
{
  int i, k;
  int allflag = 0;
  int abortflag = 0;

  for (i = 0; i < totalfilecount; i++)
    {
      if ( *ob[i].marked && !abortflag )
        {
          strcpy(selfile, currentpwd);
          if (!check_last_char(selfile, "/")){
            strcat(selfile, "/");
          }
          strcat(selfile, ob[i].name);
          if ( allflag )
            {
              delete_file(selfile);
            } else {
            delete_multi_file_confirm(selfile);
            k = 1;
            while(k)
              {
                *pc = getch();
                switch(*pc)
                  {
                  case 'y':
                    delete_file(selfile);
                    k = 0;
                    break;
                  case 'a':
                    allflag = 1;
                    k = 0;
                    delete_file(selfile);
                    break;
                  case 's':
                    abortflag = 1;
                    k = 0;
                    break;
                  case 'n':
                    k = 0;
                    break;
                  }
              }
          }
        }
    }
}

void sort_view_inputs()
{
  viewMode = 3;
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 27: // ESC Key
          printMenu(0, 0, fileMenuText);
          directory_view_menu_inputs0();
          break;
        case 'n':
          strcpy(sortmode, "name");
          reverse = 0;
          break;
        case 'd':
          strcpy(sortmode, "date");
          reverse = 0;
          break;
        case 's':
          strcpy(sortmode, "size");
          reverse = 0;
          break;
        case 'N':
          strcpy(sortmode, "name");
          reverse = 1;
          break;
        case 'D':
          strcpy(sortmode, "date");
          reverse = 1;
          break;
        case 'S':
          strcpy(sortmode, "size");
          reverse = 1;
          break;
        }
      clear_workspace();
      reorder_ob(ob, sortmode);
      display_dir(currentpwd, ob, topfileref, selected);
      printMenu(0, 0, fileMenuText);
      directory_view_menu_inputs0();
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
          printMenu(0, 0, fileMenuText);
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
  int i;

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
      sprintf(errmessage, "Invalid group: %s", groupinput);
      topLineMessage(errmessage);
    }
  } else {
    sprintf(gids, "%d", gresult->gr_gid);

    if ( CheckMarked(ob) ){
      //topLineMessage("Multi file owner coming soon");
      for (i = 0; i < totalfilecount; i++)
        {
          if ( *ob[i].marked )
            {
              strcpy(ofile, currentpwd);
              if (!check_last_char(ofile, "/")){
                strcat(ofile, "/");
              }
              strcat(ofile, ob[i].name);
              UpdateOwnerGroup(ofile, uids, gids);
            }
        }
    } else {
      strcpy(ofile, currentpwd);
      if (!check_last_char(ofile, "/")){
        strcat(ofile, "/");
      }
      strcat(ofile, ob[selected].name);

      if (UpdateOwnerGroup(ofile, uids, gids) == -1) {
        sprintf(errmessage, "Error: %s", strerror(errno));
        topLineMessage(errmessage);
      }
    }
    ob = get_dir(currentpwd);
    clear_workspace();
    reorder_ob(ob, sortmode);
    display_dir(currentpwd, ob, topfileref, selected);

    printMenu(0, 0, fileMenuText);
    directory_view_menu_inputs0();
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
      sprintf(errmessage, "Invalid user: %s", ownerinput);
      topLineMessage(errmessage);
    }
  } else {
    sprintf(uids, "%d", presult->pw_uid);
    modify_group_input();
  }
}

void modify_permissions_input()
{
  int newperm, i;
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

  if ( CheckMarked(ob) ) {
    //topLineMessage("Multi file permissions coming soon");
    for (i = 0; i < totalfilecount; i++)
      {
        if ( *ob[i].marked )
          {
            strcpy(pfile, currentpwd);
            if (!check_last_char(pfile, "/")){
              strcat(pfile, "/");
            }
            strcat(pfile, ob[i].name);
            chmod(pfile, newperm);
          }
      }
  } else {
    strcpy(pfile, currentpwd);
    if (!check_last_char(pfile, "/")){
      strcat(pfile, "/");
    }
    strcat(pfile, ob[selected].name);
    chmod(pfile, newperm);
  }
  ob = get_dir(currentpwd);
  clear_workspace();
  reorder_ob(ob, sortmode);
  display_dir(currentpwd, ob, topfileref, selected);

  printMenu(0, 0, fileMenuText);
  directory_view_menu_inputs0();

}

void modify_key_menu_inputs()
{
  viewMode = 2;
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
          printMenu(0, 0, fileMenuText);
          directory_view_menu_inputs0();
          break;
        }
    }
}

void directory_view_menu_inputs1()
{
  viewMode = 1;
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
          printMenu(0, 0, fileMenuText);
          printMenu(LINES-1, 0, functionMenuText);
          display_dir(currentpwd, ob, topfileref, selected);
          directory_view_menu_inputs0();
          break;
        case 'e':
          edit_file_input();
          printMenu(0, 0, fileMenuText);
          printMenu(LINES-1, 0, functionMenuText);
          display_dir(currentpwd, ob, topfileref, selected);
          directory_view_menu_inputs0();
          break;
        case 'h':
          topLineMessage("Documentation coming soon");
          break;
        case 'q':
          quit_menu();
          break;
        case 's':
          show_directory_input();
          show_directory_inputs();
          break;
        case 27:
          printMenu(0, 0, fileMenuText);
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
  int e = 0;
  viewMode = 0;
  while(1)
    {
      //signal(SIGWINCH, refreshScreen );
      //sigaction(SIGWINCH, &sa, NULL);
      *pc = getch();
      switch(*pc)
        {
        case 'c':
          if ( CheckMarked(ob) ) {
            copy_multi_file_input(ob, currentpwd);
          } else {
            strcpy(selfile, currentpwd);
            if (!check_last_char(selfile, "/")){
              strcat(selfile, "/");
            }
            strcat(selfile, ob[selected].name);
            if (!check_dir(selfile)){
              copy_file_input(selfile);
            }
          }
          break;
        case 'd':
          if ( CheckMarked(ob) ) {
            delete_multi_file_confirm_input(ob);
            ob = get_dir(currentpwd);
            clear_workspace();
            reorder_ob(ob, sortmode);
            display_dir(currentpwd, ob, topfileref, selected);
            printMenu(0, 0, fileMenuText);
            directory_view_menu_inputs0();
          } else {
            strcpy(selfile, currentpwd);
            if (!check_last_char(selfile, "/")){
              strcat(selfile, "/");
            }
            strcat(selfile, ob[selected].name);
            if (!check_dir(selfile)){
              printMenu(0,0, "Delete file? (!Yes/!No)");
              delete_file_confirm_input(selfile);
            }
          }
          break;
        case 'e':
          strcpy(chpwd, currentpwd);
          if (!check_last_char(chpwd, "/")){
            strcat(chpwd, "/");
          }
          strcat(chpwd, ob[selected].name);
          if (!check_dir(chpwd)){
            SendToEditor(chpwd);
            printMenu(0, 0, fileMenuText);
            printMenu(LINES-1, 0, functionMenuText);
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
          printMenu(0, 0, modifyMenuText);
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
            historyref = 0; // Reset historyref here. A hacky workaround due to the value occasionally dipping to minus numbers.
            quit_menu();
          }
          break;
        case'r':
          if ( CheckMarked(ob) ) {
            rename_multi_file_input(ob, currentpwd);
          } else {
            strcpy(selfile, currentpwd);
            if (!check_last_char(selfile, "/")){
              strcat(selfile, "/");
            }
            strcat(selfile, ob[selected].name);
            rename_file_input(selfile);
          }
          break;
        case 's':
          strcpy(chpwd, currentpwd);
          if (!check_last_char(chpwd, "/")){
            strcat(chpwd, "/");
          }
          strcat(chpwd, ob[selected].name);
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
            e = SendToPager(chpwd);
            printMenu(0, 0, fileMenuText);
            printMenu(LINES-1, 0, functionMenuText);
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 27:
          printMenu(0, 0, globalMenuText);
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
          strcpy(selfile, currentpwd);
          if (!check_last_char(selfile, "/")){
            strcat(selfile, "/");
          }
          strcat(selfile, ob[selected].name);
          if (!check_dir(selfile)){
            if ( *ob[selected].marked ){
              *ob[selected].marked = 0;
              clear_workspace();
            } else {
              *ob[selected].marked = 1;
            }
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 271: // F7
          markall = 1;
          ob = get_dir(currentpwd);
          markall = 0; // Leaving this set as 1 keeps things marked even after refresh. This is bad
          clear_workspace();
          reorder_ob(ob, sortmode);
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 272: // F8
          markall = 0;
          ob = get_dir(currentpwd);
          clear_workspace();
          reorder_ob(ob, sortmode);
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 273: // F9
          printMenu(0, 0, sortMenuText);
          sort_view_inputs();
          break;
        case 274: // F10
          refreshScreen();
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
    }
}
void directory_change_menu_inputs()
{
  viewMode = 4;
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
          printMenu(0, 0, globalMenuText);
          //printMenu(0, 0, fileMenuText);
          //printMenu(LINES-1, 0, functionMenuText);
          //display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 'e':
          edit_file_input();
          printMenu(0, 0, globalMenuText);
          break;
        case 'h':
          topLineMessage("Documentation coming soon");
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

void topLineMessage(const char *message){
  move(0,0);
  clrtoeol();
  attron(A_BOLD);
  mvprintw(0,0, "%s", message);
  attroff(A_BOLD);
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        default: // Where's the "any" key?
          printMenu(0, 0, fileMenuText);
          directory_view_menu_inputs0();
          break;
        }
    }
}

