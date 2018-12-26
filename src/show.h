#define PROGRAM_NAME "show"
#include <limits.h>

// Defining enum for help and version char (method borrowed from coreutils)
enum
  {
   GETOPT_HELP_CHAR = (CHAR_MIN - 2),
   GETOPT_VERSION_CHAR = (CHAR_MIN - 3),
   GETOPT_SI_CHAR = (CHAR_MIN - 4),
   GETOPT_AUTHOR_CHAR = (CHAR_MIN - 5),
   GETOPT_TIMESTYLE_CHAR = (CHAR_MIN - 6),
   GETOPT_NODANGER_CHAR = (CHAR_MIN - 7),
   GETOPT_COLOR_CHAR = (CHAR_MIN - 8),
   GETOPT_THEME_CHAR = (CHAR_MIN - 9),
   GETOPT_MARKED_CHAR = (CHAR_MIN - 10),
   GETOPT_ENVPAGER_CHAR = (CHAR_MIN - 11),
   GETOPT_SHOWONENTER_CHAR = (CHAR_MIN - 12)
  };

void readConfig(const char * confFile);
int directory_view(char * currentpwd);
char *getcwd(char *buf, size_t size);
int global_menu();
void refreshScreen();
int exittoshell();
void printHelp(char* programName);
int checkStyle(char* styleinput);
int setColor(char* colorinput);
int setMarked(char* markedinput);
