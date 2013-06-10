#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXLINE 1024
#define MAXARGS (MAXLINE/2+1)

pid_t Fork (void);
int Execve (const char *filename, char *argv[],
            char *envp[]);
void unix_error(char *msg); /* unix-style error */ // grifted from CSAPP
int parsecmd(char *cmdline, char *argv[], char sep);
void evalcmd(char *argv[], int bg);

extern char **environ; // array of environment variables from unistd.h

int main () {
  char cmdline[MAXLINE];
  char *argv[MAXARGS];
  int bg;
  while (1) {
    printf("> "); //prompt
    fgets(cmdline, MAXLINE, stdin); //read
    bg = parsecmd(cmdline, argv, ' '); //parse
    evalcmd(argv, bg); //eval
  }
}

/* unix-style error */
void unix_error (char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

pid_t Fork (void) {
  pid_t pid;
  if ((pid = fork()) < 0)
    unix_error("fork error");
  return pid;
}

int Execve (const char *filename, char *argv[],
            char *envp[]) {
  int ret;
  if ((ret = execve(filename, argv, environ)) < 0)
    unix_error("Could not find command:");
  return ret;
}

int Waitpid (pid_t pid, int *status, int options) {
  if (waitpid(pid, status, options) < 0)
    unix_error("Waitpid error");
}

int parsecmd (char *cmdline, char *argv[], char sep) {
  int argc = 0, bg = 0, len = strnlen(cmdline, MAXLINE);
  cmdline[len - 1] = sep; // loop below search & replaces sep with \0
  if (cmdline[len - 2] == '&') {
    bg = 1;
    cmdline[len - 2] = sep;
  }
  char *b = cmdline, *e; // beginning and end arg pointers
  while ((e = strchr(b, sep)) && argc < MAXARGS-1) { // find separator
    argv[argc] = b; // set argv
    *e = '\0'; // null terminate char array
    b = e + 1; // move beginning to end
    while (*b == sep)
      ++b; // get first non-space char
    argc++;
  }
  argv[argc] = 0; // execve requries argv to be null-terminated
  return bg;
}

void evalcmd(char *argv[], int bg) {
  int status;
  pid_t pid;
  if ((pid = Fork()) == 0) // run cmd as child
    Execve(argv[0], argv, environ);
  if (!bg)
    Waitpid(pid, &status, 0);
  else {
    Waitpid(pid, &status, WNOHANG);
    printf("%d %s\n", pid, argv[0]);
  }
  return;
}
