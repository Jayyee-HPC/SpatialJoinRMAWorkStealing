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
    int errors = 0;
    long unsigned int *base, *my_base;
    MPI_Aint size;
    MPI_Win global_win;
    int disp_unit;
    int get_target;

    MPI_Comm_size(MPI_COMM_WORLD,&numWorldNodes);
    MPI_Comm_rank(MPI_COMM_WORLD,&myWorldRank);

    /* Allocate ELEM_PER_PROC integers for each process */
    MPI_Win_allocate_shared(sizeof(long unsigned int) * ELEM_PER_PROC, sizeof(long unsigned int), MPI_INFO_NULL,
                            MPI_COMM_WORLD, &my_base, &global_win);

    /* Write local data */
    //local job not finished yet
    my_base[0] = ALL_LOCAL_JOB_NOT_COMPLETED;
    //not starve process check in yet
    my_base[1] = ALL_LOCAL_JOB_NOT_COMPLETED;


    MPI_Win_create(my_base, sizeof(long unsigned int) * ELEM_PER_PROC, sizeof(long unsigned int), MPI_INFO_NULL, MPI_COMM_WORLD,
                   &global_win);

    double t1 = MPI_Wtime();

    WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*> queue;
    //WorkStealingQueue<int> queue;

    double t2 = MPI_Wtime();

    //delete vEnvStrs1;
    //delete vEnvStrs2;
      
    double t3 = MPI_Wtime();

    printf("Geoms sizes %d :: %d \n", lGeoms1->size(), lGeoms2->size());

    if(myWorldRank != 0){

      checkOtherProcessForJob(&queue, &global_win);
      printf("Other processes finished!\n");
      return 0;

    }

    /************************Build index************************************************************************/
      geos::index::strtree::STRtree indexForLayer1;
    for(list<Geometry* >::iterator itr = lGeoms1->begin(); itr != lGeoms1->end(); ++itr){
         Geometry* tmpGeom =  *itr;
      
         indexForLayer1.insert(tmpGeom->getEnvelopeInternal(), tmpGeom);
    }

    /************************Build index end************************************************************************/
      
    double t4 = MPI_Wtime();
    for(list<Geometry*>::iterator ownerItr = lGeoms2->begin(); ownerItr != lGeoms2->end(); ++ownerItr )
    {
      Geometry* tmpGeom = *ownerItr;			
      std::vector<void *> results;

      const Envelope* tmpEnv=  tmpGeom->getEnvelopeInternal();

      indexForLayer1.query(tmpEnv, results);

      if(results.size() != 0){
      		vector<Geometry *> *tempGeomVector = new vector<Geometry *>(results.size());
      		int tempCounter = 0;
      		for(vector<void *>::iterator voidItr = results.begin(); voidItr != results.end(); voidItr ++)
          {
        		void *tempGeomPtr = *voidItr;
         		Geometry* qrdGeom = (Geometry*)tempGeomPtr;
         		tempGeomVector->at(tempCounter) = qrdGeom;
         		++tempCounter;
      		}

      		pair<Geometry*, vector<Geometry *> *>* tempPair = new pair<Geometry*, vector<Geometry *>*>;

      		tempPair->first = tmpGeom;
      		tempPair->second = tempGeomVector;

      		queue.push(tempPair);
      }
    } 

    double t5 = MPI_Wtime();
    /************************Find candidates:: Main thread*********************************************************/
    double mtbegin = MPI_Wtime();

    triggerSpatialJoin(&queue, &global_win);

    double mtend = MPI_Wtime();

    long unsigned int putBuf[2] = {ALL_LOCAL_JOB_COMPLETED, ALL_LOCAL_JOB_COMPLETED};
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, myWorldRank, 0, global_win);
    MPI_Put(&putBuf, 2, MPI_UNSIGNED_LONG, myWorldRank,
      	 0, 2, MPI_UNSIGNED_LONG, global_win);

    MPI_Win_flush(myWorldRank, global_win);
    MPI_Win_unlock(myWorldRank, global_win);

    checkOtherProcessForJob(&queue, &global_win);
      
    /************************Spatial Join end**********************************************************/
      	
    //cout<<"Time for loading files to strings::"<<t2 - t1<<endl;

    //cout<<"Time for parsing strings::"<<t3 - t2<<endl;

    cout<<"Time for building RTree::"<<t4 - t3 <<endl;

    cout<<"Time for filling queue::"<<t5 - t4 << endl;

    cout<<"Time for Spatial Join::"<<mtend - mtbegin<<endl;
          
    cout<<"Total time except parsing::"<<mtend- t3<<endl;

    return 0;
}
