#include "my_lib.h"


int main(){
    int conf_id, sem_id_matrice, mat_id, sem_id_mutex, sem_id_zero, ms_gp, r, c, posso_muovermi, rit;
    int * matrice;
    struct shared_set * set;
    struct msg_p_g gioc_pedina;
    struct statopedina pedina;
    struct timespec ts;

    setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
    ts.tv_sec = 0;
    ts.tv_nsec = set->SO_MIN_HOLD_NSEC;

    /* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	mat_id = shmget(KEY_1, sizeof(int)*set->SO_BASE*set->SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);
    
    /* SEMAFORI SCACCHIERA */
    sem_id_matrice = semget(KEY_3,(set->SO_ALTEZZA*set->SO_BASE), IPC_CREAT | 0666);
    sem_id_mutex = semget(KEY_5,2, IPC_CREAT | 0666);

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
    sem_id_zero = semget(KEY_0,4, IPC_CREAT | 0666);
	sem_reserve(sem_id_zero,1);
    
    /* ATTENDO LA STRATEGIA E LA LEGGO*/
    msgrcv(ms_gp,&gioc_pedina,sizeof(int)*6,getpid(),0);

    /* SBLOCCO IL GIOCATORE */
    sem_reserve(sem_id_zero,1);

    /* ATTENDO INIZIO GIOCO */
    aspetta_zero(sem_id_zero,3);
    /* RICERCA BANDIERINE */
    posso_muovermi = 2;
    r = pedina.r;
    c = pedina.c;
    while (posso_muovermi > 0){  
        /* MI SPOSTO PER RIGHE */  
        while (pedina.r != gioc_pedina.r_b && pedina.mosse > 0){
            if (pedina.r < gioc_pedina.r_b){
                rit = sem_reserve_wait_time(sem_id_matrice,posizione(r+1,c,set->SO_BASE));
                if (rit == -1 && errno == EAGAIN){
                    posso_muovermi--;
                    break;
                }
                else {
                    posso_muovermi = 2;
                    sem_release(sem_id_matrice,posizione(r,c,set->SO_BASE));
                    /*nanosleep(&ts,NULL); /* controllare */
                    matrice[posizione(r,c,set->SO_BASE)] = 0;
                    r++;
                    matrice[posizione(r,c,set->SO_BASE)] = pedina.giocatore;
                    pedina.c = c;
                    pedina.r = r;
                    pedina.mosse--;
                }
            }
            else {
                if (pedina.r > gioc_pedina.r_b){
                rit = sem_reserve_wait_time(sem_id_matrice,posizione(r-1,c,set->SO_BASE));
                if (rit == -1 && errno == EAGAIN){
                    posso_muovermi--;
                    break;
                }
                else {
                    posso_muovermi = 2;
                    sem_release(sem_id_matrice,posizione(r,c,set->SO_BASE));
                    /*nanosleep(&ts,NULL); /* controllare */
                    matrice[posizione(r,c,set->SO_BASE)] = 0;
                    r--;
                    matrice[posizione(r,c,set->SO_BASE)] = pedina.giocatore;
                    pedina.c = c;
                    pedina.r = r;
                    pedina.mosse--;
                }
            }
            }
        }
        /* MI SPOSTO PER COLONNE */
        while (pedina.c != gioc_pedina.c_b && pedina.mosse > 0){
            if (pedina.c < gioc_pedina.c_b){
                rit = sem_reserve_wait_time(sem_id_matrice,posizione(r,c+1,set->SO_BASE)); 
                if (rit == -1 && errno == EAGAIN){
                    posso_muovermi--;
                    break;
                }
                else {
                    posso_muovermi = 2;
                    sem_release(sem_id_matrice,posizione(r,c,set->SO_BASE));
                    /*nanosleep(&ts,NULL); /* controllare */
                    matrice[posizione(r,c,set->SO_BASE)] = 0;
                    c++;
                    matrice[posizione(r,c,set->SO_BASE)] = pedina.giocatore;
                    pedina.c = c;
                    pedina.r = r;
                    pedina.mosse--;
                }
            }
            else {
                if (pedina.c > gioc_pedina.c_b){
                rit = sem_reserve_wait_time(sem_id_matrice,posizione(r,c-1,set->SO_BASE));
                if (rit == -1 && errno == EAGAIN){
                    posso_muovermi--;
                    break;
                }
                else {
                    posso_muovermi = 2;
                    sem_release(sem_id_matrice,posizione(r,c,set->SO_BASE));
                    /*nanosleep(&ts,NULL); /* controllare */
                    matrice[posizione(r,c,set->SO_BASE)] = 0;
                    c--;
                    matrice[posizione(r,c,set->SO_BASE)] = pedina.giocatore;
                    pedina.c = c;
                    pedina.r = r;
                    pedina.mosse--;
                }
            }
            }
        }
    if (pedina.mosse == 0 || (posizione(pedina.r,pedina.c,set->SO_BASE) == posizione(gioc_pedina.r_b,gioc_pedina.c,set->SO_BASE)))
        break;
    }
}