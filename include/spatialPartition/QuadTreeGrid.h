#ifndef __QUAD_TREE_GRID_H_INCLUDED__
#define __QUAD_TREE_GRID_H_INCLUDED__
#include <mpi.h>
#include <list>
#include <cstdlib>
//#include <tuple>
#include <geos/geom/Geometry.h>
#include <geos/index/strtree/STRtree.h>

using namespace std;
using namespace geos::geom;

class QuadTreeGrid
{
 private:
 unsigned int numCells;
// MPI_Offset totalComplexity;
 //Index tree;
 
 geos::index::strtree::STRtree index;
 //Index tree;
 void buildRTree(list<pair<Coordinate*, MPI_Offset>>* basis);
 
 MPI_Offset calculateTotalComplexity(list<pair<Coordinate*, MPI_Offset>>* basis);
 int partition(list<const Envelope*>* basis, const Envelope *universe);
 int partition(list<pair<const Envelope*, MPI_Offset>>* basis, const Envelope *universe);
 int partition(list<pair<Coordinate*, MPI_Offset>>* basis, const Envelope *universe);
 
 int partitionInto4Cells(const Envelope* env, list<const Envelope*>* basis);
 int partitionInto4Cells(const Envelope* env, list<pair<const Envelope*, MPI_Offset>>* basis );
 int partitionInto4Cells(const Envelope* env, list<pair<Coordinate*, MPI_Offset>>* basis );
  
 list<pair<const Envelope*,int>> *gridCellsWBasis;
 list<pair<const Envelope*,MPI_Offset>> *gridCellsWBasisComplexity;
 int getPairNumForCell(list<const Envelope*>* basis, const Envelope* env);
 
 //pair<MPI_Offset,MPI_Offset> limit;
 MPI_Offset getComplexityForCell(list<pair<const Envelope*, MPI_Offset>>* basis, const Envelope* env);
 MPI_Offset getComplexityForCell(list<pair<Coordinate*, MPI_Offset>>* basis, const Envelope* env);
 public:

 QuadTreeGrid(int cells, const Envelope *universe, list<const Envelope*>* basis)
 {
	 numCells = cells;
   gridCellsWBasis = new list<pair<const Envelope*,int>>; 
   
   partition(basis, universe);
 }
 
 QuadTreeGrid(int cells, const Envelope *universe, list<pair<const Envelope*, MPI_Offset>>* basis)
 {
	 numCells = cells;
   gridCellsWBasisComplexity = new list<pair<const Envelope*,MPI_Offset>>; 
   
   partition(basis, universe);
 }
 
  QuadTreeGrid(int cells, const Envelope *universe, list<pair<Coordinate*, MPI_Offset>>* basis)
 {
	 numCells = cells;
	 buildRTree(basis); 
	// totalComplexity = calculateTotalComplexity(basis);
   gridCellsWBasisComplexity = new list<pair<const Envelope*,MPI_Offset>>; 
   
   partition(basis, universe);
 }
 
 list<pair<const Envelope*,int>> * getGrid(){
	 return gridCellsWBasis;
 }
 
  list<pair<const Envelope*,MPI_Offset>> * getGridComplexity(){
	 return gridCellsWBasisComplexity;
 }
 
// void printGridCoordinates();
 
// int* numShapesPerCell(); 
 
 //int populateGridCells(list<Geometry*>* shapes, bool isBaseLayer);
};


#endif