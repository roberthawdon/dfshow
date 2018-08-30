/*
  DF-SHOW - A clone of 'SHOW' directory browser from DF-EDIT by Larry Kroeker
  Copyright (C) 2018  Robert Ian Hawdon

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE
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
#include <regex.h>
#include "common.h"
#include "showfunctions.h"
#include "show.h"
#include "colors.h"

int c;
int * pc = &c;

char chpwd[1024];
char selfile[1024];

char ownerinput[256];
char groupinput[256];
char uids[24];
char gids[24];
char errmessage[256];
char currentfilename[512];

int s;
char *buf;
char *rewrite;

int blockstart = -1;
int blockend = -1;

int abortinput = 0;

extern results* ob;
extern history* hs;
extern char currentpwd[1024];

extern int historyref;
extern int selected;
extern int topfileref;
extern int hpos;
extern int maxdisplaywidth;
extern int totalfilecount;
extern int displaysize;
extern int displaycount;
extern int showhidden;
extern int markall;
extern int viewMode;

extern int reverse;

extern int invalidstart;

extern char fileMenuText[256];
extern char globalMenuText[256];
extern char functionMenuText[256];
extern char functionMenuTextShort[256];
extern char functionMenuTextLong[256];
extern char modifyMenuText[256];
extern char sortMenuText[256];

extern struct sigaction sa;

extern char sortmode[5];

extern int commandL, infoL, inputL, selectL, displayL, dangerL, dirL, slinkL, exeL, suidL, sgidL, hiliteL;

extern char *objectWild;

//char testMenu[256];

int sanitizeTopFileRef(int topfileref)
{
  if ( (selected - topfileref ) > displaysize ){
    // We don't want the selected item off the bottom of the screen
    topfileref = selected - displaysize + 1;
  } else if ( (topfileref > totalfilecount) ){
    // If the top file ref exceeds the number of files, we'll want to do something about that
    topfileref = totalfilecount - (displaysize);
  } else if ( (selected - topfileref) < 0 ) {
    // We certainly don't want the top file ref in the negatives
    topfileref = 0;
  } else if ((topfileref + displaysize) > totalfilecount){
    // If we end up with the top file ref in a position where the list of files ends before the end of the screen, we need to sort that too.
    topfileref = totalfilecount - (displaysize);
  }
  if ( selected == 0 ) {
    // Just in case we're thrust to the top of the list - like when in a hidden directory and hidden files are switched off
    topfileref = 0;
  }
  if (topfileref < 0){
    // Likewise, we don't want the topfileref < 0 here either
    topfileref = 0;
  }
  if ( totalfilecount < displaysize ){
    // Finally, to override all of the above, if we have less files than display, reset top file ref to 0.
    topfileref = 0;
  }

  return topfileref;
}

void refreshDirectory(char *sortmode, int origtopfileref, int origselected)
{
  char currentselectname[512];
  strcpy(currentselectname, ob[origselected].name);
  ob = get_dir(currentpwd);
  clear_workspace();
  reorder_ob(ob, sortmode);
  selected = findResultByName(ob, currentselectname);
  // topfileref = sanitizeTopFileRef(origtopfileref);
  if (((selected - topfileref) < 0 ) || (selected - topfileref) > displaysize ){
    topfileref = sanitizeTopFileRef(selected);
  }
  display_dir(currentpwd, ob, topfileref, selected);
}

void directory_view_menu_inputs(); // Needed to allow menu inputs to switch between each other
void global_menu_inputs();

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
  testSlash:
  if (check_last_char(currentpwd, "/") && strcmp(currentpwd, "/")){
    currentpwd[strlen(currentpwd) - 1] = '\0';
    goto testSlash;
  }
  if ((strcmp(currentpwd, oldpwd) && strcmp(currentpwd, "")) || !historyref){
    objectWild = objectFromPath(currentpwd);
    if ( strchr(objectWild, MULTICHAR) || strchr(objectWild, ONECHAR)){
      strcpy(currentpwd, dirFromPath(currentpwd));
    } else {
      strcpy(objectWild, "");
    }

    if (check_first_char(currentpwd, "~")){
      rewrite = str_replace(currentpwd, "~", getenv("HOME"));
      strcpy(currentpwd, rewrite);
      free(rewrite);
    }
    // if (!check_dir(currentpwd)){
    //   global_menu();
    // }
    if ( invalidstart ){
      invalidstart = 0;
      set_history(currentpwd, "", "", 0, 0);
    } else {
      set_history(currentpwd, objectWild, ob[selected].name, topfileref, selected);
    }
    topfileref = 0;
    selected = 0;
    chdir(currentpwd);
    refreshDirectory(sortmode, 0, selected);
  } else {
    strcpy(currentpwd, oldpwd); // Copying old value back if the input was aborted
  }
  directory_view_menu_inputs();
}

int replace_file_confirm_input(char *filename)
{
  char message[1024];
  sprintf(message, "Replace file [<%s>]? (!Yes/!No)", filename);
  printMenu(0,0, message);
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
    if (check_first_char(newfile, "~")){
      rewrite = str_replace(newfile, "~", getenv("HOME"));
      strcpy(newfile, rewrite);
      free(rewrite);
    }
    if ( check_file(newfile) )
      {
        if ( replace_file_confirm_input(newfile) )
          {
            copy_file(file, newfile);
            refreshDirectory(sortmode, 0, selected);
          }
      } else {
      copy_file(file, newfile);
      refreshDirectory(sortmode, 0, selected);
    }
  }
  directory_view_menu_inputs();
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
    if (check_first_char(dest, "~")){
      rewrite = str_replace(dest, "~", getenv("HOME"));
      strcpy(dest, rewrite);
      free(rewrite);
    }
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
                  if ( replace_file_confirm_input(destfile) )
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
  directory_view_menu_inputs();
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
    if (check_first_char(dest, "~")){
      rewrite = str_replace(dest, "~", getenv("HOME"));
      strcpy(dest, rewrite);
      free(rewrite);
    }
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
                  if ( replace_file_confirm_input(destfile) )
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
    refreshDirectory(sortmode, 0, selected);
  }

  directory_view_menu_inputs();
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
    if (check_first_char(dest, "~")){
      rewrite = str_replace(dest, "~", getenv("HOME"));
      strcpy(dest, rewrite);
      free(rewrite);
    }
    if ( check_file(dest) )
      {
        if ( replace_file_confirm_input(dest) )
          {
            RenameObject(file, dest);
            refreshDirectory(sortmode, 0, selected);
          }
      } else {
      RenameObject(file, dest);
      refreshDirectory(sortmode, 0, selected);
    }
  }
  directory_view_menu_inputs();
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
    if (check_first_char(newdir, "~")){
      rewrite = str_replace(newdir, "~", getenv("HOME"));
      strcpy(newdir, rewrite);
      free(rewrite);
    }
    mk_dir(newdir);
    curs_set(FALSE);
    refreshDirectory(sortmode, 0, selected);
  }
  directory_view_menu_inputs();
}

char * execute_argument_input(const char *exec)
{
  char *strout;
  int execlen = strlen(exec);
  strout = malloc(sizeof(char) * 1024);
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, "Args to pass to %s:", exec);
  curs_set(TRUE);
  move(0, 18 + execlen);
  if (readline(strout, 1024, "") == -1){
    abortinput = 1;
  }
  curs_set(FALSE);
  return strout;
}

int huntCaseSelectInput()
{
  int result = 0;
  char message[1024];
  sprintf(message,"Case Sensitive, !Yes/!No/<ESC> (enter = no)");
  printMenu(0,0, message);
  while(1)
    {
    huntCaseLoop:
      *pc = getch();
      switch(*pc)
        {
        case 'y':
          result = 1;
          break;
	case 10:
        case 'n':
          result = 0;
          break;
        case 27:
          result = -1;
          break;
        default:
          goto huntCaseLoop;
        }
      break;
    }
  return(result);
}

void huntInput(int selected, int charcase)
{
  int regexcase;
  int i;
  char regexinput[1024];
  char inputmessage[32];
  if (charcase){
    regexcase = 0;
    strcpy(inputmessage, "Match Case - Enter string:");
  } else {
    regexcase = REG_ICASE;
    strcpy(inputmessage, "Ignore Case - Enter string:");
  }
  move(0,0);
  clrtoeol();
  mvprintw(0, 0, inputmessage);
  curs_set(TRUE);
  move(0, strlen(inputmessage) + 1);
  curs_set(FALSE);
  if (readline(regexinput, 1024, "") == -1) {
    abortinput = 1;
  } else {
    if (!CheckMarked(ob)){
      strcpy(chpwd, currentpwd);
      if (!check_last_char(chpwd, "/")){
        strcat(chpwd, "/");
      }
      strcat(chpwd, ob[selected].name);
      if (huntFile(chpwd, regexinput, regexcase)){
        *ob[selected].marked = 1;
      }
    } else {
      for (i = 0; i < totalfilecount; i++){
        if ( *ob[i].marked ){
          strcpy(chpwd, currentpwd);
          if (!check_last_char(chpwd, "/")){
            strcat(chpwd, "/");
          }
          strcat(chpwd, ob[i].name);
          if (huntFile(chpwd, regexinput, regexcase)){
            *ob[i].marked = 1;
          } else {
            *ob[i].marked = 0;
          }
          clear_workspace();
        }
      }
    }
  }
}

void delete_file_confirm_input(char *file)
{
  printMenu(0,0, "Delete file? (!Yes/!No)");
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 'y':
          delete_file(file);
          refreshDirectory(sortmode, topfileref, selected);
          // Not breaking here, intentionally dropping through to the default
        default:
          directory_view_menu_inputs();
          break;
        }
    }
}

void delete_multi_file_confirm_input(results* ob)
{
  int i, k;
  int allflag = 0;
  int abortflag = 0;
  char message[1024];

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
            sprintf(message,"Delete file [<%s>]? (!Yes/!No/!All/!Stop)", selfile);
            printMenu(0,0, message);
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
  printMenu(0, 0, sortMenuText);
  viewMode = 3;
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        case 27: // ESC Key
          directory_view_menu_inputs();
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
      refreshDirectory(sortmode, topfileref, selected);
      directory_view_menu_inputs();
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
    refreshDirectory(sortmode, topfileref, selected);

    directory_view_menu_inputs();
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
  refreshDirectory(sortmode, topfileref, selected);

  directory_view_menu_inputs();

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
          directory_view_menu_inputs();
          break;
        }
    }
}

void directory_view_menu_inputs()
{
  int e = 0;
  char *updir;
  char *execArgs;
  viewMode = 0;
  printMenu(0, 0, fileMenuText);
  printMenu(LINES-1, 0, functionMenuText);
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
            refreshDirectory(sortmode, topfileref, selected);
            directory_view_menu_inputs();
          } else {
            strcpy(selfile, currentpwd);
            if (!check_last_char(selfile, "/")){
              strcat(selfile, "/");
            }
            strcat(selfile, ob[selected].name);
            if (!check_dir(selfile)){
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
            // display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 'h':
          strcpy(currentfilename, ob[selected].name);
          if (showhidden == 0) {
            showhidden = 1;
          } else {
            showhidden = 0;
          }
          ob = get_dir(currentpwd);
          clear_workspace();
          reorder_ob(ob, sortmode);
          // // Selecting top item to avoid buffer underflows
          // selected = 0;
          // topfileref = 0;
          selected = findResultByName(ob, currentfilename);
          topfileref = sanitizeTopFileRef(topfileref);
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 'm':
          printMenu(0, 0, modifyMenuText);
          modify_key_menu_inputs();
          break;
        case 'q':
          if (historyref > 1){
            strcpy(chpwd, hs[historyref - 2].path);
            objectWild = hs[historyref - 2].objectWild;
            historyref--;
            if (check_dir(chpwd)){
              //free(objectWild);
              //objectWild = objectFromPath(currentpwd);
              // if ( strchr(objectWild, MULTICHAR) || strchr(objectWild, ONECHAR)){
              //   strcpy(currentpwd, dirFromPath(currentpwd));
              // } else {
              //   strcpy(objectWild, "");
              // }
              strcpy(currentpwd, chpwd);
              chdir(currentpwd);
              ob = get_dir(currentpwd);
              reorder_ob(ob, sortmode);
              //selected = hs[historyref].selected;
              selected = findResultByName(ob, hs[historyref].name);
              topfileref = sanitizeTopFileRef(hs[historyref].topfileref);
              clear_workspace();
              // mvprintw(2,0,"totalfilecount: %i\ntopfileref: %i\nselected: %i\ndisplaysize: %i\nLINES: %i", totalfilecount, topfileref, selected, displaysize, LINES);
              display_dir(currentpwd, ob, topfileref, selected);
            }
            break;
          } else {
            historyref = 0; // Reset historyref here. A hacky workaround due to the value occasionally dipping to minus numbers.
            global_menu();
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
          if (!strcmp(ob[selected].name, "..")) {
            if (strcmp(currentpwd, "/")){
              updir = dirFromPath(currentpwd);
              strcpy(chpwd, updir);
              free(updir);
              objectWild = "";
              testSlash:
              if (check_last_char(chpwd, "/") && strcmp(chpwd, "/")){
                chpwd[strlen(chpwd) - 1] = '\0';
                goto testSlash;
              }
              set_history(chpwd, objectWild, ob[selected].name, topfileref, selected);
              topfileref = 0;
              selected = 0;
              strcpy(currentpwd, chpwd);
              chdir(currentpwd);
              refreshDirectory(sortmode, topfileref, selected);
            }
          } else if (!strcmp(ob[selected].name, ".")) {
            break;
          } else {
            if (check_dir(chpwd)){
              objectWild = "";
              set_history(chpwd, objectWild, ob[selected].name, topfileref, selected);
              topfileref = 0;
              selected = 0;
              strcpy(currentpwd, chpwd);
              chdir(currentpwd);
              refreshDirectory(sortmode, topfileref, selected);
            } else {
              e = SendToPager(chpwd);
              // display_dir(currentpwd, ob, topfileref, selected);
            }
          }
          break;
        case 'u':
          e = huntCaseSelectInput();
          if (e != -1){
            huntInput(selected, e);
          }
          abortinput = 0;
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 'x':
          strcpy(chpwd, currentpwd);
          if (!check_last_char(chpwd, "/")){
            strcat(chpwd, "/");
          }
          strcat(chpwd, ob[selected].name);
          if (check_exec(chpwd)){
            execArgs = execute_argument_input(ob[selected].name);
            if (!abortinput){
              LaunchExecutable(chpwd, execArgs);
              free(execArgs);
            }
            abortinput = 0;
            display_dir(currentpwd, ob, topfileref, selected);
          } else {
            topLineMessage("Error: Permission denied");
          }
          break;
        case 27:
          global_menu_inputs();
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
          if (hpos > 0){
            hpos--;
            clear_workspace();
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 261: // Right Arrow
          if (hpos < (maxdisplaywidth - COLS)){
            hpos++;
            clear_workspace();
            display_dir(currentpwd, ob, topfileref, selected);
          }
          break;
        case 338: // PgDn - Drop through
        case 265: // F1
          if (selected < (totalfilecount - 1) ) {
            clear_workspace();
            topfileref = topfileref + displaycount;
            if (topfileref > (totalfilecount - displaycount)){
              topfileref = totalfilecount - displaycount;
            }
            selected = selected + displaycount;
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
          refreshDirectory(sortmode, topfileref, selected);
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
            if (selected < (totalfilecount - 1)) {
              selected++;
              if (selected > ((topfileref + displaysize) - 1)){
                topfileref++;
                clear_workspace();
              }
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
          sort_view_inputs();
          break;
        case 274: // F10
          strcpy(selfile, currentpwd);
          if (!check_last_char(selfile, "/")){
            strcat(selfile, "/");
          }
          strcat(selfile, ob[selected].name);
          if ( *ob[selected].marked == 0 ){
            if ( blockstart == -1 ){
              blockstart = selected;
              if (!check_dir(selfile)){
                *ob[selected].marked = 1;
              }
              if (selected < (totalfilecount - 1)) {
                selected++;
                if (selected > ((topfileref + displaysize) - 1)){
                  topfileref++;
                  clear_workspace();
                }
                display_dir(currentpwd, ob, topfileref, selected);
              }
            } else {
              blockend = selected;
              if (blockstart > blockend){
                // While we're still on the second item, let's flip them around if the second selected file is higher up the list.
                blockend = blockstart;
                blockstart = selected;
              }
              for(; blockstart < blockend + 1; blockstart++){
                strcpy(selfile, currentpwd);
                if (!check_last_char(selfile, "/")){
                  strcat(selfile, "/");
                }
                strcat(selfile, ob[blockstart].name);
                if (!check_dir(selfile)){
                  *ob[blockstart].marked = 1;
                }
              }
              blockstart = blockend = -1;
              display_dir(currentpwd, ob, topfileref, selected);
              }
            }
          break;
        case 276: // F12
          // clear();
          // endwin();

          // printf("0: %s - %s\n", hs[0].path, hs[0].objectWild);
          // printf("1: %s - %s\n", hs[1].path, hs[1].objectWild);
          // printf("2: %s - %s\n", hs[2].path, hs[2].objectWild);
          // printf("3: %s - %s\n", hs[3].path, hs[3].objectWild);
          // printf("4: %s - %s\n", hs[4].path, hs[4].objectWild);
          // printf("5: %s - %s\n", hs[5].path, hs[5].objectWild);
          // printf("6: %s - %s\n", hs[6].path, hs[6].objectWild);

          // exit(0);
          //refreshScreen();
          break;
        case 262: // Home
          selected = topfileref;
          display_dir(currentpwd, ob, topfileref, selected);
          break;
        case 360: // End
          selected = topfileref + (displaycount - 1);
          display_dir(currentpwd, ob, topfileref, selected);
          break;
          // default:
          //     mvprintw(LINES-2, 1, "Character pressed is = %3d Hopefully it can be printed as '%c'", c, c);
          //     refresh();
        }
    }
}
void global_menu_inputs()
{
  printMenu(0, 0, globalMenuText);
  if (historyref == 0){
    viewMode = 4;
  } else {
    viewMode = 1;
  }
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
          if (historyref == 0){
            printMenu(0, 0, globalMenuText);
          } else {
            // display_dir(currentpwd, ob, topfileref, selected);
            directory_view_menu_inputs();
          }
          break;
        case 'e':
          edit_file_input();
          if (historyref == 0){
            printMenu(0, 0, globalMenuText);
          } else {
            //   display_dir(currentpwd, ob, topfileref, selected);
            directory_view_menu_inputs();
          }
          break;
        case 'h':
          showManPage("show");
          refreshScreen();
          if (historyref == 0){
            printMenu(0, 0, globalMenuText);
          } else {
            //   display_dir(currentpwd, ob, topfileref, selected);
            directory_view_menu_inputs();
          }
          break;
        case 'q':
          if (historyref == 0){
            free(hs);
            exittoshell();
            refresh();
          } else {
            historyref = 0;
            global_menu();
          }
          break;
        case 's':
          show_directory_input();
          break;
        case 27:
          if (historyref != 0){
            directory_view_menu_inputs();
          }
          break;
        }
    }
}
