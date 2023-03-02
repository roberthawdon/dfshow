#include <wchar.h>
#include "customtypes.h"

#define NOSORT 0
#define SORT 1

int cmp_menu_ref(const void *lhs, const void *rhs);
void addMenuItem(menuDef **dfMenu, int *pos, char* refLabel, char* displayLabel, int defaultHotKey, bool sort);
void updateMenuItem(menuDef **dfMenu, int *menuSize, char* refLabel, char* displayLabel);
wchar_t * genMenuDisplayLabel(char* preMenu, menuDef* dfMenu, int size, char* postMenu, int comma);
int menuHotkeyLookup(menuDef* dfMenu, char* refLabel, int size);
int altHotkey(int key);
int wPrintMenu(int line, int col, wchar_t *menustring);
int printMenu(int line, int col, const char *menustring);
int dynamicMenuLabel(wchar_t **label, const char *str);
