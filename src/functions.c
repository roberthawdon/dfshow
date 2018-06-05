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
#include "functions.h"
#include "views.h"
#include "menus.h"

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
int displaysize; // Calculate area to print
int historyref = 0;
int sessionhistory = 0;

int showhidden = 0;

int markall = 0;

unsigned long int savailable = 0;
unsigned long int sused = 0;

history *hs;

void readline(char *buffer, int buflen, char *oldbuf)
/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
{
  int old_curs = curs_set(1);
  int pos;
  int len;
  int oldlen;
  int x, y;

  oldlen = strlen(oldbuf);
  attron(COLOR_PAIR(3));

  pos = oldlen;
  len = oldlen;

  getyx(stdscr, y, x);

  strcpy(buffer, oldbuf);

  for (;;) {
    int c;

    buffer[len] = ' ';
    mvaddnstr(y, x, buffer, len+1); // Prints buffer on screen
    move(y, x+pos); //
    c = getch();

    if (c == KEY_ENTER || c == '\n' || c == '\r') {
      attron(COLOR_PAIR(1));
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
    } else {
      beep();
    }
  }
  buffer[len] = '\0';
  if (old_curs != ERR) curs_set(old_curs);
}

char * dirFromPath (const char* myStr){

  char *outStr = (char *) malloc(sizeof(myStr));

  strcpy(outStr, myStr);

  char *del = &outStr[strlen(outStr)];

  while (del > outStr && *del != '/')
    del--;

  if (*del== '/')
    *del= '\0';

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
  char ch;
  FILE *source, *target;

  source = fopen(source_input, "r");


  if ( check_dir(target_input) ){
    if ( !check_last_char(target_input, "/")){
      strcat(target_input, "/");
    }
    strcat(target_input, basename(source_input));
  }
  target = fopen(target_input, "w");

  while( ( ch = fgetc(source) ) != EOF )
    fputc(ch, target);

  fclose(source);
  fclose(target);
}

void delete_file(char *source_input)
{
  remove(source_input);
}

void SendToPager(const char* object)
{
  char page[1024];
  char esc[1024];
  int pset = 0;
  if ( getenv("PAGER")) {
    strcpy(page, getenv("PAGER"));
    pset = 1;
  }
  if ( pset ) {
    strcat(page, " ");
    strcpy(esc, "'");
    strcat(esc, object);
    strcat(esc, "'");
    strcat(page, esc);
    clear();
    endwin();
    system(page);
    initscr();
  } else {
    topLineMessage("Error: No pager set.");
  }
}

void SendToEditor(const char* object)

{
  char editor[1024];
  char esc[1024];
  int eset = 0;
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
    strcat(esc, object);
    strcat(esc, "'");
    strcat(editor, esc);
    clear();
    endwin();
    system(editor);
    initscr();
  } else {
    topLineMessage("Error: No editor set.");
  }
}

long GetAvailableSpace(const char* path)
{
  struct statvfs stat;

  if (statvfs(path, &stat) != 0) {
    // error happens, just quits here
    return -1;
  }

  // the available size is f_bsize * f_bavail
  return stat.f_bsize * stat.f_bavail;
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

  results *dforderA = (results *)lhs;
  results *dforderB = (results *)rhs;

  return strcmp(dforderA->name, dforderB->name);

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
    DIR *folder = opendir ( path );
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

int RenameObject(const char* source, const char* dest)
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
      return e;
    } else {
      // Destination is NOT in the same filesystem, the file will need copying then deleting.
      //mvprintw(0,66,"FAIL: %s:%s", sourceDevId, destDevId); // test fail
    }
  } else {
    // Destination directory not found
    //mvprintw(0,66, "FAIL: NO DIR"); // test
  }
  free(destPath);
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

void set_history(char *pwd, int topfileref, int selected)
{
  if (sessionhistory == 0){
    history *hs = malloc(sizeof(history));
  }

  if (historyref == sessionhistory) {
    hs = realloc(hs, (historyref +1) * sizeof(history));
    sessionhistory++;
  }

  strcpy(hs[historyref].path, pwd);
  hs[historyref].topfileref = topfileref;
  hs[historyref].selected = selected;
  historyref++;


  //mvprintw(0, 66, "%s", hs[historyref -1].path);

}

results* get_dir(char *pwd)
{
  savailable = GetAvailableSpace(pwd);
  sused = 0; // Resetting used value
  //sused = GetUsedSpace(pwd); // Original DF-EDIT added the sizes to show what was used in that directory, rather than the whole disk.
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

  results *ob = malloc(sizeof(results)); // Allocating a tiny amount of memory. We'll expand this on each file found.

  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)){
    DIR *folder = opendir ( path );

    if (access ( path, F_OK ) != -1 ){
      if ( folder ){
        while ( ( res = readdir ( folder ) ) ){
          if ( showhidden == 0 && check_first_char(res->d_name, ".") && strcmp(res->d_name, ".") && strcmp(res->d_name, "..") ) {
            continue; // Skipping hidden files
          }
          ob = realloc(ob, (count +1) * sizeof(results)); // Reallocating memory.
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
          sprintf(sizestr, "%lu", buffer.st_size);

          // Writing our structure
          if ( markall && !(buffer.st_mode & S_IFDIR) ) {
            *ob[count].marked = 1;
          } else {
            *ob[count].marked = 0;
          }
          strcpy(ob[count].perm, perms);
          *ob[count].hlink = buffer.st_nlink;
          *ob[count].hlinklens = strlen(hlinkstr);
          strcpy(ob[count].owner, pw->pw_name);
          strcpy(ob[count].group, gr->gr_name);
          *ob[count].size = buffer.st_size;
          *ob[count].sizelens = strlen(sizestr);
          strcpy(ob[count].date, filedatetime);
          strcpy(ob[count].name, res->d_name);

          sused = sused + buffer.st_size; // Adding the size values

          count++;
        }


        totalfilecount = count;
        closedir ( folder );

        return ob;
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

  displaysize = LINES - 5;
  size_t list_count = 0;
  int count = totalfilecount;
  selected = selected - topfileref;

  if (displaysize > count){
    displaysize = count;
  }

  for(list_count = 0; list_count < displaysize; ){
    // Setting highlight
    if (list_count == selected) {
      attron(A_BOLD);
      attron(COLOR_PAIR(4));
    } else {
      attroff(A_BOLD);
      attron(COLOR_PAIR(1));
    }

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
    sizeobjectstart = datestart - 1 - *ob[list_count + topfileref].sizelens;
    namestart = datestart + 18;
    hlinkstart = ownstart - 1 - *ob[list_count + topfileref].hlinklens;

    if ( *ob[list_count + topfileref].marked ){
      mvprintw(4 + list_count, 2, "*");
    }
    mvprintw(4 + list_count, 4,"%s",ob[list_count + topfileref].perm);
    mvprintw(4 + list_count, hlinkstart,"%i",*ob[list_count + topfileref].hlink);
    mvprintw(4 + list_count, ownstart,"%s",ob[list_count + topfileref].owner);
    mvprintw(4 + list_count, groupstart,"%s",ob[list_count + topfileref].group);
    mvprintw(4 + list_count, sizeobjectstart,"%lu",*ob[list_count + topfileref].size);
    mvprintw(4 + list_count, datestart,"%s",ob[list_count + topfileref].date);
    mvprintw(4 + list_count, namestart,"%s",ob[list_count + topfileref].name);
    list_count++;
    }

  //mvprintw(0, 66, "%d %d", historyref, sessionhistory);
  attron(COLOR_PAIR(2));
  attroff(A_BOLD); // Required to ensure the last selected item doesn't bold the header
  mvprintw(1, 2, "%s", pwd);
  mvprintw(2, 2, "%i Objects   %lu Used %lu Available", count, sused, savailable);// Parcial Placeholder for PWD info
  mvprintw(3, 4, "---Attrs---");
  mvprintw(3, ownstart, "-Owner & Group-");
  mvprintw(3, datestart - 7, "-Size-");
  mvprintw(3, datestart, "---Date & Time---");
  mvprintw(3, namestart, "----Name----");
  attron(COLOR_PAIR(1));
}
