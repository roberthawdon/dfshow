#include <limits.h>

#define LINE_WRAP 0
#define WORD_WRAP 1

enum
  {
   GETOPT_HELP_CHAR = (CHAR_MIN - 2),
   GETOPT_VERSION_CHAR = (CHAR_MIN - 3),
   GETOPT_THEME_CHAR = (CHAR_MIN - 4)
  };

void file_view(char * currentfile);
void fileShowStatus(const char * currentfile, int top);
void displayFile(const char * currentfile, int top);
void sigwinchHandle(int sig);
int findInFile(const char * currentfile, const char * search, int charcase);
void refreshScreen();
