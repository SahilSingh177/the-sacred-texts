#include <unistd.h>
#include <charconv>
#include <string.h>

template <size_t N>
inline void append_(char*& ptr, const char (&str)[N]){
    memcpy(ptr,str,N-1);
    ptr+=N-1;
}

unsigned int wakeup(unsigned int secs){
    unsigned int unslept = sleep(secs);
    unsigned int time_elapsed = secs-unslept+1;
    char buf[64];
    char *ptr = buf;
    append_(ptr,"Woke up at ");
    auto [ptr1,ec1] = std::to_chars(ptr,buf+sizeof(buf),time_elapsed);
    ptr = ptr1;
    append_(ptr," secs.");
    *ptr++='\n';
    ::write(1,buf,ptr-buf);
    return unslept;
}

int main()noexcept{
    wakeup(4);
    return 0;
}