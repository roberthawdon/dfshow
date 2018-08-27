#include <limits.h>

enum
  {
   GETOPT_HELP_CHAR = (CHAR_MIN - 2),
   GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
  };

void file_view(char * currentfile);
void fileShowStatus(const char * currentfile, int top);
void displayFile(const char * currentfile, int top);
void sigwinchHandle(int sig);
