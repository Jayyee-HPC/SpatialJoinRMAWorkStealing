#include "../include/globalHeader.h"

bool IS_ALL_JOB_COMPLETED = false;

void sj_delay(unsigned long ms)
{
	double t_begin = MPI_Wtime();
	double t_end = MPI_Wtime();

	while((t_end - t_begin)*1e3 < ms)
	{
		t_end = MPI_Wtime();
	}

	return;
}
void thiefThread(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue)
{
	int myWorldRank;
	MPI_Comm_rank(MPI_COMM_WORLD,&myWorldRank);

	while(queue->empty())
	{
		//Wait for jobs
	}

	sj_delay(100); // wait 0.1 second for the main thread to fill the queue

	int counter = 0;
	while(!queue->empty()) 
	{
		std::optional<pair<Geometry*, vector<Geometry *>*>*> item = queue->steal();
		pair<Geometry*, vector<Geometry *>*> *tempPairGeoms = item.value();

		for(vector<Geometry *>::iterator voidItr = tempPairGeoms->second->begin(); voidItr != tempPairGeoms->second->end(); voidItr ++)
		{
   			Geometry* qrdGeom = *voidItr;
			try{
				//if(tempPairGeoms->first->intersects(qrdGeom))
					//++counter;
				std::unique_ptr<Geometry> geoms_intersection = tempPairGeoms->first->intersection(qrdGeom);
				counter += geoms_intersection->getNumPoints();
			}
			catch(exception &e)
			{
				//Do nothing, just skip the exception
			}
		}
	}
	cout<<"Rank:" << myWorldRank << "  Result:"<<counter<<endl;
}	

void ownerThread(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, MPI_Win *win, 
		std::list<Geometry* > *lGeoms1, std::list<Geometry* > *lGeoms2)
{
	int myWorldRank;
	size_t leftItems;
	int counter = 0;
	int iterationCounter = THRESH_HOLD_FOR_CHECK_JOB_REQUEST;
	long unsigned int* checkBuf;
	MPI_Request request;
	list<pair<Geometry*, vector<Geometry *>*> *> *listOfGeometriesToSend = 
		new list<pair<Geometry*, vector<Geometry *>*> *>();

	if(queue == NULL || win == NULL || lGeoms1 == NULL || lGeoms2 == NULL)
		return;

	if(lGeoms1->empty() || lGeoms2->empty())
		return;

	/************************Build index************************************************************************/
    geos::index::strtree::STRtree indexForLayer1;
    for(list<Geometry* >::iterator itr = lGeoms1->begin(); itr != lGeoms1->end(); ++itr)
    {
         Geometry* tmpGeom =  *itr;
      
         indexForLayer1.insert(tmpGeom->getEnvelopeInternal(), tmpGeom);
    }

    /************************Build index end************************************************************************/

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

      		queue->push(tempPair);
      }
    } 

	//Send four numbers for sending geometries. 0 # of geometries from layer1; 1 size of the array of sizes of geometries from layer1;
	// 2 # of geometries from layer2; 3 size of the array of sizes of geometries from layer2;
	// TODO: geometry types getGeometryTypeId()
	long unsigned int *sendBuf0; 
	double *sendBufGeoms;
	long unsigned int *sendBufSizes;
	long unsigned int *sendMappingArray;

	sendBuf0 = (long unsigned int*)malloc(SEND_BUF_0_SIZE * sizeof(long unsigned int));
	checkBuf = (long unsigned int*)malloc(ELEM_PER_PROC * sizeof(long unsigned int));

	MPI_Comm_rank(MPI_COMM_WORLD,&myWorldRank);
	
	while(!queue->empty()) 
	{
		std::optional<pair<Geometry*, vector<Geometry *>*>*> item = queue->steal();
		pair<Geometry*, vector<Geometry *>*> *tempPairGeoms = item.value();

		for(vector<Geometry *>::iterator voidItr = tempPairGeoms->second->begin(); voidItr != tempPairGeoms->second->end(); voidItr ++)
		{
   			Geometry* qrdGeom = *voidItr;
			try{
				//if(tempPairGeoms->first->intersects(qrdGeom))
					//++counter;

				std::unique_ptr<Geometry> geoms_intersection = tempPairGeoms->first->intersection(qrdGeom);
				counter += geoms_intersection->getNumPoints();
			}
			catch(exception &e)
			{

			}

		}
		++iterationCounter;

		if(iterationCounter >= THRESH_HOLD_FOR_CHECK_JOB_REQUEST)
		{
			MPI_Win_lock(MPI_LOCK_EXCLUSIVE, myWorldRank, 0, *win);
			MPI_Get(checkBuf, ELEM_PER_PROC, MPI_UNSIGNED_LONG, myWorldRank,
                    0, ELEM_PER_PROC, MPI_UNSIGNED_LONG, *win);

			if(checkBuf[0] == STARVE_PROCESS_WAITING)
			{
				//printf("Found starve process %d n", checkBuf[1]);
				leftItems = queue->size();

				if(leftItems < NUM_JOBS_TO_MARK_FINISHED + NUM_JOBS_TO_SEND)
				{
					for(int i = 0; i < SEND_BUF_0_SIZE; ++i)
					{
						sendBuf0[i] = ALL_LOCAL_JOB_COMPLETED;
					}
					
					MPI_Isend(sendBuf0, SEND_BUF_0_SIZE, MPI_UNSIGNED_LONG, checkBuf[1], 0, MPI_COMM_WORLD, &request);
					MPI_Put(sendBuf0, 2, MPI_UNSIGNED_LONG, myWorldRank, 0, 2, MPI_UNSIGNED_LONG, *win);


				}
				else
				{
					//Currently sending a fixed number of jobs, NUM_JOBS_TO_SEND				sendBuf0[0] = NUM_JOBS_TO_SEND;
					for(int i = 0; i < NUM_JOBS_TO_SEND; ++i)
					{
						std::optional<pair<Geometry*, vector<Geometry *>*>*> tempItemToSend = queue->steal();
						pair<Geometry*, vector<Geometry *>*> *tempPairGeomsToSend = tempItemToSend.value();

						listOfGeometriesToSend->push_back(tempPairGeomsToSend);
					}

					generateSendBuf(listOfGeometriesToSend, sendBuf0, sendBufGeoms, sendBufSizes, sendMappingArray);

					//transfer jobs when starve processes found
					//float arrays 
					//isend
					MPI_Isend(sendBuf0, SEND_BUF_0_SIZE, 
							MPI_UNSIGNED_LONG, checkBuf[1], 0, MPI_COMM_WORLD, &request);
					MPI_Isend(sendBufGeoms, (sendBuf0[1] + sendBuf0[3])*2, 
							MPI_DOUBLE, checkBuf[1], checkBuf[1], MPI_COMM_WORLD, &request);
					MPI_Isend(sendBufSizes, sendBuf0[0] + sendBuf0[2], 
							MPI_UNSIGNED_LONG, checkBuf[1], checkBuf[1] + 1, MPI_COMM_WORLD, &request);
					MPI_Isend(sendMappingArray, sendBuf0[0], 
							MPI_UNSIGNED_LONG, checkBuf[1], checkBuf[1] + 2, MPI_COMM_WORLD, &request);

				}
			}
			MPI_Win_flush(myWorldRank, *win);
			MPI_Win_unlock(myWorldRank, *win);
			iterationCounter = 0;

		}
	}
	IS_ALL_JOB_COMPLETED = true;
	cout<<"Rank:" << myWorldRank << "  Result:"<<counter<<endl;
}

void triggerSpatialJoin(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, MPI_Win *win, 
	list<Geometry* > *lGeoms1, list<Geometry* > *lGeoms2)
{
	// only one thread can push and pop
	std::thread owner = thread(ownerThread, queue, win, lGeoms1, lGeoms2);
	size_t numThives = NUM_THIVES;

	std::thread *thief = new std::thread[numThives];

	for(size_t i = 0; i < numThives; ++i)
	{
		thief[i] = thread(thiefThread, queue);
	}

	owner.join();
	for(size_t i = 0; i < numThives; ++i)
		thief[i].join();

}	

void triggerSpatialJoinWithoutOwner(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue)
{
	// only one thread can push and pop
	size_t numThives = NUM_THIVES + 1;
               
	std::thread *thief = new std::thread[numThives];

	for(size_t i = 0; i < numThives; ++i)
	{
		thief[i] = thread(thiefThread, queue);
	}

	//owner.join();
	for(size_t i = 0; i < numThives; ++i)
		thief[i].join();

}	