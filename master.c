#include "my_lib.h"

void handle_signal(int signal);
int sem_id_zero, sem_id_mutex, mat_id, conf_id, sem_id_matrice,ms_gp,ms_mg, sem_round, SO_BASE, SO_ALTEZZA, SO_NUM_G;
int * matrice, * fork_value;
struct shared_set  * set;

int main(){
	int SO_NUM_P, SO_MAX_TIME, SO_FLAG_MIN, SO_FLAG_MAX, SO_ROUND_SCORE, SO_N_MOVES, SO_MIN_HOLD_NSEC;
	int i,x,y, pos, n_flag, g;
	int pt_bandierina, pt_totali, media, flag, contamosse;
	int * punteggio_g, * mosse_g;
	struct msg_m_g master_giocatore;
	char * args[2];
	char str[4];

	struct sigaction sa;
	bzero(&sa,sizeof(sa));
	sa.sa_handler = handle_signal;
	/*sigaction(SIGINT,&sa,NULL);*/
	sigaction(SIGALRM,&sa,NULL);

	/* CONFIGURAZIONE E GENERAZIONE SCACCHIERA */
	args[1] = NULL;
	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	configure_settings();
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	SO_NUM_G = set->SO_NUM_G;
	SO_NUM_P = set->SO_NUM_P;
	SO_MAX_TIME = set->SO_MAX_TIME;
	SO_BASE = set->SO_BASE;
	SO_ALTEZZA = set->SO_ALTEZZA;
	SO_FLAG_MIN = set->SO_FLAG_MIN;
	SO_FLAG_MAX = set->SO_FLAG_MAX;
	SO_ROUND_SCORE = set->SO_ROUND_SCORE;
	SO_N_MOVES = set->SO_N_MOVES;
	SO_MIN_HOLD_NSEC = set->SO_MIN_HOLD_NSEC;
	mat_id = shmget(KEY_1, (sizeof(int)*(SO_BASE)*(SO_ALTEZZA)), IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);

	/* SETTAGGIO INIZIALE MATRICE E PUNTEGGIO/MOSSE GIOCATORI */
	for (pos = 0; pos < SO_BASE*SO_ALTEZZA; pos++)
		matrice[pos] = 0;
	punteggio_g = malloc(sizeof(int)*SO_NUM_G);
	mosse_g = malloc(sizeof(int)*SO_NUM_G);
	for (i = 0; i < SO_NUM_G; i++){
		punteggio_g[i] = 0;
		mosse_g[i] = 0;
	}

	/* CREO SEMAFORI PER SCACCHIERA */
	sem_id_matrice = semget(KEY_3,(SO_ALTEZZA*SO_BASE), IPC_CREAT | 0666);
	for (i = 0; i < (SO_ALTEZZA*SO_BASE); i++)
		sem_set_val(sem_id_matrice,i,1);

	/* SEMAFORO MUTUA ESCLUSIONE: giocatori piazzano a turno le pedine */
	sem_id_mutex = semget(KEY_5,SO_NUM_G, IPC_CREAT | 0666); 
	sem_set_val(sem_id_mutex,0,1);
	for (i = 1; i < SO_NUM_G; i++)
		sem_set_val(sem_id_mutex,i,0);

	/* SEMAFORI ASPETTA ZERO */
	sem_id_zero = semget(KEY_0, 4, IPC_CREAT | 0666);

	/* GENERAZIONE GIOCATORI E CREAZIONE CODA PER COMUNICARE */
	ms_mg = msgget(KEY_6, IPC_CREAT | 0666);
	fork_value = malloc(sizeof(int)*SO_NUM_G);
	for (i = 0; i < SO_NUM_G; i++){
		switch (fork_value[i] = fork()){
			case -1:
				fprintf(stderr, "Errore nella creazione dei giocatori\n");
				exit(EXIT_FAILURE);

			case 0:
				sprintf(str,"%d", 65 + i);
				args[0] = str;
				if (execve("./giocatore",args,NULL) == -1){
					fprintf(stderr, "Execve error\n");
					exit(EXIT_FAILURE);
			}
		}
	}

	sem_round = semget(KEY_7,2,0666 | IPC_CREAT);
	sem_set_val(sem_round,0,1);
	
	contamosse = 1;
	while (contamosse > 0){
	/* SEMAFORO PER ATTENDERE CHE I GIOCATORI PIAZZINO LE PEDINE */
	sem_set_val(sem_id_zero, 0, SO_NUM_G);
	sem_set_val(sem_id_zero,2,1);
	
	sem_reserve(sem_round,0);
	aspetta_zero(sem_id_zero, 0); /* ATTENDE FINCHE' NON VALE 0 */

	/* PIAZZO BANDIERINE */
	pt_totali = SO_ROUND_SCORE;
	srand(time(NULL));
	n_flag = rand()%((SO_FLAG_MAX)-(SO_FLAG_MIN)+1)+(SO_FLAG_MIN);
	flag = n_flag;	
	printf("\nPUNTEGGIO TOTALE PARTITA %d, BANDIERINE TOTALI %d\n",SO_ROUND_SCORE,flag);
	while ((n_flag) > 0){	
		if (n_flag == 1)
			pt_bandierina = pt_totali;
		else {
			media = pt_totali/n_flag;
			pt_bandierina = rand()% media + 1;
		}
		do{
			x = rand() % SO_ALTEZZA;
			y = rand() % SO_BASE;			
		}
		while (matrice[posizione(x,y,SO_BASE)] != 0);
		matrice[posizione(x,y,SO_BASE)] = pt_bandierina;
		pt_totali = pt_totali - pt_bandierina;
		n_flag--;
	}

	stampa_scacchiera(SO_BASE,SO_ALTEZZA);
	
	sem_set_val(sem_id_zero, 0, SO_NUM_G); 
	sem_reserve(sem_id_zero,2); /* sblocco i giocatori */
		
	/* ASPETTO CHE I GIOCATORI DANNO LE INDICAZIONI ALLE PEDINE */
	aspetta_zero(sem_id_zero, 0); /* ATTENDE FINCHE' NON VALE 0 */


	alarm(SO_MAX_TIME);
	sem_reserve(sem_id_zero,2); /* AVVIO ROUND */	

	for (i = 0; i < SO_NUM_G*SO_NUM_P; i++) {
		msgrcv(ms_mg,&master_giocatore,sizeof(int)*3,1,0);
		mosse_g[i] = 0;
		mosse_g[(-master_giocatore.giocatore)-65]+= master_giocatore.mosse_residue;
		punteggio_g[(-master_giocatore.giocatore)-65]+= master_giocatore.bandierina;
		if (master_giocatore.bandierina > 0) flag--;
		}
	while (flag > 0);
	alarm(0);
	
	stampa_scacchiera(SO_BASE,SO_ALTEZZA);
/*i =0;
for (x = 0; x<SO_ALTEZZA;x++){
	printf("\n");
	for (y= 0; y<SO_BASE; y++){
		printf("%d ",semctl(sem_id_matrice,i++,GETVAL));
	}
}*/
	contamosse = 0;
	for (i = 0; i < SO_NUM_G; i++){
		printf("giocatore %c punteggio %d mosse %d \n",65+i,punteggio_g[i],mosse_g[i]);
		contamosse = contamosse + mosse_g[i];
		}	
}
	for (i = 0; i < SO_NUM_G; i++)
		kill(fork_value[i],SIGINT);
	/* ELIMINO SEMAFORI E MEMORIE CONDIVISE*/
	printf("\n");
	shmctl(mat_id, IPC_RMID, NULL); 
	shmctl(conf_id, IPC_RMID, NULL);
	shmdt(matrice);
	shmdt(set);
	semctl(sem_id_zero,0,IPC_RMID); /* 0 è ignorato*/
	semctl(sem_id_matrice,0,IPC_RMID);
	semctl(sem_id_mutex,0,IPC_RMID);
	semctl(sem_round,0,IPC_RMID);
	msgctl(ms_mg,IPC_RMID,NULL);
}


void handle_signal(int signal){
	int i;
	printf("PARTITA FINITA\n");
	stampa_scacchiera(SO_BASE,SO_ALTEZZA);
	for (i = 0; i < SO_NUM_G; i++)
		kill(fork_value[i],SIGINT);
	shmctl(mat_id, IPC_RMID, NULL); 
	shmctl(conf_id, IPC_RMID, NULL);
	semctl(sem_round,0,IPC_RMID);
	semctl(sem_id_zero,0,IPC_RMID); /* 0 è ignorato*/
	semctl(sem_id_matrice,0,IPC_RMID);
	semctl(sem_id_mutex,0,IPC_RMID);
	msgctl(ms_mg,IPC_RMID,NULL);
	shmdt(matrice);
	shmdt(set);
	exit(1);
}