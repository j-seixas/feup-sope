#include <stdio.h>
#include <string.h>


int main(int argc, char* argv[]){
	
	if(argc == 3){
		FILE *f1, *f2;
		f1 = fopen(argv[1], "r");
		f2 = fopen(argv[2], "w");

		char buffer[256];
		int var;

		while(0 < (var = fread(buffer, 1, sizeof(buffer), f1))){
		
			fwrite(buffer,1, var, f2);

		}

		fclose(f1);
		fclose(f2);
	}
return 0;

}