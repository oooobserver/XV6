#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]){
    int fd[2];
    pipe(fd);
    int nums[34];
    for (int i = 0; i < 34; i++){
        nums[i] = i + 2;
    }
    write(fd[1], nums, 136);

    int pid = fork();
    int buf[34];

start:

    // Child process
    if (pid == 0){
        read(fd[0], buf, 136);
        printf("prime %d\n", buf[0]);
        if (buf[1] < buf[0]){
            exit(0);
        }
        int temp[34];
        int j = 0;
        // Select the primes
        for (int i = 1; i < 34; i++){
            if(buf[i]%buf[0] != 0){
                temp[j] = buf[i];
                j++;
            }
        }
        write(fd[1], temp, 136);
        pid = fork();
        goto start;
    }else if(pid > 0){
        close(0);
        close(1);
        wait(0);
        exit(0);
    }else{
        fprintf(2, "primes: fork failed\n");
        exit(1);
    }
}