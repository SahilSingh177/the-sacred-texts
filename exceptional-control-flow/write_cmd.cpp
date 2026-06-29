#include <unistd.h> // library from write command

int main(){
    write(1,"hello, world\n",13); // Args: 1 means send output to stdout, seq. of bytes to write, 13 = no. of bytes to write 
    _exit(0);
}