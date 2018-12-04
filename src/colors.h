#define COMMAND_PAIR    1
#define INFO_PAIR       2
#define INPUT_PAIR      3
#define SELECT_PAIR     4
#define DISPLAY_PAIR    5
#define DANGER_PAIR     6
#define DIR_PAIR        7
#define SLINK_PAIR      8
#define EXE_PAIR        9
#define SUID_PAIR      10
#define SGID_PAIR      11
#define HILITE_PAIR    12
#define ERROR_PAIR     13
#define HEADING_PAIR   14
#define DEADLINK_PAIR  15

#define DEFAULT_COLOR  -1
#define BRIGHT_BLACK    8
#define BRIGHT_RED      9
#define BRIGHT_GREEN   10
#define BRIGHT_YELLOW  11
#define BRIGHT_BLUE    12
#define BRIGHT_MAGENTA 13
#define BRIGHT_CYAN    14
#define BRIGHT_WHITE   15

#define COLORMENU_PAIR_0  100
#define COLORMENU_PAIR_1  101
#define COLORMENU_PAIR_2  102
#define COLORMENU_PAIR_3  103
#define COLORMENU_PAIR_4  104
#define COLORMENU_PAIR_5  105
#define COLORMENU_PAIR_6  106
#define COLORMENU_PAIR_7  107
#define COLORMENU_PAIR_8  108
#define COLORMENU_PAIR_9  109
#define COLORMENU_PAIR_A  110
#define COLORMENU_PAIR_B  111
#define COLORMENU_PAIR_C  112
#define COLORMENU_PAIR_D  113
#define COLORMENU_PAIR_E  114
#define COLORMENU_PAIR_F  115


#define DEFAULT_COLOR_PAIR 255

void setColorMode(int mode);
void setColors(int pair);
void themeBuilder();
