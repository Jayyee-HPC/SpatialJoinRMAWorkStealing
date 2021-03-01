#include "../include/quadTreeGrid.h"

#include <climits>

#define MAX_SIZE LLONG_MAX

void QuadTreeGrid :: buildRTree(list<pair<Coordinate*, int>* >* basis){
   // double t1 = MPI_Wtime();
   for(list<pair<Coordinate*,int>* >::iterator itr = basis->begin(); itr != basis->end(); ++itr){
       pair<Coordinate*,int>* tmpPair = *itr;
       index.insert(new   Envelope(*(tmpPair->first)),&tmpPair->second);
   }
   //double t2 = MPI_Wtime();
   //cout<<"Time to build index for partition::"<<t2-t1<<endl;
}

int QuadTreeGrid :: partition(list<pair<Coordinate*, int>* >* basis,   Envelope *universe)
{
	gridCellsWBasisComplexity->push_back(new pair<Envelope*,MPI_Offset>(universe, LLONG_MAX));
	while(gridCellsWBasisComplexity->size() < numCells){
		partitionInto4Cells(gridCellsWBasisComplexity->back()->first, basis);		
	}
  return 0;
}

int QuadTreeGrid :: partitionInto4Cells( Envelope* env, list<pair<Coordinate*, int>* >* basis ){
	list<  Envelope*>* lEnvs= new list<  Envelope*>;
	double minX = env->getMinX();
	double minY = env->getMinY();
	double maxX = env->getMaxX();
	double maxY = env->getMaxY();
	
	
	double hight = env->getHeight();
	double width = env->getWidth();
	//calculate centre
	double midX = minX + width/2;
	double midY = minY + hight/2;
	
	lEnvs->push_back(new   Envelope(minX, midX, minY, midY));
	lEnvs->push_back(new   Envelope(minX, midX, midY, maxY));
	lEnvs->push_back(new   Envelope(midX, maxX, minY, midY));
	lEnvs->push_back(new   Envelope(midX, maxX, midY, maxY));
	
	for(list<  Envelope*>::iterator itr = lEnvs->begin() ; itr != lEnvs->end(); itr++){
			  Envelope* tmpEnv = *itr;
			pair<  Envelope*,MPI_Offset>* tmpPair = new pair<  Envelope*,MPI_Offset>(tmpEnv, 0);
			tmpPair->second = getComplexityForCell(basis, tmpEnv);
			//cout<<tmpPair.second<<endl;
			if(tmpPair->second!=0)
				gridCellsWBasisComplexity->push_front(tmpPair);			
	}

	//pop the cell being parted
	gridCellsWBasisComplexity->pop_back();
	
	gridCellsWBasisComplexity->sort([](  pair<  Envelope*,MPI_Offset> * a,   pair<  Envelope*,MPI_Offset> * b) { return a->second < b->second; });
	
	return 0;
}

MPI_Offset QuadTreeGrid :: getComplexityForCell(list<pair<Coordinate*, int>* >* basis,   Envelope* env){
	MPI_Offset complexity = 0;
	//MPI_Offset counter =0;
	//MPI_Offset limit = basis->size() / (MPI_Offset)numCells / 3;
	
	std::vector<void *> results;

	index.query(env, results);
	for(vector<void *>::iterator itr = results.begin(); itr != results.end(); ++itr){
			void* tmpVoid = *itr;
			//pair<Coordinate*, MPI_Offset> tmpPair = (pair<Coordinate*, MPI_Offset>)tmpVoid;
			int* tmpWeight = (int*)tmpVoid;
			
			if(LLONG_MAX - complexity < *tmpWeight){
					 complexity = LLONG_MAX;
						//return pairNum;
				}else{
					complexity += *tmpWeight;
			}
	 }
	/*	 
	for (list<pair<Coordinate*, MPI_Offset>>::iterator itr = basis->begin() ; itr != basis->end(); itr++) {
		pair<Coordinate*, MPI_Offset> candidateEnv = *itr;
		if(env->getMinX() <= candidateEnv.first->x &&
			env->getMaxX() >= candidateEnv.first->x &&
			env->getMinY() <= candidateEnv.first->y &&
			env->getMaxY() >= candidateEnv.first->y){
			if(LLONG_MAX - complexity < candidateEnv.second){
					complexity = LLONG_MAX;
					return complexity;
			}else{
					complexity += candidateEnv.second;
			}
			
		//	++counter;
		}	
	}*/

	//if(counter < limit){
	//	return 1;
//	}else{
		return complexity;
//	}
}

 MPI_Offset QuadTreeGrid ::calculateTotalComplexity(list<pair<Coordinate*, int>* >* basis){
	MPI_Offset counter =0;
	for (list<pair<Coordinate*, int>* >::iterator itr = basis->begin() ; itr != basis->end(); itr++) {
		pair<Coordinate*, int>* candidateEnv = *itr;
		if(LLONG_MAX - counter < candidateEnv->second){
				//pairNum = LLONG_MAX;
				cout<<"Exceed max weight"<<endl;
				exit(1);
				//return pairNum;
		}else{
			counter += candidateEnv->second;
		}
			
	}
	
	return counter/numCells;
}

