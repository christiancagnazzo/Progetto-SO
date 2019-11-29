#include "my_lib.h"

int main(){
	int i;
	int sem_id_padre;
	int mat_id;
	int conf_id;
	char * matrice;
	int sem_id_matrice;
	int x;
	int y;
	int pos;
	struct shared_set  * set;


	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	configure_settings();
	
	/* CONFIGURAZIONE E GENERAZIONE SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(char)*(set->SO_BASE)*(set->SO_ALTEZZA), IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);

	/* CREO SEMAFORI PER SCACCHIERA */
	sem_id_matrice = semget(KEY_3,(set->SO_ALTEZZA*set->SO_BASE), IPC_CREAT | 0666);
	for (i = 0; i < (set->SO_ALTEZZA*set->SO_BASE); i++)
		sem_set_val(KEY_3,i,1);


	/* GENERAZIONE GIOCATORI */  
	for (i = 0; i < set->SO_NUM_G; i++){
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
	sem_id_padre = semget(KEY_0, 1, IPC_CREAT | 0666);
	sem_set_val(sem_id_padre, 0, set->SO_NUM_G);
	aspetta_zero(sem_id_padre, 0); /* ATTENDE FINCHE' NON VALE 0 */	
	
printf("padre sbloccato");

	/* ELIMINO SEMAFORI E MEMORIE CONDIVISE*/
	shmctl(mat_id, IPC_RMID, NULL); 
	shmctl(conf_id, IPC_RMID, NULL);
	shmdt(matrice);
	shmdt(set);
	semctl(sem_id_padre,0,IPC_RMID); /* 0 è ignorato*/
	semctl(sem_id_matrice,0,IPC_RMID);
}

