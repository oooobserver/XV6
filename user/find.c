#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"


void find(char const * path, char const *target){
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	// Permission check
	if((fd = open(path, O_RDONLY)) < 0){
		fprintf(2, "ls: cannot open %s\n", path);
		exit(1);
	}
	// If can stat this dir
	if(fstat(fd, &st) < 0){
		fprintf(2, "ls: cannot stat %s\n", path);
		close(fd);
		exit(1);
	}
	// Path name is too long to put in buffer
	if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      fprintf(2, "ls: path too long\n");
      exit(1);
    }
	// Copy the path to the buffer, set up the path
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
		if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
			continue;
		// Move the name to p, get the whole path
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;

		// If can stat this file/dir
		if(stat(buf, &st) < 0){
			printf("ls: cannot stat %s\n", buf);
			continue;
		}

		if(st.type == T_DIR){
      		find(buf, target);
      	}else if (st.type == T_FILE){
      		if (strcmp(de.name, target) == 0){
      			printf("%s\n", buf);
      		}
      	}
    }
	close(fd);
}


int main(int argc, char const *argv[]){
	if (argc != 3){
		fprintf(2, "find: less parameters\n");
		exit(1);
	}

	char  const *start_path = argv[1];
	char  const *target_name = argv[2];

	find(start_path, target_name);
	exit(0);
}