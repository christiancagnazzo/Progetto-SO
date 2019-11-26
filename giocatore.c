#include "settings.h"
#include "my_lib.h"

int main(){
	int i;
	int sem_id; 

	/* CREAZIONE PEDINE */
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
	
	/* POSIZIONAMENTO PEDINE */

	/* SBLOCCO IL MASTER*/
	sem_id = semget(KEY_1,1, 0666);
	sem_reserve(sem_id,0);

	/* ASPETTO INIZIO ROUND */
}