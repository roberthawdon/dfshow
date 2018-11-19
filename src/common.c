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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include "colors.h"
#include "config.h"

DIR *folder;
FILE *file;

extern int * pc;

extern char fileMenuText[256];

int exittoshell()
{
  clear();
  endwin();
  exit(0);
  return 0;
}

void printVersion(char* programName){
  printf (("%s %s\n"), programName, VERSION);
  fputs (("\
Copyright (C) 2018 Robert Ian Hawdon\n\
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
          printMenu(0, 0, fileMenuText);
          //directory_view_menu_inputs();
          return;
          break;
        }
    }
}

int readline(char *buffer, int buflen, char *oldbuf)
/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
{
  int old_curs = curs_set(1);
  int pos;
  int len;
  int oldlen;
  int x, y, c;
  int status = 0;

  oldlen = strlen(oldbuf);
  setColors(INPUT_PAIR);
  // attron(COLOR_PAIR(INPUT_PAIR));

  pos = oldlen;
  len = oldlen;

  getyx(stdscr, y, x);

  strcpy(buffer, oldbuf);

  for (;;) {

    buffer[len] = ' ';
    mvaddnstr(y, x, buffer, len+1); // Prints buffer on screen
    move(y, x+pos); //
    c = getch();

    if (c == KEY_ENTER || c == '\n' || c == '\r') {
      // attron(COLOR_PAIR(COMMAND_PAIR));
      setColors(COMMAND_PAIR);
      break;
    } else if (isprint(c)) {
      if (pos < buflen-1) {
        memmove(buffer+pos+1, buffer+pos, len-pos);
        buffer[pos++] = c;
        len += 1;
      } else {
        beep();
      }
    } else if (c == KEY_LEFT) {
      if (pos > 0) pos -= 1; else beep();
    } else if (c == KEY_RIGHT) {
      if (pos < len) pos += 1; else beep();
    } else if ((c == KEY_BACKSPACE) || (c == 127)) {
      if (pos > 0) {
        memmove(buffer+pos-1, buffer+pos, len-pos);
        pos -= 1;
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == KEY_DC) {
      if (pos < len) {
        memmove(buffer+pos, buffer+pos+1, len-pos-1);
        len -= 1;
        clrtoeol();
      } else {
        beep();
      }
    } else if (c == 270) {
      // F6 deletes to the end of line. Recently discovered in DF-EDIT 2.3d hidden in program documentation (2018-08-18)
      if ( pos < len ) {
        memmove(buffer+pos, buffer+pos+(len-pos), 0);
        len = pos;
        clrtoeol();
      }
    } else if (c == 27) {
      //pos = oldlen;
      //len = oldlen;
      //strcpy(buffer, oldbuf); //abort
      status = -1;
      pos = 0;
      len = 0;
      strcpy(buffer, ""); //abort by blanking
      // attron(COLOR_PAIR(COMMAND_PAIR));
      setColors(COMMAND_PAIR);
      break;
    } else {
      beep();
    }
  }
  buffer[len] = '\0';
  if (old_curs != ERR) curs_set(old_curs);
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
    for (count = 0; tmp = strstr(ins, rep); ++count) {
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
