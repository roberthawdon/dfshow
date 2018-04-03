#include <stdio.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
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
  const char *path = pwd;

  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    DIR *folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        while ( ( res = readdir ( folder ) ) ){
          if ( strcmp( res->d_name, "." ) && strcmp( res->d_name, ".." ) ){
            mvprintw(5 + count, 56,"%s\n",res->d_name);
            count++;
          }
        }

        attron(COLOR_PAIR(2));
        mvprintw(2, 2, "%s", pwd);
        mvprintw(3, 2, "%i Objects   00000 Used 00000000 Available", count); // Parcial Placeholder for PWD info
        mvprintw(4, 4, "---Attrs--- -Owner & Group-  -Size- --Date & Time-- ----Name----"); // Header
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
