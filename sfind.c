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
#include <ctype.h>


typedef struct{
  char *name, *perm_mode;
  char toPrint, toDelete, hasName, hasType, hasPerm, hasExec;
} Flags;

void initFlags(Flags *flags){
  flags->name  = malloc(256 * sizeof(char));
  flags->perm_mode = malloc(3 * sizeof(char));
	flags->toPrint = 0;
	flags->toDelete = 0;
	flags->hasName = 0;
	flags->hasType = 0;
	flags->hasPerm = 0;
	flags->hasExec = 0;
}

char validArguments(Flags *flags){
    return !(
        ((!strcmp(flags->name, "")) && flags->hasName) ||
        ((!strcmp(flags->name, "")) && flags->hasPerm) ||
        ((!strcmp(flags->perm_mode, "")) && flags->hasPerm) ||
        ((!strcmp(flags->name, "")) && flags->toDelete)
      );
}

char isNumber(const char* str) {
  if(str == NULL)
    return 0;
  int i = 0;
  char ch;
  while( (ch = str[i]) != '\0') {
    if(!isdigit(ch))
      return 0;
    i++;
  }
  return 1;
}

int main(int argc, char *argv[])
{
	if (argc < 2){
		printf("Wrong Usage\n");
		return 1;
	}

  Flags flags;
	initFlags(&flags);

  int args = 2;
  while (args < argc){
    if(!strcmp(argv[args] , "-print"))
      flags.toPrint = 1;
    else if(!strcmp(argv[args] , "-delete"))
      flags.toDelete = 1;
    else if(!strcmp(argv[args] , "-name"))
      flags.hasName = 1;
    else if(!strcmp(argv[args] , "-perm"))
      flags.hasPerm = 1;
    else if(isNumber(argv[args]))
      strcpy(flags.perm_mode, argv[args]);
    else
      strcpy(flags.name, argv[args]);
    args++;
  }

  //printf("Name: |%s|\n", flags.name);
  //printf("Perms: |%s|\n", flags.perm_mode);

  if(!validArguments(&flags)){
    printf("Invalid Arguments\n");
    return 1;
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


		}
	}

  rewinddir(curr_dir);
  while ( (dir_info = readdir(curr_dir) ) != NULL){

      //printf("File |%s|\n", dir_info->d_name);

			if(!strcmp(flags.name, dir_info->d_name) && flags.toPrint)
				printf("\n    ----> %d - %s/%s\n\n", getpid() , argv[1], dir_info->d_name );
			else if(!strcmp(flags.name, dir_info->d_name) && !flags.toPrint)
				printf("%d - %s/%s\n",getpid() ,  argv[1], dir_info->d_name );
			else if(flags.toPrint)
				printf("%d - %s/%s\n",getpid() ,  argv[1], dir_info->d_name );


			if(flags.toDelete && !strcmp(flags.name, dir_info->d_name)){
				if( (pid = fork()) == 0){
					int size = 1 + snprintf(NULL, 0, "%s/%s", argv[1], dir_info->d_name);
        	argv_new[1] = malloc(size);
        	snprintf(argv_new[1], size, "%s/%s", argv[1], dir_info->d_name);
					execlp("rm", "-i", "-r", "-f", argv_new[1], NULL);
					free(argv_new[1]);
        	free(argv_new);
        	printf("Error in process %d\n", getpid());
					return 1;
				}
			}


      if(flags.hasPerm && !strcmp(flags.name, dir_info->d_name)){
        //printf("Changing perms of file |%s|\n", dir_info->d_name);

        if( (pid = fork()) == 0){
          int size = 1 + snprintf(NULL, 0, "%s/%s", argv[1], dir_info->d_name);
          argv_new[1] = malloc(size);
          snprintf(argv_new[1], size, "%s/%s", argv[1], dir_info->d_name);
          execlp("chmod", "-f", flags.perm_mode, argv_new[1], NULL);
          free(argv_new[1]);
          free(argv_new);
          printf("Error in process %d\n", getpid());
          return 1;
        }
      }


		}

	return 0;

}
