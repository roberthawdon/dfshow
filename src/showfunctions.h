#include <sys/stat.h>

typedef struct {
  int marked[1];
  char perm[11];
  int hlink[4];
  int hlinklens[5];
  char owner[128];
  char group[128];
  char author[128];
  size_t size;
  int sizelens[32];
  int major;
  int minor;
  time_t date;
  wchar_t datedisplay[33];
  char name[512];
  char slink[1024];
  int color;
} results;

typedef struct {
  char path[1024];
  char name[512];
  char objectWild[256];
  int topfileref;
  int selected;
} history;

int wildcard(const char *value, char *wcard);
int findResultByName(results *ob, char *name);
char *dateString(time_t date, char *style);
int check_object(const char *object);
void copy_file(char *source_input, char *target_input);
void delete_file(char *source_input);
int seglength(const void *seg, char *segname, int LEN);
int cmp_str(const void *lhs, const void *rhs);
int cmp_int(const void *lhs, const void *rhs);
int cmp_dflist_name(const void *lhs, const void *rhs);
int cmp_dflist_date(const void *lhs, const void *rhs);
int cmp_dflist_size(const void *lhs, const void *rhs);
results* get_dir(char *pwd);
results* reorder_ob(results* ob, char *order);
void display_dir(char *pwd, results* ob, int topfileref, int selected);
void set_history(char *pwd, char *objectWild, char *name, int topfileref, int selected);
size_t GetAvailableSpace(const char* path);
long GetUsedSpace(const char* path);
int SendToPager(char* object);
int SendToEditor(char* object);
void CheckEnv(const char* key);
void LaunchShell();
void LaunchExecutable(const char* object, const char* args);
int UpdateOwnerGroup(const char* object, const char* pwdstr, const char* grpstr);
int RenameObject(char* source, char* dest);
int CheckMarked(results* ob);
void printEntry(int start, int hlinklen, int ownerlen, int grouplen, int authorlen, int sizelen, int majorlen, int minorlen, int datelen, int namelen, int selected, int listref, int topref, results* ob);
void padstring(char *str, int len, char c);
char *genPadding(int num_of_spaces);
void resizeDisplayDir(results* ob);
char *readableSize(double size, char *buf, int si);
int writePermsEntry(char * perms, mode_t mode);
void writeResultStruct(results* ob, const char * filename, struct stat buffer, int count);
char *markedDisplay(results* ob);
int huntFile(const char * file, const char * search, int charcase);

#define MULTICHAR '*'
#define ONECHAR '?'
