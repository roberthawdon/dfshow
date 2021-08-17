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
#include <utime.h>
#include <sys/statvfs.h>
#include <libgen.h>
#include <errno.h>
#include <wchar.h>
#include <math.h>
#include <regex.h>
#include <sys/acl.h>
#include <stdint.h>
#include "settings.h"
#include "common.h"
#include "display.h"
#include "config.h"
#include "showfunctions.h"
#include "showmenus.h"
#include "colors.h"
#include "show.h"

#if HAVE_ACL_LIBACL_H
# include <acl/libacl.h>
#endif

#if HAVE_SYS_SYSMACROS_H
# include <sys/sysmacros.h>
#endif

#if HAVE_HURD_H
# include <hurd.h>
#endif

#if HAVE_SELINUX_SELINUX_H
# include <selinux/selinux.h>
# else
#endif

#if HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

// It turns out most systems don't have an ST_AUTHOR, so for those systems, we set the author as the owner. Yup, `ls` does this too.
#if ! HAVE_STRUCT_STAT_ST_AUTHOR
# define st_author st_uid
#endif

char hlinkstr[6], sizestr[32], sizeblocksstr[32], majorstr[6], minorstr[6];
char headAttrs[12], headOG[25], headSize[14], headDT[18], headName[13], headContext[14], headSizeBlocks[9];

int hlinklen;
int ownerlen;
int grouplen;
int authorlen;
int sizelen;
int sizeblockslen;
int majorlen;
int minorlen;
int datelen;
int namelen;
int slinklen;
int contextlen;
int nameSegLen;
int xattrKeyLen;

int nameAndSLink = 0;

int entryMetaLen, entryNameLen, entrySLinkLen = 0;
int charPos = 0;

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
int lineStart = 0;
int hpos = 0;
int maxdisplaywidth;
int displaysize; // Calculate area to print
int displaycount;
int historyref = 0;
int sessionhistory = 0;
int displaystart;

int showhidden = 0;

int markall = 0;
int automark = 0;

int mmMode = 0;

int dirAbort = 0;

int axDisplay = 0;

int markedSegmentLen, sizeBlocksSegmentLen, attrSegmentLen, hlinkSegmentLen, ownerSegmentLen, contextSegmentLen, sizeSegmentLen, dateSegmentLen, nameSegmentDataLen, sizeBlocksSegmentLen, linkSegmentLen, tmpSegmentLen;

uintmax_t savailable = 0;
unsigned long int sused = 0;

history *hs;

time_t currenttime;

int skippable = 0;

xattrList *xa;
int xattrPos;

int lineCount;
int bottomFileRef;
int visibleObjects;

int visibleOffset;

int listLen;
entryLines *el;

extern DIR *folder;

extern int segOrder[9];

extern int block_size;

extern int messageBreak;
extern char currentpwd[4096];
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
extern int useEnvPager;
extern int showProcesses;
extern int exitCode;
extern int showContext;
extern int showSizeBlocks;
extern int oneLine;
extern int skipToFirstFile;
extern int showXAttrs;
extern int showAcls;
extern bool dirOnly;
extern bool scaleSize;
extern bool useDefinedEditor;
extern char * visualPath;

extern char sortmode[9];

#ifdef HAVE_MOVE_BETWEEN_DEVICES
bool moveBetweenDevices = true;
#else
bool moveBetweenDevices = false;
#endif

/* Formatting time in a similar fashion to `ls` */
static char const *long_time_format[2] =
  {
   // With year, intended for if the file is older than 6 months.
   "%b %e  %Y",
   // Without year, for recent files.
   "%b %e %H:%M"
  };

void freeResults(results *ob, int count)
{
  int i;
  for (i = 0; i < (count - 1); i++){
    free(ob[i].perm);
    free(ob[i].name);
    free(ob[i].owner);
    free(ob[i].group);
    free(ob[i].author);
    free(ob[i].slink);
    free(ob[i].datedisplay);
    free(ob[i].contextText);
    free(ob[i].xattrs);
    // acl_free(ob[i].acl);
  }
  free(ob);
  free(el);
}

void freeHistory(history *hs, int count)
{
  int i;
  for (i = 0; i < (count - 1); i++){
    free(hs[i].path);
    free(hs[i].name);
    free(hs[i].objectWild);
  }
  free(hs);
}

void freeXAttrs(xattrList *xa, int count)
{
  int i;
  for (i = 0; i < (count - 1); i++){
    free(xa[i].name);
    free(xa[i].xattr);
  }
  free(xa);
}

int checkRunningEnv(){
  int i;
  if (!getenv("DFS_RUNNING")){
    i = 0;
  } else {
    i = atoi(getenv("DFS_RUNNING"));
  }
  return i;
}

int processXAttrs(xattrList **xa, char *name, unsigned char *xattrs, size_t xattrLen, int pos, int *xattrsNum)
{
  char *xattrTmp;
  int i, n;
  bool reset = false; //To-Do, swap all 0/1 ints to bools
  xattrList *tmp;

  xattrTmp = malloc(sizeof(char));

  for (i = 0; i < xattrLen + 1; i++){
    if (i == 0 || reset == true){
      n = 0;
      free(xattrTmp);
      xattrTmp = calloc(xattrLen, sizeof(char));
      if (pos == 0){
        tmp = malloc(sizeof(xattrList) * 2);
      } else {
        tmp = realloc(*xa, (pos + 1) * (sizeof(xattrList) + 1) );
      }
      if (tmp){
        *xa = tmp;
      }
      reset = false;
    }
    if (xattrs[i] != '\0'){
      xattrTmp[n] = xattrs[i];
      n++;
    }
    if (xattrs[i] == '\0' && xattrTmp[0] != '\0'){
      (*xa)[pos].name = calloc(strlen(name) + 1, sizeof(char));
      strcpy((*xa)[pos].name, name);
      (*xa)[pos].xattr = calloc(strlen(xattrTmp) + 1, sizeof(char));
      strcpy((*xa)[pos].xattr, xattrTmp);
      // (*xa)[pos].xattrSize = strlen((*xa)[pos].xattr);
    #ifdef HAVE_ACL_TYPE_EXTENDED
      (*xa)[pos].xattrSize = getxattr(name, xattrTmp, NULL, 0, 0, XATTR_NOFOLLOW);
    #else
      #ifdef HAVE_SYS_XATTR_H
        (*xa)[pos].xattrSize = lgetxattr(name, xattrTmp, NULL, 0);
      #else
        (*xa)[pos].xattrSize = 0;
      #endif
    #endif
      // endwin();
      // printf("%s - %s - %zu\n", (*xa)[pos].name, (*xa)[pos].xattr, (*xa)[pos].xattrSize);
      reset = true;
      pos++;
      ++*xattrsNum;
    }
  }
  free(xattrTmp);

  return pos;
}

char *getRelativePath(char *file, char *target)
{
  char *result = malloc(sizeof(char) + 1);
  int i, j, e, c, resultLen, targetUp, fileUp;
  pathDirs *fileStruct, *targetStruct;
  int  fileLen, targetLen, commonPath = 0;

  targetUp = fileUp = 0;

  // Store sections of file in structure
  e = splitPath(&fileStruct, file);
  fileLen = e + 1;

  // Store sections of target in structure
  e = splitPath(&targetStruct, target);
  targetLen = e + 1;

  // Find the smallest of our structures
  if (fileLen > targetLen){
    c = targetLen;
  } else {
    c = fileLen;
  }

  // Count the common directories
  for(i = 0; i < c; i++){
    if (!strcmp(fileStruct[i].directories, targetStruct[i].directories)){
      commonPath++;
    } else {
      break;
    }
  }

  c = 0;
  targetUp = (targetLen - commonPath - 1);
  fileUp = (fileLen - commonPath);
  if (targetUp > 0){
    result = realloc(result, sizeof(char) * (targetUp * 3) + 1);
    for (i = 0; i < targetUp; i++){
      if (c == 0){
        sprintf(result, "%s/", "..");
      } else {
        sprintf(result, "%s%s/", result, "..");
      }
      c++;
    }
    for(i=(fileLen - fileUp); i < fileLen; i++){
      j = strlen(fileStruct[i].directories);
      result = realloc(result, sizeof(char) * (strlen(result) + j + 1));
      if (i == fileLen - 1){
        sprintf(result, "%s%s", result, fileStruct[i].directories);
      } else {
        sprintf(result, "%s%s/", result, fileStruct[i].directories);
      }
    }
  } else if ((targetUp < 1) && (fileUp > 1)){
    for(i=commonPath; i < fileLen; i++){
      j = strlen(fileStruct[i].directories);
      result = realloc(result, sizeof(char) * (strlen(result) + j + 1));
      if (c == 0){
        sprintf(result, "%s/", fileStruct[i].directories);
      } else if (i == fileLen - 1){
        sprintf(result, "%s%s", result, fileStruct[i].directories);
      } else {
        sprintf(result, "%s%s/", result, fileStruct[i].directories);
      }
      c++;
    }
  } else {
    // Assume we're in the same directory at this point
    j = strlen(fileStruct[fileLen - 1].directories);
    result = realloc(result, sizeof(char) * (j + 1));
    sprintf(result, "%s", fileStruct[fileLen - 1].directories);
  }

  // result[resultLen - 1] = '\0'; // This seems to cause no end of grief on FreeBSD and I can't even remember why it's here.


  free(fileStruct);
  free(targetStruct);

  return(result);
}

int wildcard(const char *value, char *wcard)
{

    int vsize = (int)strlen(value);
    int wsize = (int)strlen(wcard);
    int match = 0;
    int v = 0;
    int w = 0;
    int lookAhead = 0;
    int searchMode = 0;
    char search = '\0';

    if (vsize == 0 &&  wsize == 0) {
        match = 1;
    } else {


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
                    if (v == vsize){
                      break;
                    } else {
                      w = 0;
                    }
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

int writePermsEntry(char * perms, mode_t mode, int axFlag, int sLinkCheck){

  typecolor = DISPLAY_PAIR;

  sexec = 0;

  if (S_ISDIR(mode)) {
    perms[0] = 'd';
    typecolor = DIR_PAIR;
  } else if (S_ISCHR(mode)){
    if (!sLinkCheck){
      mmMode = 1;
    }
    perms[0] = 'c';
  } else if (S_ISLNK(mode)){
    perms[0] = 'l';
  } else if (S_ISFIFO(mode)){
    perms[0] = 'p';
  } else if (S_ISBLK(mode)){
    if (!sLinkCheck){
      mmMode = 1;
    }
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
    if (S_ISDIR(mode) && (mode & S_IWOTH)){
      typecolor = STICKY_OW_PAIR;
    } else if (S_ISDIR(mode) && !(mode & S_IWOTH)) {
      typecolor = STICKY_PAIR;
    }
  } else if (mode & S_ISVTX) {
    perms[9] = 'T';
    if (S_ISDIR(mode) && (mode & S_IWOTH)){
      typecolor = STICKY_OW_PAIR;
    } else if (S_ISDIR(mode) && !(mode & S_IWOTH)) {
      typecolor = STICKY_PAIR;
    }
  } else if (mode & S_IXOTH){
    perms[9] = 'x';
    if (typecolor != DIR_PAIR && !sexec){
      typecolor = EXE_PAIR;
    }
  } else {
    perms[9] = '-';
  }

  if (axFlag == ACL_XATTR){
    perms[10] = '@';
  } else if (axFlag == ACL_TRUE){
    perms[10] = '+';
  } else if (axFlag == ACL_SELINUX){
    perms[10] = '.';
  } else {
    perms[10] = ' ';
  }

  return typecolor;

}

void writeResultStruct(results* ob, const char * filename, struct stat buffer, int count, bool xAcl, ssize_t xattr, int seLinuxCon, char * contextText, char * xattrs, const int xattrsNum){
  char perms[12] = {0};
  struct group *gr;
  struct passwd *pw;
  struct passwd *au;
  char *filedate;
  ssize_t cslinklen = 0, datedisplayLen = 0;
  int axFlag = 0;
  int i;

  ob[count].xAcl = xAcl;
  ob[count].xattr = xattr;
  ob[count].xattrsNum = xattrsNum;

  if (xAcl){
    axFlag = ACL_TRUE;
  }

  if (xattr > 0){
    axFlag = ACL_XATTR;
  }

  if (seLinuxCon > 0){
    axFlag = ACL_SELINUX;
  }

  ob[count].xattrs = malloc(sizeof(char) * xattr);
  // strcpy(ob[count].xattrs, xattrs);
  for ( i = 0; i < xattr; i++ ){
    ob[count].xattrs[i] = xattrs[i];
  }

  writePermsEntry(perms, buffer.st_mode, axFlag, 0);

  // Writing our structure
  if ( markall && !(buffer.st_mode & S_IFDIR) ) {
    *ob[count].marked = 1;
  } else {
    *ob[count].marked = 0;
  }

  ob[count].mode = buffer.st_mode;

  ob[count].perm = malloc(sizeof(char) * (strlen(perms) + 1));
  strcpy(ob[count].perm, perms);
  *ob[count].hlink = buffer.st_nlink;
  *ob[count].hlinklens = strlen(hlinkstr);

  if (!getpwuid(buffer.st_uid)){
    ob[count].owner = malloc(sizeof(char) * 6);
    sprintf(ob[count].owner, "%i", buffer.st_uid);
  } else {
    pw = getpwuid(buffer.st_uid);
    ob[count].owner = malloc(sizeof(char) * (strlen(pw->pw_name) + 1));
    strcpy(ob[count].owner, pw->pw_name);
  }

  if (!getgrgid(buffer.st_gid)){
    ob[count].group = malloc(sizeof(char) * 6);
    sprintf(ob[count].group, "%i", buffer.st_gid);
  } else {
    gr = getgrgid(buffer.st_gid);
    ob[count].group = malloc(sizeof(char) * (strlen(gr->gr_name) + 1));
    strcpy(ob[count].group, gr->gr_name);
  }

  if (!getpwuid(buffer.st_author)){
    ob[count].author = malloc(sizeof(char) * 6);
    sprintf(ob[count].author, "%i", buffer.st_author);
  } else {
    au = getpwuid(buffer.st_author);
    ob[count].author = malloc(sizeof(char) * (strlen(au->pw_name) + 1));
    strcpy(ob[count].author, au->pw_name);
  }

  ob[count].size = buffer.st_size;
  *ob[count].sizelens = strlen(sizestr);

  // ob[count].sizeBlocks = buffer.st_blocks;
  ob[count].sizeBlocks = (buffer.st_blocks * 512) / block_size;

  if (S_ISCHR(buffer.st_mode) || S_ISBLK(buffer.st_mode)){
    ob[count].major = major(buffer.st_rdev);
    ob[count].minor = minor(buffer.st_rdev);
  } else {
    ob[count].major = ob[count].minor = 0; // Setting a default
  }

  ob[count].date = buffer.st_mtime;
  ob[count].adate = buffer.st_atime;

  filedate = dateString(ob[count].date, timestyle);
  ob[count].datedisplay = malloc(sizeof(wchar_t) * 64);
  datedisplayLen = (mbstowcs(ob[count].datedisplay, filedate, 64) + 1);
  free(ob[count].datedisplay);
  ob[count].datedisplay = malloc(sizeof(wchar_t) * datedisplayLen);
  mbstowcs(ob[count].datedisplay, filedate, datedisplayLen);
  // ob[count].datedisplay[datedisplayLen] = '\0';

  ob[count].name = malloc(sizeof(char) * (strlen(filename) + 1));
  strcpy(ob[count].name, filename);

  if (S_ISLNK(buffer.st_mode)) {
    ob[count].slink = malloc(sizeof(char) * 4096);
    cslinklen = readlink(filename, ob[count].slink, 4095);
    ob[count].slink[cslinklen] = '\0';
    // Now we know the size, lets clear the memory and read again.
    free(ob[count].slink);
    ob[count].slink = malloc(sizeof(char) * (cslinklen + 1));
    readlink(filename, ob[count].slink, cslinklen);
    ob[count].slink[cslinklen] = '\0';
  } else {
    ob[count].slink = malloc(sizeof(char) + 1);
    strcpy(ob[count].slink, "");
  }

  ob[count].contextText = malloc(sizeof(char) * (strlen(contextText) + 1));
  strcpy(ob[count].contextText, contextText);

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
  // if (skipToFirstFile == 1 && skippable == 1){
  //   return 2;
  // } else {
  //   return 0;
  // }
  return -1;
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
  } else {
    // Default back to locale
    long_time_format[0] = "%b %e  %Y";
    long_time_format[1] = "%b %e %H:%M";
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
  char *dest;
  int i;
  if (num_of_spaces < 1){
    i = 1;
  } else {
    i = num_of_spaces;
  }
  dest = malloc (sizeof (char) * (i + 1));
  if (num_of_spaces > 0){
    sprintf(dest, "%*s", num_of_spaces, " ");
  } else {
    strcpy(dest, "");
  }
  return dest;
}

wchar_t *wWriteSegment(int segLen, wchar_t *text, int align){
  wchar_t *segment;
  int paddingLen;
  char *padding;
  int textLen;
  int i;

  textLen = wcslen(text);
  paddingLen = segLen - textLen;

  segment = calloc((segLen + 2), sizeof(wchar_t));

  if (align == LEFT){
    padding = genPadding(paddingLen);
    swprintf(segment, (segLen + 2), L"%ls%s ", text, padding);
  } else {
    if (paddingLen < 0){
      padding = genPadding(paddingLen - 1);
    } else {
      padding = genPadding(paddingLen);
    }
    swprintf(segment, (segLen + 2), L"%s%ls ", padding, text);
  }
  free(padding);
  return(segment);
}

char *writeSegment(int segLen, char *text, int align){
  // Another wrapper for char to wchar_t
  char *segment;
  wchar_t *inputText;
  wchar_t *wWriteSegmentString;
  inputText = calloc((strlen(text) + 1), sizeof(wchar_t));
  swprintf(inputText, (strlen(text) + 1), L"%s", text);
  segment = calloc((segLen + 2), sizeof(char));
  wWriteSegmentString = wWriteSegment(segLen, inputText, align);
  free(inputText);
  sprintf(segment, "%ls", wWriteSegmentString);
  free(wWriteSegmentString);
  return(segment);
}

void printXattr(int start, int selected, int listref, int currentItem, int subIndex, xattrList* xa, results* ob){
  int i;

  int maxlen = COLS - start - 1;

  char *tmpXattrAt;
  int xattrAtPos;
  bool hasXattr = false;
  int tmpXattrDataLen;
  char *tmpXattrPrint;
  char *tmpXattrSize;
  int tmpXattrSizeLen;
  char *tmpXattrPadding;
  int linepadding;
  int xattrKeySegmentLen;
  char *xattrKeySegment;
  int xattrSizeSegmentLen;
  char *xattrSizeSegment;
  char xattrPrePad[13] = "            ";
  char *paddingE0;

  charPos = 0;

  // Setting highlight
  if (selected) {
    setColors(SELECT_PAIR);
  } else {
    setColors(DISPLAY_PAIR);
  }

  for (i = 0; i < xattrPos; i++){
    if (!strcmp(xa[i].name, ob[currentItem].name)) {
      xattrAtPos = i;
      break;
    }
  }
  if (human){
    tmpXattrSize = malloc(sizeof(char) * 10);
    readableSize(xa[xattrAtPos + subIndex].xattrSize, tmpXattrSize, si);
  } else {
    tmpXattrSizeLen = snprintf(NULL, 0, "%zu", xa[xattrAtPos + subIndex].xattrSize);
    tmpXattrSize = malloc(sizeof(char) * tmpXattrSizeLen);
    sprintf(tmpXattrSize, "%zu", xa[xattrAtPos + subIndex].xattrSize);
  }

  xattrKeySegment = writeSegment(xattrKeyLen, xa[xattrAtPos + subIndex].xattr, LEFT);

  tmpXattrPrint = calloc(COLS, sizeof(char));
  // tmpXattrDataLen = snprintf(NULL, 0, "            %s        %s", xa[xattrAtPos + subIndex].xattr, tmpXattrSize);
  // tmpXattrPadding = genPadding(COLS - tmpXattrDataLen);
  sprintf(tmpXattrPrint, "%s%s%s", xattrPrePad, xattrKeySegment, tmpXattrSize);
  for (i = 0; i < maxlen; i++){
    mvprintw(displaystart + listref, start + charPos, "%c", tmpXattrPrint[i]);
    charPos++;
    if (i == strlen(tmpXattrPrint) - 1){
      break;
    }
  }
  // mvprintw(displaystart + listref + offset + i, start + charPos, "            %s", xa[(xattrAtPos + (i - 1))].xattr);
  free(tmpXattrPrint);
  free(tmpXattrSize);
  free(tmpXattrPadding);

  free(xattrKeySegment);

  linepadding = COLS - charPos - start;

  if (linepadding > 0){
    if ((charPos + start) > 0){
      paddingE0 = genPadding(linepadding);
      mvprintw(displaystart + listref, charPos + start, "%s", paddingE0);
    } else {
      paddingE0 = genPadding(COLS);
      mvprintw(displaystart + listref, 0, "%s", paddingE0);
    }
    free(paddingE0);
  }
  // mvprintw(displaystart + listref + start, 0, "            %s        %s", "uk.me.robertianhawdon.test", "27");
}

void printEntry(int start, int hlinklen, int sizeblocklen, int ownerlen, int grouplen, int authorlen, int sizelen, int majorlen, int minorlen, int datelen, int namelen, int contextlen, int selected, int listref, int currentitem, results* ob){

  int i, n, t;

  char marked[2];
  // wchar_t *entryMeta  = malloc(sizeof(wchar_t) + 1);
  wchar_t *entryName  = malloc(sizeof(wchar_t) + 1);
  wchar_t *entrySLink = malloc(sizeof(wchar_t) + 1);
  int maxlen = COLS - start - 1;

  // int currentitem = listref + topref;
  int nameminlen = strlen(headName); // Length of "Name" heading
  int ogminlen = strlen(headOG); // Length of "Owner & Group" heading
  int sizeminlen = strlen(headSize); // Length of "Size" heading
  int dateminlen = strlen(headDT); // Length of "Date" heading
  int contextminlen = strlen(headContext); // Length of "Context" heading
  int sizeblockminlen = strlen(headSizeBlocks); // Length of "Size (Blocks)" heading

  int oggap, gagap = 0;

  int oglen = 0;

  char *ogaval;

  int hlinkCharLen;
  char *hlinkChar;

  int ogseglen = ogalen + ogapad;

  int ogpad = 0;
  int sizepad = 0;
  int mmpad = 0;
  int contextpad = 0;

  int datepad = 0;

  // char *s1, *s2, *s3, *s4, *s5, *s6;

  char *sizePadding;

  char *sizestring;

  char *contextText;

  char *sizeBlocksString;

  int printSegment, printNameSegment = 0;

  int nameCombineLen, nameFullSegPadding;

  char *markedSegment;
  char *attrSegment;
  char *sizeBlocksSegment;
  char *hlinkSegment;
  char *ownerSegment;
  char *contextSegment;
  char *sizeSegment;
  wchar_t *dateSegment;
  wchar_t *linkSegment;
  nameStruct *nameSegmentData;

  wchar_t *tmpSegment;

  int linepadding;
  int colpos;

  char tmpperms[12];

  char *printPerm;

  struct stat buffer;
  int status;

  char slinkpoint[5];

  char *paddingG0, *paddingG1, *paddingE0;

  strcpy(slinkpoint, " -> \0");

  charPos = 0;

  // Owner, Group, Author
  switch(ogavis){
  case 0:
    ogaval = malloc (sizeof (char) + 1);
    strcpy(ogaval,"");
    break;
  case 1:
    oglen = (strlen(ob[currentitem].owner));
    ogaval = malloc (sizeof (char) * (oglen + 2));
    sprintf(ogaval, "%s", ob[currentitem].owner);
    break;
  case 2:
    oglen = (strlen(ob[currentitem].group));
    ogaval = malloc (sizeof (char) * (oglen + 1));
    sprintf(ogaval, "%s", ob[currentitem].group);
    break;
  case 3:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].group));
    ogaval = malloc (sizeof (char) * (oglen + 1));
    paddingG0 = genPadding(oggap);
    sprintf(ogaval, "%s%s%s", ob[currentitem].owner, paddingG0, ob[currentitem].group);
    free(paddingG0);
    break;
  case 4:
    oglen = (strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * (oglen + 1));
    sprintf(ogaval, "%s", ob[currentitem].author);
    break;
  case 5:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * (oglen + 1));
    paddingG0 = genPadding(oggap);
    sprintf(ogaval, "%s%s%s", ob[currentitem].owner, paddingG0, ob[currentitem].author);
    free(paddingG0);
    break;
  case 6:
    gagap = grouplen - strlen(ob[currentitem].group) + 1;
    oglen = (strlen(ob[currentitem].group) + gagap + strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * (oglen + 1));
    paddingG0 = genPadding(gagap);
    sprintf(ogaval, "%s%s%s", ob[currentitem].group, paddingG0, ob[currentitem].author);
    break;
  case 7:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    gagap = grouplen - strlen(ob[currentitem].group) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].group) + gagap + strlen(ob[currentitem].author));
    ogaval = malloc (sizeof (char) * (oglen + 1));
    paddingG0 = genPadding(oggap);
    paddingG1 = genPadding(gagap);
    sprintf(ogaval, "%s%s%s%s%s", ob[currentitem].owner, paddingG0, ob[currentitem].group, paddingG1, ob[currentitem].author);
    free(paddingG0);
    free(paddingG1);
    break;
  default:
    oggap = ownerlen - strlen(ob[currentitem].owner) + 1;
    oglen = (strlen(ob[currentitem].owner) + oggap + strlen(ob[currentitem].group));
    ogaval = malloc (sizeof (char) * (oglen + 1));
    paddingG0 = genPadding(oggap);
    sprintf(ogaval, "%s%s%s", ob[currentitem].owner, paddingG0, ob[currentitem].group);
    free(paddingG0);
    break;
  }

  if (!ogavis){
    ogpad = 0;
    ownerSegment = malloc(sizeof(char));
    sprintf(ownerSegment, "");
  } else {
    if ( (ogminlen - ogseglen) > 0 ) {
      ownerSegmentLen = ogminlen;
    } else {
      ownerSegmentLen = ogseglen;
    }
    ownerSegment = writeSegment(ownerSegmentLen, ogaval, LEFT);
  }


  if (showContext){
    // Test
    // endwin();
    // printf("%s - %s\n", ob[currentitem].name, ob[currentitem].contextText);
    contextText = malloc(sizeof(char) * (strlen(ob[currentitem].contextText) + 1));
    sprintf(contextText, "%s", ob[currentitem].contextText);
    if (contextlen < contextminlen) {
      contextSegmentLen = contextminlen;
    } else {
      contextSegmentLen = contextlen;
    }
    contextSegment = writeSegment(contextSegmentLen, contextText, LEFT);
  } else {
    contextSegmentLen = 1;
    contextText = malloc(sizeof(char) * contextSegmentLen);
    sprintf(contextText, "");
    contextSegment = malloc(sizeof(char) * contextSegmentLen);
    sprintf(contextSegment, "");
  }

  if (showSizeBlocks){
    sprintf(sizeblocksstr, "%lld", ob[currentitem].sizeBlocks);
    if (sizeblocklen < sizeblockminlen) {
      sizeBlocksSegmentLen = sizeblockminlen;
    } else {
      sizeBlocksSegmentLen = sizeblocklen;
    }
    sizeBlocksString = malloc(sizeof(char) * (sizeBlocksSegmentLen + 1));
    sprintf(sizeBlocksString, "%s", sizeblocksstr);
    sizeBlocksSegment = writeSegment(sizeBlocksSegmentLen, sizeBlocksString, RIGHT);
  } else {
    sizeBlocksSegmentLen = 1;
    sizeBlocksString = malloc(sizeof(char) * sizeBlocksSegmentLen);
    sprintf(sizeBlocksString, "");
    sizeBlocksSegment = malloc(sizeof(char) * sizeBlocksSegmentLen);
    sprintf(sizeBlocksSegment, "");
  }

  if (ob[currentitem].minor > 1){
    mmpad = sizelen - (log10(ob[currentitem].minor + 1)) + 2;
  } else {
    mmpad = sizelen + 1;
  }

  if (mmpad > 0 ){
    sizePadding = genPadding(mmpad);
  }

  if ((ob[currentitem].major > 0) || (ob[currentitem].minor > 0)){
    // If either of these are not 0, then we're dealing with a Character or Block device.
    // sizestring = malloc (sizeof (char) * (sizelen + 5));
    sizelen = snprintf(NULL, 0, "%i,%s%i", ob[currentitem].major, sizePadding, ob[currentitem].minor);
    sizestring = malloc (sizeof (char) * (sizelen + 1));
    sprintf(sizestring, "%i,%s%i", ob[currentitem].major, sizePadding, ob[currentitem].minor);
  } else {
    if ((human) && (!scaleSize)){
      sizestring = malloc (sizeof (char) * 10);
      readableSize(ob[currentitem].size, sizestring, si);
    } else {
      sizestring = malloc (sizeof (char) * (sizelen + 1));
      if (scaleSize){
        sprintf(sizestring, "%lu", (ob[currentitem].size / block_size + 1));
      } else {
        sprintf(sizestring, "%lu", ob[currentitem].size);
      }
    }
  }

  // Redefining width of Size value if the all sizes are smaller than the header.
  if ( sizelen < sizeminlen ) {
    sizelen = sizeminlen;
  }

  sizeSegmentLen = sizelen; // this is a mess

  sizepad = (sizelen - strlen(sizestring));

  // Experemental segment
  sizeSegment = writeSegment(sizeSegmentLen, sizestring, RIGHT);

  if ( (dateminlen - datelen) > 0 ) {
    dateSegmentLen = dateminlen;
  } else {
    dateSegmentLen = datelen;
  }
  dateSegment = wWriteSegment(dateSegmentLen, ob[currentitem].datedisplay, LEFT);

  // if (hlinkstart > -1){
  //   s1 = genPadding(hlinkstart);
  // }
  // if (sizepad > -1){
  //   s2 = genPadding(sizepad);
  // }
  // if (datepad > -1){
  //   s3 = genPadding(datepad);
  // }

  markedSegmentLen = 3;

  if ( *ob[currentitem].marked ){
    markedSegment = writeSegment(markedSegmentLen, "*", RIGHT);
  } else {
    markedSegment = writeSegment(markedSegmentLen, " ", RIGHT);
  }

  if (axDisplay){
    attrSegmentLen = 11;
  } else {
    attrSegmentLen = 10;
  }

  if (*ob[currentitem].hlink > 0){
    hlinkCharLen = (log10(*ob[currentitem].hlink) + 2);
  } else {
    hlinkCharLen = 2;
  }

  hlinkChar = malloc(sizeof(char) * (hlinkCharLen));

  sprintf(hlinkChar, "%i", *ob[currentitem].hlink);

  hlinkSegmentLen = hlinklen; // messy

  hlinkSegment = writeSegment(hlinkSegmentLen, hlinkChar, RIGHT);

  free(hlinkChar);

  printPerm = malloc(sizeof(char) * attrSegmentLen + 1);
  for (i = 0; i < attrSegmentLen; i++){
    printPerm[i] = ob[currentitem].perm[i];
    printPerm[attrSegmentLen] = '\0';
  }

  attrSegment = writeSegment(attrSegmentLen, printPerm, LEFT);

  // entryMetaLen = snprintf(NULL, 0, "%s%s%s%s%s%s%ls", markedSegment, attrSegment, hlinkSegment, ownerSegment, contextSegment, sizeSegment, dateSegment);

  // entryMeta = realloc(entryMeta, sizeof(wchar_t) * (entryMetaLen + 1));

  // swprintf(entryMeta, (entryMetaLen + 1), L"%s%s%s%s%s%s%ls", markedSegment, attrSegment, hlinkSegment, ownerSegment, contextSegment, sizeSegment, dateSegment);

  free(printPerm);

  // entryMetaLen = wcslen(entryMeta);

  // Writing Name Segment Data
  nameSegmentData = malloc(sizeof(nameStruct));
  nameSegmentData[0].name = malloc(sizeof(wchar_t) * (strlen(ob[currentitem].name) + 1));
  swprintf(nameSegmentData[0].name, (strlen(ob[currentitem].name) + 1), L"%s", ob[currentitem].name);
  if ( !strcmp(ob[currentitem].slink, "") ){
    nameSegmentData[0].linkStat = 0;
    nameSegmentData[0].link = malloc(sizeof(wchar_t));
  } else {
    nameSegmentData[0].linkStat = 1;
    nameSegmentData[0].link = malloc(sizeof(wchar_t) * (strlen(ob[currentitem].slink) + 1));
    swprintf(nameSegmentData[0].link, (strlen(ob[currentitem].slink) + 1), L"%s", ob[currentitem].slink);
  }
  if ( nameSegmentData[0].linkStat ){
    nameCombineLen = (wcslen(nameSegmentData[0].name) + wcslen(nameSegmentData[0].link) + 4);
  } else {
    nameCombineLen = wcslen(nameSegmentData[0].name);
  }
  if (nameSegLen > nameminlen){
    nameFullSegPadding = nameSegLen - nameCombineLen + 1;
    nameSegmentDataLen = nameSegLen;
  } else {
    nameFullSegPadding = nameminlen - nameCombineLen + 1;
    nameSegmentDataLen = nameminlen;
  }
  if ( nameFullSegPadding < 1 ){
    nameFullSegPadding = 1;
  }
  nameSegmentData[0].padding = genPadding(nameFullSegPadding);


  entryNameLen = snprintf(NULL, 0, "%s", ob[currentitem].name) + 1;

  entryName = realloc(entryName, sizeof(wchar_t) * (entryNameLen + 1));

  swprintf(entryName, (entryNameLen + 1), L"%s", ob[currentitem].name);

  entryNameLen = wcslen(entryName);

  if ( !strcmp(ob[currentitem].slink, "") ){
    entrySLinkLen = 1;
    swprintf(entrySLink, entrySLinkLen, L"");
  } else {
    entrySLinkLen = snprintf(NULL, 0, "%s", ob[currentitem].slink);
    entrySLink = realloc(entrySLink, sizeof(wchar_t) * (entrySLinkLen + 1));
    swprintf(entrySLink, (entrySLinkLen + 1), L"%s\0", ob[currentitem].slink);
    entrySLinkLen = wcslen(entrySLink);
  }

  // entryMetaLen = wcslen(entryMeta);
  // entryNameLen = wcslen(entryName);
  // entrySLinkLen = wcslen(entrySLink);
  // mvprintw(4 + listref, start, "%s", entry);

  // Setting highlight
  if (selected) {
    setColors(SELECT_PAIR);
  } else {
    setColors(DISPLAY_PAIR);
  }

  // // Temp
  // if (showXAttrs){
  //   hasXattr = false;
  //   xattrAtPos = 0;
  //   for(i = 0; i < xattrPos; i++){
  //     if (!strcmp(ob[currentitem].name, xa[i].name)){
  //       xattrAtPos = i;
  //       hasXattr = true;
  //       break;
  //     }
  //   }
  //   // endwin();
  //   // printf("\n%s:\n", ob[currentitem].name);
  //   for(i = 0; i < ob[currentitem].xattrsNum + 1; i++){
  //     // printf("%lu < %i\n", (listref - (ob[currentitem].xattrsNum - i)), visibleObjects);
  //     if ((listref - (ob[currentitem].xattrsNum - i)) < (visibleObjects + visibleOffset)){
  //       if (hasXattr && i != 0){
  //         if (human){
  //           tmpXattrSize = malloc(sizeof(char) * 10);
  //           readableSize(xa[xattrAtPos + (i - 1)].xattrSize, tmpXattrSize, si);
  //         } else {
  //           tmpXattrSizeLen = snprintf(NULL, 0, "%zu", xa[xattrAtPos + (i - 1)].xattrSize);
  //           tmpXattrSize = malloc(sizeof(char) * tmpXattrSizeLen);
  //           sprintf(tmpXattrSize, "%zu", xa[xattrAtPos + (i - 1)].xattrSize);
  //         }
  //         tmpXattrPrint = calloc(COLS, sizeof(char));
  //         tmpXattrDataLen = snprintf(NULL, 0, "            %s        %s", xa[(xattrAtPos + (i - 1))].xattr, tmpXattrSize);
  //         tmpXattrPadding = genPadding(COLS - tmpXattrDataLen);
  //         sprintf(tmpXattrPrint, "            %s        %s%s", xa[(xattrAtPos + (i - 1))].xattr, tmpXattrSize, tmpXattrPadding);
  //         for (n = 0; n < strlen(tmpXattrPrint); n++){
  //           mvprintw(displaystart + listref + offset + i, start + charPos + n, "%c", tmpXattrPrint[n]);
  //         }
  //         // mvprintw(displaystart + listref + offset + i, start + charPos, "            %s", xa[(xattrAtPos + (i - 1))].xattr);
  //         free(tmpXattrPrint);
  //         free(tmpXattrSize);
  //         free(tmpXattrPadding);
  //       }
  //     }
  //   }
  // }

  for ( n = 0; n < (sizeof(segOrder) / sizeof(segOrder[0])); n++){
    t = segOrder[n];
    switch(t){
    case COL_MARK:
      printSegment = 1;
      tmpSegmentLen = markedSegmentLen + 2;
      tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
      swprintf(tmpSegment, tmpSegmentLen, L"%s", markedSegment);
      break;
    case COL_SIZEBLOCKS:
      if (showSizeBlocks){
        printSegment = 1;
        tmpSegmentLen = sizeBlocksSegmentLen + 2;
        tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
        swprintf(tmpSegment, tmpSegmentLen, L"%s", sizeBlocksSegment);
      }
      break;
    case COL_ATTR:
      printSegment = 1;
      tmpSegmentLen = attrSegmentLen + 2;
      tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
      swprintf(tmpSegment, tmpSegmentLen, L"%s", attrSegment);
      break;
    case COL_HLINK:
      printSegment = 1;
      tmpSegmentLen = hlinkSegmentLen + 2;
      tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
      swprintf(tmpSegment, tmpSegmentLen, L"%s", hlinkSegment);
      break;
    case COL_OWNER:
      if (ogavis){
        printSegment = 1;
        tmpSegmentLen = ownerSegmentLen + 2;
        tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
        swprintf(tmpSegment, tmpSegmentLen, L"%s", ownerSegment);
      }
      break;
    case COL_CONTEXT:
      if (showContext){
        printSegment = 1;
        tmpSegmentLen = contextSegmentLen + 2;
        tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
        swprintf(tmpSegment, tmpSegmentLen, L"%s", contextSegment);
      }
      break;
    case COL_SIZE:
      printSegment = 1;
      tmpSegmentLen = sizeSegmentLen + 2;
      tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
      swprintf(tmpSegment, tmpSegmentLen, L"%s", sizeSegment);
      break;
    case COL_DATE:
      printSegment = 1;
      tmpSegmentLen = dateSegmentLen + 2;
      tmpSegment = malloc(sizeof(wchar_t) * tmpSegmentLen);
      swprintf(tmpSegment, tmpSegmentLen, L"%ls", dateSegment);
      break;
    case COL_NAME:
      printSegment = 0;
      printNameSegment = 1;
      break;
    default:
      printSegment = 0;
      break;
    }

    if (printSegment){
      for ( i = 0; i < maxlen; i++ ){
        mvprintw(displaystart + listref, start + charPos, "%lc", tmpSegment[i]);
        charPos++;
        if (i == tmpSegmentLen - 2){
          break;
        }
      }
      free(tmpSegment);
      printSegment = 0;
    }

    if (printNameSegment){
      if (filecolors && !selected){
        if ( strcmp(ob[currentitem].slink, "" )) {
          if (check_file(ob[currentitem].slink)){
            setColors(SLINK_PAIR);
          } else {
            setColors(DEADLINK_PAIR);
          }
        } else {
          setColors(ob[currentitem].color);
        }
      }

      for ( i = 0; i < maxlen; i++ ){
        mvprintw(displaystart + listref, start + charPos, "%lc", nameSegmentData[0].name[i]);
        charPos++;
        if ( i == wcslen(nameSegmentData[0].name) - 1 ){
          break;
        }
      }

      if ( nameSegmentData[0].linkStat ){
        if (!selected){
          setColors(DISPLAY_PAIR);
        }

        for ( i = 0; i < strlen(slinkpoint); i++) {
          mvprintw(displaystart + listref, start + charPos, "%c", slinkpoint[i]);
          charPos++;
        }

        if (filecolors && !selected){
          if ( nameSegmentData[0].linkStat ) {
            if ( check_dir(ob[currentitem].slink) ){
              setColors(DIR_PAIR);
            } else if ( !check_file(ob[currentitem].slink) ){
              setColors(DEADLINK_PAIR);
            } else {
              // setColors(ob[currentitem].color);
              status = lstat(ob[currentitem].slink, &buffer);
              setColors(writePermsEntry(tmpperms, buffer.st_mode, -1, 1));
            }
          }
        }

          for ( i = 0; i < maxlen; i++ ){
            mvprintw(displaystart + listref, start + charPos,"%lc", nameSegmentData[0].link[i]);
            charPos++;
            if ( i == wcslen(nameSegmentData[0].link) - 1 ){
              break;
            }
          }
        }

      if (filecolors && !selected){
        setColors(DISPLAY_PAIR);
      }

      for ( i = 0; i < maxlen; i++){
        mvprintw(displaystart + listref, start + charPos, "%c", nameSegmentData[0].padding[i]);
        charPos++;
        if ( i == strlen(nameSegmentData[0].padding) - 1 ){
          break;
        }
      }

      printNameSegment = 0;
    }
  }

  free(nameSegmentData[0].name);
  free(nameSegmentData[0].link);
  free(nameSegmentData[0].padding);
  free(nameSegmentData);

  linepadding = COLS - charPos - start;

  if (linepadding > 0){
    if ((charPos + start) > 0){
      paddingE0 = genPadding(linepadding);
      mvprintw(displaystart + listref, charPos + start, "%s", paddingE0);
    } else {
      paddingE0 = genPadding(COLS);
      mvprintw(displaystart + listref, 0, "%s", paddingE0);
    }
    free(paddingE0);
  }

  // Free segments
  free(markedSegment);
  free(sizeBlocksSegment);
  free(attrSegment);
  free(hlinkSegment);
  free(ownerSegment);
  free(contextSegment);
  free(sizeSegment);
  free(dateSegment);

  // free(s1);
  // free(s2);
  // free(s3);
  // free(s4);
  // free(s5);
  // free(s6);
  free(sizestring);
  free(sizePadding);
  // free(entryMeta);
  free(entryName);
  free(entrySLink);
  free(ogaval);
}

void LaunchShell()
{
  char c[5];
  sprintf(c, "%i", showProcesses);
  setenv("DFS_RUNNING", c, 1);
  clear();
  // def_prog_mode();
  endwin();
  // system("clear"); // Not exactly sure if I want this yet.
  // printf("\nUse 'exit' to return to Show.\n\n");
  write(STDOUT_FILENO, "\nUse 'exit' to return to Show.\n\n", 32);
  system(getenv("SHELL"));
  // launchExternalCommand(getenv("SHELL"), NULL, M_NORMAL);
  // reset_prog_mode();
  // newterm(NULL, stderr, stdin); 
  // initscr();
  refreshScreen();
}

void LaunchExecutable(const char* object, const char* args)
{
  char *command = malloc(sizeof(char) * (strlen(object) + strlen(args) + 4));
  sprintf(command, "'%s' %s", object, args);
  system("clear"); // Just to be sure
  system(command);
  free(command);
  refreshScreen();
}

void copy_file(char *source_input, char *target_input, mode_t mode)
{
  char *targetmod = malloc(sizeof(char) * (strlen(target_input) + 1));
  FILE *source = NULL;
  FILE *target = NULL;
  char ch = '\0';
  size_t n, m;
  unsigned char buff[8192];

  strcpy(targetmod, target_input);

  source = fopen(source_input, "rb");


  if ( check_dir(targetmod) ){
    if ( !check_last_char(targetmod, "/")){
      targetmod = realloc(targetmod, sizeof(char) * (strlen(targetmod) + 2));
      strcat(targetmod, "/");
    }
    targetmod = realloc(targetmod, sizeof(char) * (strlen(basename(source_input)) + 1));
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

  free(targetmod);
  fclose(source);
  fclose(target);
  chmod(target_input, mode);
}

void delete_file(char *source_input)
{
  remove(source_input);
}

int SendToPager(char* object)
{
  char *page = malloc(sizeof(char) * 2);
  char *pagerCommand;
  int pset = 0;
  int e = 0;
  char *escObject = str_replace(object, "'", "'\"'\"'");

  if (access(object, R_OK) == 0){
    if (can_run_command("sf")){
      if (!useEnvPager){
        setenv("DFS_THEME_OVERRIDE", "TRUE", 1);
        // page = realloc(page, (sizeof(char) * 3));
        // sprintf(page, "sf");
        free(page);
        page = commandFromPath("sf");
        pset = 1;
      }
    } else {
      useEnvPager = 1;
    }
  
    if (useEnvPager){
      if ( getenv("PAGER")) {
        // page = realloc(page, (sizeof(char) * (strlen(getenv("PAGER")) + 1)));
        // sprintf(page, "%s", getenv("PAGER"));
        page = commandFromPath(getenv("PAGER"));
        pset = 1;
      }
    }
  
    if ( pset ) {
      pagerCommand = malloc(sizeof(char) * (strlen(page) + strlen(escObject) + 4));
      sprintf(pagerCommand, "%s %s", page, escObject);
      char *args[countArguments(pagerCommand)];
      buildCommandArguments(pagerCommand, args, countArguments(pagerCommand));
      launchExternalCommand(args[0], args, M_NONE);
      free(escObject);
      free(pagerCommand);
    } else {
      topLineMessage("Please export a PAGER environment variable to define the utility program name.");
    }
    free(page);
    } else {
      topLineMessage("Error: Permission denied");
    }
  return 0;
}

int SendToEditor(char* object)
{
  // char *editor = malloc(sizeof(char) * 2);
  char *originalCmd;
  char *editor;
  char *editorCommand;
  char fullCommand[2048];
  int eset = 0;
  int e = 0;
  int editorLen, escObjectLen, fullCommandLen;
  char *escObject = str_replace(object, "'", "'\"'\"'");
  int i;
  int noOfArgs = 0;

  if (access(object, R_OK) == 0){
    originalCmd = malloc(sizeof(char) + 1);
    if ( !eset ) {
      if (!useDefinedEditor){
        if (getenv("EDITOR")){
          originalCmd = realloc(originalCmd, sizeof(char) * (strlen(getenv("EDITOR") + 1)));
          strcpy(originalCmd, getenv("EDITOR"));
        } else if (getenv("VISUAL")){
          originalCmd = realloc(originalCmd, sizeof(char) * (strlen(getenv("VISUAL") + 1)));
          strcpy(originalCmd, getenv("VISUAL"));
        }
          noOfArgs = countArguments(originalCmd);
      } else {
        originalCmd = realloc(originalCmd, sizeof(char) * (strlen(visualPath) + 1));
        strcpy(originalCmd, visualPath);
        noOfArgs = countArguments(originalCmd);
      }
      char *launchCommand[countArguments(originalCmd)];
      buildCommandArguments(originalCmd, launchCommand, countArguments(originalCmd));
      editor = commandFromPath(launchCommand[0]);
      if (can_run_command(editor)){
        sprintf(fullCommand, "%s", editor);
        for (i = 1; i < (countArguments(originalCmd)); i++){
          sprintf(fullCommand, "%s %s", fullCommand, launchCommand[i]);
        }
        eset = 1;
      }
    }
    if ( eset ){
      fullCommandLen = strlen(fullCommand);
      escObjectLen = strlen(escObject);
      editorCommand = malloc(sizeof(char) * (fullCommandLen + escObjectLen + 4));
      sprintf(editorCommand, "%s %s", fullCommand, escObject);
      char *args[countArguments(editorCommand)];
      buildCommandArguments(editorCommand, args, countArguments(editorCommand));
      launchExternalCommand(args[0], args, M_NONE);
      free(escObject);
      free(editorCommand);
    } else {
      topLineMessage("Please set a valid editor utility program command in settings.");
    }
    free(originalCmd);
    free(editor);
  } else {
    topLineMessage("Error: Permission denied");
  }
  return 0;
}

uintmax_t GetAvailableSpace(const char* path)
{
  struct statvfs stat;
  uintmax_t bavail;
  uintmax_t frsize;
  uintmax_t result;

  if (statvfs(path, &stat) != 0) {
    // error happens, just quits here, but returns 0
    return 0;
  }

  bavail = stat.f_bavail;
  frsize = stat.f_frsize;
  result = bavail * frsize;

  // the available size is f_bsize * f_bavail
  // return stat.f_bsize * stat.f_bavail;
  // endwin();
  // clear();
  // printf("f_bavail: %jd\nf_frsize: %jd\n", (uintmax_t) bavail, (uintmax_t) frsize);
  // printf("f_bavail * f_frsize: %jd\n", (uintmax_t) (bavail * frsize));
  // printf("f_bfree: %li\n", stat.f_bfree);
  // // exit(0);
  return result;
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

  // This seems unnessessary, what was I thinking?


  // if (!strcmp(segname, "owner")) {
  //   longest = strlen(dfseg[0].owner);
  // }
  // else if (!strcmp(segname, "group")) {
  //   longest = strlen(dfseg[0].group);
  // }
  // else if (!strcmp(segname, "author")) {
  //   longest = strlen(dfseg[0].author);
  // }
  // else if (!strcmp(segname, "hlink")) {
  //   sprintf(hlinkstr, "%d", *dfseg[0].hlink);
  //   longest = strlen(hlinkstr);
  // }
  // else if (!strcmp(segname, "size")) {
  //   if (human){
  //     readableSize(dfseg[0].size, sizestr, si);
  //   } else {
  //     sprintf(sizestr, "%lu", dfseg[0].size);
  //   }
  //   longest = strlen(sizestr);
  // }
  // else if (!strcmp(segname, "major")) {
  //   sprintf(majorstr, "%d", dfseg[0].major);
  //   longest = strlen(majorstr);
  // }
  // else if (!strcmp(segname, "minor")) {
  //   sprintf(minorstr, "%d", dfseg[0].minor);
  //   longest = strlen(minorstr);
  // }
  // else if (!strcmp(segname, "datedisplay")) {
  //   longest = wcslen(dfseg[0].datedisplay);
  // }
  // else if (!strcmp(segname, "name")) {
  //   longest = strlen(dfseg[0].name);
  // }
  // else if (!strcmp(segname, "slink")) {
  //   longest = strlen(dfseg[0].slink);
  // }
  // else if (!strcmp(segname, "contextText")) {
  //   longest = strlen(dfseg[0].contextText);
  // }
  // else {
  //   longest = 0;
  // }

  longest = 0;

  for(i = 0; i < LEN; i++)
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
      else if (!strcmp(segname, "sizeBlocks")) {
        sprintf(sizeblocksstr, "%lld", dfseg[i].sizeBlocks);
        len = strlen(sizeblocksstr);
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
        len = wcslen(dfseg[i].datedisplay);
      }
      else if (!strcmp(segname, "name")) {
        len = strlen(dfseg[i].name);
      }
      else if (!strcmp(segname, "slink")) {
        len = strlen(dfseg[i].slink);
      }
      else if (!strcmp(segname, "contextText")) {
        len = strlen(dfseg[i].contextText);
      }
      else if (!strcmp(segname, "nameSegBlock")) {
        if (strlen(dfseg[i].slink) > 0){
          len = (strlen(dfseg[i].name) + strlen(dfseg[i].slink) + 4);
        } else {
          len = strlen(dfseg[i].name);
        }
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
  time_t compare;
  int output;
  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  if (reverse){
    // Oldest to Newest
    compare = (dforderA->date - dforderB->date);
  } else {
    // Newest to Oldest
    compare = (dforderB->date - dforderA->date);
  }

  if (compare > 0){
    output = 1;
  } else if (compare < 0) {
    output = -1;
  } else {
    output = 0;
  }

  return output;

}

int cmp_dflist_size(const void *lhs, const void *rhs)
{
  ptrdiff_t compare;
  int output;
  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  if (reverse){
    // Smallest to Largest
    compare = (dforderA->size - dforderB->size);
  } else {
    // Largest to Smallest
    compare = (dforderB->size - dforderA->size);
  }

  if (compare > 0){
    output = 1;
  } else if (compare < 0) {
    output = -1;
  } else {
    output = 0;
  }

  return output;

}

int check_object(const char *object){
  struct stat sb;
  if (stat(object, &sb) == 0 ){
    if (S_ISDIR(sb.st_mode)){
      if (access(object, F_OK|X_OK) == 0){
        return 1;
      } else {
        return 0;
      }
    } else if (S_ISREG(sb.st_mode) || S_ISBLK(sb.st_mode) || S_ISFIFO(sb.st_mode) || S_ISLNK(sb.st_mode) || S_ISCHR(sb.st_mode)){
      return 2;
    } else {
      return 0;
    }
    return 0;
  }
  return 0;
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
  // char sourceDevId[256];
  // char destDevId[256];
  char *destPath;
  struct stat sourcebuffer;
  struct stat destbuffer;
  struct utimbuf touchDate;
  int e;

  destPath = dirFromPath(dest);

  // mvprintw(0,66,"Stripped: %s", dirFromPath(dest)); // test in the usual place

  if (check_dir(destPath)){
    lstat(source, &sourcebuffer);
    lstat(destPath, &destbuffer);

    // sprintf(sourceDevId, "%d", sourcebuffer.st_dev);
    // sprintf(destDevId, "%d", destbuffer.st_dev);

    // if (!strcmp(sourceDevId, destDevId)) {
    if (sourcebuffer.st_dev == destbuffer.st_dev) {
      // Destination is on the same filesystem.
      //mvprintw(0,66,"PASS: %s:%s %s", sourceDevId, destDevId, dest); // test pass
      e = rename(source, dest);
      free(destPath);
      return e;
    } else {
      // Destination is NOT in the same filesystem, the file will need copying then deleting.
      //mvprintw(0,66,"FAIL: %s:%s", sourceDevId, destDevId); // test fail
      if (moveBetweenDevices){
        // To Do
        copy_file(source, dest, sourcebuffer.st_mode);
        touchDate.actime = sourcebuffer.st_atime;
        touchDate.modtime = sourcebuffer.st_mtime;
        e = utime(dest, &touchDate);
        if (e == 0){
          e = chown(dest, sourcebuffer.st_uid, sourcebuffer.st_gid);
        }
        if (e == 0){
          e = remove(source);
        }
        return e;
      } else {
        topLineMessage("Error: Unable to move file between mount points");
        free(destPath);
        return 1;
      }
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
        result++;
      }
    }
  return(result);
}

void set_history(char *pwd, char *objectWild, char *name, int lineStart, int selected)
{
  if (sessionhistory == 0){
    history *hs = malloc(sizeof(history));
  }

  if (historyref == sessionhistory) {
    hs = realloc(hs, (historyref +1) * sizeof(history));
    sessionhistory++;
  } else if (historyref < sessionhistory){
    free(hs[historyref].path);
    free(hs[historyref].name);
    free(hs[historyref].objectWild);
  }

  hs[historyref].path = malloc(sizeof(char) * (strlen(pwd) + 1));
  hs[historyref].name = malloc(sizeof(char) * (strlen(name) + 1));
  hs[historyref].objectWild = malloc(sizeof(char) * (strlen(objectWild) + 1));
  strcpy(hs[historyref].path, pwd);
  strcpy(hs[historyref].objectWild, objectWild);
  strcpy(hs[historyref].name, name);
  hs[historyref].lineStart = lineStart;
  hs[historyref].selected = selected;
  hs[historyref].visibleObjects = visibleObjects;
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
    while (( line = read_line(fin) )) {

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
      markedSizeString = malloc (sizeof (char) * (log10(markedSize) + 2));
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
  int i;
  size_t count = 0;
  size_t file_count = 0;
  size_t dirErrorSize = 0;
  struct dirent *res;
  struct stat sb;
  char *path = pwd;
  struct stat buffer;
  int         status;
  int         pass = 0;
  int axFlag = 0;
  char *dirError = malloc(sizeof(char) + 1);
  // char direrror[1024];
  // char filename[256];
  acl_t acl;
  acl_entry_t dummy;
  int haveAcl;
  ssize_t xattr;
  char *xattrs;
  unsigned char *uXattrs;
  int seLinuxCon = 0;
  int xattrsNum;
  #ifdef HAVE_SELINUX_SELINUX_H
    security_context_t context;
  #endif
  char *contextText;
  char *fullFilePath;

  results *ob = malloc(sizeof(results)); // Allocating a tiny amount of memory. We'll expand this on each file found.
  // xattrList *xa = malloc(sizeof(xattrList));
  // int xattrPos = 0;
  xa = malloc(sizeof(xattrList));
  xattrPos = 0;

  fetch:

  mmMode = 0;
  nameAndSLink = 0;
  time ( &currenttime );
  savailable = GetAvailableSpace(pwd);
  sused = 0; // Resetting used value
  axDisplay = 0;


  if (oneLine){
    segOrder[0] = COL_MARK;
    segOrder[1] = COL_NAME;
    for (i = 2; i < (sizeof(segOrder) / sizeof(segOrder[0])); i++){
      segOrder[i] = -1;
    }
  }

  //if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
  if (check_object(path) == 1){
  reload:
    folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        while ( ( res = readdir ( folder ) ) != NULL ){
          fullFilePath = calloc((strlen(pwd) + 2 + (strlen(res->d_name))), sizeof(char));
          sprintf(fullFilePath, "%s/%s", pwd, res->d_name);
          acl = NULL; // Resetting
          xattr = 0; // Resetting
          seLinuxCon = 0; //Resetting
          xattrsNum = 0;
          haveAcl = 0;
          contextText = malloc(sizeof(char) * 2);
          lstat(res->d_name, &sb);
          if ( showhidden == 0 && check_first_char(res->d_name, ".") && strcmp(res->d_name, ".") && strcmp(res->d_name, "..") ) {
            continue; // Skipping hidden files
          }
          if ( !showbackup && check_last_char(res->d_name, "~") ) {
            continue; // Skipping backup files
          }
          if ( dirOnly ) {
            if (!S_ISDIR(sb.st_mode)){
              continue;
            }
          }
          if ( strcmp(objectWild, "")){
            if (!wildcard(res->d_name, objectWild) && strcmp(res->d_name, ".") && strcmp(res->d_name, "..")){
              continue;
            }
          }
          // filename = res->d_name;
          if (pass == 0){
            count++;
          } else {
            if (count > totalfilecount){
              ob = realloc(ob, (count +1) * sizeof(results)); // Reallocating memory. Just in case a file is added between passes.
            }
            status = lstat(res->d_name, &buffer);

            sprintf(hlinkstr, "%d", buffer.st_nlink);
            sprintf(sizestr, "%lld", (long long)buffer.st_size);

            // axflag here

            // axDisplay = 1;
            // axFlag = 0;

            #ifdef HAVE_ACL_TYPE_EXTENDED
            // acl = acl_get_file(res->d_name, ACL_TYPE_EXTENDED);
              acl = acl_get_link_np(fullFilePath, ACL_TYPE_EXTENDED);
              if (acl && acl_get_entry(acl, ACL_FIRST_ENTRY, &dummy) == -1) {
                acl_free(acl);
                acl = NULL;
              }
              if (acl != NULL) {
                haveAcl = 1;
              }
              xattr = listxattr(res->d_name, NULL, 0, XATTR_NOFOLLOW);
              if (xattr < 0){
                xattr = 0;
              }
              xattrs = malloc(sizeof(char) * xattr);
              listxattr(res->d_name, xattrs, xattr, XATTR_NOFOLLOW);
              uXattrs = calloc(xattr, sizeof(unsigned char));
              memcpy(uXattrs, xattrs, xattr);
              // endwin();
              // printf("%s\n", uXattrs);
              // xa = realloc(xa, (count +1) * sizeof(xattrList));
              xattrPos = processXAttrs(&xa, res->d_name, uXattrs, xattr, xattrPos, &xattrsNum);
              free(uXattrs);
              // endwin();
              // printf("%s - %zu - %i\n", res->d_name, count, xattrPos);
            #else
              #ifdef HAVE_SYS_ACL_H
                xattrs = malloc(sizeof(char) * 1);
                strcpy(xattrs, "");
                #if HAVE_ACL_LIBACL_H
                  haveAcl = acl_extended_file(res->d_name);
                  if (haveAcl == -1){
                    haveAcl = 0;
                  }
               #endif
                // acl = acl_get_file(res->d_name, ACL_TYPE_ACCESS);
                // if (errno == ENOENT) {
                //   acl_free(acl);
                //   acl = NULL;
                // }
              #endif
              #ifdef HAVE_SELINUX_SELINUX_H
              seLinuxCon = lgetfilecon(res->d_name, &context);
              if (seLinuxCon > 0){
                contextText = realloc(contextText, sizeof(char) * seLinuxCon);
                strcpy(contextText, context);
              } else {
                seLinuxCon = 0;
              }
              freecon(context);
              #endif
            #endif

            if (haveAcl != 0 || seLinuxCon != 0 || xattr != 0){
              axDisplay = 1;
            }

            if (seLinuxCon == 0){
              sprintf(contextText, "?");
            }

            writeResultStruct(ob, res->d_name, buffer, count, haveAcl, xattr, seLinuxCon, contextText, xattrs, xattrsNum);

            acl_free(acl);

            free(contextText);
            free(xattrs);
            free(fullFilePath);

            sused = sused + buffer.st_size; // Adding the size values

            // Finding the longest name and symlink pair
            if ((strlen(ob[count].slink) + strlen(ob[count].name) + 4) > nameAndSLink){
              nameAndSLink = strlen(ob[count].slink) + strlen(ob[count].name) + 4;
            }

            count++;
          }
        }

        totalfilecount = count;

        closedir ( folder );

        if ( count == 0 ) {
          // This is a hacky mitigation for drivefs returning 0 objects, it should prevent the crash of #82. However, it doesn't fix the inablity to load Google Drive Stream directories properly. The GNU version of 'ls' also has similar issues, so the bug is most likely in the underlying library rather than DF-SHOW itself.
          dirErrorSize = 36;
          dirError = realloc(dirError, sizeof(char) * dirErrorSize);
          sprintf(dirError, "Error: Directory Returned 0 Objects" );
          topLineMessage(dirError);
          historyref--;
          if (historyref > 0){
            strcpy(path, hs[historyref - 1].path);
            chdir(path);
            dirAbort = 1;
            goto reload;
          } else {
            exitCode = 1;
            exittoshell();
          }
        }

        if (pass == 0){
          ob = realloc(ob, sizeof(results) * (totalfilecount) + 1);
          count = 0;
          pass = 1;
          goto reload;
        }

        // free(objectWild);

        // return ob;
      }else{
        // May want to use error no. here.
        dirErrorSize = 29;
        dirError = realloc(dirError, sizeof(char) * dirErrorSize);
        sprintf(dirError, "Could not open the directory" );
        topLineMessage(dirError);
        historyref--;
        // return ob;
      }
    }

  } else if (check_object(path) == 2){

    objectWild = objectFromPath(path);
    strcpy(currentpwd, dirFromPath(path));

    goto fetch;

  } else {
    dirErrorSize = snprintf(NULL, 0, "The location %s cannot be opened or is not a directory", path);
    dirError = realloc(dirError, sizeof(char) * (dirErrorSize + 1));
    sprintf(dirError, "The location %s cannot be opened or is not a directory", path);
    topLineMessage(dirError);
    historyref--;
    if (historyref > 0){
      strcpy(path, hs[historyref - 1].path);
      chdir(path);
      dirAbort = 1;
      goto reload;
    } else {
      exitCode = 1;
      exittoshell();
    }
  }
  hlinklen = seglength(ob, "hlink", count);
  ownerlen = seglength(ob, "owner", count);
  grouplen = seglength(ob, "group", count);
  authorlen = seglength(ob, "author", count);
  sizelen = seglength(ob, "size", count);
  sizeblockslen = seglength(ob, "sizeBlocks", count);
  majorlen = seglength(ob, "major", count);
  minorlen = seglength(ob, "minor", count);
  datelen = seglength(ob, "datedisplay", count);
  namelen = seglength(ob, "name", count);
  slinklen = seglength(ob, "slink", count);
  contextlen = seglength(ob, "contextText", count);

  // nameSegLen = namelen + slinklen + 4; // The 4 is the length of " -> "
  nameSegLen = seglength(ob, "nameSegBlock", count);

  xattrKeyLen = 0;

  for (i = 0; i < xattrPos; i++){
    if (xattrKeyLen < strlen(xa[i].xattr)){
      xattrKeyLen = strlen(xa[i].xattr);
    }
  }

  free(dirError);
  free(res);
  return ob;
}

results* reorder_ob(results* ob, char *order){
  //mvprintw(2,66,"%i",*ob[0].sys);
  int count = totalfilecount;

  skippable = 0;

  if ( !strcmp(order, "name")){
    qsort(ob, count, sizeof(results), cmp_dflist_name);
  }
  else if ( !strcmp(order, "date")){
    qsort(ob, count, sizeof(results), cmp_dflist_name);
    qsort(ob, count, sizeof(results), cmp_dflist_date);
  }
  else if ( !strcmp(order, "size")){
    qsort(ob, count, sizeof(results), cmp_dflist_name);
    qsort(ob, count, sizeof(results), cmp_dflist_size);
  }

  if (count > 2){
    if (!strcmp(ob[0].name, ".") && !strcmp(ob[1].name, "..")){
      skippable = 1;
    }
  }

  return ob;
}

void generateEntryLineIndex(results *ob){
  int i, n, t;
  acl_entry_t	entry = NULL;

  listLen = totalfilecount;

  if (showXAttrs) {
    listLen = totalfilecount + xattrPos;
  }

  el = calloc(listLen, sizeof(entryLines));
  n = 0;
  for (i = 0; i < totalfilecount; i++){
    entry = NULL;
    el[n].entryLineType = ET_OBJECT;
    el[n].subIndex = 0;
    el[n].fileRef = i;
    n++;
    if (showXAttrs){
      if (ob[i].xattrsNum > 0){
        for (t = 0; t < ob[i].xattrsNum; t++){
          el[n].entryLineType = ET_XATTR;
          el[n].subIndex = t;
          el[n].fileRef = i;
          n++;
        }
      }
    }
    // if (showAcls){
    //   if (ob[i].acl != NULL){
    //     entry = NULL;
    //     //endwin();
    //     //printf("%i - %i: %s\n", acl_get_entry(ob[i].acl, entry == NULL ? ACL_FIRST_ENTRY : ACL_NEXT_ENTRY, &entry), errno, strerror(errno));
    //     for (t = 0; acl_get_entry(ob[i].acl, entry == NULL ? ACL_FIRST_ENTRY : ACL_NEXT_ENTRY, &entry) == 0; t++) {
    //       listLen++;
    //       el = realloc(el, sizeof(entryLines) * listLen);
    //       el[n].entryLineType = ET_ACL;
    //       el[n].subIndex = t;
    //       el[n].fileRef = i;
    //       n++;
    //     }
    //   }
    // }
  }

  // // Test
  // endwin();
  // for (i = 0; i < listLen; i++){
  //   printf("I: %d, F: %d, T: %d, S: %d\n", i, el[i].fileRef, el[i].entryLineType, el[i].subIndex);
  // }
  // exit(4);
}

void adjustViewForSelected(int selected, entryLines* el, int listLen, int displaysize){
  int i;

  for (i = 0; i < listLen; i++){
    if ((el[i].fileRef == selected) && (el[i].entryLineType == ET_OBJECT)){
      if (listLen < displaysize) {
        lineStart = 0;
      } else if ((i + displaysize) > listLen){
        lineStart = listLen - displaysize;
      } else {
        lineStart = i;
      }
      topfileref = el[lineStart].fileRef;
      if ((lineStart + displaysize) > listLen){
        bottomFileRef = el[listLen - 1].fileRef;
      } else {
        bottomFileRef = el[(lineStart + displaysize) - 1].fileRef;
      }
      break;
    }
  }

}

int lineStartFromBottomFileRef(int fileRef, entryLines* el, int listLen, int displaySize){
  int i;
  int output = 0;

  for (i = 0; i < listLen; i++){
    if ((el[i].fileRef == fileRef) &&(el[i].entryLineType == ET_OBJECT)){
      if ((i - displaySize) > -1){
        output = i - displaySize + 1;
      } else {
        output = 0;
      }
      break;
    }
  }
  return output;
}

void display_dir(char *pwd, results* ob){

  int i, n, t;
  size_t list_count = 0;
  // int count = totalfilecount;
  int count = listLen;
  int printSelect = 0;
  //char sizeHeader[256], headings[256];
  char *sizeHeader = malloc(sizeof(char) + 1);
  char *headings = malloc(sizeof(char) + 1);
  size_t sizeHeaderLen;
  size_t headingsLen;
  int padIntHeadOG, padIntHeadContext, padIntHeadSize, padIntHeadDT;
  int headerpos, displaypos;
  char *susedString, *savailableString;
  wchar_t *pwdPrint = malloc(sizeof(wchar_t) + 1);
  size_t pwdPrintSize;
  char *markedInfoLine;
  // char *padCharHeadAttrs, *padCharHeadOG, *padCharHeadContext, *padCharHeadSize, *padCharHeadDT;
  char *headerCombined = malloc(sizeof(char) + 1);
  int headerCombinedLen = 1;
  char *markedHeadSeg, *attrHeadSeg, *hlinkHeadSeg, *ownerHeadSeg, *contextHeadSeg, *sizeHeadSeg, *dateHeadSeg, *nameHeadSeg, *sizeBlocksHeadSeg;
  int xattrOffset = 0;
  int origTopFileRef;
  int currentItem;

  maxdisplaywidth = 0;

  topfileref = el[lineStart].fileRef;

  if (markedinfo == 2 && (CheckMarked(ob) > 0)){
    automark = 1;
  } else {
    automark = 0;
  }

  if (markedinfo == 1 || automark == 1){
    displaysize = LINES - 6;
    displaystart = 5;
  } else{
    displaysize = LINES - 5;
    displaystart = 4;
    // TO DO: REFACTOR
    // if ((totalfilecount >= displaysize) && (lineStart + (displaysize ) > totalfilecount )){
    //   lineStart--;
    // }
  }

  // endwin();
  if ((lineStart + displaysize) > listLen){
    bottomFileRef = el[listLen - 1].fileRef;
  } else {
    bottomFileRef = el[(lineStart + displaysize) - 1].fileRef;
  }

  // Replacement to "sanitizeTopFileRef" - should be simpler with the lookup table.
  if ((selected > bottomFileRef) || ((selected < topfileref + 1)) || ((bottomFileRef - topfileref - 1) > displaysize)){
    adjustViewForSelected(selected, el, listLen, displaysize);
  }

  i = 0;

 rerunCalc:

  // Hacky, but this will stop show hanging if the topfileref check fails after 50 times. It's in place as if CTRL-C is disabled, then this lock can't be exited without killing show from another terminal.
  if (i > 50){
    exit(27);
  }

  lineCount = 0;
  // bottomFileRef = 0;
  visibleOffset = 0;

  // origTopFileRef = topfileref;

  // for(list_count = topfileref; list_count < totalfilecount; list_count++ ){
  //   lineCount++;

  //   if (showXAttrs){
  //     lineCount += ob[list_count].xattrsNum;
  //     // visibleOffset += ob[list_count].xattrsNum;
  //   }

  //   if (lineCount > displaysize){
  //     // lineCount--;
  //     break;
  //   }
  //   // bottomFileRef = list_count;
  // }

  // visibleObjects = bottomFileRef - topfileref + 1;

  // // Hacky, but it works.
  // if ((visibleObjects + visibleOffset) > displaysize){
  //   visibleOffset -= (visibleObjects + visibleOffset) - displaysize;
  // }

  if (displaysize > count){
    displaycount = count + 1;
  } else {
    displaycount = displaysize;
  }

  // displaycount = visibleObjects;

  if (displaycount < 0){
    displaycount = 0;
  }

  // topfileref = sanitizeTopFileRef(topfileref);
  // endwin();
  // printf("OTFR: %i, NTFR: %i\n", origTopFileRef, topfileref);

  // if (topfileref != origTopFileRef){
  //   i++;
  //   goto rerunCalc;
  // }

  // printf("\n");

  // selected = selected - topfileref;


  pwdPrintSize = (strlen(pwd) + strlen(objectWild) + 2);

  pwdPrint = realloc(pwdPrint, (sizeof(wchar_t) * pwdPrintSize));

  if (strcmp(objectWild, "")){
    swprintf(pwdPrint, pwdPrintSize, L"%s/%s", pwd, objectWild);
  } else {
    swprintf(pwdPrint, pwdPrintSize, L"%s", pwd);
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
      susedString = malloc (sizeof (char) * (log10(sused) + 2));
    }
    if (savailable == 0){
      savailableString = malloc (sizeof (char) * 1);
    } else {
      savailableString = malloc (sizeof (char) * (log10(savailable) + 2));
    }
    sprintf(susedString, "%lu", sused);
    sprintf(savailableString, "%ju", (uintmax_t) savailable);
  }

  strcpy(headAttrs, "---Attrs---");

  if (showContext){
    strcpy(headContext, "---Context---");
  } else {
    strcpy(headContext, "");
  }

  if (showSizeBlocks){
    strcpy(headSizeBlocks, "-Blocks-");
  } else {
    strcpy(headSizeBlocks, "");
  }

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

  // mvprintw(LINES-1, 0, "bottomFileRef: %i", bottomFileRef);
  // mvprintw(LINES-1, 0, "visibleObjects: %i", visibleObjects);
  // if (showXAttrs){
  //   if (display)
  // } else {
  // }

  if (el[lineStart].entryLineType != ET_OBJECT){
    topfileref++;
  }

  for(list_count = 0; list_count < displaycount + 1; list_count++ ){
    // endwin();
    // printf("%zu, %i, %i, - %i\n", list_count, lineStart, listLen, displaycount + 1);
    if ((list_count + lineStart) < listLen){
      // Setting highlight
      // if (list_count == selected) {
      if (el[(list_count + lineStart)].fileRef == selected) {
        printSelect = 1;
      } else {
        printSelect = 0;
      }

      // ownstart = hlinklen + 2;
      // hlinkstart = ownstart - 1 - *ob[list_count + lineStart].hlinklens;

      displaypos = 0 - hpos;

      currentItem = el[(list_count + lineStart)].fileRef;

      // endwin();
      // printf("LC: %i, TFR: %i, DC: %i\n", list_count, topfileref, displaycount);

      // printf("DP: %i, HL: %i, OL: %i, GL: %i, AL: %i, SL: %i, MaL: %i, MiL: %i, DL: %i, NL: %i, CL: %i, PS: %i, LC: %i, TF: %i, XO: %i\n", displaypos, hlinklen, ownerlen, grouplen, authorlen, sizelen, majorlen, minorlen, datelen, namelen, contextlen, printSelect, list_count, topfileref, xattrOffset);
      // mvprintw(list_count + 4, 0, "DP: %i, HL: %i, OL: %i, GL: %i, AL: %i, SL: %i, MaL: %i, MiL: %i, DL: %i, NL: %i, CL: %i, PS: %i, LC: %i, TF: %i, XO: %i", displaypos, hlinklen, ownerlen, grouplen, authorlen, sizelen, majorlen, minorlen, datelen, namelen, contextlen, printSelect, list_count, topfileref, xattrOffset);
      if (el[(list_count + lineStart)].entryLineType == ET_OBJECT){
        printEntry(displaypos, hlinklen, sizeblockslen, ownerlen, grouplen, authorlen, sizelen, majorlen, minorlen, datelen, namelen, contextlen, printSelect, list_count, currentItem, ob);
        // mvprintw(displaystart + list_count, 0, "%i - THIS SHOULDN'T BE HERE! - %s", printSelect, ob[el[(list_count + lineStart)].fileRef].name);
      } else if (el[(list_count + lineStart)].entryLineType == ET_ACL) {
        // Not implemented yet
      } else if (el[(list_count + lineStart)].entryLineType == ET_XATTR) {
        // Not implemented yet
        printXattr(displaypos, printSelect, list_count, currentItem, el[(list_count + lineStart)].subIndex, xa, ob);
      } else {
        mvprintw(displaystart + list_count, 0, "THIS SHOULDN'T BE HERE! - entryLineType: %d", el[(list_count + lineStart)].entryLineType);
      }

      if (charPos > maxdisplaywidth){
        maxdisplaywidth = charPos;
      }

      // if (showXAttrs && ob[list_count + topfileref].xattrsNum > 0){
      //   // displaysize = displaysize - ob[list_count + topfileref].xattrsNum;
      //   // displaycount = displaycount + ob[list_count + topfileref].xattrsNum;
      //   // list_count = list_count + ob[list_count + topfileref].xattrsNum;
      //   xattrOffset += ob[list_count + topfileref].xattrsNum;
      // }

      //list_count++;
    } else {
      break;
    }
  }


  // if (slinklen == 0){
  //   maxdisplaywidth = charPos + namelen;
  // } else {
  //   // maxdisplaywidth = entryMetaLen + namelen + slinklen + 4;
  //   maxdisplaywidth = charPos + nameAndSLink;
  // }

  //mvprintw(0, 66, "%d %d", historyref, sessionhistory);

  // the space between the largest owner and largest group should always end up being 1... in theory.
  // 2018-07-05: That assumption was solid, until we added a third element (Owner, Group, and Author)
  if (!ogavis){
    padIntHeadOG = 0;
  } else {
    if ( (ogalen + ogapad) > strlen(headOG)){
      padIntHeadOG = (ogalen + ogapad) - strlen(headOG) + 1;
    } else {
      padIntHeadOG = 1;
    }
  }

  if ( showContext ){
    if ( contextlen > strlen(headContext)){
      padIntHeadContext = ((contextlen - strlen(headContext)) + 1 );
    } else {
      padIntHeadContext = 1;
    }
  } else {
    padIntHeadContext = 0;
  }

  if ( sizelen > strlen(headSize)) {
    padIntHeadSize = sizelen - strlen(headSize);
  } else {
    padIntHeadSize = 0;
  }

  if ( datelen > strlen(headDT)) {
    padIntHeadDT = (datelen - strlen(headDT)) + 1;
  } else {
    padIntHeadDT = 1;
  }

  sizeHeaderLen = snprintf(NULL, 0, "%i Objects   %s Used %s Available", totalfilecount, susedString, savailableString);

  sizeHeader = realloc(sizeHeader, sizeof(char) * (sizeHeaderLen + 1));

  sprintf(sizeHeader, "%i Objects   %s Used %s Available", totalfilecount, susedString, savailableString);

  // padCharHeadAttrs = genPadding(hlinklen + 1 + axDisplay);
  // padCharHeadOG = genPadding(padIntHeadOG);
  // padCharHeadContext = genPadding(padIntHeadContext);
  // padCharHeadSize = genPadding(padIntHeadSize);
  // padCharHeadDT = genPadding(padIntHeadDT);

  // headingsLen = snprintf(NULL, 0, "%s%s%s%s%s%s%s%s%s%s%s%s", headAttrs, padCharHeadAttrs, headOG, padCharHeadOG, headContext, padCharHeadContext, padCharHeadSize, headSize, " ", headDT, padCharHeadDT, headName);

  // headings = realloc(headings, sizeof(char) * (headingsLen + 1));

  // sprintf(headings, "%s%s%s%s%s%s%s%s%s%s%s%s", headAttrs, padCharHeadAttrs, headOG, padCharHeadOG, headContext, padCharHeadContext, padCharHeadSize, headSize, " ", headDT, padCharHeadDT, headName);

  markedHeadSeg = writeSegment(3, "", RIGHT);
  attrHeadSeg = writeSegment(attrSegmentLen, headAttrs, LEFT);
  hlinkHeadSeg = writeSegment(hlinkSegmentLen, "", RIGHT);
  ownerHeadSeg = writeSegment(ownerSegmentLen, headOG, LEFT);
  contextHeadSeg = writeSegment(contextSegmentLen, headContext, LEFT);
  sizeHeadSeg = writeSegment(sizeSegmentLen, headSize, RIGHT);
  dateHeadSeg = writeSegment(dateSegmentLen, headDT, LEFT);
  sizeBlocksHeadSeg = writeSegment(sizeBlocksSegmentLen, headSizeBlocks, RIGHT);
  nameHeadSeg = writeSegment(nameSegmentDataLen, headName, LEFT);

  sprintf(headerCombined, "");
  for ( n = 0; n < (sizeof(segOrder) / sizeof(segOrder[0])); n++){
    t = segOrder[n];
    switch(t){
    case COL_MARK:
      headerCombinedLen = (headerCombinedLen + strlen(markedHeadSeg));
      headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
      strcat(headerCombined, markedHeadSeg);
      break;
    case COL_SIZEBLOCKS:
      if (showSizeBlocks){
        headerCombinedLen = (headerCombinedLen + strlen(sizeBlocksHeadSeg));
        headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
        strcat(headerCombined, sizeBlocksHeadSeg);
      }
      break;
    case COL_ATTR:
      headerCombinedLen = (headerCombinedLen + strlen(attrHeadSeg));
      headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
      strcat(headerCombined, attrHeadSeg);
      break;
    case COL_HLINK:
      headerCombinedLen = (headerCombinedLen + strlen(hlinkHeadSeg));
      headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
      strcat(headerCombined, hlinkHeadSeg);
      break;
    case COL_OWNER:
      if (ogavis){
        headerCombinedLen = (headerCombinedLen + strlen(ownerHeadSeg));
        headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
        strcat(headerCombined, ownerHeadSeg);
      }
      break;
    case COL_CONTEXT:
      if (showContext){
        headerCombinedLen = (headerCombinedLen + strlen(contextHeadSeg));
        headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
        strcat(headerCombined, contextHeadSeg);
      }
      break;
    case COL_SIZE:
      headerCombinedLen = (headerCombinedLen + strlen(sizeHeadSeg));
      headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
      strcat(headerCombined, sizeHeadSeg);
      break;
    case COL_DATE:
      headerCombinedLen = (headerCombinedLen + strlen(dateHeadSeg));
      headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
      strcat(headerCombined, dateHeadSeg);
      break;
    case COL_NAME:
      headerCombinedLen = (headerCombinedLen + strlen(nameHeadSeg));
      headerCombined = realloc(headerCombined, sizeof(char) * headerCombinedLen);
      strcat(headerCombined, nameHeadSeg);
      break;
    default:
      break;
    }
  }

  // Freeing Segments
  free(markedHeadSeg);
  free(attrHeadSeg);
  free(hlinkHeadSeg);
  free(ownerHeadSeg);
  free(contextHeadSeg);
  free(sizeHeadSeg);
  free(dateHeadSeg);
  free(nameHeadSeg);
  free(sizeBlocksHeadSeg);

  // free(padCharHeadAttrs);
  // free(padCharHeadOG);
  // free(padCharHeadContext);
  // free(padCharHeadSize);
  // free(padCharHeadDT);

  if ( danger ) {
    setColors(DANGER_PAIR);
  } else {
    setColors(INFO_PAIR);
  }
  // attroff(A_BOLD); // Required to ensure the last selected item doesn't bold the header
  wPrintLine(1, 2, pwdPrint);

  free(pwdPrint);

  printLine(2, 2, sizeHeader);

  if (markedinfo == 1 || (markedinfo == 2 && (CheckMarked(ob) > 0))){
    markedInfoLine = markedDisplay(ob);
    printLine (3, 4, markedInfoLine);
    free(markedInfoLine);
  }

  if ( danger ) {
    setColors(DANGER_PAIR);
  } else {
    setColors(HEADING_PAIR);
  }

  // headerpos = 4 - hpos;

  if (markedinfo == 1 || (markedinfo == 2 && (CheckMarked(ob) > 0))){
    // printLine (4, headerpos, headings);
    printLine (4, (0 - hpos), headerCombined);
  } else {
    // printLine (3, headerpos, headings);
    printLine (3, (0 - hpos), headerCombined);
  }
  setColors(COMMAND_PAIR);
  free(susedString);
  free(savailableString);
  free(sizeHeader);
  free(headings);
  free(headerCombined);
}

void resizeDisplayDir(results* ob){
  displaysize = (LINES - 5);
  if ( (selected - lineStart) > (LINES - 6 )) {
    lineStart = selected - (LINES - 6);
  } else if ( lineStart + (LINES - 6) > totalfilecount ) {
    if (totalfilecount < (LINES - 6)){
      lineStart = 0;
    } else {
      lineStart = totalfilecount - (LINES - 5);
    }
  }
  refreshDirectory(sortmode, lineStart, selected, -1);
  // display_dir(currentpwd, ob);
}

