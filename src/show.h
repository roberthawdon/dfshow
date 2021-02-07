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
   GETOPT_SHOWONENTER_CHAR = (CHAR_MIN - 12),
   GETOPT_SHOWRUNNING_CHAR = (CHAR_MIN - 13),
   GETOPT_FULLTIME_CHAR = (CHAR_MIN - 14),
   GETOPT_THEMEEDIT_CHAR = (CHAR_MIN - 15),
   GETOPT_OPTIONSMENU_CHAR = (CHAR_MIN - 16),
   GETOPT_SKIPTOFIRST_CHAR = (CHAR_MIN - 17),
   GETOPT_BLOCKSIZE_CHAR = (CHAR_MIN - 18)
  };

void readConfig(const char * confFile);
void saveConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex);
int directory_view(char * currentpwd);
char *getcwd(char *buf, size_t size);
int global_menu();
void refreshScreen();
int exittoshell();
int setBlockSize(const char * arg);
void printHelp(char* programName);
int checkStyle(char* styleinput);
int setColor(char* colorinput);
int setMarked(char* markedinput);
void applySettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount);
void settingsMenuView();
