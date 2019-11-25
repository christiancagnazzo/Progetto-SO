#ifndef HARD
#ifndef EASY
#define EASY
#endif
#endif

#ifdef HARD
#define SO_NUM_G 4
#define SO_NUM_P 400
#define SO_MAX_TIME 1
#define SO_BASE 120
#define SO_ALTEZZA 40
#define SO_FLAG_MIN 5
#define SO_FLAG_MAX 40
#define SO_ROUND_SCORE 200
#define SO_N_MOVES 200
#endif

#ifdef EASY
#define SO_NUM_G 2
#define SO_NUM_P 10
#define SO_MAX_TIME 3
#define SO_BASE 60
#define SO_ALTEZZA 20
#define SO_FLAG_MIN 5
#define SO_FLAG_MAX 5
#define SO_ROUND_SCORE 10
#define SO_N_MOVES 20
#endif