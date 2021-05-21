#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/capability.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

#include <pwd.h>

#define LAST_CAP_PATH "/proc/sys/kernel/cap_last_cap"
static void printcaps(
    void)
{
  cap_t caps;
  char *capstr;
  /* Print capabilities */
  caps = cap_get_proc();
  capstr = cap_to_text(caps, NULL);
  printf("Capabilities:\n%s\n\n", capstr);
  cap_free(capstr);
  cap_free(caps);
}

static void print_status(
    void)
{
  char buf[8192] = {0};
  FILE *f = fopen("/proc/self/status", "r");
  if (!f)
    err(EXIT_FAILURE, "fopen");

  fread(buf, 8192, 1, f);
  printf("%s\n", buf);
  fclose(f);
}

/* Obtain the numerically last capability that the kernel supports */
static int get_last_cap(
    void)
{
  char buf[64] = {0};
  int last = -1, fd = -1;

  fd = open(LAST_CAP_PATH, O_RDONLY|O_CLOEXEC);
  if (fd < 0)
    err(EXIT_FAILURE, "Cannot find last cap path");

  if (read(fd, buf, 63) < 0)
    err(EXIT_FAILURE, "Unable to read last cap file");

  /* strtol() can return ERANGE so be sure to reset errno first */
  errno = 0;
  last = strtol(buf, NULL, 10);
  if (errno != 0) 
    err(EXIT_FAILURE, "Unexpected output reading last_caps file");

  if (last < 8)
    err(EXIT_FAILURE, "Unexpected output reading last_caps file");

  close(fd);
  return last;
}

static void curse_user(
    void)
{
  int ncaps = get_last_cap();
  cap_t caps = cap_get_proc();

  if (!caps)
    err(EXIT_FAILURE, "Cannot obtain capabilities");

  /* With caps raised, clear the ambient set */
  if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0) < 0)
    err(EXIT_FAILURE, "Cannot drop ambient capabilities");

  /* Lock permissions */
  if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0)
    err(EXIT_FAILURE, "Cannot lock permission bits");

  /* Drop all remaining caps */
  if (cap_clear(caps) < 0)
    err(EXIT_FAILURE, "Cannot clear capability setting permission");

  if (cap_set_proc(caps) < 0)
    err(EXIT_FAILURE, "Cannot lower capability setting permission");

  cap_free(caps);
}


/* Get the process capabilities, then raise the ambient caps
 * where the processes inherited and permitted set allow it */
static void bless_user(
    void)
{
  int capability;
  int ncaps = get_last_cap();
  char *amb = NULL;
  bool blessed = false;
  cap_flag_value_t *cap_list = NULL;

  cap_flag_value_t permitted, inherited;
  cap_t caps = cap_get_proc();
  cap_list = calloc(ncaps, sizeof(cap_flag_value_t));

  if (!cap_list)
    err(EXIT_FAILURE, "Cannot retain memory for capability list");

  if (!caps)
    err(EXIT_FAILURE, "Cannot obtain capabilities");

  /* Iterate over each capability */
  for (capability=0; capability < ncaps; capability++) {

    /* Skip over unknown caps that the library doesn't know how to handle */
    if (cap_get_flag(caps, capability, CAP_PERMITTED, &permitted) < 0) 
      continue;
    if (cap_get_flag(caps, capability, CAP_INHERITABLE, &inherited) < 0)
      continue;

    /* When both the permitted and inherited bits are set, raise the ambient set */
    if (permitted == inherited && permitted == CAP_SET) {
      blessed = true;
      cap_list[capability] = CAP_SET;
      if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, capability, 0, 0) < 0)
        err(EXIT_FAILURE, "Cannot raise ambient capability set");
    }
  }

  /* Present users blessings */
  if (blessed) {
    fprintf(stderr, "Shell blessed with capabilities:\n");
    for (capability=0; capability < ncaps; capability++) {
      if (cap_list[capability] == CAP_SET) {
        amb = cap_to_name(capability);
        fprintf(stderr, "  %s\n", amb);
        cap_free(amb);
      }
    }
    fprintf(stderr, "\n");
  }
  else {
    fprintf(stderr, "No blessings bestowed.\n\n");
  }

  cap_free(caps);
  free(cap_list);
}


int main(
    int argc,
    const char **argv)
{
  int rc = -1;
  uid_t uid = -1;
  char *base = NULL;
  struct passwd *pwd = NULL;

  uid = getuid();
  if (uid < 0)
    err(EXIT_FAILURE, "Getting UID failed.");

  if (!isatty(STDERR_FILENO) ||
      !isatty(STDOUT_FILENO) ||
      !isatty(STDIN_FILENO)) {
    errx(EXIT_FAILURE, "Must be connected to a valid TTY device. Exiting.");
  }

  /* Make process undumpable */
  if (prctl(PR_SET_DUMPABLE, 0) < 0)
    err(EXIT_FAILURE, "Unable to set dumpable flag on program");

  pwd = getpwuid(uid);
  if (!pwd)
    err(EXIT_FAILURE, "Cannot identify user with UID %d. Exiting", uid);

  base = basename(argv[0]);
  if (!base)
    errx(EXIT_FAILURE, "Cannot derive basename for program");
  else if (strcmp(base, "bless") == 0)
    bless_user();
  else if (strcmp(base, "curse") == 0)
    curse_user();
  else
    errx(EXIT_FAILURE, "Program must either bless or curse. "
                       "No other operation is supplied. "
                       "Did you rename this program?");

  base = basename(pwd->pw_shell);
  if (!base)
    errx(EXIT_FAILURE, "Cannot obtain basename of shell");

  if (execl(pwd->pw_shell, base, "-", NULL) < 0)
    err(EXIT_FAILURE, "Cannot execute a shell");

  exit(EXIT_FAILURE);
}
