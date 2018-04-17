#include <stdio.h>
#include <ncurses.h>
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
#include "functions.h"
#include "views.h"

char hlinkstr[5], sizestr[32];

int hlinklen;
int ownerlen;
int grouplen;
int sizelen;
int namelen;

int hlinkstart;
int ownstart;
int groupstart;
int sizestart;
int sizeobjectstart;
int datestart;
int namestart;

int totalfilecount;

int selected;
int topfileref = 0;

int seglength(const void *seg, char *segname, int LEN)
{

  size_t longest, len;

  results *dfseg = (results *)seg;

  if (!strcmp(segname, "owner")) {
    longest = strlen(dfseg[0].owner);
  }
  else if (!strcmp(segname, "group")) {
    longest = strlen(dfseg[0].group);
  }
  else if (!strcmp(segname, "hlink")) {
    sprintf(hlinkstr, "%d", *dfseg[0].hlink);
    longest = strlen(hlinkstr);
  }
  else if (!strcmp(segname, "size")) {
    sprintf(sizestr, "%d", *dfseg[0].size);
    longest = strlen(sizestr);
  }
  else if (!strcmp(segname, "name")) {
    longest = strlen(dfseg[0].name);
  }
  else {
    longest = 0;
  }

  size_t j = 0;

  for(size_t i = 1; i < LEN; i++)
    {
      if (!strcmp(segname, "owner")) {
        len = strlen(dfseg[i].owner);
      }
      else if (!strcmp(segname, "group")) {
        len = strlen(dfseg[i].group);
      }
      else if (!strcmp(segname, "hlink")) {
        sprintf(hlinkstr, "%d", *dfseg[i].hlink);
        len = strlen(hlinkstr);
      }
      else if (!strcmp(segname, "size")) {
        sprintf(sizestr, "%d", *dfseg[i].size);
        len = strlen(sizestr);
      }
      else if (!strcmp(segname, "name")) {
        len = strlen(dfseg[i].name);
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

  //struct dfobject *lhs;
  //struct dfobject *rhs;

  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  return strcmp(dforderA->name, dforderB->name);

  //return (dforderA->size - dforderB->size);

}

int cmp_dflist_date(const void *lhs, const void *rhs)
{
  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  return strcmp(dforderA->date, dforderB->date);

}

int cmp_dflist_size(const void *lhs, const void *rhs)
{
  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  return (*dforderA->size - *dforderB->size);

}

int check_dir(char *pwd)
{
  const char *path = pwd;
  struct stat sb;
  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    if (access ( path, F_OK ) != -1 ){
      return 1;
    } else {
      return 0;
    }
  } else {
  return 0;
  }
}

results* get_dir(char *pwd)
{
  size_t count = 0;
  size_t file_count = 0;
  struct dirent *res;
  struct stat sb;
  struct group *gr;
  struct passwd *pw;
  const char *path = pwd;
  struct stat buffer;
  int         status;
  char filedatetime[17];
  char perms[11] = {0};
  // char result[11][4][128][128][32][17][512];

  //  struct dfobject {
  //    char perm[11];
  //    int hlink[4];
  //    char owner[128];
  //    char group[128];
  //    int size[32];
  //    char date[17];
  //    char name[512];
  //  };

  results *ob = malloc(sizeof(results)); // Allocating a tiny amount of memory. We'll expand this on each file found.

  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    DIR *folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        //while (( res = readdir( folder )) ) {
        //    file_count++;
        //}
        while ( ( res = readdir ( folder ) ) ){
          ob = realloc(ob, (count +1) * sizeof(results)); // Reallocating memory.
          //if ( strcmp( res->d_name, "." ) && strcmp( res->d_name, ".." ) ){
          lstat(res->d_name, &sb);
          struct passwd *pw = getpwuid(sb.st_uid);
          struct group *gr = getgrgid(sb.st_gid);
          status = lstat(res->d_name, &buffer);
          strftime(filedatetime, 20, "%Y-%m-%d %H:%M", localtime(&(buffer.st_mtime)));


          if ( buffer.st_mode & S_IFDIR ) {
            perms[0] = 'd';
          } else if ( S_ISLNK(buffer.st_mode) ) {
            perms[0] = 'l';
          } else {
            perms[0] = '-';
          }
          perms[1] = buffer.st_mode & S_IRUSR? 'r': '-';
          perms[2] = buffer.st_mode & S_IWUSR? 'w': '-';
          perms[3] = buffer.st_mode & S_IXUSR? 'x': '-';
          perms[4] = buffer.st_mode & S_IRGRP? 'r': '-';
          perms[5] = buffer.st_mode & S_IWGRP? 'w': '-';
          perms[6] = buffer.st_mode & S_IXGRP? 'x': '-';
          perms[7] = buffer.st_mode & S_IROTH? 'r': '-';
          perms[8] = buffer.st_mode & S_IWOTH? 'w': '-';
          perms[9] = buffer.st_mode & S_IXOTH? 'x': '-';

          sprintf(hlinkstr, "%d", buffer.st_nlink);
          sprintf(sizestr, "%lld", buffer.st_size);

          // Writing our structure
          strcpy(ob[count].perm, perms);
          *ob[count].hlink = buffer.st_nlink;
          *ob[count].hlinklens = strlen(hlinkstr);
          strcpy(ob[count].owner, pw->pw_name);
          strcpy(ob[count].group, gr->gr_name);
          *ob[count].size = buffer.st_size;
          *ob[count].sizelens = strlen(sizestr);
          strcpy(ob[count].date, filedatetime);
          strcpy(ob[count].name, res->d_name);

          // grp = getgrgid(res->d_ino);
          //mvprintw(4 + count, 4,"%s %i  %s %s      %i  %s  %s\n",ob[count].perm,buffer.st_nlink,pw->pw_name,ob[count].group,buffer.st_size,ob[count].date,ob[count].name);
          // mvprintw(4 + count, 4,"%s",ob[count].perm);
          // mvprintw(4 + count, 15,"%i",*ob[count].hlink);
          // mvprintw(4 + count, 18,"%s",ob[count].owner);
          // mvprintw(4 + count, 22,"%s",ob[count].group);
          // mvprintw(4 + count, 35,"%i",*ob[count].size);
          // mvprintw(4 + count, 42,"%s",ob[count].date);
          // mvprintw(4 + count, 60,"%s",ob[count].name);

          count++;
            //}
        }


        totalfilecount = count;
        closedir ( folder );
        return ob;
        //free(*ob); // Freeing memory
      }else{
        perror ( "Could not open the directory" );
        return ob;
      }
    }

  }else{
    printf("The %s it cannot be opened or is not a directory\n", path);
    return ob;
  }
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

  int displaysize = ((LINES -1) - 4); // Calculate area to print
  size_t list_count = 0;
  int count = totalfilecount;

  if (displaysize > count){
    displaysize = count;
  }

  //for(list_count = 0; list_count < count; ){
  for(list_count = 0; list_count < displaysize; ){
    // Setting highlight
    if (list_count == selected) {
      attron(A_BOLD);
      attron(COLOR_PAIR(4));
    } else {
      attroff(A_BOLD);
      attron(COLOR_PAIR(1));
    }
    // //TEMP Emulate listed item
    // if (list_count == 4) {
    //   attron(A_BOLD);
    //   attron(COLOR_PAIR(4));
    // }

    hlinklen = seglength(ob, "hlink", count);
    ownerlen = seglength(ob, "owner", count);
    grouplen = seglength(ob, "group", count);
    sizelen = seglength(ob, "size", count);
    namelen = seglength(ob, "name", count);

    ownstart = 15 + hlinklen + 1;
    groupstart = ownstart + ownerlen + 1;
    if (ownerlen + 1 + grouplen < 16) {
      sizestart = ownstart + 16;
    } else {
      sizestart = groupstart + grouplen + 1;
    }
    if (sizelen < 7) {
      datestart = sizestart + 7;
    } else {
      datestart = sizestart + sizelen + 1;
    }
    sizeobjectstart = datestart - 1 - *ob[list_count].sizelens;
    namestart = datestart + 18;
    hlinkstart = ownstart - 1 - *ob[list_count].hlinklens;

    mvprintw(4 + list_count, 4,"%s",ob[list_count].perm);
    mvprintw(4 + list_count, hlinkstart,"%i",*ob[list_count].hlink);
    mvprintw(4 + list_count, ownstart,"%s",ob[list_count].owner);
    mvprintw(4 + list_count, groupstart,"%s",ob[list_count].group);
    mvprintw(4 + list_count, sizeobjectstart,"%lli",*ob[list_count].size);
    mvprintw(4 + list_count, datestart,"%s",ob[list_count].date);
    mvprintw(4 + list_count, namestart,"%s",ob[list_count].name);
    // //TEMP Emulate listed item
    // if (list_count == 4) {
    //   attron(COLOR_PAIR(1));
    //   attroff(A_BOLD);
    // }
    list_count++;
    }

  // mvprintw(4 + count + 2, 4,"Hlink: %i",hlinklen);
  // mvprintw(4 + count + 3, 4,"Owner: %i",ownerlen);
  // mvprintw(4 + count + 4, 4,"Group: %i",grouplen);
  // mvprintw(4 + count + 5, 4,"Size:  %i",sizelen);
  // mvprintw(4 + count + 6, 4,"Name:  %i",namelen);

  attron(COLOR_PAIR(2));
  attroff(A_BOLD); // Required to ensure the last selected item doesn't bold the header
  mvprintw(1, 2, "%s", pwd);
  mvprintw(2, 2, "%i Objects   00000 Used 00000000 Available", count); // Parcial Placeholder for PWD info
  mvprintw(3, 4, "----Attrs----");
  mvprintw(3, ownstart, "-Owner & Group-");
  mvprintw(3, datestart - 7, "-Size-");
  mvprintw(3, datestart, "---Date & Time---");
  mvprintw(3, namestart, "----Name----");
  //mvprintw(3, 4, "----Attrs---- -Owner & Group-  -Size- ---Date & Time--- ----Name----"); // Header
  attron(COLOR_PAIR(1));
}
