#include <stdlib.h>
#include <time.h>
#include "rng.h"

static int initialized = 0; //Flag if seed was initialized

double randomUniform() {
    if(initialized == 0) {
        srand(time(NULL));
        initialized = 1;
    }

    //Get random number with the seed, make it modulo rand_max+1 and divide by rand_max to get [0, 1]
    return (double)(rand() % (RAND_MAX+1))/(double)(RAND_MAX);
}