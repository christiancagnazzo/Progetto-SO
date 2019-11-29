#include "my_lib.h"

int main(){
    int conf_id;
    int mat_id;
    char * matrice;
    struct shared_set * set;
    struct msg_p_g mess;
	int ms_id;

    setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */

    /* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(char)*set->SO_BASE*set->SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);

    /* ATTENDO MESSAGGIO DAL GIOCATORE */
    ms_id = msgget(KEY_4, IPC_CREAT | 0666);
    msgrcv(ms_id, &mess, sizeof(int)*2, getpid(), 0);
printf("qui non dovrei arrivare");
}