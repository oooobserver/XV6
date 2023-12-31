#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"


int get_input_line(char *sub_argv[], int start){
    char buf[1];
    char string[256];
    memset(string, 0, 256);
    int i = 0;
    int arg_index = start;
    int count = 0;

    while (read(0, &buf, sizeof(buf)) > 0) {
        if (buf[0] == ' '|| buf[0] == '\n') {
            count++;
            sub_argv[arg_index] = malloc(strlen(string) + 1);
            strcpy(sub_argv[arg_index], string);
            arg_index++;
            if (arg_index == MAXARG) {
                fprintf(2, "xargs: too many arguments\n");
                exit(1);
            }
            if (buf[0] == '\n'){
                return count;
            }
            // Clear the string 
            memset(string, 0, sizeof(string));
            i = 0;
        } else {
            string[i] = buf[0];
            i++;
        }
    }
    return count;
}


int main(int argc, char *argv[]){
    char *sub_argv[MAXARG];
    sub_argv[0] = "null";

    // Get the command
    char * program = malloc(strlen(argv[1]) + 1);
    strcpy(program, argv[1]);
 
    // Construct a new arguments array
    for (int i =0; i < argc-2;i++){
        sub_argv[1+i] = argv[2+i];
    }

    while(1){
        // Get new command argument variable and argument numbers
        int num = get_input_line(sub_argv, argc-1);
        if (num == 0){
            break;
        }
        if(fork()==0){
            exec(program, sub_argv);
        }

        for (int i = argc-1; i < argc - 1 + num; i++){
            free(sub_argv[i]);
            sub_argv[i] = 0;
        }
        wait(0);
    }

    free(program);
    exit(0);
}
