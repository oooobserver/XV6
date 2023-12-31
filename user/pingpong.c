#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int fd[2];
    pipe(fd);
    char buf[2];

    int parent_pid = getpid();
    int which = fork();
    if (which == 0){
        int child_pid = getpid();
        read(fd[0], buf, sizeof(buf));
        fprintf(0, "%d: received ping\n", child_pid);

        write(fd[1], "p", 2);
    }else if (which > 0){
        write(fd[1], "q", 2);

        read(fd[0], buf, sizeof(buf));
        fprintf(0, "%d: received pong\n", parent_pid);
    }else{
        fprintf(2, "pingpone: fork failed\n");
        exit(1);
    }

    // Close the file descriptor
    close(fd[0]);
    close(fd[1]);
    
    exit(0);
}