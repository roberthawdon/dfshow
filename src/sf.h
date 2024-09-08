#include <limits.h>

#define PROGRAM_NAME "sf"
#define LINE_WRAP 0
#define WORD_WRAP 1

enum
  {
   GETOPT_HELP_CHAR = (CHAR_MIN - 2),
   GETOPT_VERSION_CHAR = (CHAR_MIN - 3),
   GETOPT_THEME_CHAR = (CHAR_MIN - 4),
   GETOPT_OPTIONSMENU_CHAR = (CHAR_MIN - 5),
   GETOPT_ENABLE_MOUSE = (CHAR_MIN - 6)
  };

void sigwinchHandle(int sig);
