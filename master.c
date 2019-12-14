#include "my_lib.h"

int main(){
	int i, sem_id_zero, sem_id_mutex, mat_id, conf_id, sem_id_matrice, x,y,ms_gp, pos, ms_mg, n_flag;
	int pt_bandierina, pt_totali, media, flag;
	int * matrice;
	struct shared_set  * set;
	int * fork_value;
	struct msg_m_g master_giocatore;
	char * args[2];
	char str[4];
	int * punteggio_g, * mosse_g;

	setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	configure_settings();
	
	/* CONFIGURAZIONE E GENERAZIONE SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, (sizeof(int)*(set->SO_BASE)*(set->SO_ALTEZZA)), IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);

	/* SETTAGGIO INIZIALE MATRICE*/
	for (pos = 0; pos < set->SO_BASE*set->SO_ALTEZZA; pos++)
		matrice[pos] = 0;

	/* CREO SEMAFORI PER SCACCHIERA */
	sem_id_matrice = semget(KEY_3,(set->SO_ALTEZZA*set->SO_BASE), IPC_CREAT | 0666);
	for (i = 0; i < (set->SO_ALTEZZA*set->SO_BASE); i++)
		sem_set_val(sem_id_matrice,i,1);

	/* SEMAFORI MUTUA ESCLUSIONE */
	sem_id_mutex = semget(KEY_5,set->SO_NUM_G, IPC_CREAT | 0666); /* giocatore a turno piazza pedine */
	sem_set_val(sem_id_mutex,0,1);
	for (i = 1; i < set->SO_NUM_G; i++)
		sem_set_val(sem_id_mutex,i,0);

	args[1] = NULL;
	/* GENERAZIONE GIOCATORI E CODA PER COMUNICAZIONE */
	fork_value = malloc(sizeof(int)*set->SO_NUM_G);
	for (i = 0; i < set->SO_NUM_G; i++){
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
	
	/* SEMAFORO PER ATTENDERE CHE I GIOCATORI PIAZZINO LE PEDINE */
	ms_mg = msgget(KEY_6, IPC_CREAT | 0666);
	sem_id_zero = semget(KEY_0, 4, IPC_CREAT | 0666);
	sem_set_val(sem_id_zero, 0, set->SO_NUM_G);
	sem_set_val(sem_id_zero,2,1); /* semaforo per far attendere giocatori */
	aspetta_zero(sem_id_zero, 0); /* ATTENDE FINCHE' NON VALE 0 */

	/* PIAZZO BANDIERINE */
	pt_totali = set->SO_ROUND_SCORE;
	srand(time(NULL));
	n_flag = rand()%((set->SO_FLAG_MAX)-(set->SO_FLAG_MIN)+1)+(set->SO_FLAG_MIN);
	flag = n_flag;	
	while ((n_flag) > 0){	
		if (n_flag == 1)
			pt_bandierina = pt_totali;
		else {
			media = pt_totali/n_flag;
			pt_bandierina = rand()% media + 1;
		}
		do{
			x = rand() % set->SO_ALTEZZA;
			y = rand() % set->SO_BASE;			}
		while (matrice[posizione(x,y,set->SO_BASE)] != 0);
		matrice[posizione(x,y,set->SO_BASE)] = pt_bandierina;
		pt_totali = pt_totali - pt_bandierina;
		n_flag--;
	}

	punteggio_g = malloc(sizeof(int)*set->SO_NUM_G);
	mosse_g = malloc(sizeof(int)*set->SO_NUM_G);
	for (i = 0; i < set->SO_NUM_G; i++){
		punteggio_g[i] = 0;
		mosse_g[i] = 0;
	}

	/* STAMPO STATO */
	printf("PUNTEGGIO TOTALE PARTITA %d, BANDIERINE TOTALI %d\n",set->SO_ROUND_SCORE,flag);
	for (i = 0; i < set->SO_NUM_G; i++){
		msgrcv(ms_mg, &master_giocatore,((sizeof(int)*3)), fork_value[i], 0);
		punteggio_g[65+master_giocatore.giocatore] = 0;
		mosse_g[65+master_giocatore.giocatore] = 0;
		printf("GIOCATORE: %c, MOSSE TOTALI: %d, PUNTEGGIO: %d\n",65+i,set->SO_N_MOVES,0);
	}
	stampa_scacchiera(set->SO_BASE,set->SO_ALTEZZA);

	sem_set_val(sem_id_zero, 0, set->SO_NUM_G);
	sem_reserve(sem_id_zero,2);

	/* ASPETTO CHE I GIOCATORI DANNO LE INDICAZIONI ALLE PEDINE */
	aspetta_zero(sem_id_zero, 0); /* ATTENDE FINCHE' NON VALE 0 */

	alarm(set->SO_MAX_TIME);
	sem_reserve(sem_id_zero,2); /* AVVIO ROUND */	

	for (i = 0; i < set->SO_NUM_G*set->SO_NUM_P; i++) {
		msgrcv(ms_mg,&master_giocatore,sizeof(int)*3,1,0);
		mosse_g[(-master_giocatore.giocatore)-65]+= master_giocatore.mosse_residue;
		punteggio_g[(-master_giocatore.giocatore)-65]+= master_giocatore.bandierina;
		flag--;
	}

while (wait(NULL) != -1);
stampa_scacchiera(set->SO_BASE,set->SO_ALTEZZA);

for (i = 0; i < set->SO_NUM_G; i++){
	printf("giocatore %c punteggio %d mosse %d \n",65+i,punteggio_g[i],mosse_g[i]);
}

	/* ELIMINO SEMAFORI E MEMORIE CONDIVISE*/
	printf("\n");
	shmctl(mat_id, IPC_RMID, NULL); 
	shmctl(conf_id, IPC_RMID, NULL);
	shmdt(matrice);
	shmdt(set);
	semctl(sem_id_zero,0,IPC_RMID); /* 0 è ignorato*/
	semctl(sem_id_matrice,0,IPC_RMID);
	for (i = 0; i < set->SO_NUM_G; i++){ /* controllare errore di segmentazione*/
		ms_gp = msgget(fork_value[i],IPC_CREAT | 0666);
		msgctl(ms_gp,IPC_RMID,NULL);
	}
	msgctl(ms_mg,IPC_RMID,NULL);
}
