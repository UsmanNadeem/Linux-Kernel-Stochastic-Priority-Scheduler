#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>

void handle_err(int ret, char *func)
{
 perror(func);
 exit(EXIT_FAILURE);
}

void get_process_priority(pid_t pid, struct sched_param sp) {
    int ret;
    sp.sched_priority = -1;
    ret = sched_getparam(pid, &sp);
    if(ret == -1)
        handle_err(ret, "sched_getparam");
    printf("PID: %d\t-\tPriority: %d\n", pid, sp.sched_priority);
}




void fork_processes (int process_count) {
    unsigned long i, iter = 0;
    int ret = -1;
    pid_t pid;
    struct sched_param sp;
    unsigned long MAX_ITER = 10000000000;
    //while(iter++ < MAX_ITER);
    iter = 0;
    for (i = 1; i <= process_count; i++) {
        pid = fork();
        if (pid == -1) {
            handle_err(ret, "fork error");
            return;
        }
        if (pid == 0) {
            printf("I am a child: %d PID: %d\n",i, getpid());
            /*Get the priority (nice/RT) */
            return;
        } else {
            /**/
            int ret_status;
            waitpid(pid, &ret_status, 0);
            if (ret_status == -1) 
                handle_err(ret, "child return");
        }
     }
}

int main(int argc, char *argv[]) {
 
    unsigned long i, iter = 0;
    int ret = -1;
    pid_t pid;
    struct sched_param sp;
    unsigned long MAX_ITER = 3000000000;
 
    //if ( argc != 2) 
    //    handle_err(-1, "invalid input");
    
    pid = getpid();

    get_process_priority(pid, sp);            
    //printf("Changing scheduling class and priority\n");
    sp.sched_priority = (i%10)+1;
    ret = sched_setscheduler(getpid(), SCHED_NEWPOLICY, &sp);

    if(ret == -1)
        handle_err(ret, "sched_setscheduler");
	     
	//printf("Changed policy\n");
    //get_process_priority(pid, sp);
        
    while(iter++ < MAX_ITER);
  
    //get_process_priority(getpid(), sp);            
    //iter = 0;
    //while(iter++ < MAX_ITER);
    //get_process_priority(getpid(), sp);
    printf("%d completed\n", pid);
}

