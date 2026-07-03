#include "utils.hpp"
#include <unistd.h>
#include <charconv>
#include <sys/uio.h>

int main(int argc,char *argv[], char *envp[]){
    int i;
    ::write(1,"Command Line argument:\n",23);
    for(i=0;argv[i]!=NULL;i++){
        char buf[64];
        char *ptr=buf;
        append_(ptr,"    argv[");
        append_var(ptr,buf,i);
        append_(ptr, "] : ");
        append_string(ptr,argv[i]);
        append_char(ptr, '\n');
        ::write(1,buf,ptr-buf);
    }
    ::write(1,"\n Environment variables:\n",25);
    // for(i=0;envp[i]!=NULL;i++){
    //     char buf[4096];
    //     char *ptr = buf;
    //     append_(ptr, "    envp[");
    //     append_var(ptr, buf, i);
    //     append_(ptr, "] : ");
    //     ::write(1, buf, ptr - buf);
    //     ::write(1,envp[i],strlen(envp[i]));
    //     ::write(1,"\n",1);
    // }
    // Optimised env var loop
    for (i = 0; envp[i] != NULL; i++)
    {
            char buf[64];
            char *ptr = buf;
            append_(ptr, "    envp[");
            append_var(ptr, buf, i);
            append_(ptr, "] : ");
            struct iovec iov[3];
            iov[0].iov_base = buf;
            iov[0].iov_len = ptr-buf;
            iov[1].iov_base = envp[i];
            iov[1].iov_len = strlen(envp[i]);
            iov[2].iov_base = const_cast<char*> ("\n");
            iov[2].iov_len = 1;
            ::writev(1, iov, 3); // Scatter/Gather I/O (non contiguous pointers and length) 
            // writev allows to write in just 1 context switch unlinke write where each write needs 1 context switch
        }
        return 0;
    }