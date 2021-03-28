#include "hyb_reduc.h"

#include <mpi.h>
#include <stdlib.h>

void shared_reduc_init(shared_reduc_t *sh_red, int nthreads, int nvals)
{
    /* A COMPLETER */
    
	//affectation directe 
	sh_red->nvals=nvals;
	sh_red->nb_threads=nthreads;
	
	//allocation 
	sh_red->red_val=malloc(sizeof(double)*nvals);
	sh_red->semaphore=malloc(sizeof(sem_t));
	sh_red->mutex=malloc(sizeof(pthread_mutex_t));
	sh_red->barriere=malloc(sizeof(pthread_barrier_t));
	
	//initialisation a 0
	sh_red->nb_thread_finish_work=0;
	memset(sh_red->red_val,0,sizeof(double)*nvals);
	sem_init(sh_red->semaphore,0,0);
	pthread_mutex_init(sh_red->mutex,NULL);
	pthread_barrier_init(sh_red->barriere,NULL,nthreads);
	
}

void shared_reduc_destroy(shared_reduc_t *sh_red)
{
    /* A COMPLETER */
}

/*
 * Reduction  hybride MPI/pthread
 * in  : tableau des valeurs a reduire (de dimension sh_red->nvals)
 * out : tableau des valeurs reduites  (de dimension sh_red->nvals)
 */
void hyb_reduc_sum(double *in, double *out, shared_reduc_t *sh_red)
{
    /* A COMPLETER */
}


