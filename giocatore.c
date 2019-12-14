#include "my_lib.h"

int main(int argc, const char * args[]){
	int i, sem_id_zero, sem_id_mutex, mat_id, x,y,g,c,r,rit, conf_id, ms_gp, ms_mg;
	int sem_id_matrice, distanza_min, r_flag, c_flag, b, cont, del;
	int * matrice;
	struct shared_set * set;
	struct msg_p_g gioc_pedina;
	int * fork_value;
	struct statogiocatore giocatore;
	struct msg_m_g master_giocatore;
	int * pos_r, * pos_c, * band_r, * band_c;

	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */

	/* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(int)*set->SO_BASE*set->SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);
	sem_id_matrice = semget(KEY_3,(set->SO_ALTEZZA*set->SO_BASE), IPC_CREAT | 0666);

	giocatore.giocatore = -(atoi(args[0]));
	giocatore.id = getpid();
	giocatore.mosse_residue = 0;
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
	ms_gp = msgget(getpid(), IPC_CREAT | 0666);

	/* SEMAFORO PER LA MUTUA ESCLUSIONE */
	sem_id_mutex = semget(KEY_5,set->SO_NUM_G, IPC_CREAT | 0666); /* giocatore a turno piazza pedine */

	pos_r = malloc(sizeof(int)*set->SO_NUM_P);
	pos_c = malloc(sizeof(int)*set->SO_NUM_P);

	sem_id_zero = semget(KEY_0, 4, IPC_CREAT | 0666);

	/* INDICAZIONI INIZIALI PEDINE */		
	for (i = 0; i < set->SO_NUM_P; i++){	
		srand(fork_value[i]);
		x = rand() % (set->SO_ALTEZZA);
		y = rand() % (set->SO_BASE);
		sem_reserve(sem_id_mutex,(-giocatore.giocatore)-65);
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
		gioc_pedina.giocatore = giocatore.giocatore;
		gioc_pedina.mosse = set->SO_N_MOVES;
		sem_set_val(sem_id_zero, 1, 1);
		msgsnd(ms_gp,&gioc_pedina,((sizeof(int)*7)),0);
		/* SEMAFORO PER ATTENDERE CHE LE MIE PEDINE SI PIAZZINO */
		aspetta_zero(sem_id_zero, 1); /* ATTENDE FINCHE' NON VALE 0 */	
		sem_release(sem_id_mutex,((-giocatore.giocatore)-65+1)%set->SO_NUM_G);
	}
	
	/* SBLOCCO IL MASTER E DO IL MIO STATO */
	ms_mg = msgget(KEY_6, IPC_CREAT | 0666);
	master_giocatore.type = giocatore.id; /* PID */
	master_giocatore.giocatore = giocatore.giocatore;
	master_giocatore.mosse_residue = giocatore.mosse_residue;
	master_giocatore.bandierina = 0;
	msgsnd(ms_mg,&master_giocatore,((sizeof(int)*3)),0);
	sem_reserve(sem_id_zero,0);

	/* ASPETTO VIA LIBERA DAL MASTER */
	aspetta_zero(sem_id_zero,2);
	
	sem_set_val(sem_id_zero, 3, set->SO_NUM_G); /* SEMAFORO PER FAR ASPETTARE ALLE PEDINE L'INIZIO ROUND */
	sem_set_val(sem_id_zero, 1, set->SO_NUM_P); /* SEMAFORO PER ASPETTARE LE PEDINE */
	band_r = malloc(sizeof(int)*set->SO_FLAG_MAX);
	band_c = malloc(sizeof(int)*set->SO_FLAG_MAX);
	cont = 0;
	i = 0;
	for (r = 0; r < set->SO_ALTEZZA; r++){
		for (c = 0; c < set->SO_BASE; c++){
			if (matrice[posizione(r,c,set->SO_BASE)] > 0){
				cont++;
				band_r[i] = r;
				band_c[i++] = c;
			}
		}
	}	

	distanza_min = (set->SO_N_MOVES+1);
	for (i = 0; i < set->SO_NUM_P; i++){
		for (b = 0; b < cont; b++ ){
			if ((((abs(pos_r[i]-band_r[b]))+(abs(pos_c[i]-band_c[b]))) <= set->SO_N_MOVES) 
						&& (((abs(pos_r[i]-band_r[b]))+(abs(pos_c[i]-band_c[b]))) < distanza_min))
				if (band_r[b] >= 0){	
					distanza_min = ((abs(pos_r[i]-band_r[b]))+(abs(pos_c[i]-band_c[b])));
					del = b;
					r_flag = band_r[b];
					c_flag = band_c[b];
				}
		}
		if (distanza_min > set->SO_N_MOVES){
			gioc_pedina.r_b = pos_r[i];
			gioc_pedina.c_b = pos_c[i];
		}
		else{	
			gioc_pedina.r_b = r_flag;
			gioc_pedina.c_b = c_flag;
			band_r[del] = -1;
		}
		gioc_pedina.type = fork_value[i];
		msgsnd(ms_gp,&gioc_pedina,((sizeof(int)*7)),0);
		distanza_min = (set->SO_N_MOVES+1);
	}

	sem_set_val(sem_id_zero,2,1);	
	/* SBLOCCO IL MASTER */
	sem_reserve(sem_id_zero,0);
	
	/* ATTENDO INIZIO GIOCO */
	aspetta_zero(sem_id_zero,2);

	/* SBLOCCO MOVIMENTO PEDINE E MI METTO IN READ*/
	sem_reserve(sem_id_zero,3);	
	for (i = 0; i < set->SO_NUM_P; i++){
		msgrcv(ms_gp,&gioc_pedina,sizeof(int)*7,fork_value[i],0);
		master_giocatore.type = 1;
		master_giocatore.giocatore = giocatore.giocatore;
		master_giocatore.bandierina = gioc_pedina.bandierina;
		master_giocatore.mosse_residue = gioc_pedina.mosse;
		msgsnd(ms_mg,&master_giocatore,sizeof(int)*3,0);
		giocatore.mosse_residue+= gioc_pedina.mosse;
		giocatore.punteggio+= gioc_pedina.bandierina; 
		
	}
}