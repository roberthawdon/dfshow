/*
  DF-SHOW: An interactive directory/file browser written for Unix-like systems.
  Based on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.
  Copyright (C) 2018-2024  Robert Ian Hawdon

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
#include <libintl.h>
#include "menu.h"
#include "colors.h"
#include "config.h"
#include "common.h"
#include "banned.h"
#include "i18n.h"

DIR *folder;
FILE *file;

MEVENT event;

int c;
int * pc = &c;

bool parentShow = false;

int exitCode = 0;
int enableCtrlC = 0;

char globalConfLocation[128];
char homeConfLocation[128];

char themeName[128] = "default";

char *errmessage;

int displaycount;

int i, s;

int abortinput = 0;

int displaysize;

char fileName[4096];

int viewMode = 0;

char *programName;

int returnCode;

extern int * pc;

extern int resized;

extern bool topMenu;
extern bool bottomMenu;
extern wchar_t *topMenuBuffer;
extern wchar_t *bottomMenuBuffer;

void refreshScreenShow();
void refreshScreenSf();

void refreshScreen(char *application)
{
  endwin();
  clear();
  cbreak();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  refresh();
  #ifdef APPLICATION_SHOW
    if (!strcmp(application, "show")) {
      refreshScreenShow();
    }
  #endif
  #ifdef APPLICATION_SF
    if (!strcmp(application, "sf")) {
      refreshScreenSf();
    }
  #endif
}

// void unloadMenuLabels();

// void freeSettingVars();

int getch10th (void) {
  int ch;
  do {
    if (resized) {
      resized = 0;
      refreshScreen(programName);
      ch = -1;
      break;
    }
    halfdelay (1);
    ch = getch();
  } while (ch == ERR || ch == KEY_RESIZE);
  return ch;
}

int setDynamicWChar(wchar_t **str, const wchar_t *format, ...)
{
  int length = 0;
  wchar_t tmpBuf[1024];
  va_list pArg1, pArg2;
  va_start(pArg1, format);
  va_copy(pArg2, pArg1);

  length = vswprintf(tmpBuf, 1024, format, pArg1);
  length++; // Adds 1 for null terminator

  *str = malloc(sizeof(wchar_t) * length);
  vswprintf(*str, length, format, pArg2);

  va_end(pArg1);
  va_end(pArg2);
  return(length);
}

int setDynamicChar(char **str, const char *format, ...)
{
  int length = 0;
  va_list pArg1, pArg2;
  va_start(pArg1, format);
  va_copy(pArg2, pArg1);

  length = vsnprintf(NULL, 0, format, pArg1);
  length++; // Adds 1 for null terminator

  *str = malloc(sizeof(char) * length);
  vsnprintf(*str, length, format, pArg2);

  va_end(pArg1);
  va_end(pArg2);
  return(length);
}

int splitString(splitStrStruct **result, char *input, int splitChar, bool filePath){
  int e, i, j, c;
  splitStrStruct *tmp;

  tmp = malloc(sizeof(splitStrStruct));
  if (tmp){
    *result = tmp;
  }

  e = -1;
  j = 0;
  c = strlen(input);

  for(i = 0; i < c; i++){
    if (input[i] == splitChar){
      if (e > -1){
        (*result)[e].subString[j] = '\0';
        if (filePath){
          if(!strcmp((*result)[e].subString, "..")){
            // assmue .. and remove the element before
            (*result)[e] = (*result)[e - 1];
            (*result)[e - 1] = (*result)[e - 2];
            e--;
            (*result) = realloc((*result), sizeof(splitStrStruct) * (2 + e));
          } else if (!strcmp((*result)[e].subString, ".")){
            // strip single .
            (*result)[e].subString[0]=0;
          } else {
            // If element created is NOT ..
            e++;
            (*result) = realloc((*result), sizeof(splitStrStruct) * (2 + e));
          }
        } else {
          e++;
          (*result) = realloc((*result), sizeof(splitStrStruct) * (2 + e));
        }
      } else {
        e++;
        (*result) = realloc((*result), sizeof(splitStrStruct) * (2 + e));
      }
      j=0;
    } else {
      (*result)[e].subString[j] = input[i];
      j++;
    }
  }
  (*result)[e].subString[j] = '\0';
  if (!strcmp((*result)[e].subString, ".")){
    (*result)[e].subString[0]=0;
    e--;
  }

  return(e);
}

int createParentsInput(char *path)
{
  int result = 0;
  char *message;

  setDynamicChar(&message, _("Directory [<%s>] does not exist. Create it? !Yes/!No (enter = no)"), path);

  printMenu(0,0, message);

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
  splitStrStruct *targetPath;
  char *tempPath = malloc(sizeof(char) * 1);
  char *tempPathWork;
  int e, i, t = 0;

  e = splitString(&targetPath, path, '/', true);

  tempPath[0]=0;
  for (i = 0; i < e; i++){
    t = setDynamicChar(&tempPathWork, "%s/%s", tempPath, targetPath[i].subString);
    tempPath = realloc(tempPath, sizeof(char) + t);
    memcpy(tempPath, tempPathWork, t);
    free(tempPathWork);
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
  snprintf(globalConfLocation, 128, "%s/%s", SYSCONFIG, CONF_NAME);

  snprintf(homeConfLocation, 128, "%s/%s/%s", getenv("HOME"), HOME_CONF_DIR, CONF_NAME);
}

int exittoshell()
{
  clear();
  // unloadMenuLabels();
  // freeSettingVars();
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
    snprintf(outStr, (i + 2), "/");
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
Copyright (C) 2024 Robert Ian Hawdon\n\
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
    int tmp_size;

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
    
    tmp_size = (strlen(orig) + ((len_with - len_rep) * count) + 1);
    tmp = result = malloc(tmp_size);

    if (!result){
      return NULL;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        memcpy(tmp, orig, len_front);
        tmp = tmp + len_front;
        memcpy(tmp, with, len_with);
        tmp = tmp + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    memcpy(tmp, orig, (strlen(orig) + 1));
    return result;
}

char * read_line(FILE *fin) {
  char *buffer;
  int read_chars = 0;
  int initBufsize = 8192;
  int bufsize = initBufsize;
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
    } else {
      bufsize = bufsize + initBufsize;
      line = realloc(line, bufsize);
      buffer = line + read_chars;
    }
  }
  return NULL;
}

void showManPage(const char * prog)
{
  char *mancmd;
  setDynamicChar(&mancmd,"%s %s", commandFromPath("man"), prog);
  clear();
  char *args[countArguments(mancmd)];
  buildCommandArguments(mancmd, args, countArguments(mancmd));
  free(mancmd);
  launchExternalCommand(args[0], args, M_NONE);
}

int can_run_command(const char *cmd) {
  const char *path = getenv("PATH");
  char *buf;
  char *p;
  if(cmd[0] == '\0'){
    return 0;
  }
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
    snprintf(p, (strlen(path)+strlen(cmd)+3), "%s", cmd);
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
  if (cmd[0] == '\0'){
    outStr = malloc(sizeof(char) + 1);
    outStr[0] = '\0';
    return outStr;
  }
  if(strchr(cmd, '/')) {
      free(outStr);
      outStr = malloc(strlen(cmd)+1);
      snprintf(outStr, (strlen(cmd)+1), "%s", cmd);
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
    snprintf(p, (strlen(path)+strlen(cmd)+3) - strlen(path), "%s", cmd);
    if(access(outStr, X_OK)==0) {
        return outStr;
    }
    if(!*path) break;
  }
  free(outStr);
  outStr = malloc(sizeof(char) + 1);
  outStr[0] = '\0';
  return outStr;
}

int countArguments(const char *cmd)
{
  int i, cmdLen, countArgs;
  bool reset = true;
  bool quote = false;
  bool doubleQuote = true;

  // Getting the length of the input
  cmdLen = strlen(cmd);

  countArgs = 1;

  // First sweep to get the number of args, and length
  for (i = 0; i < (cmdLen); i++){
    if (cmd[i] == '"'){
      if (!doubleQuote){
        doubleQuote = true;
      } else {
        doubleQuote = false;
      }
    }
    if (cmd[i] == '\'' && !doubleQuote){
      if (!quote){
        quote = true;
      } else {
        quote = false;
      }
    }
    if (cmd[i] == ' '){
      if (!quote){
        if (!reset){
          countArgs++;
        }
        reset = true;
      }
    } else {
      reset = false;
    }
  }

  return countArgs;

}

void buildCommandArguments(const char *cmd, char **args, size_t items)
{
  int k, cmdLen; // , countArgs;
  int i, itemCount, argCharCount;
  int cmdPos = 0;
  int cmdOffset = 0;
  int* itemLen = (int*) malloc(items * sizeof(int));
  int* itemLen_copy = itemLen;
  bool reset = true;
  bool quote = false;
  bool doubleQuote = false;
  char *tempStr;

  // Getting the length of the input
  cmdLen = strlen(cmd);

  // First sweep to get the number of args, and length
  itemCount = 0;
  argCharCount = 0;
  itemLen[0] = 0;
  for (i = 0; i < (cmdLen); i++){
    if (cmd[i] == '"'){
      if (!doubleQuote){
        doubleQuote = true;
      } else {
        doubleQuote = false;
      }
    }
    if (cmd[i] == '\'' && !doubleQuote){
      if (!quote){
        quote = true;
      } else {
        quote = false;
      }
    }
    if (cmd[i] == ' '){
      if (!quote){
        if (!reset){
          itemCount++;
          argCharCount = 0;
        }
        reset = true;
      } else {
        argCharCount++;
      }
    } else {
      argCharCount++;
      itemLen[itemCount] = argCharCount;
      reset = false;
    }
  }

  for (i = 0; i < (itemCount + 1); i++){
    tempStr = calloc(itemLen[i] + 1, sizeof(char));
    args[i] = calloc(itemLen[i] + 1, sizeof(char));
    for (k = 0; k < itemLen[i]; k++){
      if ((cmdPos + cmdOffset + k) > cmdLen - 1){
        // Hacky workaround to avoid buffer overflow
        break;
      }
      checkBlank:
      if (cmd[cmdPos + cmdOffset + k] == ' ' && k == 0){
        cmdOffset++;
        goto checkBlank;
      }
      if (cmd[cmdPos + cmdOffset + k] == '\'' && !doubleQuote){
        cmdOffset++;
      }
      if (cmd[cmdPos + cmdOffset + k] == '"'){
        if (!doubleQuote){
          doubleQuote = true;
        } else {
          doubleQuote = false;
          cmdOffset++;
        }
        cmdOffset++;
        goto checkBlank;
      }
      tempStr[k] = cmd[cmdPos + cmdOffset + k]; 
      if (k == (itemLen[i] - 1)){
        tempStr[k + 1] = '\0';
      }
    }
    memcpy(args[i], tempStr, strlen(tempStr));
    cmdPos += itemLen[i];
    free(tempStr);
  }  

  // The last argument MUST be NULL, so we'll add this here.
  args[itemCount + 1] = NULL;

  free(itemLen_copy);
  return;

}

int launchExternalCommand(char *cmd, char **args, ushort_t mode)
{
  sigset_t newMask, oldMask;
  pid_t pid;

  sigemptyset(&newMask);
  sigemptyset(&oldMask);

  // char *arguments[] = {cmd, *args, NULL};


  if (mode == M_NORMAL){
    curs_set(TRUE);
    echo();
    nocbreak();
    keypad(stdscr, FALSE);
    endwin();
  } else if (mode == M_NONE) {
    erase();
    refresh();
  }

  pid = fork();
  if (pid == -1){
    // Catch error
    return -1;
  } else if ( pid == 0) {
    execv(cmd, args);
    _exit(EXIT_FAILURE);
  } else if ( pid > 0 ) {
    int status;
    sigaddset(&newMask, SIGWINCH);
    sigprocmask(SIG_BLOCK, &newMask, &oldMask);
    waitpid(pid, &status, 0);
    sigprocmask(SIG_SETMASK, &oldMask, NULL);
  }
  refreshScreen("show");
  return 0;
}

void sigintHandle(int sig){
  // Does nothing
}

