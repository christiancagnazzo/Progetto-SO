#include "my_lib.h"

int main(int argc, const char * args[]){
	int i, sem_id_zero, sem_id_mutex, mat_id, x,y,g,c,r,rit, conf_id, ms_gp, ms_mg;
	int sem_id_matrice, distanza_min, r_flag, c_flag;
	int * matrice;
	struct shared_set * set;
	struct msg_p_g gioc_pedina;
	int * fork_value;
	char id_giocatore;
	struct statogiocatore giocatore;
	struct msg_m_g master_giocatore;
	int * pos_r, * pos_c;

	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	
	/* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(char)*set->SO_BASE*set->SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);
	sem_id_matrice = semget(KEY_3,(set->SO_ALTEZZA*set->SO_BASE), IPC_CREAT | 0666);


	id_giocatore = -(atoi(args[0]));
	giocatore.id = getpid();
	giocatore.giocatore = id_giocatore;
	giocatore.mosse_residue = (set->SO_N_MOVES*set->SO_NUM_P);
	giocatore.punteggio = 0;
	
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

	/* SEMAFORO PER LA MUTUA ESCLUSIONE PEDINE */
	sem_id_mutex = semget(KEY_5,1, IPC_CREAT | 0666);

	pos_r = malloc(sizeof(int)*set->SO_NUM_P);
	pos_c = malloc(sizeof(int)*set->SO_NUM_P);

	/* SEZIONE CRITICA */	
	sem_reserve(sem_id_mutex,0);		
	for (i = 0; i < set->SO_NUM_P; i++){	
		srand(fork_value[i]);
		x = rand() % (set->SO_ALTEZZA);
		y = rand() % (set->SO_BASE);
		rit = sem_reserve_nowait(sem_id_matrice,posizione(x,y,set->SO_BASE));
    	while (rit == -1 && errno == EAGAIN){
    	    if (posizione(x,y,set->SO_BASE) == ((set->SO_BASE*set->SO_ALTEZZA)-1)){
        	    x = 0;
				y = 0; /* se sono alla fine riparto dall'inizio*/
        	}else 
            	y = (y+1); /* provo ad andare avanti */
        rit = sem_reserve_nowait(sem_id_matrice,posizione(x,y,set->SO_BASE));
    	}	
		pos_r[i] = x;
		pos_c[i] = y; 
		gioc_pedina.type = fork_value[i];
		gioc_pedina.r = x;	
		gioc_pedina.c = y;	
		gioc_pedina.giocatore = id_giocatore;
		gioc_pedina.mosse = set->SO_N_MOVES;
		msgsnd(ms_gp,&gioc_pedina,((sizeof(int)*6)),0);
	}
	/* SEMAFORO PER ATTENDERE CHE LE MIE PEDINE SI PIAZZINO */
	sem_id_zero = semget(KEY_0, 3, IPC_CREAT | 0666);
	sem_set_val(sem_id_zero, 1, set->SO_NUM_P);
	aspetta_zero(sem_id_zero, 1); /* ATTENDE FINCHE' NON VALE 0 */
	sem_release(sem_id_mutex,0);
	/* FINE SEZIONE CRITICA*/
	
	/* SBLOCCO IL MASTER E DO IL MIO STATO */
	ms_mg = msgget(KEY_6, IPC_CREAT | 0666);
	master_giocatore.type = giocatore.id; /* PID */
	master_giocatore.giocatore = giocatore.giocatore;
	master_giocatore.mosse_residue = giocatore.mosse_residue;
	master_giocatore.punteggio = giocatore.punteggio;
	msgsnd(ms_mg,&master_giocatore,((sizeof(int)*3)),0);
	sem_reserve(sem_id_zero,0);

	/* ASPETTO VIA LIBERA DAL MASTER */
	aspetta_zero(sem_id_zero,2);
	sem_set_val(sem_id_zero, 1, set->SO_NUM_P); /* SEMAFORO PER ASPETTARE LE PEDINE */
	
	distanza_min = (set->SO_N_MOVES+1);
	for (i = 0; i < set->SO_NUM_P; i++){	
		for (r = 0; r < set->SO_ALTEZZA; r++){
			for (c = 0; c < set->SO_BASE; c++){
				if (matrice[posizione(r,c,set->SO_BASE)] > 0){
					if ((abs(pos_r[i]-r)+abs(pos_c[i]-c)) <= set->SO_N_MOVES && ((abs(pos_r[i]-r)+abs(pos_c[i]-c)) < distanza_min)){
						distanza_min = (abs(pos_r[i]-r)+abs(pos_c[i]-c)); 
						r_flag = r;
						c_flag = c;
					}				
				}
			}
		}
		if (distanza_min == set->SO_N_MOVES+1){
			gioc_pedina.r_b = pos_r[i];
			gioc_pedina.c_b = pos_c[i];
		}
		else{	
			gioc_pedina.r_b = r_flag;
			gioc_pedina.c_b = c_flag;
		}
		gioc_pedina.type = fork_value[i];
		msgsnd(ms_gp,&gioc_pedina,((sizeof(int)*6)),0);
		distanza_min = (set->SO_N_MOVES+1);
	}
	
	aspetta_zero(sem_id_zero, 1);
	
	/* SBLOCCO IL MASTER */
	sem_reserve(sem_id_zero,0);
	/* e aspetto l'inizio del gioco*/
}