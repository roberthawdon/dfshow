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

void setColorMode(int mode);
void setColors(int pair);
