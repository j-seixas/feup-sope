#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int parseDirectory( char *directory);

int main(int argc, char *argv[])
{
	if (argc != 2){
		printf("Wrong Usage\n");
		return 1;
	}
	DIR *curr_dir;
	struct dirent *dir_info;
	int pid = 0;
	char *directory = strcat(argv[1],"/");

	printf("Trying to open dir %s\n",directory);
	if ( (curr_dir = opendir(directory))  == NULL){
		perror("Error 1 ");
		exit(1);
	}

	while ( (dir_info = readdir(curr_dir) ) != NULL){
		printf("%d - %s\n",getpid() ,  dir_info->d_name );
		if ( dir_info->d_type == DT_DIR && (strcmp(dir_info->d_name,".") != 0 && (strcmp(dir_info->d_name,"..") != 0))){
			if( (pid = fork()) == 0)
				execl(argv[0],argv[0],strcat(directory , dir_info->d_name),NULL);
			else
				waitpid(pid,NULL,0);

		}
	}
	return 0;

}
