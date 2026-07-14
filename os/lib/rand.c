/* ---------------------------------------------------------------- */
/* - rand.c                                                       - */
/* ---------------------------------------------------------------- */

/* This is a pseudo random generator. At a (loosely) performed test
   with 1,000,000 elements the maximum deviance from the average
   was 5% which I consider good enough for this project.            */

#include <ansi.h>

static unsigned int rand_seed=0;

void srand(unsigned int seed) 
{
 rand_seed=seed;
}

int rand(void) 
{
 rand_seed+=(rand_seed*69069+1);
 rand_seed=rand_seed%RAND_MAX;
 return (int)rand_seed;
}

