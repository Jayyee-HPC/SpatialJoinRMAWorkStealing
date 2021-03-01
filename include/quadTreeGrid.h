#ifndef __QUAD_TREE_GRID_H_INCLUDED__
#define __QUAD_TREE_GRID_H_INCLUDED__

#include <mpi.h>
#include <list>
#include <cstdlib>

#include <geos/geom/Geometry.h>
#include <geos/index/strtree/STRtree.h>

using namespace std;
using namespace geos::geom;

class QuadTreeGrid
{
 private:
 unsigned int numCells;
 
 geos::index::strtree::STRtree index;
 
 void buildRTree(list<pair<Coordinate*, int>* >* basis);
 
 MPI_Offset calculateTotalComplexity(list<pair<Coordinate*, int>* >* basis);
 int partition(list<pair<Coordinate*, int>* >* basis,   Envelope *universe);

 int partitionInto4Cells(  Envelope* env, list<pair<Coordinate*, int>* >* basis );
  
 list<pair<  Envelope*,MPI_Offset>* > *gridCellsWBasis;
 list<pair<  Envelope*,MPI_Offset>* > *gridCellsWBasisComplexity;
 MPI_Offset getComplexityForCell(list<pair<Coordinate*, int>* >* basis,   Envelope* env);
 
 public:
 
  QuadTreeGrid(int cells,   Envelope *universe, list<pair<Coordinate*, int>* >* basis)
 {
	 numCells = cells;
	 buildRTree(basis); 
	// totalComplexity = calculateTotalComplexity(basis);
   gridCellsWBasisComplexity = new list<pair< Envelope*,MPI_Offset>* >; 
   
   partition(basis, universe);
 }
 
 list<pair< Envelope*,MPI_Offset>* > * getGrid(){
	 return gridCellsWBasis;
 }
 
  list<pair<Envelope*,MPI_Offset>* > * getGridComplexity(){
	 return gridCellsWBasisComplexity;
 }
};


#endif
