#include "hash.h"

#define DEFAULT_AUTH_FILE  "/var/log/auth.log"
#define DEFAULT_DENY_FILE  "/etc/hosts.deny"

#define TRUE     1
#define FALSE    0
#define INVALID -1

extern HASH **hash_init();
extern HASH  *hash_install(HASH **hashtab, char *ipstr);
extern HASH *hash_lookup(HASH **hashtab, char *ipstr);
extern char *trim(char *s);
extern off_t getfilesize(char *name);
extern int fget_logline_return(char *buf, size_t size, FILE *f, int follow_flag);
extern void err(int daemon_flag, char *fmt, ...);
extern void daemonize(int);
extern void pid_write_file(char *argv0, char *arg);
