#include "my_lib.h"


int main(){
    int SO_BASE, SO_ALTEZZA, SO_MIN_HOLD_NSEC, SO_NUM_G,sem_round;
    int conf_id, sem_id_matrice, mat_id, sem_id_zero, ms_gp, r, c, posso_muovermi_c, posso_muovermi_r, rit, bandierina;
    int * matrice;
    struct shared_set * set;
    struct msg_p_g gioc_pedina;
    struct statopedina pedina;
    struct timespec ts;

   
    /* CONFIGURAZIONE E COLLEGAMENTO ALLA SCACCHIERA */
    setvbuf(stdout, NULL, _IONBF, 0); /* NO BUFFER */
	conf_id = shmget(KEY_2, sizeof(int)*10, IPC_CREAT | 0600);
	set = shmat(conf_id, NULL, 0);
	SO_BASE = set->SO_BASE;
	SO_ALTEZZA = set->SO_ALTEZZA;
    SO_NUM_G = set->SO_NUM_G;
	SO_MIN_HOLD_NSEC = set->SO_MIN_HOLD_NSEC;
    ts.tv_sec = 0;
    ts.tv_nsec = SO_MIN_HOLD_NSEC;
	mat_id = shmget(KEY_1, sizeof(int)*SO_BASE*SO_ALTEZZA, IPC_CREAT | 0666);
	matrice = shmat(mat_id, NULL, 0);
    sem_id_matrice = semget(KEY_3,(SO_ALTEZZA*SO_BASE), IPC_CREAT | 0666); 

    /* ATTENDO MESSAGGIO DAL GIOCATORE */
    ms_gp = msgget(getppid(), IPC_CREAT | 0666);
    msgrcv(ms_gp, &gioc_pedina, ((sizeof(int)*7)), getpid(), 0);
    pedina.id = getpid();
    pedina.r = gioc_pedina.r;
    pedina.c = gioc_pedina.c;    
    pedina.mosse = gioc_pedina.mosse;
    pedina.giocatore = gioc_pedina.giocatore;
    matrice[posizione(pedina.r,pedina.c,SO_BASE)] = pedina.giocatore;   

    /* SBLOCCO IL GIOCATORE */
    sem_id_zero = semget(KEY_0,4, IPC_CREAT | 0666);
	sem_reserve(sem_id_zero,1);

    sem_round = semget(KEY_7,2,0666|IPC_CREAT); 

    while(1){
        /* ATTENDO LA STRATEGIA */
        msgrcv(ms_gp,&gioc_pedina,sizeof(int)*7,getpid(),0);

        /* ATTENDO INIZIO GIOCO */
        aspetta_zero(sem_id_zero,3);

        /* RICERCA BANDIERINE */
        bandierina = 0;  
        r = pedina.r;
        c = pedina.c;
        posso_muovermi_r = 1;
        posso_muovermi_c = 1;
        while ((pedina.r != gioc_pedina.r_b || pedina.c != gioc_pedina.c_b)){  
            /* SPOSTAMENTO PER RIGA */
            if (pedina.mosse == 0) break;
            if (posso_muovermi_r == 1){
            if (pedina.r < gioc_pedina.r_b){
                rit = sem_reserve_wait_time(sem_id_matrice,posizione(r+1,c,SO_BASE));
                if (rit == -1 && errno == EAGAIN){
                    posso_muovermi_r = 0;
                    continue;
                }
                else {
                    posso_muovermi_c = 1;
                    sem_release(sem_id_matrice,posizione(r,c,SO_BASE));
                    matrice[posizione(r,c,SO_BASE)] = 0;
                    nanosleep(&ts,NULL);
                    r++;
                    if ((matrice[posizione(r,c,SO_BASE)]) > 0) 
                        bandierina = (matrice[posizione(r,c,SO_BASE)]); 
                    matrice[posizione(r,c,SO_BASE)] = pedina.giocatore;
                    pedina.r = r;
                    pedina.mosse--;
                    if (bandierina != 0){
                        gioc_pedina.type = getpid();
                        gioc_pedina.mosse = pedina.mosse;
                        gioc_pedina.bandierina = bandierina;
                        gioc_pedina.r = pedina.r;
                        gioc_pedina.c = pedina.c;
                        msgsnd(ms_gp,&gioc_pedina,sizeof(int)*7,0);
                        break;   
                    }
                }
            }
            else {
                if (pedina.r > gioc_pedina.r_b){
                    rit = sem_reserve_wait_time(sem_id_matrice,posizione(r-1,c,SO_BASE));
                    if (rit == -1 && errno == EAGAIN){
                        posso_muovermi_r = 0;
                        continue;
                    }
                    else {
                        posso_muovermi_c = 1;
                        sem_release(sem_id_matrice,posizione(r,c,SO_BASE));
                        matrice[posizione(r,c,SO_BASE)] = 0;
                        nanosleep(&ts,NULL);
                        r--;
                        if ((matrice[posizione(r,c,SO_BASE)]) > 0) 
                            bandierina = (matrice[posizione(r,c,SO_BASE)]);
                        matrice[posizione(r,c,SO_BASE)] = pedina.giocatore;
                        pedina.r = r;
                        pedina.mosse--;
                        if (bandierina != 0){
                            gioc_pedina.type = getpid();
                            gioc_pedina.mosse = pedina.mosse;
                            gioc_pedina.bandierina = bandierina;
                            gioc_pedina.r = pedina.r;
                            gioc_pedina.c = pedina.c;
                            msgsnd(ms_gp,&gioc_pedina,sizeof(int)*7,0);
                            break;   
                    }
                    }
                }
                else {
                    if (posso_muovermi_c == 0) break;
                }
            }
            }
            if (pedina.mosse == 0) break;
            /* SPOSTAMENTO PER COLONNA */
            if (posso_muovermi_c == 1){
            if (pedina.c < gioc_pedina.c_b){
                rit = sem_reserve_wait_time(sem_id_matrice,posizione(r,c+1,SO_BASE));
                if (rit == -1 && errno == EAGAIN){
                    posso_muovermi_c = 0;
                    continue;
                }
                else {
                    posso_muovermi_r = 1;
                    sem_release(sem_id_matrice,posizione(r,c,SO_BASE));
                    matrice[posizione(r,c,SO_BASE)] = 0;
                    nanosleep(&ts,NULL);
                    c++;
                    if ((matrice[posizione(r,c,SO_BASE)]) > 0) 
                        bandierina = (matrice[posizione(r,c,SO_BASE)]);
                    matrice[posizione(r,c,SO_BASE)] = pedina.giocatore;
                    pedina.c = c;
                    pedina.mosse--;
                    if (bandierina != 0){
                        gioc_pedina.type = getpid();
                        gioc_pedina.mosse = pedina.mosse;
                        gioc_pedina.bandierina = bandierina;
                        gioc_pedina.r = pedina.r;
                        gioc_pedina.c = pedina.c;
                        msgsnd(ms_gp,&gioc_pedina,sizeof(int)*7,0);
                        break;   
                    }
                }
            }
            else {
                if (pedina.c > gioc_pedina.c_b){
                    rit = sem_reserve_wait_time(sem_id_matrice,posizione(r,c-1,SO_BASE));
                    if (rit == -1 && errno == EAGAIN){
                        posso_muovermi_c = 0;
                        continue;
                    }
                    else {
                        posso_muovermi_r = 1;
                        sem_release(sem_id_matrice,posizione(r,c,SO_BASE));
                        matrice[posizione(r,c,SO_BASE)] = 0;
                        nanosleep(&ts,NULL); 
                        c--;
                        if ((matrice[posizione(r,c,SO_BASE)]) > 0) 
                            bandierina = (matrice[posizione(r,c,SO_BASE)]);
                        matrice[posizione(r,c,SO_BASE)] = pedina.giocatore;
                        pedina.c = c;
                        pedina.mosse--;
                        if (bandierina != 0){
                            gioc_pedina.type = getpid();
                            gioc_pedina.mosse = pedina.mosse;
                            gioc_pedina.bandierina = bandierina;
                            gioc_pedina.r = pedina.r;
                            gioc_pedina.c = pedina.c;
                            msgsnd(ms_gp,&gioc_pedina,sizeof(int)*7,0);
                            break;   
                    }
                    }
                }
                else {
                    if (posso_muovermi_r == 0) break;
                }
            }
            }
            if (posso_muovermi_r == 0 && posso_muovermi_c == 0) break;
        }
        if (bandierina == 0){
            gioc_pedina.type = getpid();
            gioc_pedina.mosse = pedina.mosse;
            gioc_pedina.bandierina = bandierina;
            gioc_pedina.r = pedina.r;
            gioc_pedina.c = pedina.c;
            msgsnd(ms_gp,&gioc_pedina,sizeof(int)*7,0);
        }
        sem_set_val(sem_round,1,SO_NUM_G);
        aspetta_zero(sem_round,1);
    }
}
