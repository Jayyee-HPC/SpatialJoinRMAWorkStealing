#include "../include/globalHeader.h"

//#define EVEREST 1

using namespace std;
using namespace geos::geom;


//./prog 4 ~/data/sports_data_env ~/data/cemet_data_env
//mpicxx -lgeos -I/opt/geos/include -std=c++11 -Wfatal-errors -o prog adaptiveGrider.cpp readStruct.cpp
//g++ -lgeos -I/opt/geos/include -std=c++11 -Wfatal-errors -o prog adaptiveGrider.cpp readStruct.cpp


int spatialJoin(list<Geometry* > *lGeoms1, list<Geometry* > *lGeoms2)
{
    //MPI initial
    int myWorldRank;
    int numWorldNodes;
    long unsigned int *my_base = NULL;
    long unsigned int *putBuf = NULL;
    MPI_Win global_win;

    MPI_Comm_size(MPI_COMM_WORLD,&numWorldNodes);
    MPI_Comm_rank(MPI_COMM_WORLD,&myWorldRank);

    //my_base = (long unsigned int *)malloc(sizeof(long unsigned int) * ELEM_PER_PROC);
    MPI_Alloc_mem(sizeof(long unsigned int) * ELEM_PER_PROC, MPI_INFO_NULL, &my_base);
    MPI_Alloc_mem(sizeof(long unsigned int) * ELEM_PER_PROC, MPI_INFO_NULL, &putBuf);
    /* Write local data */
    //local job not finished yet
    my_base[0] = ALL_LOCAL_JOB_NOT_COMPLETED;
    //not starve process check in yet
    my_base[1] = ALL_LOCAL_JOB_NOT_COMPLETED;

    /* Allocate ELEM_PER_PROC integers for each process */
    //MPI_Win_allocate_shared(sizeof(long unsigned int) * ELEM_PER_PROC, sizeof(long unsigned int), MPI_INFO_NULL,
      //                      MPI_COMM_WORLD, &my_base, &global_win);

    MPI_Win_create(my_base, sizeof(long unsigned int) * ELEM_PER_PROC, sizeof(long unsigned int), MPI_INFO_NULL, MPI_COMM_WORLD,
                   &global_win);


    MPI_Barrier(MPI_COMM_WORLD);
    WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*> queue;

    printf("Geoms sizes %ld :: %ld \n", lGeoms1->size(), lGeoms2->size());

    if(myWorldRank != 0){

      checkOtherProcessForJob(&queue, global_win);
      printf("Other processes finished!\n");
      return 0;

    }


    
    /************************Find candidates:: Main thread*********************************************************/
    double t_pthread_begin = MPI_Wtime();

    triggerSpatialJoin(&queue, &global_win, lGeoms1, lGeoms2);

    double t_pthread_end = MPI_Wtime();

    putBuf[0] = putBuf[1] = ALL_LOCAL_JOB_COMPLETED;
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, myWorldRank, 0, global_win);

    //MPI_Request put_req;
    //MPI_Rput(buf, MAX_SIZE, MPI_INT, 0, 0, MAX_SIZE, MPI_INT, window, &put_req);
           // MPI_Wait(&put_req, MPI_STATUS_IGNORE);

    MPI_Put(putBuf, 2, MPI_UNSIGNED_LONG, myWorldRank,
      	 0, 2, MPI_UNSIGNED_LONG, global_win);

    MPI_Win_flush(myWorldRank, global_win);
    MPI_Win_unlock(myWorldRank, global_win);

    checkOtherProcessForJob(&queue, global_win);
      
    double t_end = MPI_Wtime();
    /************************Spatial Join end**********************************************************/

    cout<<"Time for Spatial Join::"<<(double)(t_pthread_end - t_pthread_begin)<<endl;
          
    cout<<"Total time except parsing::"<<(double)(t_end- t_pthread_begin)<<endl;

    return 0;
}
