#define COMMAND_PAIR    1
#define DISPLAY_PAIR    2
#define ERROR_PAIR      3
#define INFO_PAIR       4
#define HEADING_PAIR    5
#define DANGER_PAIR     6
#define SELECT_PAIR     7
#define HILITE_PAIR     8
#define INPUT_PAIR      9
#define DIR_PAIR       10
#define SLINK_PAIR     11
#define DEADLINK_PAIR  12
#define EXE_PAIR       13
#define SUID_PAIR      14
#define SGID_PAIR      15
#define STICKY_PAIR    16
#define STICKY_OW_PAIR 17

#define DEFAULT_COLOR  -1
#define BRIGHT_BLACK    8
#define BRIGHT_RED      9
#define BRIGHT_GREEN   10
#define BRIGHT_YELLOW  11
#define BRIGHT_BLUE    12
#define BRIGHT_MAGENTA 13
#define BRIGHT_CYAN    14
#define BRIGHT_WHITE   15

#define COLORMENU_PAIR_0  40
#define COLORMENU_PAIR_1  41
#define COLORMENU_PAIR_2  42
#define COLORMENU_PAIR_3  43
#define COLORMENU_PAIR_4  44
#define COLORMENU_PAIR_5  45
#define COLORMENU_PAIR_6  46
#define COLORMENU_PAIR_7  47
#define COLORMENU_PAIR_8  48
#define COLORMENU_PAIR_9  49
#define COLORMENU_PAIR_A  50
#define COLORMENU_PAIR_B  51
#define COLORMENU_PAIR_C  52
#define COLORMENU_PAIR_D  53
#define COLORMENU_PAIR_E  54
#define COLORMENU_PAIR_F  55

#define COLORMENU_SELECT   61
#define DEFAULT_BOLD_PAIR  62
#define DEFAULT_COLOR_PAIR 63

typedef struct {
  char name[24];
  int foreground;
  int background;
  int bold;
} colorPairs;

void processListThemes(const char * pathName);
void listThemes();
void setColorPairs(int pair, int foreground, int background, int bold);
void updateColorPair(int code, int location);
void refreshColors();
int applyTheme(const char *filename);
void saveTheme();
void loadTheme();
void loadAppTheme(const char *themeName);
int useTheme(const char * confFile);
int itemLookup(int menuPos);
void lowColorCheck();
void theme_menu_inputs();
void setDefaultTheme();
void setColors(int pair);
void setCursorPos(int prev);
void themeBuilder();
