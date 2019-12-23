#include "my_lib.h"

int * fork_value;
int ms_gp, SO_NUM_P;
void handle_signal(int signal);


int main(int argc, const char * args[]){
	int SO_BASE, SO_ALTEZZA, SO_NUM_G, SO_N_MOVES, SO_FLAG_MAX;
	int i, sem_id_zero, sem_id_mutex, mat_id, x,y,g,c,r,rit, conf_id, ms_mg;
	int sem_id_matrice, distanza_min, r_flag, c_flag, b, cont, del, sem_round;
	int * matrice, * pos_r, * pos_c, * band_r, * band_c;
	struct shared_set * set;
	struct msg_p_g gioc_pedina;
	struct statogiocatore giocatore;
	struct msg_m_g master_giocatore;

	struct sigaction sa1;
	bzero(&sa1,sizeof(sa1));
	sa1.sa_handler = handle_signal;
	sigaction(SIGINT,&sa1,NULL);

	/* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	SO_NUM_G = set->SO_NUM_G;
	SO_NUM_P = set->SO_NUM_P;
	SO_BASE = set->SO_BASE;
	SO_ALTEZZA = set->SO_ALTEZZA;
	SO_N_MOVES = set->SO_N_MOVES;
	SO_FLAG_MAX = set->SO_FLAG_MAX;
	mat_id = shmget(KEY_1, sizeof(int)*SO_BASE*SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);
	sem_id_matrice = semget(KEY_3,(SO_ALTEZZA*SO_BASE), IPC_CREAT | 0666);

	/* AGGIORNAMENTO STATO GIOCATORE */
	giocatore.giocatore = -(atoi(args[0]));
	giocatore.id = getpid();
	giocatore.mosse_residue = 0;
	giocatore.punteggio = 0;

	/* CREO CODA DI MESSAGGI PER COMUNICARE CON LE PEDINE  E CON IL MASTER */
	ms_mg = msgget(KEY_6, IPC_CREAT | 0666);
	ms_gp = msgget(getpid(), IPC_CREAT | 0666);
	/* SEMAFORO PER LA MUTUA ESCLUSIONE */
	sem_id_mutex = semget(KEY_5,SO_NUM_G, IPC_CREAT | 0666); /* giocatore a turno piazza pedine */
	/* SEMAFORI ASPETTA ZERO */
	sem_id_zero = semget(KEY_0, 4, IPC_CREAT | 0666);
	
	/* CREAZIONE PEDINE */
	fork_value = malloc(sizeof(int)*SO_NUM_P);
	for (i = 0; i < SO_NUM_P; i++){
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
	x = 0;
	y = 0;
	/* INDICAZIONI INIZIALI PEDINE */	
	pos_r = malloc(sizeof(int)*SO_NUM_P); /* righe mie pedine */
	pos_c = malloc(sizeof(int)*SO_NUM_P); /* colonne mie pedine */	
	for (i = 0; i < SO_NUM_P; i++){	
		if (SO_NUM_G == 2 && SO_BASE == 60 && SO_ALTEZZA == 20){
			switch(-giocatore.giocatore){
				case 65:	
					if (i < 6){
						if (i == 0) y = 6;
						else if (y >= 54) y = 6;
						else y = y + 24;
						if (i < 3) x = 3;
						else x = 11;
					}
					else {
						if (i % 2 == 0) y = 18;
						else y = 42;
						if (i < 8) x = 7;
						else x = 16;
					}
					break;
				case 66:
					if (i < 6){
						if (i == 0) y = 6;
						else if (y >= 54) y = 6;
						else y = y + 24;
						if (i < 3) x = 7;
						else x = 16;
					}
					else {
						if (i % 2 == 0) y = 18;
						else y = 42;
						if (i < 8) x = 3;
						else x = 11;
					}
					break;
				}
			}
			else {
				srand(fork_value[i]);
				x = rand() % (SO_ALTEZZA);
				y = rand() % (SO_BASE);
			}
		sem_reserve(sem_id_mutex,(-giocatore.giocatore)-65);
		rit = sem_reserve_nowait(sem_id_matrice,posizione(x,y,SO_BASE));	
		while (rit == -1 && errno == EAGAIN){
    	    if (posizione(x,y,SO_BASE) == ((SO_BASE*SO_ALTEZZA)-1)){
        	    x = 0;
				y = 0; /* se sono alla fine riparto dall'inizio*/
        	}else 
            	y = (y+1); /* provo ad andare avanti */
        rit = sem_reserve_nowait(sem_id_matrice,posizione(x,y,SO_BASE));
    	}	
		pos_r[i] = x;
		pos_c[i] = y; 
		gioc_pedina.type = fork_value[i];
		gioc_pedina.r = x;	
		gioc_pedina.c = y;	
		gioc_pedina.giocatore = giocatore.giocatore;
		gioc_pedina.mosse = SO_N_MOVES;
		sem_set_val(sem_id_zero, 1, 1);
		msgsnd(ms_gp,&gioc_pedina,((sizeof(int)*7)),0);
		aspetta_zero(sem_id_zero, 1); /* ATTENDE FINCHE' NON VALE 0 */
		sem_release(sem_id_mutex,((-giocatore.giocatore)-65+1)%SO_NUM_G);
	}
	
	sem_round = semget(KEY_7,2, 0666 | IPC_CREAT);	
	sem_set_val(sem_round,1,SO_NUM_G);

	band_r = malloc(sizeof(int)); /* righe bandierine */
	band_c = malloc(sizeof(int)); /* colonne bandierine */	
while(1){
	/* SBLOCCO IL MASTER E DO IL MIO STATO */
	sem_reserve(sem_id_zero,0);
	/* ASPETTO VIA LIBERA DAL MASTER */
	aspetta_zero(sem_id_zero,2);
	sem_reserve(sem_round,1);
	
	sem_set_val(sem_id_zero, 3, SO_NUM_G); /* SEMAFORO PER FAR ASPETTARE ALLE PEDINE L'INIZIO DEL GIOCO */
	cont = 0;
	i = 0;
	for (r = 0; r < SO_ALTEZZA; r++){
		for (c = 0; c < SO_BASE; c++){
			if (matrice[posizione(r,c,SO_BASE)] > 0){
				cont++;
				band_r = realloc(band_r,sizeof(int)*cont);
				band_c = realloc(band_c,sizeof(int)*cont);
				band_r[i] = r;
				band_c[i++] = c;
			}
		}
	}

	distanza_min = (SO_N_MOVES+1);
	for (i = 0; i < SO_NUM_P; i++){
		for (b = 0; b < cont; b++ ){
			if ((((abs(pos_r[i]-band_r[b]))+(abs(pos_c[i]-band_c[b]))) <= SO_N_MOVES) 
						&& (((abs(pos_r[i]-band_r[b]))+(abs(pos_c[i]-band_c[b]))) < distanza_min))
				if (band_r[b] >= 0){	
					distanza_min = ((abs(pos_r[i]-band_r[b]))+(abs(pos_c[i]-band_c[b])));
					del = b;
					r_flag = band_r[b];
					c_flag = band_c[b];
				}
		}
		if (distanza_min > SO_N_MOVES){
			gioc_pedina.r_b = pos_r[i];
			gioc_pedina.c_b = pos_c[i];
		}
		else{	
			gioc_pedina.r_b = r_flag;
			gioc_pedina.c_b = c_flag;
			if ((SO_NUM_P*SO_NUM_G)/cont < 10) band_r[del] = -1;
		}
		gioc_pedina.type = fork_value[i];
		msgsnd(ms_gp,&gioc_pedina,((sizeof(int)*7)),0);
		distanza_min = (SO_N_MOVES+1);
	}

	sem_set_val(sem_id_zero,2,1); /* semaforo per attendere inizio gioco */	
	/* SBLOCCO IL MASTER */
	sem_reserve(sem_id_zero,0);
	
	/* ATTENDO INIZIO GIOCO */
	aspetta_zero(sem_id_zero,2);
 
	/* SBLOCCO MOVIMENTO PEDINE E MI METTO IN READ*/
	sem_reserve(sem_id_zero,3);	
	for (i = 0; i < SO_NUM_P; i++){
		msgrcv(ms_gp,&gioc_pedina,sizeof(int)*7,fork_value[i],0);
		master_giocatore.type = 1;
		master_giocatore.giocatore = giocatore.giocatore;
		master_giocatore.bandierina = gioc_pedina.bandierina;
		master_giocatore.mosse_residue = gioc_pedina.mosse;
		msgsnd(ms_mg,&master_giocatore,sizeof(int)*3,0);
		giocatore.mosse_residue+= gioc_pedina.mosse;
		giocatore.punteggio+= gioc_pedina.bandierina; 
	}
	sem_set_val(sem_round,0,1);
	aspetta_zero(sem_round,0);
}
	for (i = 0; i < 2; i++)
		kill(fork_value[i],SIGINT);
	msgctl(ms_gp,IPC_RMID,NULL);
}

void handle_signal(int signal){
	int i;
	msgctl(ms_gp,IPC_RMID,NULL);
	for (i = 0; i < SO_NUM_P; i++)
		kill(fork_value[i],SIGINT);
	exit(1);	
}