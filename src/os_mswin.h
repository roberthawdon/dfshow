typedef int uid_t;
typedef int gid_t;

ssize_t getline(char **lineptr, size_t *n, FILE *stream);
uid_t getuid();
uid_t geteuid();
gid_t getgid();

int setenv(const char *name, const char *value, int overwrite);

