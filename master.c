#include "my_lib.h"
#include "settings.h"

	int main(){
	int sem_id;
	int i;
	
	/* GENERAZIONE SCACCHIERA */
	/* ... */

	/* GENERAZIONE GIOCATORI */  
	for (i = 0; i < set("SO_NUM_G"); i++){
		switch (fork()){
			case -1:
				fprintf(stderr, "Errore nella creazione dei giocatori\n");
				exit(EXIT_FAILURE);

			case 0:
				if (execve("./giocatore",NULL,NULL) == -1){
					fprintf(stderr, "Execve error\n");
					exit(EXIT_FAILURE);
			}
		}
	}

	/* SEMAFORO PER ATTENDERE CHE I GIOCATORI PIAZZINO LE PEDINE */
	sem_id = semget(KEY_0, 1, IPC_CREAT | 0666);
	sem_set_val(sem_id, 0, 5);
	aspetta_zero(sem_id, 0); /* ATTENDE FINCHE' NON VALE 0 */
}