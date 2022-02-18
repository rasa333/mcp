#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <libgen.h>

#include "defs.h"


static HASH **hash_deny;


static void append_deny_list(int daemon_flag, char *file, char *ipstr)
{
    FILE *f;

    if (!strcmp(file, "-"))
        f = stdout;
    else
        f = fopen(file, "a+");
    if (f == NULL) {
        err(daemon_flag, "%s: %s", file, strerror(errno));
        exit(1);
    }
    fprintf(f, "ALL: %s\n", ipstr);
    fclose(f);
    syslog(LOG_NOTICE, "Appended ip-address '%s' to file '%s'", ipstr, file);
    if (!daemon_flag)
        printf("Appended ip-address '%s' to file '%s'\n", ipstr, file);
}


static void load_deny_list(char *file)
{
    FILE *f;
    char buf[1024], *p, *q;

    hash_deny = hash_init();
    f = fopen(file, "r");
    while (fgets(buf, sizeof(buf), f)) {
        q = strchr(buf, '#');
        if (q != NULL)
            *q = 0;
        p = trim(buf);
        if (*p == 0 || (q = strchr(p, ':')) == NULL)
            continue;
        q = trim(q + 1);
        hash_install(hash_deny, q);
    }
    fclose(f);
}

/*
static void printit(HASH *h)
{
  printf("%-16s %8d\n", h->ipstr, h->count);
}
*/

static void usage()
{
    printf("Usage: mcp [options]\n");
    printf(" -D       Daemonize and follow authority file as it grows\n");
    printf(" -a file  Set name of authority file [%s]\n", DEFAULT_AUTH_FILE);
    printf(" -d file  Set name of deny file [%s]\n", DEFAULT_DENY_FILE);
}


int main(int argc, char **argv)
{
    FILE *f;
    char buf[1024], *p, *q;
    HASH **hash_auth, *hash_record;
    int opt;
    int daemon_flag = FALSE;
    char *auth_file = NULL;
    char *deny_file = NULL;
    off_t filesize, newfilesize;

    while ((opt = getopt(argc, argv, "Da:d:")) != -1) {
        switch (opt) {
            case 'D':
                daemon_flag = TRUE;
                break;
            case 'a':
                auth_file = strdup(optarg);
                break;
            case 'd':
                deny_file = strdup(optarg);
                break;
            default:
                usage();
                exit(1);
        }
    }
    if (auth_file == NULL)
        auth_file = strdup(DEFAULT_AUTH_FILE);
    if (deny_file == NULL)
        deny_file = strdup(DEFAULT_DENY_FILE);

    nice(15);
    openlog(basename(argv[0]), LOG_PID, LOG_AUTH);

    if (daemon_flag == TRUE) {
        daemonize(FALSE);
        pid_write_file(argv[0], NULL);
    }

    load_deny_list(deny_file);
    hash_auth = hash_init();
    if (!strcmp(auth_file, "-")) {
        f = stdin;
        filesize = 0;
    } else {
        f = fopen(auth_file, "r");
        filesize = getfilesize(auth_file);
    }
    if (f == NULL) {
        err(daemon_flag, "%s: %s", auth_file, strerror(errno));
        exit(1);
    }
    if (daemon_flag && strcmp(auth_file, "-"))
        fseek(f, 0L, SEEK_END);
    while (3) {
        if (fget_logline_return(buf, sizeof(buf), f, daemon_flag) == EOF && daemon_flag == FALSE)
            break;

        if (daemon_flag == TRUE && strcmp(auth_file, "-")) {
            if ((newfilesize = getfilesize(auth_file)) < filesize) {
                fclose(f);
                do {
                    sleep(1);
                } while ((filesize = getfilesize(auth_file)) == -1);
                f = fopen(auth_file, "r");
                if (f == NULL) {
                    err(daemon_flag, "%s: %s", auth_file, strerror(errno));
                    exit(1);
                }
                syslog(LOG_NOTICE, "file '%s' reopened", auth_file);
                continue;
            }
            filesize = newfilesize;
        }

        if (strstr(buf, "Failed") && (p = strstr(buf, "from"))) {
            q = strchr(p + 5, ' ');
            if (q == NULL)
                continue;
            *q = 0;
            hash_record = hash_install(hash_auth, p + 5);
            if (++hash_record->count > 2 && hash_lookup(hash_deny, p + 5) == NULL) {
                append_deny_list(daemon_flag, deny_file, p + 5);
                hash_install(hash_deny, p + 5);
            }
        }

        if (strstr(buf, "Accepted") && (p = strstr(buf, "from"))) {
            q = strchr(p + 5, ' ');
            if (q == NULL)
                continue;
            *q = 0;
            if (hash_lookup(hash_deny, p + 5) == NULL) {
                hash_record = hash_install(hash_auth, p + 5);
                if (hash_record->count > 0) {
                    hash_record->count = 0;
                    syslog(LOG_NOTICE, "Reset ip-address '%s'", p + 5);
                }
            }
        }
    }
    fclose(f);
    //  hash_foreach(hash_auth, printit);
    //  puts("---");
    //  hash_foreach(hash_deny, printit);
    exit(0);
}
