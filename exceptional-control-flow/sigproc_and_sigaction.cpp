#include <sys/types.h>
#include <unistd.h>
#include <charconv>
#include <string.h>
#include "utils.hpp"
#include <signal.h>
#include <sys/wait.h>
using handler_t = void (*)(int);

[[noreturn]] [[gnu::cold]] void sio_error(const char *msg) noexcept
{
    ::write(2, msg, ::strlen(msg));
    ::write(2, "\n", 1);
    ::_exit(1);
}

volatile sig_atomic_t active_jobs = 0;

void initjobs()
{
    active_jobs = 0;
}

void addjob(pid_t pid)
{
    active_jobs++;
    char msg[] = "Parent: Added job to list.\n";
    ::write(1, msg, sizeof(msg) - 1);
}

void deletejob(pid_t pid)
{
    active_jobs--;
    char msg[] = "Handler: Deleted job from list.\n";
    ::write(1, msg, sizeof(msg) - 1);
}

handler_t Signal(int signum, handler_t handler){
    struct sigaction action, old_action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); // Block sigs of type being handled
    action.sa_flags = SA_RESTART; // Restart syscalls if possible 
    if(sigaction(signum,&action,&old_action)<0){
        unix_error("Signal error");
    }
    return old_action.sa_handler;
}

void handler(int sig)
{
    int olderrno = errno;
    sigset_t mask_all, prev_all;
    pid_t pid;

    sigfillset(&mask_all);

    // Reap all available zombie children
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        // Block all signals while modifying global data structures
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        deletejob(pid);
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    if (errno != ECHILD)
    {
        sio_error("waitpid error");
    }

    errno = olderrno;
}

int main(int argc, char **argv) noexcept
{
    pid_t pid;
    sigset_t mask_all, mask_one, prev_one;

    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    Signal(SIGCHLD, handler);
    initjobs();

    for (int i=0; i<3; i++)
    {
        // Block SIGCHLD before forking to prevent race conditions
        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);

        if ((pid = Fork()) == 0)
        {
            // The child inherits the blocked mask. It MUST unblock it before execve.
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            char *const args[] = {const_cast<char *>("date"), nullptr};
            char *const env[] = {nullptr};
            ::execve("/bin/date", args, env);
            unix_error("execve failed"); // Only reached if execve fails
        }
        // Block ALL signals while writing to the global job list
        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(pid);

        // Restore the original mask (which unblocks SIGCHLD)
        sigprocmask(SIG_SETMASK, &prev_one, NULL);
        usleep(50000);
    }

    // Wait for the handler to finish reaping before exiting
    while (active_jobs > 0)
    {
        pause();
    }
    return 0;
}