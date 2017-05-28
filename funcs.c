#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>

#include "defs.h"

void pid_write_file(char *argv0, char *arg)
{
  char *n = basename(argv0);
  char file[1024], path[1024], pid[256];
  int fd, val;
  struct flock lock;

  if (arg == NULL) {
    strcpy(file, n);
  } else {
    snprintf(file, sizeof(file), "%s:%s", n, arg);
  }

  snprintf(path, sizeof(path), "/var/lock/%s.pid", file);
  fd = open(path, O_WRONLY | O_CREAT, 0644);
  if (fd < 0) {
    syslog(LOG_ERR, "%s: %s", path, strerror(errno));
    exit(1);
  }
  
  lock.l_type = F_WRLCK;
  lock.l_start = 0;
  lock.l_whence = SEEK_SET;
  lock.l_len = 0;

  if (fcntl(fd, F_SETLK, &lock) < 0) {
    if (errno == EACCES || errno == EAGAIN) {
      syslog(LOG_ERR, "%s seems to be already running - exit", file);
      exit(1);
    } else {
      syslog(LOG_ERR, "%s: %s", path, strerror(errno));
      exit(1);
    }
  }
  if (ftruncate(fd, 0) < 0) {
    syslog(LOG_ERR, "%s: %s", path, strerror(errno));
    exit(1);
  }
  snprintf(pid, sizeof(pid), "%d\n", getpid());
  if (write(fd, pid, strlen(pid)) != strlen(pid)) {
    syslog(LOG_ERR, "%s: %s", path, strerror(errno));
    exit(1);
  }
  
  /* set close-on-exec flag */

  val = fcntl(fd, F_GETFD, 0);
  val |= FD_CLOEXEC;
  fcntl(fd, F_SETFD, val);
}


inline char *trim(char *s)
{
  char *p;
 
  while(isspace(*s))
    s++;
 
  if (!*s)
    return s;
 
  p = s;
  while(*p)
    p++;
  p--;
  while(isspace(*p))
    p--;
  *(p+1) = 0;
 
  return s;
}



void daemonize(int verbose_flag)
{
  switch (fork()) {
  case -1:
    fprintf(stderr, "Couldn't fork.\n");
    exit(2);
  case 0:
    /* We are the child */
    break;
  default:
    /* We are the parent */
    sleep(1);
    _exit(0);
  }
  
  setsid();
  
  switch (fork()) {
  case -1:
    fprintf(stderr, "Couldn't fork.\n");
    exit(2);
  case 0:
    /* We are the child */
    break;
  default:
    /* We are the parent */
    sleep(1);
    _exit(0);
  }

  if (verbose_flag == TRUE)
    fprintf(stderr, "* *  >> [process id %d]\n", getpid());
  
  chdir("/");
  
  umask(077);  
  freopen("/dev/null", "r", stdin);
  freopen("/dev/null", "w", stdout);
  freopen("/dev/null", "w", stderr);
}



inline off_t getfilesize(char *name)
{
  struct stat st;
            
  return stat(name, &st) ? (off_t)-1 : (off_t)st.st_size;
}


int fget_logline_return(char *buf, size_t size, FILE *f, int follow_flag)
{
  int chc, ch;

  buf[0] = 0;
  chc = 0;
  do {
    ch = getc(f);
    if (ch == EOF) {
      if (!follow_flag)
        return EOF;
      sleep(1);
      clearerr(f);
      if (chc == 0)
        return 0;
    } else
      if (isascii(ch))
        buf[chc++] = ch;
  } while(chc < (size-1) && ch != '\n');

  buf[chc] = 0;

  return chc;
}


void err(int daemon_flag, char *fmt, ...)
{
  char message[1024];
  va_list ap;

  if (fmt != NULL) {
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);
  }
  if (daemon_flag == TRUE) {
    syslog(LOG_ERR, "%s", message);
  } else {
    fprintf(stderr, "%s\n", message);
  }
}
