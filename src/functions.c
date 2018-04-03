#include <stdio.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"
#include "views.h"

int list_dir(char *pwd)
{
  size_t count = 0;
  struct dirent *res;
  struct stat sb;
  struct group *gr;
  struct passwd *pw;
  const char *path = pwd;

  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    DIR *folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        while ( ( res = readdir ( folder ) ) ){
          //if ( strcmp( res->d_name, "." ) && strcmp( res->d_name, ".." ) ){
          stat(res->d_name, &sb);
          struct passwd *pw = getpwuid(sb.st_uid);
          struct group *gr = getgrgid(sb.st_gid);
          // grp = getgrgid(res->d_ino);
          mvprintw(4 + count, 4,"---------- 0  %s %s      00000  0000-00-00 00:00  %s\n",pw->pw_name,gr->gr_name,res->d_name); // A lot of placeholders here.
          count++;
            //}
        }

        attron(COLOR_PAIR(2));
        mvprintw(1, 2, "%s", pwd);
        mvprintw(2, 2, "%i Objects   00000 Used 00000000 Available", count); // Parcial Placeholder for PWD info
        mvprintw(3, 4, "----Attrs---- -Owner & Group-  -Size- ---Date & Time--- ----Name----"); // Header
        attron(COLOR_PAIR(1));

        closedir ( folder );
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
