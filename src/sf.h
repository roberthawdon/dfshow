#include <limits.h>

enum
  {
   GETOPT_HELP_CHAR = (CHAR_MIN - 2),
   GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
  };

void file_view(char * currentfile);
void readFile(const char * currentfile);
void fileShowStatus(const char * currentfile, int top);
