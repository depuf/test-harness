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

void handle_sigcont() {
    // this does nothing. (intentional)
}

/* stops processes in the running queue, appends to ready queue, 
then dequeues *cores* processes in ready queue, appends to running queue*/ 
void scheduler() {
    for (int i = 0; i < running; i++) {
        pid_t p = dequeue(running_queue);
        if (p != -1) {
            int status;
            pid_t result = waitpid(p, &status, WNOHANG);

            if (result == 0) {
                kill(p, SIGSTOP);
                enqueue(process_queue, p);
            }
        }
    }

    running = 0;
    for (int i = 0; i < cores && !(is_empty(process_queue)); i++) {
        pid_t p = dequeue(process_queue);
        if (p != -1) {
            enqueue(running_queue, p);
            kill(p, SIGCONT);
            running++;
        }
    }
}

// handling of finished child processes
void handle_child() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            remove_from_queue(process_queue, pid);
            remove_from_queue(running_queue, pid);
            running--;
        }
    }
}

void cleanup() {
    if (command != NULL) {
	    free(command);
    }

    for (int i = 0; i < arg_count; i++) {
	    free(args[i]);
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
            if (i + 1 >= argc || p1strlen(argv[i + 1]) == 0) { 
                p1putstr(STDERR_FILENO, "error: no command provided\n");
                return 1; 
            }

            command = malloc(p1strlen(argv[i + 1]) + 1);
            if (command == NULL) {
                p1perror(1, "error: malloc failed at command\n");
                return 1;
            }
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

    // all legal, https://man7.org/linux/man-pages/man2/sigaction.2.html
    struct sigaction sa;
    sa.sa_mask = (sigset_t){0};
    sa.sa_flags = 0;

    sa.sa_handler = scheduler;
    sigaction(SIGALRM, &sa, NULL);

    sa.sa_handler = handle_child;
    sigaction(SIGCHLD, &sa, NULL);

    pid_t *pid = malloc(processes * sizeof(pid_t));
    if (pid == NULL) {
        p1perror(1, "error: malloc failed at pid\n");
        cleanup();
        return 1;
    }

    for (int i = 0; i < processes; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {  
            signal(SIGCONT, handle_sigcont);
            pause(); 
            execvp(args[0], args);
            p1perror(1,"execvp failed");
            exit(1);
        } else { 
            enqueue(process_queue, pid[i]);
        }
    }

    sleep(1);

    struct timeval start,end;

    for (int i = 0; i < cores && !is_empty(process_queue); i++) {
        pid_t p = dequeue(process_queue);

        enqueue(running_queue, p);
        kill(p, SIGCONT); 
        running++;
    }

    if (gettimeofday(&start, NULL) != 0) {
        p1perror(1, "error: gettimeofday failed\n");
        cleanup();
        return 1;
    }

    struct itimerval timer;
    timer.it_interval.tv_sec = quantum / 1000;  
    timer.it_interval.tv_usec = (quantum % 1000) * 1000; 
    timer.it_value.tv_sec = quantum / 1000;
    timer.it_value.tv_usec = (quantum % 1000) * 1000;
    setitimer(ITIMER_REAL, &timer, NULL);

    // this makes sure main doesnt exit while child processes arent finished
    int finished = 0;
    while (!finished) {
        finished = 1;
    
        if (!is_empty(process_queue) || !is_empty(running_queue)) {
            finished = 0;
        }
    
        usleep(100000);
    }

    for (int i = 0; i < processes; i++) {
        waitpid(pid[i], NULL, 0);
    }

    if (gettimeofday(&end, NULL) != 0) {
        p1perror(1, "error: gettimeofday failed\n");
        cleanup();
        return 1;
    }

    free(pid);

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
    
    cleanup();

    destroy_queue(process_queue);
    destroy_queue(running_queue);

    return 0;
}