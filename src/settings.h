#define SETTING_BOOL   0
#define SETTING_SELECT 1
#define SETTING_MULTI  2
#define SETTING_FREE   3

#include <wchar.h>
#include "customtypes.h"

void settingsAction(char *action, char *application, wchar_t *settingsMenuLabel, settingIndex **settings, t1CharValues **charValues, t2BinValues **binValues, int totalCharItems, int totalBinItems, int totalItems, const char * confFile);
void addT1CharValue(t1CharValues **values, int *totalItems, int *maxItem, char *refLabel, char *value);
void addT2BinValue(t2BinValues **values, int *totalItems, int *maxItem, char *refLabel, char *settingLabel, int reset);
void populateBool(t2BinValues **values, char *refLabel, int setting, int maxValue);
void adjustBinSetting(settingIndex **settings, t2BinValues **values, char *refLabel, int maxValue);
void importSetting(settingIndex **settings, int *items, char *refLabel, char *textLabel, int type, char *charSetting, int intSetting, int maxValue, int invert);
void updateSetting(settingIndex **settings, int index, int type, int intSetting);
int intSettingValue(int *setting, int newValue);
void printSetting(int line, int col, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int index, int charIndex, int binIndex, int type, int invert);
int textValueLookup(t1CharValues **values, int *items, char *refLabel, char *value);
void settingsMenuView(wchar_t *settingsMenuLabel, int settingsMenuSize, menuDef *settingsMenu, settingIndex **settings, t1CharValues **charValues, t2BinValues **binValues, int totalCharItems, int totalBinItems, int totalItems, char *application);
