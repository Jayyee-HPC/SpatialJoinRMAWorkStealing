#include "../include/globalHeader.h"

int getMostNearNode(bool &isAllGlobalJobDone, MPI_Win &win){
	//TODO using topology to return the nearest node with jobs
	//
	int myWorldRank;
    int numWorldNodes;
	int getTarget = -1;
	int i;
	long unsigned int *checkBuf, *putBuf;

	isAllGlobalJobDone = true;

	MPI_Alloc_mem(sizeof(long unsigned int) * ELEM_PER_PROC, MPI_INFO_NULL, &checkBuf);
	MPI_Alloc_mem(sizeof(long unsigned int) * ELEM_PER_PROC, MPI_INFO_NULL, &putBuf);

	MPI_Comm_size(MPI_COMM_WORLD,&numWorldNodes);
    MPI_Comm_rank(MPI_COMM_WORLD,&myWorldRank);

    putBuf[0] = STARVE_PROCESS_WAITING;
	putBuf[1] = myWorldRank;
	checkBuf[0] = checkBuf[1] = 0;

    for(i = 0; i < numWorldNodes; ++i){
    	//Every process begin with itself to avoid too much collision
    	int tempCounter = (i + myWorldRank) % numWorldNodes;

    	if(tempCounter != myWorldRank){
    		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, tempCounter, 0, win);
			MPI_Get(checkBuf, ELEM_PER_PROC, MPI_UNSIGNED_LONG, tempCounter,
                    0, ELEM_PER_PROC, MPI_UNSIGNED_LONG, win);
			//At least one process has jobs
			if(checkBuf[0] != ALL_LOCAL_JOB_COMPLETED){
				isAllGlobalJobDone = false;
			}

			std::cout<<tempCounter<<" "<<std::hex<<checkBuf[0]<<" "<<std::hex<<checkBuf[1]<<std::endl;

			if(checkBuf[0] == ALL_LOCAL_JOB_NOT_COMPLETED){
				MPI_Put(putBuf, ELEM_PER_PROC, MPI_UNSIGNED_LONG, tempCounter,
                    0, ELEM_PER_PROC, MPI_UNSIGNED_LONG, win);
				getTarget = tempCounter;
				MPI_Win_flush(tempCounter, win);
				MPI_Win_unlock(tempCounter, win);
				return getTarget;
			}

			MPI_Win_flush(tempCounter, win);
			MPI_Win_unlock(tempCounter, win);
    	}

    }

    return getTarget;
}
void checkOtherProcessForJob(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, MPI_Win &win){
	int myWorldRank;
    int numWorldNodes;
    int target;

	double* recvBufGeoms;
	long unsigned int* recvBufSizes;
	long unsigned int* recvMappingArray;

    bool isAllGlobalJobDone = false;
    long unsigned int* recvBuf0;
    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD,&numWorldNodes);
    MPI_Comm_rank(MPI_COMM_WORLD,&myWorldRank);
	
    while(!isAllGlobalJobDone){
		target = getMostNearNode(isAllGlobalJobDone, win);

		if(isAllGlobalJobDone || target < 0 || target >= numWorldNodes)
			break;

		//TODO obtain jobs from the target process
		//Recv four numbers for receiving geometries. 0 total geometries from layer1; 1 size of the array of sizes of geometries from layer1;
		// 2 total geometries from layer2; 3 size of the array of sizes of geometries from layer2;
		//mpi recv()
		//do all recvied jobs
		recvBuf0 = (long unsigned int*)malloc(SEND_BUF_0_SIZE * sizeof(long unsigned int));
		MPI_Recv(recvBuf0, SEND_BUF_0_SIZE, MPI_UNSIGNED_LONG, target, 0, MPI_COMM_WORLD, &status);

		cout<<"Target:: "<<target<<endl;
		cout<<"Recvbuf:: ";
		for(int i = 0; i < SEND_BUF_0_SIZE; ++i){
			cout<<recvBuf0[i];
			cout<<" ";
		}
		cout<<endl;

		if(recvBuf0[0] == ALL_LOCAL_JOB_COMPLETED){
			continue;
		}else{
			recvBufGeoms = (double *)malloc((recvBuf0[1] + recvBuf0[3])*2 * sizeof(double));
			recvBufSizes = (long unsigned int*)malloc((recvBuf0[0] + recvBuf0[2]) * sizeof(long unsigned int));
			recvMappingArray = (long unsigned int*)malloc((recvBuf0[0]) * sizeof(long unsigned int));

			MPI_Recv(recvBufGeoms, (recvBuf0[1] + recvBuf0[3])*2, 
				MPI_DOUBLE, target, myWorldRank, MPI_COMM_WORLD, &status);
			MPI_Recv(recvBufSizes, recvBuf0[0] + recvBuf0[2], 
				MPI_UNSIGNED_LONG, target, myWorldRank + 1, MPI_COMM_WORLD, &status);
			MPI_Recv(recvMappingArray, recvBuf0[0], 
				MPI_UNSIGNED_LONG, target, myWorldRank + 2, MPI_COMM_WORLD, &status);

			extractGeomsFromRecvBuf(queue, recvBuf0, recvBufGeoms, recvBufSizes, recvMappingArray);
			
			triggerSpatialJoinWithoutOwner(queue);
		}
	
	}	
	return;
}	