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

    char *command = NULL;
    char *args[16];
    int arg_count = 0;

    for (int i = 1; i < argc; i++) {
        if (p1strneq(argv[i],"-l",2)) {
            command = malloc(p1strlen(argv[i+1]) + 1);
            p1strcpy(command,argv[i+1]);

            int pos = 0;
            char word[128];
            while ((pos = p1getword(command, pos, word)) != -1) {
                args[arg_count] = p1strdup(word);
                arg_count++;
            if (arg_count >= 15) break;
            }

            args[arg_count] = NULL;
            break;
        }
    }

    if ((p = getenv("TH_NPROCESSES")) != NULL) {
        processes = p1atoi(p);
    }

    if ((c = getenv("TH_NCORES")) != NULL) {
        cores = p1atoi(c);
    }

    struct timespec start,end;

    clock_gettime(CLOCK_MONOTONIC,&start);

    pid_t *pid = malloc(processes * sizeof(pid_t));

    for (int i = 0; i < processes; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            execvp(args[0],args);
            exit(0);
        }
    }

    for (int i = 0; i < processes; i++) {
	    int status;
	    waitpid(pid[i],&status,0);
    }

    free(pid);

    clock_gettime(CLOCK_MONOTONIC,&end);

    double elapsed_time = (end.tv_sec-start.tv_sec) + (end.tv_nsec-start.tv_nsec) / 1e9;
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

    free(command);
    return 0;

}
