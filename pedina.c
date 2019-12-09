#include "my_lib.h"


int main(){
    int conf_id, sem_id_matrice, mat_id, sem_id_mutex, sem_id_zero, ms_gp;
    int * matrice;
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

    /* ATTENDO MESSAGGIO DAL GIOCATORE */
    ms_gp = msgget(KEY_4, IPC_CREAT | 0666);
    msgrcv(ms_gp, &gioc_pedina, ((sizeof(int)*6)), getpid(), 0);
    pedina.id = getpid();
    pedina.r = gioc_pedina.r;
    pedina.c = gioc_pedina.c;    
    pedina.mosse = gioc_pedina.mosse;
    pedina.giocatore = gioc_pedina.giocatore;
    matrice[posizione(pedina.r,pedina.c,set->SO_BASE)] = pedina.giocatore;   

    /* SBLOCCO IL GIOCATORE */
    sem_id_zero = semget(KEY_0,2, IPC_CREAT | 0666);
	sem_reserve(sem_id_zero,1);
    
    /* ATTENDO LA STRATEGIA E LA LEGGO*/
    msgrcv(ms_gp,&gioc_pedina,sizeof(int)*6,getpid(),0);

    /* SBLOCCO IL GIOCATORE */
    sem_reserve(sem_id_zero,1);

    /*aspetta inizio gioco*/
}