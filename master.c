#include "my_lib.h"

void alarm_handle(int signal);
void sigint_handle(int signal);

int sem_id_zero, sem_id_mutex, mat_id, conf_id, sem_id_matrice,ms_mg, sem_round, SO_BASE, SO_ALTEZZA, SO_NUM_G, conta_round, SO_N_MOVES, SO_NUM_P;
int * matrice,  * fork_value, * punteggio_g, * mosse_g, *prova;
struct shared_set  * set;
time_t start;
time_t end;
double tempo;

int main(){
	int SO_MAX_TIME, SO_FLAG_MIN, SO_FLAG_MAX, SO_ROUND_SCORE, SO_MIN_HOLD_NSEC;
	int i ,x ,y ,pos ,n_flag;
	int pt_bandierina, pt_totali, media, flag, contamosse;
	struct msg_m_g master_giocatore;
	char * args[2];
	char str[4];

	struct sigaction sa;
	struct sigaction sc;
	bzero(&sa,sizeof(sa));
	sa.sa_handler = alarm_handle;
	sigaction(SIGALRM,&sa,NULL);
	bzero(&sc,sizeof(sc));
	sc.sa_handler = sigint_handle;
	sigaction(SIGINT,&sc,NULL);

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
	fork_value = malloc(sizeof(pid_t)*SO_NUM_G);
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
	for (i = 0; i < SO_NUM_G; i++)
		sem_set_val(sem_id_mutex,i,0);

	/* SEMAFORI ASPETTA ZERO */
	sem_id_zero = semget(KEY_0, 4, IPC_CREAT | 0666);
	
	/* GENERAZIONE GIOCATORI E CREAZIONE CODA PER COMUNICARE */
	ms_mg = msgget(KEY_6, IPC_CREAT | 0666); 
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

	/* CREO SEMAFORO PER GESTIRE I ROUND */
	sem_round = semget(KEY_7,2,0666 | IPC_CREAT);
	sem_set_val(sem_round,0,0);
	
	sem_set_val(sem_id_zero, 1, 0); /* Giocatore aspetterà che la pedina si piazzi */
	sem_set_val(sem_id_mutex,0,1); /* Consente al primo giocatore di piazzare la propria pedina */
	sem_set_val(sem_id_zero, 0, SO_NUM_G); /* SEMAFORO PER ATTENDERE CHE I GIOCATORI PIAZZINO LE PEDINE */
	aspetta_zero(sem_id_zero, 0); /* ATTENDE FINCHE' NON VALE 0 */


	conta_round = 0;
	start=time(NULL);
	while (1){		
		/* PIAZZO BANDIERINE */
		conta_round++;
		pt_totali = SO_ROUND_SCORE;
		srand(time(NULL));
		n_flag = rand()%((SO_FLAG_MAX)-(SO_FLAG_MIN)+1)+(SO_FLAG_MIN);
		flag = n_flag;	
		printf(RED"\nPUNTEGGIO TOTALE PARTITA %d, BANDIERINE TOTALI %d ROUND NUMERO %d\n"RESET,SO_ROUND_SCORE,flag,conta_round);
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
		/*sleep(1); /* PER RALLENTARE IL GIOCO*/
		
		sem_set_val(sem_id_zero,2,1); /* semaforo per far attendere inizio movimento gioco  ai giocatori */	
		sem_set_val(sem_id_zero, 0, SO_NUM_G); 
		sem_set_val(sem_round,0,SO_NUM_G);  /* SBLOCCO L'INIZIO DEI NUOVI ROUND */
		/* ASPETTO CHE I GIOCATORI DANNO LE INDICAZIONI ALLE PEDINE */
		aspetta_zero(sem_id_zero, 0); 

		alarm(SO_MAX_TIME);
		sem_reserve(sem_id_zero,2); /* AVVIO ROUND */	
		
		for (i = 0; i < SO_NUM_G*SO_NUM_P; i++) {
			msgrcv(ms_mg,&master_giocatore,sizeof(int)*3,1,0);
			mosse_g[i] = 0;	
			mosse_g[(-master_giocatore.giocatore)-65]+= master_giocatore.mosse_residue;	
			punteggio_g[(-master_giocatore.giocatore)-65]+= master_giocatore.bandierina;
			if (master_giocatore.bandierina > 0) flag--;
			if (flag == 0) alarm(0);
		}
		while (flag > 0); /* se le bandierine non vengono prese in tempo mi interromperà l'alarm */
				
		stampa_scacchiera(SO_BASE,SO_ALTEZZA);
	
		/* AGGIORNO MOSSE GIOCATORI */
		for (i = 0; i < SO_NUM_G; i++)
			printf(GREEN"giocatore %c punteggio %d mosse %d \n"RESET,65+i,punteggio_g[i],mosse_g[i]);
	
		sem_set_val(sem_round,1,SO_NUM_P*SO_NUM_G); /* Sblocco pedine che si sono fermate alla fine del round*/
		
		/*sleep(1); /*PER RALLENTARE IL GIOCO*/
	}
}

void alarm_handle(int signal){
	int i, punti_totali_g;
	float mosse_tot;
	
	end=time(NULL);
	tempo=((double)(end-start));

	printf(RED"\nPARTITA FINITA\n"RESET);
	punti_totali_g = 0;
	for (i = 0; i < SO_NUM_G; i++){
			printf(GREEN"giocatore %c punteggio %d mosse %d \n"RESET,65+i,punteggio_g[i],mosse_g[i]);
			punti_totali_g+= punteggio_g[i];
	}
	stampa_scacchiera(SO_BASE,SO_ALTEZZA);
	/* STAMPA METRICHE */	
	printf(RED"\n------------- METRICHE -------------\n"RESET);
	printf(YELLOW"\n- NUMERO ROUND GIOCATI: %d\n"RESET,conta_round);
	mosse_tot = SO_NUM_P*SO_N_MOVES;
	printf(GREEN"\n- RAPPORTO MOSSE UTILIZZATE E MOSSE TOTALI\n"RESET);
	for (i = 0; i < SO_NUM_G; i++)	
		printf("	GIOCATORE %c: %f\n",65+i,((SO_N_MOVES*SO_NUM_P)-mosse_g[i])/mosse_tot);
	printf(GREEN"\n- RAPPORTO PUNTI OTTENUTI E MOSSE UTILIZZATE\n"RESET);
	for (i = 0; i < SO_NUM_G; i++)
		printf("	GIOCATORE %c: %f\n",65+i,punteggio_g[i]/(mosse_tot-mosse_g[i]));
	printf(GREEN"\n- RAPPORTO PUNTI TOTALI (%d) E TEMPO DI GIOCO IN SECONDI (%f):"RESET" %f\n\n",punti_totali_g,tempo,punti_totali_g/tempo);
	shmctl(mat_id, IPC_RMID, NULL); 
	shmctl(conf_id, IPC_RMID, NULL);
	semctl(sem_round,0,IPC_RMID);
	semctl(sem_id_zero,0,IPC_RMID); /* 0 è ignorato*/
	semctl(sem_id_matrice,0,IPC_RMID);
	semctl(sem_id_mutex,0,IPC_RMID);
	msgctl(ms_mg,IPC_RMID,NULL);
	shmdt(matrice);
	shmdt(set);
	for (i = 0; i < SO_NUM_G; i++)
		kill(fork_value[i],SIGINT);
	while (wait(NULL) != -1);	
	free(punteggio_g);
	free(mosse_g);
	free(fork_value);
	exit(EXIT_SUCCESS);
}

void sigint_handle(int signal){
	int i;
	printf("\nPARTITA INTERROTTA\n");
	shmctl(mat_id, IPC_RMID, NULL); 
	shmctl(conf_id, IPC_RMID, NULL);
	semctl(sem_round,0,IPC_RMID);
	semctl(sem_id_zero,0,IPC_RMID); /* 0 è ignorato*/
	semctl(sem_id_matrice,0,IPC_RMID);
	semctl(sem_id_mutex,0,IPC_RMID);
	msgctl(ms_mg,IPC_RMID,NULL);
	shmdt(matrice);
	shmdt(set);	
	while (wait(NULL) != -1);	
	exit(EXIT_SUCCESS);
}
