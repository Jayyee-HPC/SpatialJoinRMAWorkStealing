#include "../include/globalHeader.h"

///home/jie.yang/test/mpich/build/_inst/bin/mpirun -np 2 ./prog 4 ~/data/sports_data_env ~/data/cemet_data_env

int main(int argc, char ** argv)
{

	MPI_Init(&argc,&argv);

	const string filePath1 = argv[1];
	const string filePath2 = argv[2];

	readStruct reader;

	vector<string> * vEnvStrs1 = new vector<string>;
	vector<string> * vEnvStrs2 = new vector<string>;

	list<Geometry* > *lGeoms1 = new list<Geometry*  >;
	list<Geometry* > *lGeoms2 = new list<Geometry*  >;

	reader.ReadStrsFromFile(filePath1, vEnvStrs1);
	reader.ReadStrsFromFile(filePath2, vEnvStrs2);

	reader.ReadGeosFromStrs(vEnvStrs1, lGeoms1);
    reader.ReadGeosFromStrs(vEnvStrs2, lGeoms2);

    printf("Geoms sizes %ld :: %ld \n", lGeoms1->size(), lGeoms2->size());

	spatialJoin(lGeoms1, lGeoms2);

	MPI_Finalize();

	return 0;
}
