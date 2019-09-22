#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "colors.h"
#include "typedefs.h"
#include "common.h"

// It turns out most systems don't have an ST_AUTHOR, so for those systems, we set the author as the owner. Yup, `ls` does this too.
#if ! HAVE_STRUCT_STAT_ST_AUTHOR
# define st_author st_uid
#endif

int typecolor;
int sexec;
int mmMode = 0;
int markall = 0;

char hlinkstr[6], sizestr[32], majorstr[6], minorstr[6];

char timestyle[9] = "locale";

int writePermsEntry(char * perms, mode_t mode){

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

  return typecolor;

}

void writeResultStruct(results* ob, const char * filename, struct stat buffer, int count){
  char perms[11] = {0};
  struct group *gr;
  struct passwd *pw;
  struct passwd *au;
  char *filedate;
  ssize_t cslinklen = 0, datedisplayLen = 0;

  writePermsEntry(perms, buffer.st_mode);

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

  ob[count].color = typecolor;

  free(filedate);

}
