#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <charconv>
#include <system_error>
#include <sys/wait.h>
#define lim 4

template <size_t N>
// N is deduced at compile time.
// We pass ptr by reference (char*&) so the function updates it for us!
inline void append_(char*& ptr, const char (&str)[N])
{
    // N includes the hidden '\0', so we copy N - 1 bytes
    memcpy(ptr,str,N-1);
    ptr+= (N-1);
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

int main(){
    int status,o;
    pid_t pid;
    for(int i=0;i<lim;i++){
        if((pid=Fork())==0) // child
            ::_exit(100+i);
    }

    // Parent reaps children in no particular order
    while((pid=waitpid(-1,&status,0))>0){ 
        // waitpid(pid_t pid,int *status, int options)
        // pid decides which process to pick (-1 means any child positive means process whose processId is same as pid)
        // status means status info about the child
        // options = 0 (default) means suspend calling process until the waitpid returns
        // waitpid returns pid of child if Ok, 0 (if options is WNOHANG) or -1 if error
        if(WIFEXITED(status)){
            char buf[128];
            char *ptr = buf;
            memcpy(ptr,"child ",6); // append "child "(6 bytes) to ptr
            ptr+=6; 
            auto [ptr_1,ec1] = std::to_chars(ptr,buf+sizeof(buf),pid);
            ptr = ptr_1;
            append_(ptr," terminated normally with exit status=");
            auto [ptr_2,ec2] = std::to_chars(ptr,buf+sizeof(buf),WEXITSTATUS(status));
            ptr = ptr_2;
            *ptr++='\n';
            ::write(1,buf,ptr-buf);
        } 
        else{
            char buf[64];
            char *ptr = buf;
            append_(ptr,"Child ");
            auto [ptr_1, ec1] = std::to_chars(ptr, buf + sizeof(buf), pid);
            ptr = ptr_1;
            append_(ptr, " terminated abnormally");
            *ptr++='\n';
            ::write(1,buf,ptr-buf);
        }
    }
    if(errno!=ECHILD){
        unix_error("waitpid error");
    }
    return 0;
}