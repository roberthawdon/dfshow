
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

int settingsPos = 0;
int settingsBinPos = -1;

menuDef *settingsMenu;
int settingsMenuSize = 0;
wchar_t *settingsMenuLabel;

char errmessage[256];

extern int * pc;

extern int resized;

void refreshScreen(); // This reference needs to exist to allow getch10th to be common.

void unloadMenuLabels();

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
Copyright (C) 2020 Robert Ian Hawdon\n\
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

void sigintHandle(int sig){
  // Does nothing
}

void updateSetting(settingIndex **settings, int index, int type, int intSetting)
{
  (*settings)[index].intSetting = intSetting;
}

void addT1CharValue(t1CharValues **values, int *totalItems, int *maxItem, char *refLabel, char *value)
{
  t1CharValues *tmp;
  int i = *totalItems, j, k;

  if (i == 0){
    tmp = malloc(sizeof(t1CharValues) * 2);
    j = -1;
  } else {
    tmp = realloc(*values, (i + 1) * (sizeof(t1CharValues) + 1));
  }

  if (tmp){
    *values = tmp;
  }

  for (k = 0; k < i; k++){
    if (strcmp((*values)[k].refLabel, refLabel)){
      j = -1;
    } else {
      j = (*values)[k].index;
    }
  }

  j++;
  (*values)[i].index = j;
  sprintf((*values)[i].refLabel, "%s", refLabel);
  sprintf((*values)[i].value, "%s", value);

  ++*totalItems;
  ++*maxItem;

}

void addT2BinValue(t2BinValues **values, int *totalItems, int *maxItem, char *refLabel, char *settingLabel, int reset)
{
  t2BinValues *tmp;
  int value;
  int i = *totalItems, j;

  if (i == 0){
    tmp = malloc(sizeof(t2BinValues) * 2);
    j = -1;
  } else {
    tmp = realloc(*values, (i + 1) * (sizeof(t2BinValues) + 1));
  }

  if (tmp){
    *values = tmp;
  }

  if (reset == 1){
    j = -1;
  } else {
    j = ((*values))[i - 1].index;
  }

  value = pow(2, *maxItem);

  j++;
  ((*values))[i].index = j;
  sprintf((*values)[i].refLabel, "%s", refLabel);
  sprintf((*values)[i].settingLabel, "%s", settingLabel);
  ((*values))[i].value = value;
  ((*values))[i].boolVal = 0;

  ++*totalItems;
  ++*maxItem;

}

void importSetting(settingIndex **settings, int *items, char *refLabel, wchar_t *textLabel, int type, int intSetting, int maxValue, int invert)
{
  settingIndex *tmp;
  int currentItem = *items;

  if (*items == 0){
    tmp = malloc(sizeof(settingIndex) * 2);
  } else {
    tmp = realloc(*settings, (currentItem + 1) * (sizeof(settingIndex) + 1));
  }

  if (tmp){
    *settings = tmp;
  }

  (*settings)[currentItem].type = type;
  sprintf((*settings)[currentItem].refLabel, "%s", refLabel);
  swprintf((*settings)[currentItem].textLabel, 32, L"%ls", textLabel);
  (*settings)[currentItem].intSetting = intSetting;
  (*settings)[currentItem].maxValue = maxValue;
  (*settings)[currentItem].invert = invert;

  ++*items;
}

int intSettingValue(int *setting, int newValue){
  if (newValue > -1){
    *setting = newValue;
  }
  return *setting;
}

void populateBool(t2BinValues **values, char *refLabel, int setting, int maxValue)
{
  int i;

  for (i = maxValue - 1; i > -1 ; i--){
    if (!strcmp((*values)[i].refLabel, refLabel)){
      if (setting - ((*values))[i].value > - 1){
        ((*values))[i].boolVal = 1;
        setting = setting - ((*values))[i].value;
      }
    }
  }
}

void adjustBinSetting(settingIndex **settings, t2BinValues **values, char *refLabel, int *setting, int maxValue)
{
  int i;

  for (i = 0; i < maxValue + 1; i++){
    if (!strcmp((*values)[i].refLabel, refLabel) && ((*values)[i].index == settingsBinPos)){
      if ((*values)[i].boolVal > 0){
        (*settings)[settingsPos].intSetting = (*settings)[settingsPos].intSetting - (*values)[i].value;
        (*values)[i].boolVal = 0;
      } else {
        (*settings)[settingsPos].intSetting = (*settings)[settingsPos].intSetting + (*values)[i].value;
        (*values)[i].boolVal = 1;
      }
    }
  }
}

void printSetting(int line, int col, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int index, int charIndex, int binIndex, int type, int invert)
{

  int settingWork, b, c, i, v;
  int labelLen = 0, valueLen = 0, itemAdjust = 0;
  char refLabel[16];

  labelLen = wcslen((*settings)[index].textLabel) + 2;
  sprintf(refLabel, "%s", (*settings)[index].refLabel);

  for (i = 0; i < charIndex; i++){
    if (!strcmp((*values)[i].refLabel, refLabel) && ((*values)[i].index) == 0){
      v = i;
    }
  }

  for (c = 0; c < binIndex; c++){
    if (!strcmp((*bins)[c].refLabel, refLabel) && ((*bins)[c].index) == 0){
      b = c;
    }
  }

  if (type == 0 ){
    if (invert == 1){
      if ((*settings)[index].intSetting > 0){
        settingWork = 0;
      } else {
        settingWork = 1;
      }
    } else {
      settingWork = (*settings)[index].intSetting;
    }
    setColors(HILITE_PAIR);
    mvprintw(line, col, "[");
    if (settingsPos == index){
      attron(A_REVERSE);
    }
    if (settingWork == 0){
      mvprintw(line, col + 1, " ");
    } else {
      mvprintw(line, col + 1, "*");
    }
    attroff(A_REVERSE);
    mvprintw(line, col + 2, "]");
    setColors(COMMAND_PAIR);
    mvprintw(line, col + 4, "%ls", (*settings)[index].textLabel);
  } else if (type == 1){
    setColors(HILITE_PAIR);
    mvprintw(line, col, "<");
    if (settingsPos == index){
      attron(A_REVERSE);
    }
    if ((*settings)[index].maxValue > 0) {
      mvprintw(line, col + 1, "-");
    } else {
      mvprintw(line, col + 1, "?");
    }
    attroff(A_REVERSE);
    mvprintw(line, col + 2, ">");
    setColors(COMMAND_PAIR);
    mvprintw(line, col + 4, "%ls:", (*settings)[index].textLabel);
    for(i = 0; i < ((*settings)[index].maxValue); i++){
      //Temp Test
      valueLen = strlen((*values)[i + v].value) + 3;
      if (i == (*settings)[index].intSetting){
        setColors(HILITE_PAIR);
      } else {
        setColors(COMMAND_PAIR);
      }
      mvprintw(line, (col + 4 + labelLen + itemAdjust), "<%s>", (*values)[i + v].value);
      itemAdjust = itemAdjust + valueLen;
    }
  } else if (type == 2){
    setColors(HILITE_PAIR);
    mvprintw(line, col, "<");
    if (settingsPos == index && settingsBinPos < 0){
      attron(A_REVERSE);
    }
    if ((*settings)[index].maxValue > 0) {
      mvprintw(line, col + 1, " ");
    } else {
      mvprintw(line, col + 1, "?");
    }
    attroff(A_REVERSE);
    mvprintw(line, col + 2, ">");
    setColors(COMMAND_PAIR);
    mvprintw(line, col + 4, "%ls:", (*settings)[index].textLabel);
    for(i = 0; i < ((*settings)[index].maxValue); i++){
      valueLen = strlen((*bins)[i + b].settingLabel) + 3;
      if ((*bins)[i + b].boolVal == 1){
        setColors(HILITE_PAIR);
      }
      if (settingsBinPos == (i + b) ){
        attron(A_REVERSE);
      }
      mvprintw(line, (col + 4 + labelLen + itemAdjust), "<%s>", (*bins)[i + b].settingLabel);
      attroff(A_REVERSE);
      setColors(COMMAND_PAIR);
      itemAdjust = itemAdjust + valueLen;
    }
  }
}

int textValueLookup(t1CharValues **values, int *items, char *refLabel, char *value)
{
  int i;

  for (i = 0; i < *items; i++){
    if (!strcmp((*values)[i].value, value) && !strcmp((*values)[i].refLabel, refLabel)){
      return (*values)[i].index;
    }
  }

  return -1;
}
