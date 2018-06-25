#include <limits.h>

// Defining enum for help and version char (method borrowed from coreutils)
enum
  {
   GETOPT_HELP_CHAR = (CHAR_MIN - 2),
   GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
  };

void refreshScreen();
int exittoshell();
void printHelp();
void printVersion();
