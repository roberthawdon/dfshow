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

int list_dir(char *pwd)
{
  size_t count = 0;
  struct dirent *res;
  struct stat sb;
  struct group *gr;
  struct passwd *pw;
  const char *path = pwd;
  struct stat buffer;
  int         status;
  char filedatetime[17];
  char perms[11] = {0};

  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    DIR *folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        while ( ( res = readdir ( folder ) ) ){
          //if ( strcmp( res->d_name, "." ) && strcmp( res->d_name, ".." ) ){
          lstat(res->d_name, &sb);
          struct passwd *pw = getpwuid(sb.st_uid);
          struct group *gr = getgrgid(sb.st_gid);
          status = lstat(res->d_name, &buffer);
          strftime(filedatetime, 20, "%Y-%m-%d %H:%M", localtime(&(buffer.st_ctime)));


          //perms[0] = '-'; //placeholder
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


          // grp = getgrgid(res->d_ino);
          mvprintw(4 + count, 4,"%s %i  %s %s      %i  %s  %s\n",perms,buffer.st_nlink,pw->pw_name,gr->gr_name,buffer.st_size,filedatetime,res->d_name); 
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
