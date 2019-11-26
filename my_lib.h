#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

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


