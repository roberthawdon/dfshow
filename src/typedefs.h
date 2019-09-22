#include <wchar.h>

typedef struct {
  int marked[1];
  mode_t mode;
  char *perm;
  int hlink[4];
  int hlinklens[5];
  char *owner;
  char *group;
  char *author;
  size_t size;
  int sizelens[32];
  int major;
  int minor;
  time_t date;
  time_t adate;
  wchar_t *datedisplay;
  char *name;
  char *slink;
  int color;
} results;

typedef struct {
  char *path;
  char *name;
  char *objectWild;
  int topfileref;
  int selected;
} history;


