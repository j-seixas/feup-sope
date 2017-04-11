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


typedef struct{
  char *name;
  int toPrint, toDelete, hasName, hasType, hasPerm, hasExec;
} Flags;

void initFlags(Flags *flag){
	flag->name  = malloc(sizeof(char *));
	flag->toPrint = 0;
	flag->toDelete = 0;
	flag->hasName = 0;
	flag->hasType = 0;
	flag->hasPerm = 0;
	flag->hasExec = 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2){
		printf("Wrong Usage\n");
		return 1;
	}

  Flags flags;
	initFlags(&flags);

  int args = 3;
  while (args <= argc){
    if(!strcmp(argv[args-1] , "-print"))
      flags.toPrint = 1;
    else if(!strcmp(argv[args-1] , "-delete"))
      flags.toDelete = 1;
    else if(!strcmp(argv[args-1] , "-name") && args+1 <= argc){
      flags.hasName = 1;
			strcpy(flags.name,argv[args]);
		//	printf("%d - %s\n", hasName, name);
			args++;
    }
	//	printf("%d Name - %d Delete - %d Print\n", hasName, toDelete, toPrint);
    args++;
  }

	DIR *curr_dir;
	struct dirent *dir_info;
	int pid = 0;

  char *temp = malloc(sizeof(char *));
  strcpy(temp,argv[1]);
	char *directory = strcat(temp,"/");
	//printf("Trying to open dir %s\n",directory);
	if ( (curr_dir = opendir(directory))  == NULL){
		perror("Error 1 ");
		exit(1);
	}

  char **argv_new;
  size_t a_size = sizeof *argv_new * (argc + 1);  // + 1 for the final NULL
  argv_new = malloc(a_size);
  memcpy(argv_new, argv, a_size);


	while ( (dir_info = readdir(curr_dir) ) != NULL){
		//printf("%d - %s\n",getpid() ,  dir_info->d_name );
		if ( dir_info->d_type == DT_DIR && (strcmp(dir_info->d_name,".") != 0 && (strcmp(dir_info->d_name,"..") != 0))){
			if( (pid = fork()) == 0){
        int size = 1 + snprintf(NULL, 0, "%s/%s", argv[1], dir_info->d_name);
        argv_new[1] = malloc(size);
        snprintf(argv_new[1], size, "%s/%s", argv[1], dir_info->d_name);

        execv(argv_new[0], argv_new);
        free(argv_new[1]);
        free(argv_new);
        printf("Error in process %d\n", getpid());

        return 1;
      }

			/*else
				waitpid(pid,NULL,0);*/

		}
	}

  rewinddir(curr_dir);
  while ( (dir_info = readdir(curr_dir) ) != NULL){

		if(flags.hasName){
			//printf("HAS NAME\n");

			if(!strcmp(flags.name, dir_info->d_name) && flags.toPrint)
				printf("%d - %s/%s   <---- FOUND\n",getpid() ,  argv[1], dir_info->d_name );
			else if(!strcmp(flags.name, dir_info->d_name) && !flags.toPrint)
				printf("%d - %s/%s\n",getpid() ,  argv[1], dir_info->d_name );
			else if(flags.toPrint)
				printf("%d - %s/%s\n",getpid() ,  argv[1], dir_info->d_name );


			if(flags.toDelete && !strcmp(flags.name, dir_info->d_name)){
				if( (pid = fork()) == 0){
					int size = 1 + snprintf(NULL, 0, "%s/%s", argv[1], dir_info->d_name);
        	argv_new[1] = malloc(size);
        	snprintf(argv_new[1], size, "%s/%s", argv[1], dir_info->d_name);
					execlp("rm", "-i", argv_new[1], NULL);
					free(argv_new[1]);
        	free(argv_new);
        	printf("Error in process %d\n", getpid());
					return 1;
				}
			}
		} else if (flags.toPrint)
				printf("%d - %s/%s\n",getpid() ,  argv[1], dir_info->d_name );
  }

	return 0;

}
