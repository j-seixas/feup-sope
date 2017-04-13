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
  char **exec_arguments;
  char *name, *perm_mode;
  char toPrint, toDelete, hasName, hasType, hasPerm, type;
  int perm;
} Flags;

void initFlags(Flags *flags){
  flags->exec_arguments = NULL;
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

/**
  @brief Replaces old string by new string
  @param str Pointer to string where substrings should be replaced
  @param old_substr Old substring to be replaced
  @param new_substr What to use to replace
  @return 0
*/
int strsubst(char **str , char * old_substr , char * new_substr){
  char * tem = *str;
  int n_replacements = 0;
  while ( (tem = strstr(tem,old_substr)) != NULL) {tem++ ; n_replacements++;};

  int oldstr_size = strlen(old_substr), newstr_size = strlen(new_substr),  
      new_size = strlen(*str)+newstr_size*n_replacements-oldstr_size*n_replacements;
  char *str_ptr, * temp = (char *)malloc(sizeof(char)*new_size), *remaining;
  strcpy(temp,*str);
  str_ptr = temp;

  while ( (str_ptr = strstr(str_ptr,old_substr)) != NULL ){
    int remaining_size = strlen(str_ptr), i=0, j=0;

    remaining = (char *)malloc(sizeof(char)*(remaining_size-oldstr_size));
    strcpy(remaining, &(str_ptr[oldstr_size]) );

    for (i = 0 ; i < newstr_size ; i++)//copy new string
      str_ptr[i] = new_substr[i];

    for(j=0 ; i < new_size ; i++)//copy remaining string
      str_ptr[i] = remaining[j++];
      
  }
  if (n_replacements == 1)
    temp[strlen(temp)-1] = '\0';
  *str = temp;
  return 0;
}


// find ./ -exec echo 'FILE '{}'' \;
/**
  @brief Separates the arguments of -exec
  @param arguments Array with arguments starting with first after -exec all the way to the end of argv
  @param start Position to start searching
  @param length How many positions to search (usually until end of array)
  @return Array with only the arguments of exec (substitutes the ";" by NULL)
*/
char** parseExec(char *arguments[] , int start , int length ){
  int i; 
  char **exec_args = (char **)malloc(sizeof(char*)*length+1); //to add a NULL
  for (i = 0 ; i < length ; i++){
    if ( strcmp(arguments[start+i],";") == 0 ){
      exec_args[i] = NULL;
      break;
    }
    exec_args[i] = arguments[start+i];
  }
  exec_args[i] = NULL;

  return exec_args;
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
      if(isNumber(argv[args+1])){
        strcpy(flags.perm_mode, argv[args+1]);
        flags.perm = strtol(flags.perm_mode, NULL, 8);
      }
      args++;
    }else if(!strcmp(argv[args] , "-type") && args+1 < argc){
      flags.hasType = 1;
      flags.type = argv[args+1][0];
      args++;
    }else if ( strcmp(argv[args] , "-exec" ) == 0)
      flags.exec_arguments = parseExec(argv , args+1 , argc-args-1);

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

        //Check if the current file/directory should be a argument in the exec command
        if(flags.exec_arguments != NULL){
          int n_exec_args = 0;
          while(flags.exec_arguments[n_exec_args++] != NULL) {};
          n_exec_args--; //not NULL
          char **temp_args = (char **)malloc(sizeof(char*)*n_exec_args);
          for( args = 0 ; args < n_exec_args ; args++){
            temp_args[args] = (char*)malloc(sizeof(char)*strlen(flags.exec_arguments[args]));
            unsigned int j = 0;
            for (j = 0 ; j < strlen(flags.exec_arguments[args]) ; j++)
              temp_args[args][j] = flags.exec_arguments[args][j];
          }
          //temp args now contains full copy of exec_args

          //substitute all '{}' by name of file
          for ( args = 0 ; args < n_exec_args ; args++){
            printf("    PASSED = %s\n",full_name);
            strsubst( &temp_args[args] , "{}" , full_name );
            printf("NEW = %s\n",temp_args[args]);
          }
          
          if (fork() == 0)
            execvp(temp_args[0],temp_args);

        }
      }
		}
	return 0;
}
