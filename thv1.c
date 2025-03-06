#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include "p1fxns.h"

//remove
#include <stdio.h>

char *p;
char *c;
int processes = -1;
int cores = -1;

int main(int argc, char *argv[]) {

    char command[64];

    for (int i = 1; i < argc; i++) {
        if (p1strneq(argv[i],"-l",2)) {
            p1strcpy(command,argv[i+1]);
            break;
        }
    }

    if ((p = getenv("TH_NPROCESSES")) != NULL) {
        processes = p1atoi(p);
    }

    if ((c = getenv("TH_NCORES")) != NULL) {
        cores = p1atoi(c);
    }

    struct timeval start, end;

    gettimeofday(&start,NULL);

    pid_t *pid = malloc(processes * sizeof(pid_t));

    for (int i = 0; i < processes; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            char *args[] = {command, NULL};
            execvp(command, args);
            exit(0);
        }
    }

    for (int i = 0; i < processes-1; i++) {
	    int status;
	    waitpid(pid[i],&status,0);
    }

    free(pid);

    gettimeofday(&end,NULL);

    double elapsed_time = (end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec) / 1000000.0;
    char process_str[12];
    char core_str[12];

    p1itoa(processes,process_str);
    p1itoa(cores,core_str);
    
    // processes, cores, elapsed time
    //char output = "The elapsed time to execute %d copies of \"%s\" on %d processors is %7.3fsec\n.";
    p1putstr(1, "The elapsed time to execute ");
    p1putstr(1, process_str); 
    p1putstr(1, " copies of \"");
    p1putstr(1, command); 
    p1putstr(1, "\" on ");
    p1putstr(1, core_str); 
    p1putstr(1, " processors is ");
    //p1putstr(1, elapsed_time_str);
    printf("%f",elapsed_time);
    p1putstr(1, " sec\n");

}
