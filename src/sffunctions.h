#include <limits.h>
#include "customtypes.h"

// #define PROGRAM_NAME "sf"
#define LINE_WRAP 0
#define WORD_WRAP 1

int calculateTab(int pos);
void file_view(char * currentfile);
void fileShowStatus();
void loadFile(const char * currentfile);
int updateView();
int findInFile(const char * currentfile, const char * search, int charcase);
void refreshScreenSf();
void readSfConfig(const char * confFile);
void saveSfConfig(const char * confFile, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int items, int charIndex, int binIndex);
int generateSfSettingsVars();
// void freeSettingVars();
void applySfSettings(settingIndex **settings, t1CharValues **values, int items, int valuesCount);
int findLastLineStart(int longLineLen);
