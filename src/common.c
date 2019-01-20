/*
  DF-SHOW - A clone of 'SHOW' directory browser from DF-EDIT by Larry Kroeker
  Copyright (C) 2018-2019  Robert Ian Hawdon

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

extern int * pc;

extern char fileMenuText[256];

int cmp_menu_ref(const void *lhs, const void *rhs)
{

  menuDef *dforderA = (menuDef *)lhs;
  menuDef *dforderB = (menuDef *)rhs;

  return strcoll(dforderA->refLabel, dforderB->refLabel);

}

void addMenuItem(menuDef **dfMenu, int *pos, char* refLabel, wchar_t* displayLabel, int hotKey){

  int menuPos = *pos;
  int charCount = 0;
  int i;
  menuDef *tmp;

  if (menuPos == 0){
    tmp = malloc(sizeof(menuDef) * 2);
  } else {
    tmp = realloc(*dfMenu, (menuPos + 1) * sizeof(menuDef) + 1 );
  }
  if (tmp){
    *dfMenu = tmp;
  }

  sprintf((*dfMenu)[menuPos].refLabel, "%s", refLabel);
  swprintf((*dfMenu)[menuPos].displayLabel, 32, L"%ls", displayLabel);
  (*dfMenu)[menuPos].hotKey = hotKey;

  for (i = 0; i < wcslen(displayLabel); i++)
    {
      if ( displayLabel[i] == '!' || displayLabel[i] == '<' || displayLabel[i] == '>' || displayLabel[i] == '\\') {
        i++;
        charCount++;
      } else {
        charCount++;
      }
    }
  (*dfMenu)[menuPos].displayLabelSize = charCount;

  qsort((*dfMenu), menuPos + 1, sizeof(menuDef), cmp_menu_ref);

  ++*pos;

}

wchar_t * genMenuDisplayLabel(menuDef* dfMenu, int size, int comma){
  wchar_t * output;
  int gapSize;
  int currentLen = 0;
  int i;

  output = malloc(sizeof(wchar_t) * 1);
   for (i = 0; i < size ; i++){
    output = realloc(output, ((i + 1) * sizeof(dfMenu[i].displayLabel) + 1) * sizeof(wchar_t) );
    if ( i == 0 ){
      currentLen = currentLen + dfMenu[i].displayLabelSize;
      if ( currentLen - 1 < COLS){
        wcscpy(output, dfMenu[i].displayLabel);
      }
    } else {
      if (comma){
        gapSize = 2;
      } else {
        gapSize = 1;
      }
      currentLen = currentLen + dfMenu[i].displayLabelSize + gapSize;
      if (currentLen - 1 < COLS){
        if (comma){
          wcscat(output, L",");
        }
        wcscat(output, L" ");
        wcscat(output, dfMenu[i].displayLabel);
      }
    }
   }
  return output;
}

int menuHotkeyLookup(menuDef* dfMenu, char* refLabel, int size){
  int i;
  int r = -1;
  for (i = 0; i < size; i++){
    if (!strcmp(dfMenu[i].refLabel, refLabel)){
      r = dfMenu[i].hotKey;
    }
  }
  return r;
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

  outStr = malloc(sizeof (char) * i + 1);

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
Copyright (C) 2019 Robert Ian Hawdon\n\
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n\
This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you\n\
are welcome to redistribute it under certain conditions.\n"), stdout);
}

void wPrintMenu(int line, int col, wchar_t *menustring)
{
  int i, len, charcount;
  charcount = 0;
  move(line, col);
  clrtoeol();
  len = wcslen(menustring);
  setColors(COMMAND_PAIR);
  for (i = 0; i < len; i++)
    {
      if ( menustring[i] == '!' ) {
        i++;
        setColors(HILITE_PAIR);
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        setColors(COMMAND_PAIR);
        charcount++;
      } else if ( menustring[i] == '<' ) {
        i++;
        setColors(HILITE_PAIR);
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        charcount++;
      } else if ( menustring[i] == '>' ) {
        i++;
        setColors(COMMAND_PAIR);
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        charcount++;
      } else if ( menustring[i] == '\\' ) {
        i++;
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        charcount++;
      } else {
        mvprintw(line, col + charcount, "%lc", menustring[i]);
        charcount++;
      }
    }
}

void printMenu(int line, int col, char *menustring)
{
  // Small wrapper to seemlessly forward calls to the wide char version
  wchar_t *wMenuString;
  wMenuString = malloc(sizeof(wchar_t) * strlen(menustring) + 1);
  swprintf(wMenuString, strlen(menustring) + 1, L"%s", menustring);
  wPrintMenu(line, col, wMenuString);
  free(wMenuString);
}

void wPrintLine(int line, int col, wchar_t *textString){
  int i;
  move(line,col);
  clrtoeol();
  for ( i = 0; i < wcslen(textString) ; i++){
    mvprintw(line, col + i, "%lc", textString[i]);
    if ( (col + i) == COLS ){
      break;
    }
  }
}

void printLine(int line, int col, char *textString){
  // Small wrapper to seemlessly forward calls to the wide char version
  wchar_t *wTextString;
  wTextString = malloc( sizeof ( wchar_t ) * strlen(textString) + 1);
  swprintf(wTextString, strlen(textString) + 1, L"%s", textString);
  wPrintLine(line, col, wTextString);
  free(wTextString);
}


void topLineMessage(const char *message){
  move(0,0);
  clrtoeol();
  setColors(ERROR_PAIR);
  mvprintw(0,0, "%s", message);
  setColors(COMMAND_PAIR);
  while(1)
    {
      *pc = getch();
      switch(*pc)
        {
        default: // Where's the "any" key?
          return;
          break;
        }
    }
}

int wReadLine(wchar_t *buffer, int buflen, wchar_t *oldbuf)
/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
{
  int old_curs = curs_set(1);
  int pos;
  int len;
  int oldlen;
  int x, y;
  int status = 0;
  wint_t c;

  oldlen = wcslen(oldbuf);
  setColors(INPUT_PAIR);
  // attron(COLOR_PAIR(INPUT_PAIR));

  pos = oldlen;
  len = oldlen;

  getyx(stdscr, y, x);

  wcscpy(buffer, oldbuf);

  for (;;) {

    buffer[len] = ' ';
    mvaddnwstr(y, x, buffer, len+1); // Prints buffer on screen
    move(y, x+pos); //
    //c = getch();
    get_wch(&c);

    if (c == KEY_ENTER || c == '\n' || c == '\r') {
      // Enter Key
      setColors(COMMAND_PAIR);
      break;
    } else if (c == KEY_LEFT) {
      // Left Key
      if (pos > 0) pos -= 1; else beep();
    } else if (c == KEY_RIGHT) {
      // Right Key
      if (pos < len) pos += 1; else beep();
    } else if ((c == KEY_BACKSPACE) || (c == 127)) {
      // Backspace Key
      if (pos > 0) {
        wmemmove(buffer+pos-1, buffer+pos, len-pos);
        pos -= 1;
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == KEY_DC) {
      // Delete Key
      if (pos < len) {
        wmemmove(buffer+pos, buffer+pos+1, len-pos-1);
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == 270) {
      // F6 Key
      // F6 deletes to the end of line. Recently discovered in DF-EDIT 2.3d hidden in program documentation (2018-08-18)
      if ( pos < len ) {
        wmemmove(buffer+pos, buffer+pos+(len-pos), 0);
        len = pos;
        clrtoeol();
      }
    } else if (c == 27) {
      // ESC Key
      status = -1;
      pos = 0;
      len = 0;
      wcscpy(buffer, L""); //abort by blanking
      setColors(COMMAND_PAIR);
      break;
    } else if ((c == KEY_UP) || (c == KEY_DOWN) || (c == KEY_IC) || (c == 262) || (c == 265) || (c == 266) || (c == 267) || (c == 268) || (c == 269) || (c == 270) || (c == 271) || (c == 272) || (c == 273) || (c == 274) || (c == 275) || (c == 276) || (c == 338) || (c == 339) || (c == 360)) {
      // Ignore navigation and function keys.
      continue;
    } else if (iswprint(c)) {
      // Anything else that can be printed.
      if (pos < buflen-1) {
        wmemmove(buffer+pos+1, buffer+pos, len-pos);
        buffer[pos++] = c;
        len += 1;
      } else {
        beep();
      }
    } else {
      beep();
    }
  }
  buffer[len] = '\0';
  if (old_curs != ERR) curs_set(old_curs);
  return(status);
}

int readline(char *buffer, int buflen, char *oldbuf)
{

  // Small wrapper to seemlessly forward calls to the wide char version
  wchar_t *wBuffer, *wOldBuf;
  int status;
  wBuffer = malloc(sizeof(wchar_t) * buflen);
  wOldBuf = malloc(sizeof(wchar_t) * strlen(oldbuf) + 1);
  swprintf(wOldBuf, strlen(oldbuf) + 1, L"%s", oldbuf);
  status = wReadLine(wBuffer, buflen, wOldBuf);
  sprintf(buffer, "%ls", wBuffer);
  free(wBuffer);
  free(wOldBuf);
  return(status);
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
  sprintf(mancmd, "man %s", prog);
  clear();
  endwin();
  // system("clear"); // Not exactly sure if I want this yet.
  system(mancmd);
  initscr();
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

void clear_workspace()
{
  size_t line_count = 1;
  for (line_count = 1; line_count < (LINES - 1);)
    {
      move (line_count,0);
      clrtoeol();
      line_count++;
    }
}
