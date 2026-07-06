#include <sys/types.h>
#include <unistd.h>
#include <charconv>
#include <string.h>
#include "utils.hpp"
#include <signal.h>
#include <sys/wait.h>

[[noreturn]] [[gnu::cold]] void unix_error(const char *msg) noexcept
{
    ::write(2, msg, ::strlen(msg));
    ::write(2, ": ", 2);

    const char *err_str = ::strerror(errno);
    ::write(2, err_str, ::strlen(err_str));
    ::write(2, "\n", 1);
    ::_exit(1);
}

[[nodiscard]] inline pid_t Fork() noexcept
{
    pid_t pid = ::fork();
    if (pid < 0) [[unlikely]]
    {
        unix_error("Fork error");
    }
    return pid;
}

void signal_handler(int sig){
    char buf[32];
    char* ptr = buf;
    append_(ptr,"Caught SIGINT!\n");
    ::write(1,buf,ptr-buf);
    ::_exit(1);
}

int main() noexcept
{
    pid_t pid;
    // Install the SIGINT handler
    if(signal(SIGINT,signal_handler)==SIG_ERR){
        unix_error("signal error");
    }
    pause(); // Wait for the receipt of a signal
    return 0;
}