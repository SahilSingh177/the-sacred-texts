#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <charconv>
#include <system_error>

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

int main() noexcept
{
    int x=1;
    pid_t pid = Fork();
    if(pid==0){ // Child
        ++x;
        char buf[32]="child: x=";
        auto [ptr,ec] = std::to_chars(buf+9,buf+sizeof(buf),x);
        if(ec==std::errc()){
            *ptr++='\n';
            ::write(1,buf,ptr-buf);
        }
        ::_exit(0);
    }
    // Parent
    int y = --x; // if this was x-- 1 would be printed as compiler takes 1 as value
    char buf[32]="Parent: x=";
    auto [ptr,ec] = std::to_chars(buf+10,buf+sizeof(buf),y);
    if(ec==std::errc()){
        *ptr++ = '\n';
        ::write(1,buf,ptr-buf);
    }
    return 0;
}   