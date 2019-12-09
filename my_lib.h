#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <time.h>

#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define RESET   "\x1b[0m"
#define RED "\x1b[35m"
#define BLUE    "\x1b[34m"


#define KEY_0 123 /* chiave semafori aspetta zero */ 
/* 0 master aspetta giocatori */
/* 1 giocatori aspettano pedine */
/* 2 giocatori aspettano master */
#define KEY_1 100 /* chiave memoria matrice */
#define KEY_2 634 /* chiave memoria settings*/
#define KEY_3 982 /* chiave semafori matrice */
#define KEY_4 723 /* chiave coda di messaggi giocatori pedine */
#define KEY_5 911 /* chiave semaforo mutua esclusione */
/* semaforo 0 mutua esclusione giocatore piazza pedine */
/* semaforo 1 mutua esclusione pedina si posizionano */
#define KEY_6 145 /* master giocatore*/

struct shared_set {
	int SO_NUM_G; 
	int SO_NUM_P;
	int SO_MAX_TIME; 
	int SO_BASE; 
	int SO_ALTEZZA;
	int SO_FLAG_MIN;
	int SO_FLAG_MAX;
	int SO_ROUND_SCORE; 
	int SO_N_MOVES;   
	int SO_MIN_HOLD_NSEC;
};

struct msg_p_g {
	long type;
	int r;
	int c;
	int mosse;
	int giocatore;
	int r_b;
	int c_b;
};

struct msg_m_g {
	long type;
	int mosse_residue;
	int punteggio;
	int giocatore;
};

struct statopedina
{
    int id;
    int r;
	int c;
    int mosse;
	int giocatore;
};

struct statogiocatore {
	int id;
	int giocatore;
	int mosse_residue;
	int punteggio;
};

 /*
 * Inizializzazione semaforo
 * INPUT:
 * - sem_id: ID dell'array di semafori (IPC)
 * - sem_num: posizione semaforo nell'array
 * - sem_val: inizializzazione semaforo
 */
int sem_set_val(int sem_id, int sem_num, int sem_val);

/*
 * Richiesta di accesso a risorsa
 * INPUT:
 * - sem_id: ID dell'array di semafori (IPC)
 * - sem_num: posizione semaforo nell'array
 * 
 * - Se la risorsa è disponibile (valore semaforo > 0), il semaforo
 *   è decrementato di 1
 * - Altrimenti il processo rimane in attesa che 
 *   che la risorsa diventi disponibile
 */
int sem_reserve(int sem_id, int sem_num);

/*
 * Rilascio di una risorsa
 * INPUT:
 * - sem_id: ID dell'array di semafori (IPC)
 * - sem_num: posizione semaforo nell'array
 * 
 * - Il valore del semaforo è incrementato di 1.
 */
int sem_release(int sem_id, int sem_num);

/*
 * Stampa i semafori in una stringa. 
 * my_string deve essere allocata prima
 */
int sem_getall(char * my_string, int sem_id);

/*
* Aspetta che il semaforo valga zero
*/
int aspetta_zero(int sem_id, int sem_num);
/*

 * Union necessaria
 */
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};


int sem_reserve_nowait(int sem_id, int sem_num);

void configure_settings();

int posizione(int r, int c, int col);

void stampa_scacchiera(int base, int altezza);