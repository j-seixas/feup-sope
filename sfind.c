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
  flags->name = malloc(1024 * sizeof(char));
  strcpy(flags->name, "");
  flags->perm_mode = malloc(32 * sizeof(char));
  strcpy(flags->perm_mode, "");
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

void sigint_handler(int sign){
  char c;
  do{
      printf("\n\n Are you sure you want to terminate? ");
      scanf("%c", &c);
  
      if(c == 'y' || c == 'Y')
        exit(0);
      else if(c == 'n' || c == 'N')
        return;
      else
        printf("Error! Not found a valid answer!\n");

  }while( 1 );
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
    else if(!strcmp(argv[args] , "-name") && args+1 < argc){
      flags.hasName = 1;
      strcpy(flags.name, argv[args+1]);
      args++;
    }else if(!strcmp(argv[args] , "-perm") && args+1 < argc){
      flags.hasPerm = 1;
      if(isNumber(argv[args+1]))
        strcpy(flags.perm_mode, argv[args+1]);
      args++;
    }
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

  char *directory = malloc(1024 * sizeof(char));
  strcpy(directory,argv[1]);
	strcat(directory,"/");
	//printf("Trying to open dir %s\n",directory);
	if ( (curr_dir = opendir(directory))  == NULL){
		perror("Error 1: ");
		exit(1);
	}

  struct sigaction action;
  action.sa_handler = sigint_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if ( sigaction(SIGINT, &action, NULL) < 0 ){
    perror("Error 2: ");
    exit(2);
  }

  char **argv_new;
  size_t a_size = sizeof(char*) * (argc + 1);
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
        perror("Error 3: ");
        exit(3);
      } 
      else {
        wait(NULL);
        //free(argv_new[1]);
        //free(argv_new);
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
