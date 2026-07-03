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