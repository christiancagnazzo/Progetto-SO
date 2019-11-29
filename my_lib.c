#include "my_lib.h"

#define TEST_ERROR if (errno) {fprintf(stderr,				\
				       "%s:%d: PID=%5d: Error %d (%s)\n", \
				       __FILE__,			\
				       __LINE__,			\
				       getpid(),			\
				       errno,				\
				       strerror(errno));}

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

/* Rilascio di una risorsa */
int sem_release(int sem_id, int sem_num) {
	struct sembuf sops;
  
	sops.sem_num = sem_num;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	
	return semop(sem_id, &sops, 1);
}


/* Stampa semafori in una stringa */
int sem_getall(char * my_string, int sem_id) {
	union semun arg;   /* man semctl per vedere def della union  */ 
	unsigned short * sem_vals, i;
	unsigned long num_sem;
	char cur_str[10];
	
	semctl(sem_id, 0, IPC_STAT, arg.buf);
	TEST_ERROR;
	num_sem = arg.buf->sem_nsems;
	
	sem_vals = malloc(sizeof(*sem_vals)*num_sem);
	arg.array = sem_vals;
	semctl(sem_id, 0, GETALL, arg);
	
	my_string[0] = 0;
	for (i=0; i<num_sem; i++) {
		sprintf(cur_str, "%d ", sem_vals[i]);
		strcat(my_string, cur_str);
	}
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
	fscanf(fp, "%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d\n%*s\t%*c\t%d",&par[0], &par[1], &par[2], &par[3], &par[4],&par[5], &par[6], &par[7],&par[8],&par[9]);
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


/* INSERIMENTO DATI MATRICE*/
/*	for (pos = 0; pos < set->SO_BASE*set->SO_ALTEZZA; pos++)
		matrice[pos] = '0';

	/* STAMPO MATRICE*/
/*	pos = 0;
	for (x = 0; x < set->SO_ALTEZZA; x++){
		printf("\n");
		for (y = 0; y < set->SO_BASE; y++)
			printf("%c ", matrice[pos++]);
	}
	
*/