#include "my_lib.h"


int main(){
    int conf_id, sem_id_matrice, mat_id, sem_id_mutex, sem_id_zero, ms_gp, r;
    char * matrice;
    struct shared_set * set;
    struct msg_p_g gioc_pedina;
    struct statopedina pedina;

    setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */

    /* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(char)*set->SO_BASE*set->SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);
    
    /* SEMAFORI SCACCHIERA */
    sem_id_matrice = semget(KEY_3,(set->SO_ALTEZZA*set->SO_BASE), IPC_CREAT | 0666);

    /* SEMAFORO PER LA MUTUA ESCLUSIONE */
	sem_id_mutex = semget(KEY_5,2, IPC_CREAT | 0666);
	
    /* ATTENDO MESSAGGIO DAL GIOCATORE */
    ms_gp = msgget(KEY_4, IPC_CREAT | 0666);
    msgrcv(ms_gp, &gioc_pedina, ((sizeof(int)*2)+sizeof(char)), getpid(), 0);
    pedina.id = getpid();
    pedina.pos = gioc_pedina.pos; 
    pedina.mosse = gioc_pedina.mosse;
    pedina.giocatore = gioc_pedina.giocatore;
    /* SEZIONE CRITICA */
    sem_reserve(sem_id_mutex,1);
    r = sem_reserve_nowait(sem_id_matrice,pedina.pos);
    while (r == -1 && errno == EAGAIN){
        if (pedina.pos == ((set->SO_BASE*set->SO_ALTEZZA)-1)){
            pedina.pos = 0; /* se sono alla fine riparto dall'inizio*/
        }else 
            pedina.pos = ((pedina.pos) +1); /* provo ad andare avanti */
        r = sem_reserve_nowait(sem_id_matrice,pedina.pos);
    }
    matrice[pedina.pos] = pedina.giocatore;
    sem_release(sem_id_mutex,1);
    /* FINE SEZIONE CRITICA */

    /* SBLOCCO IL GIOCATORE */
    sem_id_zero = semget(KEY_0,2, IPC_CREAT | 0666);
	sem_reserve(sem_id_zero,1);
}