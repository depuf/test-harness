#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "cirque.h"
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

int running;
queue *process_queue;
queue *running_queue;

void handle_sigusr1(int signum) {
    p1putstr(1, "i received a funny signal\n");
    execvp(args[0], args);
    perror("execvp");
    exit(1);
}

void scheduler(int signum) {
    for (int i = 0; i < running; i++) {
        pid_t p = dequeue(running_queue);
        if (p != -1) {
            p1putstr(1, "ig i stop :(\n");
            kill(p, SIGSTOP);
            enqueue(process_queue, p);
        }
    }

    running = 0;
    for (int i = 0; i < cores; i++) {
        pid_t p = dequeue(process_queue);
        if (p != -1) {
            p1putstr(1, "i will run now\n");
            kill(p, SIGCONT);
            enqueue(running_queue, p);
            running++;
        }
    }
}

void cleanup(int signum) {
    p1putstr(1, "i finished\n");
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_from_queue(process_queue, pid);
        remove_from_queue(running_queue, pid);
    }
}

int main(int argc, char *argv[]) {
    process_queue = create_queue();
    running_queue = create_queue();

    if (process_queue == NULL || running_queue == NULL) {
        p1perror(1, "error: failed to create queues\n");
        return 1;
    }

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
        if (p1strneq(argv[i], "-l", 2)) {
            command = malloc(p1strlen(argv[i + 1]) + 1);
            p1strcpy(command, argv[i + 1]);

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
            processes = p1atoi(argv[i + 1]);
        } else if (p1strneq(argv[i], "-c", 2)) {
            cores = p1atoi(argv[i + 1]);
        } else if (p1strneq(argv[i], "-q", 2)) {
            quantum = p1atoi(argv[i + 1]);
        }
    }

    if (processes <= 0 || cores <= 0 || quantum <= 0) {
        p1perror(1, "error: invalid input values\n");
        return 1;
    }

    struct sigaction sa;
    //sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sa.sa_handler = scheduler;
    sigaction(SIGALRM, &sa, NULL);

    sa.sa_handler = cleanup;
    sigaction(SIGCHLD, &sa, NULL);

    pid_t *pid = malloc(processes * sizeof(pid_t));
    for (int i = 0; i < processes; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {  
            signal(SIGUSR1, handle_sigusr1);
            pause();  
            exit(0);
        } else { 
            enqueue(process_queue, pid[i]);
        }
    }

    for (int i = 0; i < cores && !is_empty(process_queue); i++) {
        pid_t p = dequeue(process_queue);
        kill(p, SIGUSR1);  
        //kill(p, SIGCONT); 
        enqueue(running_queue, p);
        running++;
    }


    while (!is_empty(process_queue)) {
        struct itimerval timer;
        timer.it_interval.tv_sec = quantum / 1000;  // Get whole seconds
        timer.it_interval.tv_usec = (quantum % 1000) * 1000;  // Convert remaining milliseconds to microseconds
        timer.it_value.tv_sec = quantum / 1000;
        timer.it_value.tv_usec = (quantum % 1000) * 1000;
        setitimer(ITIMER_REAL, &timer, NULL);

    }
    


    while (wait(NULL) > 0);


    free(pid);
    if (command != NULL) {
        free(command);
    }
    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }

    destroy_queue(process_queue);
    destroy_queue(running_queue);

    return 0;
}