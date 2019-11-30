#include "my_lib.h"


int main(){
    int conf_id, sem_id_matrice;
    int mat_id, sem_id_mutex, sem_id_zero;
    char * matrice;
    struct shared_set * set;
    struct msg_p_g mess;
	int ms_id;
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
    ms_id = msgget(KEY_4, IPC_CREAT | 0666);
    msgrcv(ms_id, &mess, ((sizeof(int)*2)+sizeof(char)), getpid(), 0);
    pedina.id = getpid();
    pedina.pos = mess.pos; 
    pedina.mosse = mess.mosse;
    pedina.giocatore = mess.giocatore;
    /* SEZIONE CRITICA */
    sem_reserve(sem_id_mutex,1);
    sem_reserve_nowait(sem_id_matrice,pedina.pos);
    /*while (b == -1 && errno == EAGAIN){
        printf("ciao");
        pedina.pos = (mess.pos+1);
        b = sem_reserve_nowait(sem_id_matrice,pedina.pos);
    }*/
    matrice[pedina.pos] = pedina.giocatore;
    sem_release(sem_id_mutex,1);
    /* FINE SEZIONE CRITICA */

    /* SBLOCCO IL GIOCATORE */
    sem_id_zero = semget(KEY_0,2, IPC_CREAT | 0666);
	sem_reserve(sem_id_zero,1);
}