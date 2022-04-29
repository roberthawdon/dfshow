#include <wchar.h>

typedef struct {
  char refLabel[16];
  wchar_t displayLabel[32];
  int hotKey;
  int displayLabelSize;
} menuDef;

int cmp_menu_ref(const void *lhs, const void *rhs);
void addMenuItem(menuDef **dfMenu, int *pos, char* refLabel, char* displayLabel, int defaultHotKey);
void updateMenuItem(menuDef **dfMenu, int *menuSize, char* refLabel, char* displayLabel);
wchar_t * genMenuDisplayLabel(char* preMenu, menuDef* dfMenu, int size, char* postMenu, int comma);
int menuHotkeyLookup(menuDef* dfMenu, char* refLabel, int size);
int altHotkey(int key);
int wPrintMenu(int line, int col, wchar_t *menustring);
int printMenu(int line, int col, const char *menustring);
int dynamicMenuLabel(wchar_t **label, const char *str);
