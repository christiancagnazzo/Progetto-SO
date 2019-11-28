#include "settings.h"

void setting(){
#ifdef HARD
    setenv("SO_NUM_G","4",0);
    setenv("SO_NUM_P","400",0);
    setenv("SO_MAX_TIME","1",0);
    setenv("SO_BASE","120",0);
    setenv("SO_ALTEZZA","40",0);
    setenv("SO_FLAG_MIN","5",0);
    setenv("SO_FLAG_MAX","40",0);
    setenv("SO_ROUND_SCORE","200",0);
    setenv("SO_N_MOVES","200",0);
#else
    setenv("SO_NUM_G","2",0);
    setenv("SO_NUM_P","10",0);
    setenv("SO_MAX_TIME","3",0);
    setenv("SO_BASE","60",0);
    setenv("SO_ALTEZZA","20",0);
    setenv("SO_FLAG_MIN","5",0);
    setenv("SO_FLAG_MAX","5",0);
    setenv("SO_ROUND_SCORE","10",0);
    setenv("SO_N_MOVES","20",0);
#endif
}

int set(char * s){
    int i;
    i = atoi(getenv(s));
    return i;
}