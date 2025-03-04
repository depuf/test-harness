#include <stdlib.h>

char *p;
char *c;
int processes = -1;
int cores = -1;

int main() {
    if ((p = getenv(“nprocesses”)) != NULL) {
        processes = atoi(p);
    }

    if ((c = getenv(“ncores”)) != NULL) {
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
        wait(pid[i]);
    }
}
