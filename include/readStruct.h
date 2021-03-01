#ifndef __READ_STRUCT_H_INCLUDE__
#define __READ_STRUCT_H_INCLUDE__

#include <vector>
#include <fstream>
#include "mpi.h"
#include <list>
#include <stdlib.h>
#include <cstring>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>

using namespace std;
using namespace geos::geom;

typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;

typedef struct MBR
{
	double minX;
	double maxX;
	double minY;
	double maxY;
   
	MBR(double x1, double x2, double y1, double y2){
		minX = x1;
		maxX = x2;
		minY = y1;
		maxY = y2;
	}
}MBR;

class readStruct{
    public:
		void MPIReadStrsSplit(const string filepath,vector<string> *vStrs, MPI_Comm comm);
        void ReadStrsFromFile(const string filepath,vector<string> *vStrs);
        void ReadMBRsFromStrs(vector<string> *vStrs, vector<MBR *> *vMBRs);
        void ReadEnvsFromStrs(vector<string> *vStrs, vector<Envelope *> *vEnvs);
        void ReadMBRsWeightsFromStrs(vector<string> *vStrs, vector<pair<Envelope *, int>* > *vEnvs);
        void ReadGeosFromStrs(vector<string> *vStrs, list<Geometry*> *lGeoms);

    private:

};

#endif
