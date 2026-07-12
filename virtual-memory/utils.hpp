#pragma once

#include <string.h>
#include <stddef.h>
#include <charconv>
#include <system_error>

template <size_t N>
inline void append_(char *&ptr, const char (&str)[N])
{
    memcpy(ptr, str, N - 1);
    ptr += N - 1;
}

// for int,floats,etc.
template <size_t N,typename T>
inline void append_var(char*& ptr, char (&buf)[N],T x){
    auto [pt,ec] = std::to_chars(ptr,buf+N,x);
    if(ec == std::errc())
    {
        ptr = pt;
    }
}

inline void append_string(char *&ptr, const char *str)
{
    size_t len = strlen(str);
    memcpy(ptr, str, len);
    ptr += len;
}

inline void append_char(char*& ptr, char c){
    *ptr++=c;
}

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