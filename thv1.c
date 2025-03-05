#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

char *p;
char *c;
int processes = -1;
int cores = -1;

int main() {
    // stupid argument passing stuff that i cannot be bothered doing rn
    char *args[64];
    if ((p = getenv("TH_NPROCESSES")) != NULL) {
        processes = atoi(p);
    }

    if ((c = getenv("TH_NCORES")) != NULL) {
        cores = atoi(c);
    }

    pid_t pid[processes];
    for (int i = 0; i < processes-1; i++) {
        pid[i] = fork();
        if (pid == 0) {
            execvp(args[0],args);
        }
    }

    for (int i = 0; i < processes-1; i++) {
	 int status;
	 waitpid(pid[i],&status,0);
    }
}
