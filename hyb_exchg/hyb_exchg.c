#include "hyb_exchg.h"
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

/*
 * Initialisation/destruction d'une structure shared_exchg_t
 * nthreads : nombre de threads (du processus MPI) qui vont participer a l'echange
 */
void shared_exchg_init(shared_exchg_t *sh_ex, int nthreads)
{
  // initialisation des elements de la structure *sh_ex
  sh_ex->left=sh_ex->right=0.0;
  sh_ex->nb_threads = nthreads;
  sh_ex->premier=1;
  sh_ex->semaphore= malloc(sizeof(sem_t));
  sem_init(sh_ex->semaphore,1,1);
}

void shared_exchg_destroy(shared_exchg_t *sh_ex)
{
  //supression du semaphore
  sem_destroy(sh_ex->semaphore);
  // desalocation de l'espace memoire du semaphore
  free(sh_ex->semaphore);
}


/*
 * Echange hybride MPI/pthread
 * Si processus MPI existe "a gauche", lui envoie la valeur sh_arr[0] et recoit de lui *val_to_rcv_left
 * Si processus MPI existe "a droite", lui envoie la valeur sh_arr[mpi_decomp->mpi_nloc-1] et recoit de lui *val_to_rcv_right
 * Si processus voisin n'existe pas, valeur correspondante affectee a 0
 */
void hyb_exchg(
               double *sh_arr,
               shared_exchg_t *sh_ex,
               double *val_to_rcv_left, double *val_to_rcv_right,
               mpi_decomp_t *mpi_decomp)
{
  int arrive=1;
  
  //on attend le 1er thread arrivé 
  sem_wait(sh_ex->semaphore);

  if (sh_ex->premier==arrive)
  {  //commencer l'echange avec les autres premiers threads 
  
      if (mpi_decomp->mpi_rank == mpi_decomp->mpi_nproc-1) 
      {   //si on est dans une feuille 
          //donc on est dans le cas ou le processus MPI existe "a gauche"
          *val_to_rcv_right = 0.0;
          MPI_Recv(&(sh_ex->left), 1, MPI_DOUBLE, mpi_decomp->mpi_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(&(sh_arr[0]), 1, MPI_DOUBLE, mpi_decomp->mpi_rank - 1, 0, MPI_COMM_WORLD);
      }
      if (mpi_decomp->mpi_rank == 0) 
      {   //si on est a la racine (processus du rang 0)  
          //donc on est dans le cas ou le processus MPI existe "a droite"
          *val_to_rcv_left = 0.0;
          MPI_Send(&(sh_arr[mpi_decomp->mpi_nloc - 1]), 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
          MPI_Recv(&(sh_ex->right), 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
      if((mpi_decomp->mpi_rank != 0)&&(mpi_decomp->mpi_rank != mpi_decomp->mpi_nproc-1))
      {   //si on est dans un noeud (c'est a dire au milieu) on envoie et on recois dans les deux sens (gauche,droit)
          MPI_Recv(&(sh_ex->left), 1, MPI_DOUBLE, mpi_decomp->mpi_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(&(sh_arr[mpi_decomp->mpi_nloc - 1]), 1, MPI_DOUBLE, mpi_decomp->mpi_rank + 1, 0, MPI_COMM_WORLD);
          MPI_Recv(&(sh_ex->right), 1, MPI_DOUBLE, mpi_decomp->mpi_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(&(sh_arr[0]), 1, MPI_DOUBLE, mpi_decomp->mpi_rank - 1, 0, MPI_COMM_WORLD);  
      }
      if (mpi_decomp->mpi_nproc-1== 0) 
      {   //Si processus voisin n'existe pas (arbre a un seule noeud
          *val_to_rcv_left = 0.0;
          *val_to_rcv_right = 0.0;
      }
      
      sh_ex->premier = 0;
      sem_post(sh_ex->semaphore);
    }
  else 
    {
      //si on est pas dans le 1er thread on met a jour les valeurs 
      *val_to_rcv_left = sh_ex->left;
      *val_to_rcv_right = sh_ex->right;
      sem_post(sh_ex->semaphore);      
    }
}
