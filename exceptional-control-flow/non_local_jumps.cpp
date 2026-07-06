#include <setjmp.h>
#include <unistd.h>
#include <string.h>
#include "utils.hpp"
#include <sys/types.h>
#include <charconv>
jmp_buf buff;

int error1 = 1;
int error2 = 0;

void foo(void), bar(void);

int main() noexcept{
    char buf[64];
    char *ptr = buf;
    switch (setjmp(buff))
    {
    case 0:
        foo();
        break;
    case 1:
        append_(ptr, "Detected an error1 condition is foo\n");
        ::write(1, buf, ptr - buf);
        break;
    case 2:
        append_(ptr, "Detected an error2 condition is bar\n");
        ::write(1, buf, ptr - buf);
        break;
    default:
        append_(ptr, "Unknown error condition in foo\n");
        ::write(1, buf, ptr - buf);
    }
    return 0;
}

void foo(void){
    if(error1){
        longjmp(buff,1);
    }
    bar();
}

void bar(void){
    if(error2){
        longjmp(buff,2);
    }
}