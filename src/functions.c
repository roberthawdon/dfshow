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
#include <sys/vfs.h>
#include "functions.h"
#include "views.h"

typedef struct {
  char perm[11];
  int hlink[4];
  char owner[128];
  char group[128];
  int size[32];
  char date[17];
  char name[512];
} results;

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

int cmp_dflist(const void *lhs, const void *rhs)
{

  //struct dfobject *lhs;
  //struct dfobject *rhs;

  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  return strcmp(dforderA->name, dforderB->name);

  //return (dforderA->size - dforderB->size);

}

int list_dir(char *pwd)
{
  size_t count = 0;
  size_t list_count = 0;
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

  results *ob = malloc(1024 * sizeof(results)); // Needs to be dynamic

  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    DIR *folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        //while (( res = readdir( folder )) ) {
        //    file_count++;
        //}
        while ( ( res = readdir ( folder ) ) ){
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

          // Writing our structure
          strcpy(ob[count].perm, perms);
          *ob[count].hlink = buffer.st_nlink;
          strcpy(ob[count].owner, pw->pw_name);
          strcpy(ob[count].group, gr->gr_name);
          *ob[count].size = buffer.st_size;
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

        qsort(ob, count, sizeof(results), cmp_dflist);

        for(list_count; list_count < count; ){
          //TEMP Emulate listed item
          if (list_count == 4) {
            attron(A_BOLD);
            attron(COLOR_PAIR(4));
          }
          mvprintw(4 + list_count, 4,"%s",ob[list_count].perm);
          mvprintw(4 + list_count, 15,"%i",*ob[list_count].hlink);
          mvprintw(4 + list_count, 18,"%s",ob[list_count].owner);
          mvprintw(4 + list_count, 22,"%s",ob[list_count].group);
          mvprintw(4 + list_count, 35,"%i",*ob[list_count].size);
          mvprintw(4 + list_count, 42,"%s",ob[list_count].date);
          mvprintw(4 + list_count, 60,"%s",ob[list_count].name);
          //TEMP Emulate listed item
          if (list_count == 4) {
            attron(COLOR_PAIR(1));
            attroff(A_BOLD);
          }
          list_count++;
         }


        //mvprintw(4 + count + 2, 4,"Test");

        attron(COLOR_PAIR(2));
        mvprintw(1, 2, "%s", pwd);
        mvprintw(2, 2, "%i Objects   00000 Used 00000000 Available", count); // Parcial Placeholder for PWD info
        mvprintw(3, 4, "----Attrs---- -Owner & Group-  -Size- ---Date & Time--- ----Name----"); // Header
        attron(COLOR_PAIR(1));

        closedir ( folder );
        free(ob); // Freeing memory
      }else{
        perror ( "Could not open the directory" );
        return 1;
      }
    }

  }else{
    printf("The %s it cannot be opened or is not a directory\n", path);
    return 1;
  }
}
