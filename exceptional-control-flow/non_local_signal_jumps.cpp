#include <setjmp.h>
#include <unistd.h>
#include <signal.h>
#include "utils.hpp"
sigjmp_buf buff;
using handler_t = void (*)(int);

void sio_puts(const char *msg) noexcept
{
    ::write(1, msg, ::strlen(msg));
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

void handler(int sig){
    // Jump back to where 'buff' was saved.
    // The '1' is the value that sigsetjmp will return when we land.
    siglongjmp(buff,1);
}

int main() noexcept{
    // Save the execution context (CPU registers, stack pointer) into 'buff'.
    // The '1' as the second argument tells the OS: "Save my current signal mask."
    if(!sigsetjmp(buff,1)){
        Signal(SIGINT,handler);
        sio_puts("Starting\n");
    }
    else{
        sio_puts("restarting\n");
    }
    while(1){
        sleep(1);
        sio_puts("processing...\n");
    }
    return 0;
}