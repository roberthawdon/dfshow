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
#include <ncurses.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/statvfs.h>
#include <libgen.h>
#include <errno.h>
#include <wchar.h>
#include <math.h>
#include <regex.h>
#include "common.h"
#include "config.h"
#include "showfunctions.h"
#include "showmenus.h"
#include "colors.h"
#include "show.h"

#if HAVE_SYS_SYSMACROS_H
# include <sys/sysmacros.h>
#endif

#if HAVE_HURD_H
# include <hurd.h>
#endif

// It turns out most systems don't have an ST_AUTHOR, so for those systems, we set the author as the owner. Yup, `ls` does this too.
#if ! HAVE_STRUCT_STAT_ST_AUTHOR
# define st_author st_uid
#endif

char hlinkstr[5], sizestr[32], majorstr[5], minorstr[5];
char headAttrs[12], headOG[25], headSize[14], headDT[18], headName[13];

int hlinklen;
int ownerlen;
int grouplen;
int authorlen;
int sizelen;
int majorlen;
int minorlen;
int datelen;
int namelen;
int slinklen;

int entryMetaLen, entryNameLen, entrySLinkLen = 0;

int ogalen;

int hlinkstart;
int ownstart;
int groupstart;
int sizestart;
int sizeobjectstart;
int datestart;
int namestart;

int sexec;
int typecolor;

int totalfilecount;

int selected;
int topfileref = 0;
int hpos = 0;
int maxdisplaywidth;
int displaysize; // Calculate area to print
int displaycount;
int historyref = 0;
int sessionhistory = 0;
int displaystart;

int showhidden = 0;

int markall = 0;

int mmMode = 0;

unsigned long int savailable = 0;
unsigned long int sused = 0;

history *hs;

time_t currenttime;

DIR *folder;

extern int messageBreak;
extern char currentpwd[1024];
extern char timestyle[9];
extern int viewMode;
extern int reverse;
extern int human;
extern int si;
extern int ogavis;
extern int ogapad;
extern int showbackup;
extern int danger;
extern int filecolors;
extern char *objectWild;
extern int markedinfo;
extern int markedauto;

/* Formatting time in a similar fashion to `ls` */
static char const *long_time_format[2] =
  {
   // With year, intended for if the file is older than 6 months.
   "%b %e  %Y",
   // Without year, for recent files.
   "%b %e %H:%M"
  };

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

int wildcard(const char *value, char *wcard) {

    int vsize = (int)strlen(value);
    int wsize = (int)strlen(wcard);
    int match = 0;

    if (vsize == 0 &&  wsize == 0) {
        match = 1;
    }

    else {
        int v = 0;
        int w = 0;
        int lookAhead = 0;
        int searchMode = 0;
        char search = '\0';


        while (1) {
            if (wcard[w] == MULTICHAR) {
                //starts with * and the value matches the wcard
                if (w == 0 && strcmp(wcard+1,value) == 0) {
                    match = 1;
                    break;
                }
                //the * is the last character in the pattern
                if (!wcard[w+1]) {
                    match = 1;
                    break;
                }
                else {
                    //search for the next char in the pattern that is not a ?
                    while (wcard[++w] == ONECHAR) {
                        lookAhead++;
                    }

                    //if the next char in the pattern is another * we go to the start (in case we have a pattern like **a, stupid I know, but it might happen)
                    if (wcard[w] == MULTICHAR) {
                        continue;
                    }

                    search = wcard[w];
                    searchMode = 1;
                }
            }

            else {
                if (!value[v] && !wcard[w]) {
                    if (searchMode) {
                        match = 0;
                    }
                    break;
                }
                if (searchMode) {
                    char currentValue = value[v+lookAhead];
                    if (currentValue == search) {
                        match = 1;

                        searchMode = 0;
                        search = '\0';
                        lookAhead = 0;
                        w++;
                    }

                    else if (currentValue == '\0') {
                        match = 0;
                        break;
                    }

                    v++;
                    continue;
                }
                else if ((wcard[w] == ONECHAR && value[v] == '\0') || (wcard[w] != value[v] && wcard[w] != ONECHAR)) {
                    match = 0;
                    break;
                }
                else {
                    match = 1;
                }

                w++;
                v++;
            }
        }
    }

    return match;
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

const char * writePermsEntry(char * perms, mode_t mode){

  typecolor = DISPLAY_PAIR;

  sexec = 0;

  if (S_ISDIR(mode)) {
    perms[0] = 'd';
    typecolor = DIR_PAIR;
  } else if (S_ISCHR(mode)){
    mmMode = 1;
    perms[0] = 'c';
  } else if (S_ISLNK(mode)){
    perms[0] = 'l';
  } else if (S_ISFIFO(mode)){
    perms[0] = 'p';
  } else if (S_ISBLK(mode)){
    mmMode = 1;
    perms[0] = 'b';
  } else if (S_ISSOCK(mode)){
    perms[0] = 's';
  } else if (S_ISREG(mode)){
    perms[0] = '-';
  } else {
    perms[0] = '?';
  }

  perms[1] = mode & S_IRUSR? 'r': '-';
  perms[2] = mode & S_IWUSR? 'w': '-';

  perms[4] = mode & S_IRGRP? 'r': '-';
  perms[5] = mode & S_IWGRP? 'w': '-';

  perms[7] = mode & S_IROTH? 'r': '-';
  perms[8] = mode & S_IWOTH? 'w': '-';

  if ( (mode & S_ISUID) && (mode & S_IXUSR) ){
    perms[3] = 's';
    if (typecolor != DIR_PAIR){
      sexec = 1;
      typecolor = SUID_PAIR;
    }
  } else if ( (mode & S_ISUID) ){
    perms[3] = 'S';
    if (typecolor != DIR_PAIR){
      sexec = 1;
      typecolor = SUID_PAIR;
    }
  } else if ( (mode & S_IXUSR) ){
    perms[3] = 'x';
    if (typecolor != DIR_PAIR){
      typecolor = EXE_PAIR;
    }
  } else {
    perms[3] = '-';
  }

  if ( (mode & S_ISGID) && (mode & S_IXGRP) ){
    perms[6] = 's';
    if (typecolor != DIR_PAIR && !sexec){
      sexec = 1;
      typecolor = SGID_PAIR;
    }
  } else if ( (mode & S_ISGID) ){
    perms[6] = 'S';
    if (typecolor != DIR_PAIR && !sexec){
      sexec = 1;
      typecolor = SGID_PAIR;
    }
  } else if ( (mode & S_IXGRP) ){
    perms[6] = 'x';
    if (typecolor != DIR_PAIR && !sexec){
      typecolor = EXE_PAIR;
    }
  } else {
    perms[6] = '-';
  }

  if ( (mode & S_IXOTH) && (mode & S_ISVTX) ){
    perms[9] = 't';
  } else if (mode & S_ISVTX) {
    perms[9] = 'T';
  } else if (mode & S_IXOTH){
    perms[9] = 'x';
    if (typecolor != DIR_PAIR && !sexec){
      typecolor = EXE_PAIR;
    }
  } else {
    perms[9] = '-';
  }

  return perms;

}

void writeResultStruct(results* ob, const char * filename, struct stat buffer, int count){
  char perms[11] = {0};
  struct group *gr;
  struct passwd *pw;
  struct passwd *au;
  char *filedate;
  ssize_t cslinklen;

  writePermsEntry(perms, buffer.st_mode);

  // Writing our structure
  if ( markall && !(buffer.st_mode & S_IFDIR) ) {
    *ob[count].marked = 1;
  } else {
    *ob[count].marked = 0;
  }
  strcpy(ob[count].perm, perms);
  *ob[count].hlink = buffer.st_nlink;
  *ob[count].hlinklens = strlen(hlinkstr);

  if (!getpwuid(buffer.st_uid)){
    sprintf(ob[count].owner, "%i", buffer.st_uid);
  } else {
    pw = getpwuid(buffer.st_uid);
    strcpy(ob[count].owner, pw->pw_name);
  }

  if (!getgrgid(buffer.st_gid)){
    sprintf(ob[count].group, "%i", buffer.st_gid);
  } else {
    gr = getgrgid(buffer.st_gid);
    strcpy(ob[count].group, gr->gr_name);
  }

  if (!getpwuid(buffer.st_author)){
    sprintf(ob[count].author, "%i", buffer.st_author);
  } else {
    au = getpwuid(buffer.st_author);
    strcpy(ob[count].author, au->pw_name);
  }

  ob[count].size = buffer.st_size;
  *ob[count].sizelens = strlen(sizestr);

  if (S_ISCHR(buffer.st_mode) || S_ISBLK(buffer.st_mode)){
    ob[count].major = major(buffer.st_rdev);
    ob[count].minor = minor(buffer.st_rdev);
  } else {
    ob[count].major = ob[count].minor = 0; // Setting a default
  }

  ob[count].date = buffer.st_mtime;

  filedate = dateString(ob[count].date, timestyle);
  strcpy(ob[count].datedisplay, filedate);

  strcpy(ob[count].name, filename);

  if (S_ISLNK(buffer.st_mode)) {
    cslinklen = readlink(filename, ob[count].slink, 1023);
    ob[count].slink[cslinklen] = '\0';

  } else {
    strcpy(ob[count].slink, "");
  }

  ob[count].color = typecolor;

  free(filedate);

}

int findResultByName(results *ob, char *name)
{
  int i;
  for(i = 0; i < totalfilecount; ){
    if ( !strcmp(ob[i].name, name) ) {
      return i;
    }
    i++;
  }
  //If there's no match, we'll fall back to the top item in the list
  return 0;
}

char *dateString(time_t date, char *style)
{
  char *outputString = malloc (sizeof (char) * 33);
  bool recent = 0;

  if ( date > (currenttime - 31556952 / 2) ) {
    recent = 1;
  }

  if ( !strcmp(style, "long-iso") ) {
    long_time_format[0] = long_time_format[1] = "%Y-%m-%d %H:%M";
  } else if ( !strcmp(style, "full-iso") ) {
    long_time_format[0] = long_time_format[1] = "%Y-%m-%d %H:%M:%S %z";
  } else if ( !strcmp(style, "iso") ) {
    long_time_format[0] = "%Y-%m-%d ";
    long_time_format[1] = "%m-%d %H:%M";
  }
  strftime(outputString, 32, long_time_format[recent], localtime(&(date)));
  return (outputString);
}

char *readableSize(double size, char *buf, int si){
  int i = 0;
  int powers = 1024;
  const char* units[] = {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
  char unitOut[2];
  if (si){
    powers = 1000;
  }
  while ( size >= powers){
    size /= powers;
    i++;
    if ( i == 10 ){
      break; // Come back and see me when 1024+ YB is an everyday occurance
    }
  }
  sprintf(unitOut, "%s", units[i]);
  if (si){
    // si units used
    unitOut[0] = tolower(*unitOut);
  }
  sprintf(buf, "%.*f%s", i, size, unitOut);
  return (buf);
}

void padstring(char *str, int len, char c)
{
  int slen = strlen(str);
  // loop up to len-1 to leave space for null terminator
  for(; slen < len-1;slen++)
    {
      str[slen] = c;
    }
  // add the null terminator
  str[slen] = 0;
}

char *genPadding(int num_of_spaces) {
  char *dest = malloc (sizeof (char) * num_of_spaces + 1);
  if (num_of_spaces > 0){
    sprintf(dest, "%*s", num_of_spaces, " ");
  } else {
    strcpy(dest, "");
  }
  return dest;
}

void printLine(int line, int col, char *textString){
  int i;
  clrtoeol();
  for ( i = 0; i < strlen(textString) ; i++){
    mvprintw(line, col + i, "%c", textString[i]);
    if ( (col + i) == COLS ){
      break;
    }
  }
}

void printEntry(int start, int hlinklen, int ownerlen, int grouplen, int authorlen, int sizelen, int majorlen, int minorlen, int datelen, int namelen, int selected, int listref, int topref, results* ob){

  int i;

  char marked[2];
  wchar_t entryMeta[1024];
  wchar_t entryName[1024];
  wchar_t entrySLink[1024];
  int maxlen = COLS - start;

  int currentitem = listref + topref;
  int ogminlen = strlen(headOG); // Length of "Owner & Group" heading
  int sizeminlen = strlen(headSize); // Length of "Size" heading
  int dateminlen = strlen(headDT); // Length of "Date" heading

  int oggap, gagap = 0;

  int oglen = 0;

  char *ogaval;

  int ogseglen = ogalen + ogapad;

  int ogpad = 0;
  int sizepad = 0;
  int mmpad = 0;

  int datepad = 0;

  char *s1, *s2, *s3, *s4;

  char *sizestring;

  int linepadding;
  int colpos;

  // Owner, Group, Author
  switch(ogavis){
  case 0:
    ogaval = malloc (sizeof (char));
    strcpy(ogaval,"");
    break;
  case 1:
    oglen = (strlen(ob[currentitem].owner));
    ogaval = malloc (sizeof (char) * oglen + 2);
    sprintf(ogaval, "%s", ob[currentitem].owner);
    break;
  case 2:
    oglen = (strlen(ob[currentitem].group));
    ogaval = malloc (sizeof (char) * oglen + 1);
    sprintf(ogaval, "%s", ob[currentitem].group);
    break;
  case 3:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].group));
    ogaval = malloc (sizeof (char) * oglen + 1);
    sprintf(ogaval, "%s%s%s", ob[currentitem].owner, genPadding(oggap), ob[currentitem].group);
    break;
  case 4:
    oglen = (strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * oglen + 1);
    sprintf(ogaval, "%s", ob[currentitem].author);
    break;
  case 5:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * oglen + 1);
    sprintf(ogaval, "%s%s%s", ob[currentitem].owner, genPadding(oggap), ob[currentitem].author);
    break;
  case 6:
    gagap = grouplen - strlen(ob[currentitem].group) + 1;
    oglen = (strlen(ob[currentitem].group) + gagap + strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * oglen + 1);
    sprintf(ogaval, "%s%s%s", ob[currentitem].group, genPadding(gagap), ob[currentitem].author);
    break;
  case 7:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    gagap = grouplen - strlen(ob[currentitem].group) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].group) + gagap + strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * oglen + 1);
    sprintf(ogaval, "%s%s%s%s%s", ob[currentitem].owner, genPadding(oggap), ob[currentitem].group, genPadding(gagap), ob[currentitem].author);
    break;
  default:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].group));
    ogaval = malloc (sizeof (char) * oglen + 1);
    sprintf(ogaval, "%s%s%s", ob[currentitem].owner, genPadding(oggap), ob[currentitem].group);
    break;
  }

  if (!ogavis){
    ogpad = 0;
  } else {
    if ( (ogminlen - ogseglen) > 0 ) {
      ogpad = ogminlen - strlen(ogaval);
    } else {
      ogpad = ogseglen - strlen(ogaval);
    }
  }

  if (ob[currentitem].minor > 1){
    mmpad = sizelen - (log10(ob[currentitem].minor + 1)) + 2;
  } else {
    mmpad = sizelen + 1;
  }

  s4 = genPadding(mmpad);

  if ((ob[currentitem].major > 0) || (ob[currentitem].minor > 0)){
    // If either of these are not 0, then we're dealing with a Character or Block device.
    sizestring = malloc (sizeof (char) * sizelen + 5);
    sprintf(sizestring, "%i,%s%i", ob[currentitem].major, s4, ob[currentitem].minor);
  } else {
    if (human){
      sizestring = malloc (sizeof (char) * 10);
      readableSize(ob[currentitem].size, sizestring, si);
    } else {
      sizestring = malloc (sizeof (char) * sizelen + 1);
      sprintf(sizestring, "%lu", ob[currentitem].size);
    }
  }

  // Redefining width of Size value if the all sizes are smaller than the header.
  if ( sizelen < sizeminlen ) {
    sizelen = sizeminlen;
  }

  sizepad = (sizelen - strlen(sizestring)) + ogpad + 1;

  if ( (dateminlen - datelen) > 0 ) {
    datepad = dateminlen - strlen(ob[currentitem].datedisplay);
  } else {
    datepad = datelen - strlen(ob[currentitem].datedisplay);
  }

  s1 = genPadding(hlinkstart);
  s2 = genPadding(sizepad);
  s3 = genPadding(datepad);

  if ( *ob[currentitem].marked ){
    strcpy(marked, "*");
  } else {
    strcpy(marked, " ");
  }

  swprintf(entryMeta, 1024, L"  %s %s%s%i %s%s%s %s%s ", marked, ob[currentitem].perm, s1, *ob[currentitem].hlink, ogaval, s2, sizestring, ob[currentitem].datedisplay, s3);

  swprintf(entryName, 1024, L"%s", ob[currentitem].name);

  if ( !strcmp(ob[currentitem].slink, "") ){
    swprintf(entrySLink, 1024, L"");
  } else {
    swprintf(entrySLink, 1024, L"%s", ob[currentitem].slink);
  }


  entryMetaLen = wcslen(entryMeta);
  entryNameLen = wcslen(entryName);
  entrySLinkLen = wcslen(entrySLink);
  // mvprintw(4 + listref, start, "%s", entry);

  // Setting highlight
  if (selected) {
    setColors(SELECT_PAIR);
  } else {
    setColors(DISPLAY_PAIR);
  }

  for ( i = 0; i < maxlen; i++ ){
    mvprintw(displaystart + listref, start + i,"%lc", entryMeta[i]);
    if ( i == entryMetaLen ){
      break;
    }
  }

  if (filecolors && !selected){
    if ( strcmp(ob[currentitem].slink, "" )) {
      setColors(SLINK_PAIR);
    } else {
      setColors(ob[currentitem].color);
    }
  }

  for ( i = 0; i < maxlen; i++ ){
    mvprintw(displaystart + listref, (entryMetaLen + start) + i,"%lc", entryName[i]);
    if ( i == entryNameLen ){
      colpos = (entryMetaLen + start) + i;
      break;
    }
  }

  if ( strcmp(ob[currentitem].slink, "") ){
    if (!selected){
      setColors(DISPLAY_PAIR);
    }
    mvprintw(displaystart + listref, (entryMetaLen + entryNameLen + start)," -> ");

    if (filecolors && !selected){
      if ( strcmp(ob[currentitem].slink, "" )) {
        if ( check_dir(ob[currentitem].slink) ){
          setColors(DIR_PAIR);
        } else {
          setColors(ob[currentitem].color);
        }
      }
    }

    for ( i = 0; i < maxlen; i++ ){
      mvprintw(displaystart + listref, (entryMetaLen + entryNameLen + 4 + start) + i,"%lc", entrySLink[i]);
      if ( i == entrySLinkLen ){
        colpos = (entryMetaLen + entryNameLen + 4 + start) + i;
        break;
      }
    }
  }

  if (filecolors && !selected){
    setColors(DISPLAY_PAIR);
  }

  linepadding = COLS - colpos;

  if (linepadding > 0){
    mvprintw(displaystart + listref, colpos, "%s", genPadding(linepadding));
  }


  free(s1);
  free(s2);
  free(s3);
  free(s4);
  free(sizestring);
  free(ogaval);
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

void LaunchShell()
{
  clear();
  endwin();
  // system("clear"); // Not exactly sure if I want this yet.
  printf("\nUse 'exit' to return to Show.\n\n");
  system(getenv("SHELL"));
  initscr();
  refreshScreen();
}

void LaunchExecutable(const char* object, const char* args)
{
  char command[1024];
  sprintf(command, "%s %s", object, args);
  clear();
  endwin();
  // printf("%s\n", command);
  // exit(0);
  system(command);
  initscr();
  refreshScreen();
}

void showManPage()
{
  clear();
  endwin();
  // system("clear"); // Not exactly sure if I want this yet.
  system("man show");
  initscr();
  refreshScreen();
}

void mk_dir(char *path)
{
  struct stat st = {0};

  if (stat(path, &st) == -1) {
    mkdir(path, 0755);
  }
}

void copy_file(char *source_input, char *target_input)
{
  char targetmod[1024];
  FILE *source = NULL;
  FILE *target = NULL;
  char ch = '\0';
  size_t n, m;
  unsigned char buff[8192];

  strcpy(targetmod, target_input);

  source = fopen(source_input, "rb");


  if ( check_dir(targetmod) ){
    if ( !check_last_char(targetmod, "/")){
      strcat(targetmod, "/");
    }
    strcat(targetmod, basename(source_input));
  }
  target = fopen(targetmod, "wb");

  do{
    n = fread(buff, 1, sizeof(buff), source);
    if (n){
      m = fwrite(buff, 1, n, target);
    } else {
      m = 0;
    }
  } while ((n > 0) && (n == m));

  // while(1){
  //   ch = fgetc(source);

  //   if(ch == EOF){
  //     break;
  //   }

  //   fputc(ch, target);
  // }

  fclose(source);
  fclose(target);
}

void delete_file(char *source_input)
{
  remove(source_input);
}

int SendToPager(char* object)
{
  char page[1024];
  char esc[1024];
  int pset = 0;
  int e = 0;
  char *escObject = str_replace(object, "'", "'\"'\"'");

  if ( getenv("PAGER")) {
    strcpy(page, getenv("PAGER"));
    pset = 1;
  }
  if ( pset ) {
    strcat(page, " ");
    strcpy(esc, "'");
    strcat(esc, escObject);
    free(escObject);
    strcat(esc, "'");
    strcat(page, esc);
    if (access(object, R_OK) == 0){
      clear();
      endwin();
      e = system(page);
      initscr();
      refreshScreen();
      return e;
    } else {
      topLineMessage("Error: Permission denied");
    }
  } else {
    topLineMessage("Please export a PAGER environment variable to define the utility program name.");
  }
  return 0;
}

int SendToEditor(char* object)

{
  char editor[1024];
  char esc[1024];
  int eset = 0;
  int e = 0;
  char *escObject = str_replace(object, "'", "'\"'\"'");
  if ( getenv("EDITOR")) {
    strcpy(editor, getenv("EDITOR"));
    eset = 1;
  } else if ( getenv("VISUAL")) {
    strcpy(editor, getenv("VISUAL"));
    eset = 1;
  }
  if ( eset ){
    strcat(editor, " ");
    strcpy(esc, "'");
    strcat(esc, escObject);
    free(escObject);
    strcat(esc, "'");
    strcat(editor, esc);
    if (access(object, R_OK) == 0){
      clear();
      endwin();
      e = system(editor);
      initscr();
      refreshScreen();
      return e;
    } else {
      topLineMessage("Error: Permission denied");
    }
  } else {
    topLineMessage("Please export a VISUAL environment variable to define the utility program name.");
  }
  return 0;
}

size_t GetAvailableSpace(const char* path)
{
  struct statvfs stat;

  if (statvfs(path, &stat) != 0) {
    // error happens, just quits here, but returns 0
    return 0;
  }

  // the available size is f_bsize * f_bavail
  //return stat.f_bsize * stat.f_bavail;
  // // endwin();
  // // clear();
  // // printf("f_bavail: %i\nf_frsize: %i\n", stat.f_bavail, stat.f_frsize);
  // // exit(0);
  return stat.f_bavail * stat.f_frsize;
}

long GetUsedSpace(const char* path)
{
  struct statvfs stat;

  if (statvfs(path, &stat) != 0) {
    // error happens, just quits here
    return -1;
  }

  // the available size is f_bsize * f_bavail
  return (stat.f_bsize * stat.f_blocks) - (stat.f_bsize * stat.f_bavail);
}

int seglength(const void *seg, char *segname, int LEN)
{

  size_t longest, len;

  results *dfseg = (results *)seg;

  size_t j = 0;

  size_t i;

  if (!strcmp(segname, "owner")) {
    longest = strlen(dfseg[0].owner);
  }
  else if (!strcmp(segname, "group")) {
    longest = strlen(dfseg[0].group);
  }
  else if (!strcmp(segname, "author")) {
    longest = strlen(dfseg[0].author);
  }
  else if (!strcmp(segname, "hlink")) {
    sprintf(hlinkstr, "%d", *dfseg[0].hlink);
    longest = strlen(hlinkstr);
  }
  else if (!strcmp(segname, "size")) {
    if (human){
      readableSize(dfseg[0].size, sizestr, si);
    } else {
      sprintf(sizestr, "%lu", dfseg[0].size);
    }
    longest = strlen(sizestr);
  }
  else if (!strcmp(segname, "major")) {
    sprintf(majorstr, "%d", dfseg[0].major);
    longest = strlen(majorstr);
  }
  else if (!strcmp(segname, "minor")) {
    sprintf(minorstr, "%d", dfseg[0].minor);
    longest = strlen(minorstr);
  }
  else if (!strcmp(segname, "datedisplay")) {
    longest = strlen(dfseg[0].datedisplay);
  }
  else if (!strcmp(segname, "name")) {
    longest = strlen(dfseg[0].name);
  }
  else if (!strcmp(segname, "slink")) {
    longest = strlen(dfseg[0].slink);
  }
  else {
    longest = 0;
  }

  for(i = 1; i < LEN; i++)
    {
      if (!strcmp(segname, "owner")) {
        len = strlen(dfseg[i].owner);
      }
      else if (!strcmp(segname, "group")) {
        len = strlen(dfseg[i].group);
      }
      else if (!strcmp(segname, "author")) {
        len = strlen(dfseg[i].author);
      }
      else if (!strcmp(segname, "hlink")) {
        sprintf(hlinkstr, "%d", *dfseg[i].hlink);
        len = strlen(hlinkstr);
      }
      else if (!strcmp(segname, "size")) {
        if (human){
          readableSize(dfseg[i].size, sizestr, si);
        } else {
          sprintf(sizestr, "%lu", dfseg[i].size);
        }
        len = strlen(sizestr);
      }
      else if (!strcmp(segname, "major")) {
        sprintf(majorstr, "%d", dfseg[i].major);
        len = strlen(majorstr);
      }
      else if (!strcmp(segname, "minor")) {
        sprintf(minorstr, "%d", dfseg[i].minor);
        len = strlen(minorstr);
      }
      else if (!strcmp(segname, "datedisplay")) {
        len = strlen(dfseg[i].datedisplay);
      }
      else if (!strcmp(segname, "name")) {
        len = strlen(dfseg[i].name);
      }
      else if (!strcmp(segname, "slink")) {
        len = strlen(dfseg[i].slink);
      }
      else {
        len = 0;
      }

      if(longest < len)
        {
          longest = len;
          j = i;
        }
    }
  return longest;
}

int cmp_str(const void *lhs, const void *rhs)
{
  return strcmp(lhs, rhs);
}

int cmp_int(const void *lhs, const void *rhs)
{
  int aa, bb;

  aa = *(int *)lhs;
  bb = *(int *)rhs;

  return (aa - bb);
}

int cmp_dflist_name(const void *lhs, const void *rhs)
{

  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  if (reverse){
    // Names in reverse order
    return strcoll(dforderB->name, dforderA->name);
  } else {
    // Names alphabetical
    return strcoll(dforderA->name, dforderB->name);
  }

}

int cmp_dflist_date(const void *lhs, const void *rhs)
{
  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  if (reverse){
    // Oldest to Newest
    return (dforderA->date - dforderB->date);
  } else {
    // Newest to Oldest
    return (dforderB->date - dforderA->date);
  }

}

int cmp_dflist_size(const void *lhs, const void *rhs)
{
  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  if (reverse) {
    // Smallest to largest
    return (dforderA->size - dforderB->size);
  } else {
    // Largest to smallest
    return (dforderB->size - dforderA->size);
  }

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

int check_object(const char *object){
  struct stat sb;
  if (stat(object, &sb) == 0 ){
    if (S_ISDIR(sb.st_mode)){
      return 1;
    } else if (S_ISREG(sb.st_mode) || S_ISBLK(sb.st_mode) || S_ISFIFO(sb.st_mode) || S_ISLNK(sb.st_mode) || S_ISCHR(sb.st_mode)){
      return 2;
    } else {
      return 0;
    }
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

int UpdateOwnerGroup(const char* object, const char* pwdstr, const char* grpstr)
{
  // char *buf;
  // size_t bufsize;
  struct stat sb;
  struct passwd *oldpwd;
  struct group *oldgrp;
  int s, uid, gid, e;

  uid = -1;
  gid = -1;

  lstat(object, &sb);

  oldpwd = getpwuid(sb.st_uid);
  oldgrp = getgrgid(sb.st_gid);

  if (pwdstr == NULL){
    uid = oldpwd->pw_uid;
  } else {
    uid = atoi(pwdstr);
  }
  if (grpstr == NULL){
    gid = oldgrp->gr_gid;
  } else {
    gid= atoi(grpstr);
  }

  //mvprintw(0,66, "%d:%d", uid, gid); //test
  e = chown(object, uid, gid);

  return e;

}

int RenameObject(char* source, char* dest)
{
  char sourceDevId[256];
  char destDevId[256];
  char *destPath;
  struct stat sourcebuffer;
  struct stat destbuffer;
  int e;

  destPath = dirFromPath(dest);

  // mvprintw(0,66,"Stripped: %s", dirFromPath(dest)); // test in the usual place

  if (check_dir(destPath)){
    lstat(source, &sourcebuffer);
    lstat(destPath, &destbuffer);

    sprintf(sourceDevId, "%d", sourcebuffer.st_dev);
    sprintf(destDevId, "%d", destbuffer.st_dev);

    if (!strcmp(sourceDevId, destDevId)) {
      // Destination is on the same filesystem.
      //mvprintw(0,66,"PASS: %s:%s %s", sourceDevId, destDevId, dest); // test pass
      e = rename(source, dest);
      free(destPath);
      return e;
    } else {
      // Destination is NOT in the same filesystem, the file will need copying then deleting.
      //mvprintw(0,66,"FAIL: %s:%s", sourceDevId, destDevId); // test fail
      topLineMessage("Error: Unable to move file between mount points");
      free(destPath);
      return 1;
    }
  } else {
    // Destination directory not found
    //mvprintw(0,66, "FAIL: NO DIR"); // test
    topLineMessage("Error: Invalid Destination");
    free(destPath);
    return 1;
  }
  free(destPath);
  return 0;
}

int CheckMarked(results* ob)
{
  int i;
  int result = 0;

  for (i = 0; i < totalfilecount; i++)
    {
      if ( *ob[i].marked ){
        result = 1;
        break;
      }
    }
  return(result);
}

void set_history(char *pwd, char *objectWild, char *name, int topfileref, int selected)
{
  if (sessionhistory == 0){
    history *hs = malloc(sizeof(history));
  }

  if (historyref == sessionhistory) {
    hs = realloc(hs, (historyref +1) * sizeof(history));
    sessionhistory++;
  }

  strcpy(hs[historyref].path, pwd);
  strcpy(hs[historyref].objectWild, objectWild);
  strcpy(hs[historyref].name, name);
  hs[historyref].topfileref = topfileref;
  hs[historyref].selected = selected;
  historyref++;

}

int huntFile(const char * file, const char * search, int charcase)
{
  FILE *fin;
  char *line;
  regex_t regex;
  int reti;
  char msgbuf[8192];

  reti = regcomp(&regex, search, charcase);

  if (reti) {
    return(-1);
  }

  fin = fopen(file, "r");

  if ( fin ) {
    while ( line = read_line(fin) ) {

      reti = regexec(&regex, line, 0, NULL, 0);
      if (!reti) {
        fclose(fin);
        free(line);
        regfree(&regex);
        return(1);
      }
    }
  }

  free(line);
  regfree(&regex);
  fclose(fin);
  return (0);
}

char *markedDisplay(results* ob)
{

  unsigned long int markedNum = 0;
  char markedNumString[12];
  char filesWord[6];
  int i;
  size_t markedSize = 0;
  char *markedSizeString;
  char *outChar = malloc(sizeof(char) * 8);

  for (i = 0; i < totalfilecount ; i++){
    if (*ob[i].marked == 1){
      markedNum++;
      markedSize = markedSize + ob[i].size;
    }
  }

  if (human){
    markedSizeString = malloc (sizeof (char) * 8);
    readableSize(markedSize, markedSizeString, si);
  } else {
    if (markedSize == 0){
      markedSizeString = malloc (sizeof (char) * 1);
    } else {
      markedSizeString = malloc (sizeof (char) * log10(markedSize) + 1);
    }
    sprintf(markedSizeString, "%lu", markedSize);
  }

  if (markedNum == 1){
    strcpy(filesWord, "file");
  } else {
    strcpy(filesWord, "files");
  }

  sprintf(markedNumString, "%lu", markedNum);

  outChar = realloc(outChar, sizeof(char) * ( strlen(markedNumString) + strlen(markedSizeString) + strlen(filesWord) + 16));

  sprintf(outChar, "MARKED: %s in %s %s", markedSizeString, markedNumString, filesWord);

  free(markedSizeString);

  return outChar;
}

results* get_dir(char *pwd)
{
  //sused = GetUsedSpace(pwd); // Original DF-EDIT added the sizes to show what was used in that directory, rather than the whole disk.
  size_t count = 0;
  size_t file_count = 0;
  struct dirent *res;
  struct stat sb;
  const char *path = pwd;
  struct stat buffer;
  int         status;
  char direrror[1024];
  // char filename[256];

  results *ob = malloc(sizeof(results)); // Allocating a tiny amount of memory. We'll expand this on each file found.

  fetch:

  mmMode = 0;
  time ( &currenttime );
  savailable = GetAvailableSpace(pwd);
  sused = 0; // Resetting used value

  //if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
  if (check_object(path) == 1){
    folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        while ( ( res = readdir ( folder ) ) ){
          if ( showhidden == 0 && check_first_char(res->d_name, ".") && strcmp(res->d_name, ".") && strcmp(res->d_name, "..") ) {
            continue; // Skipping hidden files
          }
          if ( !showbackup && check_last_char(res->d_name, "~") ) {
            continue; // Skipping backup files
          }
          if ( strcmp(objectWild, "")){
            if (!wildcard(res->d_name, objectWild) && strcmp(res->d_name, ".") && strcmp(res->d_name, "..")){
              continue;
            }
          }
          // filename = res->d_name;
          ob = realloc(ob, (count +1) * sizeof(results)); // Reallocating memory.
          lstat(res->d_name, &sb);
          status = lstat(res->d_name, &buffer);

          sprintf(hlinkstr, "%d", buffer.st_nlink);
          sprintf(sizestr, "%lld", buffer.st_size);

          writeResultStruct(ob, res->d_name, buffer, count);

          sused = sused + buffer.st_size; // Adding the size values

          count++;
        }

        totalfilecount = count;
        closedir ( folder );

        // free(objectWild);

        // return ob;
      }else{
        sprintf(direrror, "Could not open the directory" );
        topLineMessage(direrror);
        // return ob;
      }
    }

  } else if (check_object(path) == 2){

    objectWild = objectFromPath(path);
    strcpy(currentpwd, dirFromPath(path));

    goto fetch;

  } else {
    sprintf(direrror, "The location %s cannot be opened or is not a directory\n", path);
    topLineMessage(direrror);
    // return ob;
  }
  hlinklen = seglength(ob, "hlink", count);
  ownerlen = seglength(ob, "owner", count);
  grouplen = seglength(ob, "group", count);
  authorlen = seglength(ob, "author", count);
  sizelen = seglength(ob, "size", count);
  majorlen = seglength(ob, "major", count);
  minorlen = seglength(ob, "minor", count);
  datelen = seglength(ob, "datedisplay", count);
  namelen = seglength(ob, "name", count);
  slinklen = seglength(ob, "slink", count);

  return ob;
}

results* reorder_ob(results* ob, char *order){
  //mvprintw(2,66,"%i",*ob[0].sys);
  int count = totalfilecount;

  if ( !strcmp(order, "name")){
    qsort(ob, count, sizeof(results), cmp_dflist_name);
  }
  else if ( !strcmp(order, "date")){
    qsort(ob, count, sizeof(results), cmp_dflist_date);
  }
  else if ( !strcmp(order, "size")){
    qsort(ob, count, sizeof(results), cmp_dflist_size);
  }

  return ob;
}

void display_dir(char *pwd, results* ob, int topfileref, int selected){

  size_t list_count = 0;
  int count = totalfilecount;
  int printSelect = 0;
  char sizeHeader[256], headings[256];
  int i, s1, s2, s3;
  int headerpos, displaypos;
  char *susedString, *savailableString;
  char pwdprint[1024];
  char *markedInfoLine;

  if (markedauto) {
    if (CheckMarked(ob) ){
      markedinfo = 1;
    } else {
      markedinfo = 0;
    }
  }

  if (markedinfo){
    displaysize = LINES - 6;
    displaystart = 5;
  } else{
    displaysize = LINES - 5;
    displaystart = 4;
    if ((totalfilecount >= displaysize) && (topfileref + (displaysize ) > totalfilecount )){
      topfileref--;
    }
  }
  selected = selected - topfileref;

  if (strcmp(objectWild, "")){
    sprintf(pwdprint, "%s/%s", pwd, objectWild);
  } else {
    strcpy(pwdprint, pwd);
  }

  if (human) {
    susedString = malloc (sizeof (char) * 10);
    savailableString = malloc (sizeof (char) * 10);
    readableSize(sused, susedString, si);
    readableSize(savailable, savailableString, si);
  } else {
    if (sused == 0){
      susedString = malloc (sizeof (char) * 1);
    } else {
      susedString = malloc (sizeof (char) * log10(sused) + 1);
    }
    if (savailable == 0){
      savailableString = malloc (sizeof (char) * 1);
    } else {
      savailableString = malloc (sizeof (char) * log10(savailable) + 1);
    }
    sprintf(susedString, "%lu", sused);
    sprintf(savailableString, "%lu", savailable);
  }

  strcpy(headAttrs, "---Attrs---");

  if ( mmMode ){
    strcpy(headSize, "-Driver/Size-");
  } else {
    strcpy(headSize, "-Size-");
  }
  strcpy(headDT, "---Date & Time---");
  strcpy(headName, "----Name----");

  // Decide which owner header we need:
  switch(ogavis){
  case 0:
    strcpy(headOG, "");
    break;
  case 1:
    strcpy(headOG, "-Owner-");
    ogalen = ownerlen;
    break;
  case 2:
    strcpy(headOG, "-Group-");
    ogalen = grouplen;
    break;
  case 3:
    strcpy(headOG, "-Owner & Group-");
    ogalen = ownerlen + grouplen;
    break;
  case 4:
    strcpy(headOG, "-Author-");
    ogalen = authorlen; //test
    break;
  case 5:
    strcpy(headOG, "-Owner & Author-");
    ogalen = ownerlen + authorlen;
    break;
  case 6:
    strcpy(headOG, "-Group & Author-");
    ogalen = grouplen + authorlen;
    break;
  case 7:
    strcpy(headOG, "-Owner, Group, & Author-"); // I like the Oxford comma, deal with it.
    ogalen = ownerlen + grouplen + authorlen;
    break;
  default:
    strcpy(headOG, "-Owner & Group-"); // This should never be called, but we'd rather be safe.
    ogalen = ownerlen + grouplen;
    break;
  }

  if (displaysize > count){
    displaycount = count;
  } else {
    displaycount = displaysize;
  }

  for(list_count = 0; list_count < displaycount; ){
    // Setting highlight
    if (list_count == selected) {
      printSelect = 1;
    } else {
      printSelect = 0;
    }

    ownstart = hlinklen + 2;
    hlinkstart = ownstart - 1 - *ob[list_count + topfileref].hlinklens;

    displaypos = 0 - hpos;

    printEntry(displaypos, hlinklen, ownerlen, grouplen, authorlen, sizelen, majorlen, minorlen, datelen, namelen, printSelect, list_count, topfileref, ob);

    list_count++;
    }

  if (slinklen == 0){
    maxdisplaywidth = entryMetaLen + namelen;
  } else {
    maxdisplaywidth = entryMetaLen + namelen + slinklen + 4;
  }

  //mvprintw(0, 66, "%d %d", historyref, sessionhistory);

  // the space between the largest owner and largest group should always end up being 1... in theory.
  // 2018-07-05: That assumption was solid, until we added a third element (Owner, Group, and Author)
  if (!ogavis){
    s1 = 1;
  } else {
    if ( (ogalen + ogapad) > strlen(headOG)){
      s1 = (ogalen + ogapad) - strlen(headOG) + 1;
    } else {
      s1 = 1;
    }
  }

  if ( sizelen > strlen(headSize)) {
    s2 = sizelen - strlen(headSize);
  } else {
    s2 = 0;
  }

  if ( datelen > strlen(headDT)) {
    s3 = (datelen - strlen(headDT)) + 1;
  } else {
    s3 = 1;
  }

  sprintf(sizeHeader, "%i Objects   %s Used %s Available", count, susedString, savailableString);
  sprintf(headings, "%s%s%s%s%s%s%s%s%s%s", headAttrs, genPadding(hlinklen + 1), headOG, genPadding(s1), genPadding(s2), headSize, genPadding(1), headDT, genPadding(s3), headName);

  if ( danger ) {
    setColors(DANGER_PAIR);
  } else {
    setColors(INFO_PAIR);
  }
  // attroff(A_BOLD); // Required to ensure the last selected item doesn't bold the header
  printLine(1, 2, pwdprint);
  printLine(2, 2, sizeHeader);

  if (markedinfo){
    markedInfoLine = markedDisplay(ob);
    printLine (3, 4, markedInfoLine);
    free(markedInfoLine);
  }

  if ( danger ) {
    setColors(DANGER_PAIR);
  } else {
    setColors(HEADING_PAIR);
  }

  headerpos = 4 - hpos;

  if ( markedinfo ){
    printLine (4, headerpos, headings);
  } else {
    printLine (3, headerpos, headings);
  }
  setColors(COMMAND_PAIR);
  free(susedString);
  free(savailableString);
}

void resizeDisplayDir(results* ob){
  displaysize = (LINES - 5);
  if ( (selected - topfileref) > (LINES - 6 )) {
    topfileref = selected - (LINES - 6);
  } else if ( topfileref + (LINES - 6) > totalfilecount ) {
    if (totalfilecount < (LINES - 6)){
      topfileref = 0;
    } else {
      topfileref = totalfilecount - (LINES - 5);
    }
  }
  display_dir(currentpwd, ob, topfileref, selected);
}

