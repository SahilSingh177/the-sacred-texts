#include <signal.h>
#include <sys/types.h>
#include <wait.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include "utils.hpp"

using handler_t = void (*)(int);
volatile sig_atomic_t pid; // volatile forces the compiler to read the value of g from memory every time. 
// sig_atomic_t ensures atomicity on the variable (for individual reads/writes)

void sigchild_handler(int s){
    int olderrno = errno;
    pid_t temp_pid;

    // WNOHANG loop: Reap as many children as possible without ever blocking.
    // This protects against dropped/coalesced signals.
    while ((temp_pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        pid = temp_pid; // Update global flag to wake the main loop
    }
    errno = olderrno;
}

void sigint_handler(int s){
    char buf[32];
    char* ptr = buf;
    append_(ptr,"Not stopping!");
    ::write(1,buf,ptr-buf);
}

handler_t Signal(int signum, handler_t handler)
{
    struct sigaction action, old_action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); // Block sigs of type being handled
    action.sa_flags = SA_RESTART; // Restart syscalls if possible
    if (sigaction(signum, &action, &old_action) < 0)
    {
        unix_error("Signal error");
    }
    return old_action.sa_handler;
}

int main(int argc,char **argv) noexcept{
    sigset_t mask,prev; // sigset_t is just an integer (usually 64 bit unsigned long)
    Signal(SIGCHLD,sigchild_handler);
    Signal(SIGINT,sigint_handler);
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    while(1){
        sigprocmask(SIG_BLOCK,&mask,&prev);
        if(Fork()==0){
            ::_exit(1);
        }
        pid = 0;
        while(!pid){
            sigsuspend(&prev); // replaces the current mask with prev and goes to sleep in exactly 1 CPU instruction. 
        }
        sigprocmask(SIG_SETMASK,&prev,NULL);
        ::write(1,".",1);
    }
    return 0;
}