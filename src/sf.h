#include <limits.h>

#define PROGRAM_NAME "sf"
#define LINE_WRAP 0
#define WORD_WRAP 1

enum
  {
   GETOPT_HELP_CHAR = (CHAR_MIN - 2),
   GETOPT_VERSION_CHAR = (CHAR_MIN - 3),
   GETOPT_THEME_CHAR = (CHAR_MIN - 4),
   GETOPT_OPTIONSMENU_CHAR = (CHAR_MIN - 5)
  };

void readConfig(const char * confFile);
void saveConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex);
int calculateTab(int pos);
void file_view(char * currentfile);
void fileShowStatus();
void loadFile(const char * currentfile);
void updateView();
void sigwinchHandle(int sig);
int findInFile(const char * currentfile, const char * search, int charcase);
void refreshScreen();
void applySettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount);
void settingsMenuView();
