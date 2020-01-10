#include "my_lib.h"

/* Inizializzazione semaforo */
int sem_set_val(int sem_id, int sem_num, int sem_val) {
	return semctl(sem_id, sem_num, SETVAL, sem_val);
}

/* Richiesta di accesso a risorsa */
int sem_reserve(int sem_id, int sem_num) {
	struct sembuf sops;
	
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

/* Richiesta di accesso a risorsa senza aspettare se non c'è */
int sem_reserve_nowait(int sem_id, int sem_num) {
	struct sembuf sops;
	
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = IPC_NOWAIT;
	return semop(sem_id, &sops, 1);
}

/* Richiesta di accesso a risorsa aspettando per un limite di tempo */
int sem_reserve_wait_time(int sem_id, int sem_num){
	struct sembuf sops;
	struct timespec ts;

	ts.tv_sec = 0;
    ts.tv_nsec = 1000000;

	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semtimedop(sem_id,&sops,1,&ts);
}

/* Rilascio di una risorsa */
int sem_release(int sem_id, int sem_num) {
	struct sembuf sops;
  
	sops.sem_num = sem_num;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	
	return semop(sem_id, &sops, 1);
}

/* Aspetta che il semaforo valga zero*/
int aspetta_zero(int sem_id, int sem_num) {
    struct sembuf sops;
  
	sops.sem_num = sem_num;
	sops.sem_op = 0;
	sops.sem_flg = 0;
	
	return semop(sem_id, &sops, 1);   
}


/* IMPOSTAZIONI DI GIOCO*/
void configure_settings() {
	struct shared_set  * set;
	int i = 0;
	int mem_id;
	int * par;
    FILE *fp;
	
	par = malloc(sizeof(int)*10);
	fp = fopen("settings.conf", "r");
	fscanf(fp, "%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d",
													&par[0], &par[1], &par[2], &par[3], &par[4],&par[5], &par[6], &par[7],&par[8],&par[9]);
    fclose(fp);
	mem_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0666);
	set = shmat(mem_id, NULL, 0);
	set->SO_NUM_G = par[0];
	set->SO_NUM_P = par[1];
	set->SO_MAX_TIME = par[2];
	set->SO_BASE = par[3];
	set->SO_ALTEZZA = par[4];
	set->SO_FLAG_MIN = par[5];
	set->SO_FLAG_MAX = par[6];
	set->SO_ROUND_SCORE = par[7];
	set->SO_N_MOVES = par[8];
	set->SO_MIN_HOLD_NSEC = par[9];
}

int posizione(int r, int c, int col){
	/* numero di riga * numero di colonne totali + numero di colonna*/
	return r * col + c;
}

void stampa_scacchiera(int base, int altezza){
	int pos, i, y,x, mat_id;
	int * matrice;

	pos = 0;
	
	mat_id = shmget(KEY_1, sizeof(int)*(base)*(altezza), IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);


	printf("\n");
	if (base < 99) {
		for (i = 0; i < 10; i++) printf(BLUE" %d ",i);
		for (i = 10; i < base; i++) printf(BLUE" %d",i);
	}
	printf("\n");
	for (i = 0; i < base; i++) printf(BLUE" __"RESET);
	printf("\n");
	for (x = 0; x < altezza; x++){
		for (y = 0; y < base; y++){
			if (matrice[pos] < 0)
				printf(BLUE"|"RESET"%c ", -(matrice[pos++])); /* pedina */ 
			else
				if (matrice[pos] > 0 && matrice[pos] < 10 ) 	
					printf(BLUE"|"YELLOW"%d " RESET , matrice[pos++]); /* bandierina con relativo punteggio */
				else if (matrice[pos] >= 10)
					printf(BLUE"|"YELLOW "%d" RESET, matrice[pos++]); 
				else	
					printf(BLUE"|  "RESET, matrice[pos++]);	 /* casella vuota */
		}
		printf(BLUE"|"BLUE"%d\n"RESET,x);
		for (i = 0; i < base; i++) printf(BLUE" __"RESET);
		printf(BLUE"|\n"RESET);
	}
}


