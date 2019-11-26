#include "settings.h"

void set_hard(){
    setenv("SO_NUM_G","4",1);
    setenv("SO_NUM_P","400",1);
    setenv("SO_MAX_TIME","1",1);
    setenv("SO_BASE","120",1);
    setenv("SO_ALTEZZA","40",1);
    setenv("SO_FLAG_MIN","5",1);
    setenv("SO_FLAG_MAX","40",1);
    setenv("SO_ROUND_SCORE","200",1);
    setenv("SO_N_MOVES","200",1);
}

void set_easy(){
    setenv("SO_NUM_G","2",1);
    setenv("SO_NUM_P","10",1);
    setenv("SO_MAX_TIME","3",1);
    setenv("SO_BASE","60",1);
    setenv("SO_ALTEZZA","20",1);
    setenv("SO_FLAG_MIN","5",1);
    setenv("SO_FLAG_MAX","5",1);
    setenv("SO_ROUND_SCORE","10",1);
    setenv("SO_N_MOVES","20",1);
}

int set(char * s){
    int i;
    i = atoi(getenv(s));
    return i;
}