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

int main() noexcept{
    pid_t pid;

    // Child sleeps until SIGKILL signal received, then dies
    if((pid=Fork())==0){
        pause(); // Wait for a signal to arrive
        char buf[32];
        char* ptr = buf;
        append_(ptr,"control should never reach here\n");
        ::write(1,buf,ptr-buf);
        ::_exit(1);
    }
    // Parent sends a SIGKILL signal to a child
    kill(pid,SIGKILL);
    int status;
    if(waitpid(pid,&status,0)>0){
        char buf[128];
        char *ptr = buf;
        if(WIFSIGNALED(status)){
            append_(ptr,"Child was killed by signal. ");
            append_var(ptr,buf,WTERMSIG(status));
            append_char(ptr,'\n');
        }
        append_(ptr, "the infant did not survive\n");
        ::write(1, buf, ptr - buf);
    }

    return 0;
}