typedef struct {
  char perm[11];
  int hlink[4];
  int hlinklens[5];
  char owner[128];
  char group[128];
  int size[32];
  int sizelens[32];
  char date[17];
  char name[512];
} results;

int check_dir(char *pwd);
int seglength(const void *seg, char *segname, int LEN);
int cmp_str(const void *lhs, const void *rhs);
int cmp_int(const void *lhs, const void *rhs);
int cmp_dflist_name(const void *lhs, const void *rhs);
int cmp_dflist_date(const void *lhs, const void *rhs);
int cmp_dflist_size(const void *lhs, const void *rhs);
results* get_dir(char *pwd);
results* reorder_ob(results* ob, char *order);
void display_dir(char *pwd, results* ob, int topfileref, int selected);

