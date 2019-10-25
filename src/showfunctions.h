#include <sys/stat.h>
#include <sys/acl.h>

#define MULTICHAR '*'
#define ONECHAR '?'

#define ACL_NONE    0
#define ACL_TRUE    1
#define ACL_SELINUX 2 // Linux Specific
#define ACL_XATTR   3 // macOS Specific

typedef struct {
  int marked[1];
  mode_t mode;
  acl_t acl;
  ssize_t xattr;
  int seLinuxCon;
  char *contextText;
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

int checkRunningEnv();
void freeResults(results *ob, int count);
void freeHistory(history *hs, int count);
char *getRelativePath(char *file, char *target);
int wildcard(const char *value, char *wcard);
int findResultByName(results *ob, char *name);
char *dateString(time_t date, char *style);
int check_object(const char *object);
void copy_file(char *source_input, char *target_input, mode_t mode);
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
int writePermsEntry(char * perms, mode_t mode, int axFlag);
void writeResultStruct(results* ob, const char * filename, struct stat buffer, int count, acl_t acl, ssize_t xattr, int seLinuxCon, char * contextText);
char *markedDisplay(results* ob);
int huntFile(const char * file, const char * search, int charcase);

