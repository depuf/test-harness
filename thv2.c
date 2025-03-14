#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include "p1fxns.h"

char *p;
char *c;
char *q;
int processes = -1;
int cores = -1;
int quantum = -1;

char *command = NULL;
char *args[16];
int arg_count = 0;

void handle_sigusr1() {
    p1putstr(1,"i received a funny signal");
    execvp(args[0],args);
    exit(0);
}

int main(int argc, char *argv[]) {

    if ((p = getenv("TH_NPROCESSES")) != NULL) {
        processes = p1atoi(p);
    }

    if ((c = getenv("TH_NCORES")) != NULL) {
        cores = p1atoi(c);
    }

    if ((q = getenv("TH_QUANTUM_MSEC")) != NULL) {
        quantum = p1atoi(q);
    }


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
        } else if (p1strneq(argv[i], "-p", 2)) {
            processes = p1atoi(argv[i+1]); 
        } else if (p1strneq(argv[i], "-c", 2)) {
            cores = p1atoi(argv[i+1]); 
        } else if (p1strneq(argv[i], "-q", 2)) {
            quantum = p1atoi(argv[i+1]);
        }
    }

    struct timeval start,end;

    /*
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGUSR1);
    sigaddset(&sigs, SIGSTOP);
    sigaddset(&sigs, SIGCONT);

    sigprocmask(SIG_BLOCK, &sigs, NULL);

    int received_sig;
    */

    pid_t *pid = malloc(processes * sizeof(pid_t));

    for (int i = 0; i < processes; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            signal(SIGUSR1, handle_sigusr1);
            pause();
            //sigwait(&sigs, &received_sig);
            
            //exit(0);
        }
    }

    // timer stays here i believe
    gettimeofday(&start,NULL);

    for (int i = 0; i < processes; i++) {
	    kill(pid[i], SIGUSR1);
    }

    for (int i = 0; i < processes; i++) {
	    kill(pid[i], SIGSTOP);
    }

    for (int i = 0; i < processes; i++) {
	    kill(pid[i], SIGCONT);
    }

    for (int i = 0; i < processes; i++) {
	    int status;
	    waitpid(pid[i],&status,0);
    }

    free(pid);

    gettimeofday(&end,NULL);

    double elapsed_time = (end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec) / 1e6;
    char process_str[12];
    char core_str[12];
    char elapsed_time_str[12];

    p1itoa(processes,process_str);
    p1itoa(cores,core_str);
    
    int int_part = (int)elapsed_time;
    int frac_part = (int)((elapsed_time - int_part) * 1000);
    char int_str[12], frac_str[4], temp_buf[50];

    p1itoa(int_part, int_str);
    p1itoa(frac_part, frac_str);

    if (frac_part < 100) {
        p1strcat(int_str, ".");
        if (frac_part < 10) {
            p1strcat(int_str, "00");
        } else {
            p1strcat(int_str, "0");
        }
    } else {
        p1strcat(int_str, ".");
    }

    p1strcat(int_str, frac_str);
    p1strpack(int_str, 7, ' ', temp_buf);
    p1strcpy(elapsed_time_str, temp_buf);

    p1putstr(1, "The elapsed time to execute ");
    p1putstr(1, process_str); 
    p1putstr(1, " copies of \"");
    p1putstr(1, command); 
    p1putstr(1, "\" on ");
    p1putstr(1, core_str); 
    p1putstr(1, " processors is ");
    p1putstr(1, elapsed_time_str);
    p1putstr(1, " sec.\n");
    
    if (command != NULL) {
	free(command);
    }

    for (int i = 0; i < arg_count; i++) {
	free(args[i]);
    }

    return 0;

}	
