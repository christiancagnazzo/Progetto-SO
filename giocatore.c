#include "my_lib.h"
#include "settings.h"


int main(){
	int i;
	int sem_id; 
	int mem_id;
	int * matrice;

	setting();

	/* COLLEGAMENTO ALLA SCACCHIERA */
	mem_id = shmget(KEY_1, sizeof(int)*set("SO_BASE")*set("SO_ALTEZZA"), IPC_CREAT | 0666);
	matrice = (int *) shmat(mem_id, NULL, 0);
	

	/* CREAZIONE PEDINE */
	for (i = 0; i < set("SO_NUM_P"); i++){
		switch(fork()){
			case -1:{
				fprintf(stderr, "Errore generazione pedine\n");
				exit(EXIT_FAILURE);
			}
			case 0:
				exit(1); /*temp*/ 
				if (execve("./pedina",NULL,NULL) == -1){
					fprintf(stderr, "Execve error\n");
					exit(EXIT_FAILURE);
				}
		}
	}
	
	/* POSIZIONAMENTO PEDINE */

printf("sblocco il padre\n");
	/* SBLOCCO IL MASTER*/
	sem_id = semget(KEY_0,1, 0666);
	sem_reserve(sem_id,0);

	exit(1);
	/* ASPETTO INIZIO ROUND */
}