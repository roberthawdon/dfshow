/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2021  Robert Ian Hawdon

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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libconfig.h>
#include <wchar.h>
#include <signal.h>
#include <math.h>
#include <sys/wait.h>
#include "menu.h"
#include "colors.h"
#include "config.h"
#include "common.h"

DIR *folder;
FILE *file;

int exitCode = 0;
int enableCtrlC = 0;

char globalConfLocation[128];
char homeConfLocation[128];

char themeName[128] = "default";

char errmessage[256];

extern int * pc;

extern int resized;

void refreshScreen(); // This reference needs to exist to allow getch10th to be common.

void unloadMenuLabels();

void freeSettingVars();

int getch10th (void) {
  int ch;
  do {
    if (resized) {
      resized = 0;
      refreshScreen();
    }
    halfdelay (1);
    ch = getch();
  } while (ch == ERR || ch == KEY_RESIZE);
  return ch;
}

int splitPath(pathDirs **dirStruct, char *path){
  int e, i, j, c;
  pathDirs *tmp;

  tmp = malloc(sizeof(pathDirs));
  if (tmp){
    *dirStruct = tmp;
  }

  e = -1;
  j = 0;
  c = strlen(path);

  for(i = 0; i < c; i++){
    if (path[i] == '/'){
      if (e > -1){
        (*dirStruct)[e].directories[j] = '\0';
        if(!strcmp((*dirStruct)[e].directories, "..")){
          // assmue .. and remove the element before
          (*dirStruct)[e] = (*dirStruct)[e - 1];
          (*dirStruct)[e - 1] = (*dirStruct)[e - 2];
          e--;
          (*dirStruct) = realloc((*dirStruct), sizeof(pathDirs) * (2 + e));
        } else if (!strcmp((*dirStruct)[e].directories, ".")){
          // strip single .
          strcpy((*dirStruct)[e].directories, "\0");
        } else {
          // If element created is NOT ..
          e++;
          (*dirStruct) = realloc((*dirStruct), sizeof(pathDirs) * (2 + e));
        }
      } else {
        e++;
        (*dirStruct) = realloc((*dirStruct), sizeof(pathDirs) * (2 + e));
      }
      j=0;
    } else {
      (*dirStruct)[e].directories[j] = path[i];
      j++;
    }
  }
  (*dirStruct)[e].directories[j] = '\0';
  if (!strcmp((*dirStruct)[e].directories, ".")){
    strcpy((*dirStruct)[e].directories, "");
    e--;
  }

  return(e);
}

int createParentsInput(char *path)
{
  int result = 0;
  int messageLen;
  wchar_t *message = malloc(sizeof(wchar_t) * 1);

  messageLen = (strlen(path) + 64);

  message = realloc(message, sizeof(wchar_t) * (messageLen + 1));

  swprintf(message, messageLen, L"Directory [<%s>] does not exist. Create it? !Yes/!No (enter = no)", path);
  wPrintMenu(0,0, message);
  while(1)
    {
      *pc = getch10th();
      if (*pc == 'y'){
        result = 1;
        break;
      } else if ((*pc == 'n') || (*pc == 10)){
        result = 0;
        break;
      } else if (*pc == 27){
        result = -1;
        break;
      }
    }
  free(message);
  return(result);
}

void createParentDirs(char *path){
  pathDirs *targetPath;
  char *tempPath = malloc(sizeof(char) + 1);
  int e, i = 0;

  e = splitPath(&targetPath, path);

  strcpy(tempPath, "");
  for (i = 0; i < e; i++){
    tempPath = realloc(tempPath, sizeof(char) * (strlen(tempPath) + strlen(targetPath[i].directories) + 2));
    sprintf(tempPath, "%s/%s", tempPath, targetPath[i].directories);
    if (!check_dir(tempPath)){
      mk_dir(tempPath);
    }
  }

  free(targetPath);
  free(tempPath);
  return;
}

void mk_dir(char *path)
{
  struct stat st = {0};

  if (stat(path, &st) == -1) {
    mkdir(path, 0755);
  }
}

void setConfLocations()
{
  sprintf(globalConfLocation, "%s/%s", SYSCONFIG, CONF_NAME);

  sprintf(homeConfLocation, "%s/%s/%s", getenv("HOME"), HOME_CONF_DIR, CONF_NAME);
}

int exittoshell()
{
  clear();
  unloadMenuLabels();
  freeSettingVars();
  endwin();
  exit(exitCode);
  return exitCode;
}

char * dirFromPath(const char* myStr){
  char *outStr;
  int i = strlen(myStr);
  int n = 0;

  while(i <= strlen(myStr) && myStr[i] != '/'){
    i--;
  }

  outStr = malloc(sizeof (char) * (i + 2));

  if (i < 2){
    strcpy(outStr, "/");
  } else{
    while(n <= i){
      outStr[n] = myStr[n];
      n++;
    }

    outStr[n - 1] = '\0';
  }

  return outStr;

}

char * objectFromPath(const char *myStr){
  char *outStr;
  int i = strlen(myStr);
  int n = 0;
  int c = 0;

  while(i <= strlen(myStr) && myStr[i] != '/'){
    i--;
    n++;
  }

  outStr = malloc(sizeof (char) * n);

  i++; // Removes the initial /

  for(; i < strlen(myStr); c++){
    outStr[c] = myStr[i];
    i++;
  }

  outStr[n - 1] = '\0';
  return outStr;

}

void printVersion(char* programName){
  printf (("Directory File Show (DF-SHOW) - %s %s\n"), programName, VERSION);
  fputs (("\
Copyright (C) 2021 Robert Ian Hawdon\n\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n\
This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n\
are welcome to redistribute it under certain conditions.\n"), stdout);
}

int check_dir(char *pwd)
{
  const char *path = pwd;
  struct stat sb;
  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    folder = opendir ( path );
    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        closedir ( folder );
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
  return 0;
}

int check_file(char *file){
  if( access( file, F_OK ) != -1 ) {
    return 1;
  } else {
    return 0;
  }
}

int check_exec(const char *object)
{
  if (access(object, X_OK) == 0) {
    return 1;
  } else {
    return 0;
  }
}

int check_last_char(const char *str, const char *chk)
{
  if (!strcmp(&str[strlen(str) - 1], chk)){
    return 1;
  } else {
    return 0;
  }
}

int check_first_char(const char *str, const char *chk)
{
  if (str[0] == chk[0]){
    return 1;
  } else {
    return 0;
  }
}

int check_numbers_only(const char *s)
{
  while (*s) {
    if (isdigit(*s++) == 0) return 0;
  }

  return 1;
}

// Credit for the following function must go to this guy:
// https://stackoverflow.com/a/779960
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

char * read_line(FILE *fin) {
  char *buffer;
  char *tmp;
  int read_chars = 0;
  int bufsize = 8192;
  char *line = malloc(bufsize);

  if ( !line ) {
    return NULL;
  }

  buffer = line;

  while ( fgets(buffer, bufsize - read_chars, fin) ) {
    read_chars = strlen(line);

    if ( line[read_chars - 1] == '\n' ) {
      line[read_chars - 1] = '\0';
      return line;
    }

    else {
      bufsize = 2 * bufsize;
      tmp = realloc(line, bufsize);
      if ( tmp ) {
        line = tmp;
        buffer = line + read_chars;
      }
      else {
        free(line);
        return NULL;
      }
    }
  }
  return NULL;
}

void showManPage(const char * prog)
{
  char mancmd[10];
  int i;
  sprintf(mancmd, "man %s", prog);
  clear();
  system("clear"); // Needed to ensure man pages display correctly
  system(mancmd);
}

int can_run_command(const char *cmd) {
  const char *path = getenv("PATH");
  char *buf;
  char *p;
  if(strchr(cmd, '/')) {
      return access(cmd, X_OK)==0;
  }
  if(!path){
    return -1; // something is horribly wrong...
  }
  buf = malloc(strlen(path)+strlen(cmd)+3);
  for(; *path; ++path) {
    p = buf;
    for(; *path && *path!=':'; ++path,++p) {
        *p = *path;
    }
    if(p==buf) *p++='.';
    if(p[-1]!='/') *p++='/';
    strcpy(p, cmd);
    if(access(buf, X_OK)==0) {
        free(buf);
        return 1;
    }
    if(!*path) break;
  }
  free(buf);
  return 0;
}

char * commandFromPath(const char *cmd) {
  const char *path = getenv("PATH");
  char *outStr;
  char *p;
  if(strchr(cmd, '/')) {
      outStr = malloc(strlen(cmd)+1);
      sprintf(outStr, "%s", cmd);
      return outStr;
  }
  if(!path){
    return NULL; // something is horribly wrong...
  }
  outStr = malloc(strlen(path)+strlen(cmd)+3);
  for(; *path; ++path) {
    p = outStr;
    for(; *path && *path!=':'; ++path,++p) {
        *p = *path;
    }
    if(p==outStr) *p++='.';
    if(p[-1]!='/') *p++='/';
    strcpy(p, cmd);
    if(access(outStr, X_OK)==0) {
        return outStr;
    }
    if(!*path) break;
  }
  free(outStr);
  return NULL;
}

int launchExternalCommand(char *cmd, char* args, ushort_t mode)
{
  sigset_t newMask, oldMask;
  pid_t parent = getpid();
  pid_t pid;

  sigemptyset(&newMask);
  sigemptyset(&oldMask);

  char *arguments[] = {cmd, args, NULL};

  if (mode & M_NORMAL){
    curs_set(TRUE);
    echo();
    nocbreak();
    keypad(stdscr, FALSE);
    endwin();
  } else if (mode & M_NONE) {
    clear();
    refresh();
  }

  pid = fork();
  if (pid == -1){
    // Catch error
    return -1;
  } else if ( pid == 0) {
    execv(cmd, arguments);
    _exit(EXIT_FAILURE);
  } else if ( pid > 0 ) {
    // clear();
    // refresh();
    int status;
    sigaddset(&newMask, SIGWINCH);
    sigprocmask(SIG_BLOCK, &newMask, &oldMask);
    waitpid(pid, &status, 0);
    sigprocmask(SIG_SETMASK, &oldMask, NULL);
  }
  // endwin();
  // reset_prog_mode();
  // clear();
  // cbreak();
  // noecho();
  // curs_set(FALSE);
  // keypad(stdscr, TRUE);
  // refresh();
  // initscr();
  refreshScreen();
  return 0;
}

void sigintHandle(int sig){
  // Does nothing
}

