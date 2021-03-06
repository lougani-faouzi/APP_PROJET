#include "hyb_reduc.h"

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>



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
 //initialisation a 0 des elements de la structure
 int i=0;
 while(i<nvals)
 {
  sh_red->red_val[i]=0.0;
  i++;
 }
 sem_init(sh_red->semaphore,0,0);
 pthread_mutex_init(sh_red->mutex,NULL);
 pthread_barrier_init(sh_red->barriere,NULL,nthreads);
	
}

void shared_reduc_destroy(shared_reduc_t *sh_red)
{
    /* A COMPLETER */
    
 //desalocation des elements de la structure
 free(sh_red->red_val);
 free(sh_red->semaphore);
 pthread_mutex_destroy(sh_red->mutex);
 free(sh_red->mutex);
 pthread_barrier_destroy(sh_red->barriere);
 free(sh_red->barriere);
}

/*
 * Reduction  hybride MPI/pthread
 * in  : tableau des valeurs a reduire (de dimension sh_red->nvals)
 * out : tableau des valeurs reduites  (de dimension sh_red->nvals)
 */
void hyb_reduc_sum(double *in, double *out, shared_reduc_t *sh_red)
{
    /* A COMPLETER */
   
     
    double *temp;
    int taille=0;
    int rang=0;
    
    
    //Partie a pour but de faire la reduction thread 
    pthread_mutex_lock(sh_red->mutex);
    int i=0;
    //sommer les valeurs en entrée du tableau *in dans avec celle de red_val*
    while(i<sh_red->nvals){
    sh_red->red_val[i]=sh_red->red_val[i]+*(in+i);
    i=i+1;} 
    pthread_mutex_unlock(sh_red->mutex);
    
    //Partie a pour but de faire la reduction MPI
    int detect_premier_thread=0;
    pthread_mutex_lock(sh_red->mutex);
    { 
      if(sh_red->thread_maitre==0){
        //On detecte le premier thread arrivant 
       	if(detect_premier_thread==0) detect_premier_thread=sh_red->thread_maitre=1;	
                                  }
    }
    pthread_mutex_unlock(sh_red->mutex);
    
    pthread_barrier_wait(sh_red->barriere);
  
    //on attend de detecter le premier thread entrant 
    if(detect_premier_thread==0)
    {
     i=0;
     sem_wait(sh_red->semaphore);
     while(i<sh_red->nvals){
     //on met ajour chaque thread
     *(out+i)=sh_red->red_val[i]; 
     i=i+1;}
    }
    
    if(detect_premier_thread==sh_red->thread_maitre)
    { //une fois le thread maitre entre on recupere la taille et le rang  
      MPI_Comm_size(MPI_COMM_WORLD,&taille);
      temp=malloc(sizeof(double)*taille);
      MPI_Comm_rank(MPI_COMM_WORLD,&rang);
      
      i=0;
      int j;
      while(i<sh_red->nvals)
      {
      // rasembler les valeurs des processus en un seul processus (le processus reception )cad le processus 0
      MPI_Gather(&(sh_red->red_val[i]),1,MPI_DOUBLE,temp,1,MPI_DOUBLE,0,MPI_COMM_WORLD); 
      // sommer toutes les réductions des processus dans le processus 0
       if (rang==0){ 
         j=0;
         while(j<taille){
         *(out+i)=*(out+i)+*(temp+j);
         j=j+1;}
                  } 
      i=i+1;
      }
      
      //envoyer le resultat a tous les processus 
      MPI_Bcast(out,sh_red->nvals,MPI_DOUBLE,0, MPI_COMM_WORLD);
      //mettre a jour chaque thread  
      i=0;
      while(i<sh_red->nvals){
      sh_red->red_val[i]=*(out+i);
      i=i+1;}
      
      
    }
    
    //on libére le sémaphore uniquement si on est pas au dernier thread 
    if(sh_red->nb_threads!=0)
    sem_post(sh_red->semaphore);                          
    //on desaloue le tableu temporaire precedent
    free(temp);
}

