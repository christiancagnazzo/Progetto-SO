#include "my_lib.h"

int main(){
	int i, sem_id_padre; 
	int mat_id;
	char * matrice;
	int pos;
	int conf_id;
	struct shared_set * set;
	struct msg_p_g mess;
	int ms_id;

	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	
	/* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(char)*set->SO_BASE*set->SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);

	/* CREAZIONE PEDINE */
	for (i = 0; i < set->SO_NUM_P; i++){
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
	
	/* CREO CODA DI MESSAGGI PER COMUNICARE CON LE PEDINE */
	ms_id = msgget(KEY_4, IPC_CREAT | 0666);
	/* ...  */


	
	/*devo aspettare che tutte le pedine si posizionino e poi sblocco il padre*/
printf("sblocco il padre");
	/* SBLOCCO IL MASTER*/
	sem_id_padre = semget(KEY_0,1, 0666);
	sem_reserve(sem_id_padre,0);

	/* ELIMINO SEMAFORI E MEMORIE CONDIVISE*/
	shmdt(matrice);
	shmdt(set);
}