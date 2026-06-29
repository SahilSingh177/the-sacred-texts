#include <sys/types.h> // for pid_t
#include <unistd.h> // for fork() function
#include <stdio.h> // for fprintf and stderr (global variable pointing to the standard error stream)
#include <stdlib.h> // for exit() function
#include <string.h> // for strerror
#include <errno.h> // for errno

void unix_error(const char *msg){
    fprintf(stderr,"%s: %s\n",msg,strerror(errno));
    exit(0);
}

// we enclose fork inside Fork so that it checks for errors and terminates if there are any problems.
pid_t Fork(void){
    pid_t pid;

    if((pid=fork())<0){
        unix_error("Fork error");
    }
    return pid;
}

int main(){
    pid_t pid = Fork();
    fprintf(stdout,"%d",(int)pid);
}
