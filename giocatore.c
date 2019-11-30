#include "my_lib.h"

int main(){
	int i, sem_id_zero, sem_id_mutex, mat_id, x,y,g, conf_id, ms_gp;
	char * matrice;
	struct shared_set * set;
	struct msg_p_g gioc_pedina;
	int * fork_value;
	char id_giocatore;

	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	
	/* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(char)*set->SO_BASE*set->SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);

	/* da sistemare (rischio lettera uguale) */
	srand(getpid());
	id_giocatore = 97+rand() % 26;
	
	/* CREAZIONE PEDINE */
	fork_value = malloc(sizeof(int)*set->SO_NUM_P);
	for (i = 0; i < set->SO_NUM_P; i++){
		switch(fork_value[i] = fork()){
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
	/* E INVIO LORO LA POSIZIONE */
	ms_gp = msgget(KEY_4, IPC_CREAT | 0666);

	/* SEMAFORO PER LA MUTUA ESCLUSIONE */
	sem_id_mutex = semget(KEY_5,2, IPC_CREAT | 0666);

	/* SEZIONE CRITICA */
	sem_reserve(sem_id_mutex,0);
	for (i = 0; i < set->SO_NUM_P; i++){	
		srand(fork_value[i]);
		x = rand() % (set->SO_ALTEZZA);
		y = rand() % (set->SO_BASE);	
		gioc_pedina.type = fork_value[i];
		gioc_pedina.pos = posizione(x,y,set->SO_BASE);
		gioc_pedina.giocatore = id_giocatore;
		gioc_pedina.mosse = set->SO_N_MOVES;
		msgsnd(ms_gp,&gioc_pedina,((sizeof(int)*2)+sizeof(char)),0);
	}
	/* SEMAFORO PER ATTENDERE CHE LE MIE PEDINE SI PIAZZINO */
	sem_id_zero = semget(KEY_0, 2, IPC_CREAT | 0666);
	sem_set_val(sem_id_zero, 1, set->SO_NUM_P);
	aspetta_zero(sem_id_zero, 1); /* ATTENDE FINCHE' NON VALE 0 */
	sem_release(sem_id_mutex,0);
	/* FINE SEZIONE CRITICA*/
	
	/* SBLOCCO IL MASTER E DO IL MIO STATO */
	sem_id_zero = semget(KEY_0,2, 0666);
	sem_reserve(sem_id_zero,0);
}