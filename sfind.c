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
  char toPrint, toDelete, hasName, hasType, hasPerm, hasExec, type;
  int perm;
} Flags;

void initFlags(Flags *flags){
  flags->name = malloc(NAME_MAX);
  strcpy(flags->name, "");
  flags->perm_mode = malloc(16);
  strcpy(flags->perm_mode, "");
  flags->type = '\0';
  flags->toPrint = 0;
	flags->toDelete = 0;
	flags->hasName = 0;
	flags->hasType = 0;
	flags->hasPerm = 0;
	flags->hasExec = 0;
  flags->perm = -1;
}

char validArguments(Flags *flags){
    return !(
        ((!strcmp(flags->name, "")) && flags->hasName) ||
        ((!strcmp(flags->perm_mode, "")) && flags->hasPerm) ||
        ((!strcmp(flags->name, "")) && flags->toDelete) ||
        (flags->type == '\0' && flags->hasType)
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

/**
  @brief Replaces old string by new string
  @param str Pointer to string where substrings should be replaced
  @param old_substr Old substring to be replaced
  @param new_substr What to use to replace
  @return 0
*/
int strsubst(char **str , char * old_substr , char * new_substr){
  int oldstr_size = strlen(old_substr), newstr_size = strlen(new_substr),  
      new_size = strlen(*str)+(newstr_size-oldstr_size);
  char *str_ptr, * temp = (char *)malloc(sizeof(char)*new_size), *remaining;
  memcpy(temp,*str,strlen(*str));
  str_ptr = temp;

  while ( (str_ptr = strstr(str_ptr,old_substr)) != NULL ){
    int remaining_size = strlen(str_ptr), i, j;

    remaining = (char *)malloc(sizeof(char)*(remaining_size-oldstr_size));
    strcpy(remaining, &(str_ptr[oldstr_size]) );

    for (i = 0 ; i < newstr_size ; i++) //copy new string
      str_ptr[i] = new_substr[i];

    for(j=0 ; i < new_size ; i++) //copy remaining string
      str_ptr[i] = remaining[j++];
  }
  *str = temp;
  return 0;
}

/*
// find ./ -exec echo 'FILE '{}'' \;
char** parseExec(char *arguments[] , int start , int length ){
  int i;
  char *ptr;
  for (i = 0 ; i < length ; i++){
    if( (ptr = strstr(arguments[i],"'{}'")) != NULL ){
      strsubst
    }else if ( strstr(arguments[start+i] , "\\;") != NULL)
      break;
 
  }

  printf("DONE\n");
  return 1;
}
*/
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
    printf("ARG %d -> %s\n",args,argv[args]);
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
      if(isNumber(argv[args+1])){
        strcpy(flags.perm_mode, argv[args+1]);
        flags.perm = strtol(flags.perm_mode, NULL, 8);
      }
      args++;
    }else if(!strcmp(argv[args] , "-type") && args+1 < argc){
      flags.hasType = 1;
      flags.type = argv[args+1][0];
      args++;
    }else if ( strcmp(argv[args] , "-exec" ) == 0){
      //+1 not to start in -execc
      if ( parseExec(argv , args+1 , argv-args-1) != 0 ){ 
        printf("GOOD\n");
        exit(0);
      }
    }

    args++;
  }

  //Debug
  //printf("Name: |%s|\n", flags.name);
  //printf("Perms: |%s|\n", flags.perm_mode);
  //printf("Type: |%c|\n", flags.type);
  //printf("Perm args: |%s||%d|\n", flags.perm_mode, flags.perm);

  if(!validArguments(&flags)){
    printf("Invalid Arguments\n");
    return 1;
  }

	DIR *curr_dir;
	struct dirent *dir_info;
	int pid = 0;

  char *directory = malloc(PATH_MAX);
  strcpy(directory,argv[1]);
	strcat(directory,"/");

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

  size_t a_size = sizeof(char*) * (argc + 1);
  char **argv_new = malloc(a_size);
  memcpy(argv_new, argv, a_size);

  while ( (dir_info = readdir(curr_dir) ) != NULL){
		if ( dir_info->d_type == DT_DIR && strcmp(dir_info->d_name,".") && strcmp(dir_info->d_name,"..")){
			if( (pid = fork()) == 0){
        //Calculate new path name
        argv_new[1] = malloc(PATH_MAX);
        strcpy(argv_new[1], argv[1]);
        strcat(argv_new[1], "/");
        strcat(argv_new[1], dir_info->d_name);
        //Execute this program with the new path
        execv(argv_new[0], argv_new);
        perror("Error 3: ");
        exit(3);
      }
      else {
        wait(NULL);;
      }
		}
	}

  rewinddir(curr_dir);
  while ( (dir_info = readdir(curr_dir) ) != NULL){
      //Calculate full path name
      char *full_name = malloc(PATH_MAX);
      strcpy(full_name, argv[1]);
      strcat(full_name, "/");
      strcat(full_name, dir_info->d_name);

      //Calculate permissions
      struct stat buf;
      stat(full_name, &buf);
      int mode = buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

      //Check if the current file/directory has the same properties as the arguments
      char same_name = !strcmp(flags.name, dir_info->d_name);
      char same_perm = flags.perm == mode;
      char same_type = ((dir_info->d_type == DT_REG) && (flags.type == 'f')) ||
                       ((dir_info->d_type == DT_DIR) && (flags.type == 'd')) ||
                       ((dir_info->d_type == DT_LNK) && (flags.type == 'l'));

      //Check if the current file/directory respects every restriction
      char respects_restrictions = (!flags.hasName || (flags.hasName && same_name)) &&
                                   (!flags.hasPerm || (flags.hasPerm && same_perm)) &&
                                   (!flags.hasType || (flags.hasType && same_type));

      //If the file respects all the restrictions, the actions print, delete and exec are eligible
      if(respects_restrictions) {
        //Check if the current file/directory should be printed
        if(flags.toPrint)
          printf("\n%s\n", full_name);

        //Check if the current file/directory should be deleted
        if(flags.toDelete){
          if( (pid = fork()) == 0){
            execlp("rm", "-i", "-r", "-f", full_name, NULL);
            printf("Error in process %d - Deleting file %s\n", getpid(), full_name);
            return 1;
          }
        }

        //Check if the current file/directory should be deleted
        if(flags.hasExec){
          //TODO
        }
      }
		}
    
	return 0;
}
