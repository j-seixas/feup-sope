

int main (int argc , char *argv[] ){
	if (argc != 4){
		write("Usage: gerador <n. pedidos> <max. utilizacao> <un. tempo>\n",58);
		exit(1);
	}
	int n_ped = strtol(argv[1],NULL,10),
	    max_ut= strtol(argv[2],NULL,10),
	    un_ped= strtol(argc[3],NULL,10);

}
