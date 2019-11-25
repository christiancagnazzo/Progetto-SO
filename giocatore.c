#include "settings.h"
#include "my_lib.h"

int main(){
	/* CREAZIONE PEDINE */
	int i;
	for (i = 0; i < SO_NUM_P; i++){
		switch(fork()){
			case -1:{
				fprintf(stderr, "Errore generazione pedine\n");
				exit(EXIT_FAILURE);
			}

			case 0:
				if (execve("./pedina",NULL,NULL) == -1){
					fprintf(stderr, "Execve error\n");
					exit(EXIT_FAILURE);
				}	
		}
	}
	
	/* POSIZIONAMENTO PEDINA
	...
	*/
	
	/* SBLOCCO IL MASTER*/
	int id = semget(KEY_1,1, 0666);
	sem_reserve(id,0);
	exit(1);
}