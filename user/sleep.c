#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]){
    // If no argument, exit
    if(argc <= 1){
        fprintf(2, "sleep: requiring an argument\n");
        exit(1);
    }else if(argc > 2){
        fprintf(2, "sleep: multiple arguments, only need one\n");
        exit(1);
    }

    int ticks = atoi(argv[1]);
    if (ticks < 0){
        fprintf(2, "sleep: not allowed the negative number\n");
    }
    // System call sleep
    sleep(ticks);

    exit(0);
}