#include "../include/globalHeader.h"

//Send four numbers for sending geometries. 0 # of geometries from layer1; 1 size of the array of sizes of geometries from layer1;
// 2 # of geometries from layer2; 3 size of the array of sizes of geometries from layer2;
// TODO: geometry types getGeometryTypeId()
// 
// sendBufGeoms [Pair0.geoms. x,y,x,y....][Pair0.vect. x,y,x,y...][Pair1...
// sendBufSizes [sizeof geoms of Pair0][sizeof geom0 in vect of Pair0][sizeof geom1 in vect of Pair0]...

void generateSendBuf(list<pair<Geometry*, vector<Geometry *>*> *> * listOfGeometriesToSend, 
	long unsigned int *sendBuf0, double* &sendBufGeoms, long unsigned int* &sendBufSizes, long unsigned int* &sendMappingArray)
{
	long unsigned int i, j, initSendBufGeomsCounter, initSendBufSizesCounter, mappingArrayCounter;
	//std::unique_ptr<CoordinateSequence> verticesOfGeom;

	if(sendBuf0 == NULL){
		std::cerr << "NULL pointer [sendBuf0] accssed!" << std::endl;
		exit(0);
	}

	sendBuf0[0] = listOfGeometriesToSend->size();
	sendBuf0[1] = 0;
	sendBuf0[2] = 0;
	sendBuf0[3] = 0;

	initSendBufGeomsCounter = 0;
	initSendBufSizesCounter = 0;
	mappingArrayCounter = 0;

	sendMappingArray = (long unsigned int *)malloc(sendBuf0[0] * sizeof(long unsigned int));
	if(sendMappingArray == NULL){
		std::cerr << "Malloc [sendMappingArray] failed!" << std::endl;
		exit(0);
	}	



	for(list<pair<Geometry*, vector<Geometry *>*> *>::iterator listItr = listOfGeometriesToSend->begin(); 
		listItr != listOfGeometriesToSend->end(); ++listItr){
		std::vector< Coordinate > coordsOfLayer1;
		std::vector< Coordinate > coordsOfLayer2;

		initSendBufGeomsCounter = sendBuf0[1] + sendBuf0[3];

		pair<Geometry*, vector<Geometry *>*>* tempPair = *listItr;

		tempPair->first->getCoordinates()->toVector(coordsOfLayer1);

		sendBuf0[1] += coordsOfLayer1.size();
		sendBuf0[2] += tempPair->second->size();
		sendMappingArray[mappingArrayCounter] = tempPair->second->size();
		++mappingArrayCounter;

		sendBufGeoms = (double*)realloc(sendBufGeoms, 
				(initSendBufGeomsCounter + coordsOfLayer1.size()) * 2 * sizeof(double));

		if(sendBufGeoms == NULL){
			std::cerr << "Malloc [sendBufGeoms] failed!" << std::endl;
			exit(0);
		}

		for(i = 0; i < coordsOfLayer1.size(); ++i){
			sendBufGeoms[2 * initSendBufGeomsCounter + 2 * i] = coordsOfLayer1[i].x;
			sendBufGeoms[2 * initSendBufGeomsCounter + 2 * i + 1] = coordsOfLayer1[i].y;
		}

		initSendBufGeomsCounter += coordsOfLayer1.size();

		sendBufSizes = (long unsigned int *)realloc(sendBufSizes, 
				(initSendBufSizesCounter + 1 + tempPair->second->size()) * sizeof(long unsigned int));

		if(sendBufSizes == NULL){
			std::cerr << "Malloc [sendBufGeoms] failed!" << std::endl;
			exit(0);
		}

		sendBufSizes[initSendBufSizesCounter] = coordsOfLayer1.size();

		j = 1;

		for(vector<Geometry *>::iterator vectItr =  tempPair->second->begin();
			vectItr != tempPair->second->end(); ++vectItr){
			(*vectItr)->getCoordinates()->toVector(coordsOfLayer2);
			sendBuf0[3] += coordsOfLayer2.size();

			sendBufGeoms = (double*)realloc(sendBufGeoms, 
					(initSendBufGeomsCounter + coordsOfLayer2.size()) * 2 * sizeof(double));

			if(sendBufGeoms == NULL){
				std::cerr << "Malloc [sendBufGeoms] failed!" << std::endl;
				exit(0);
			}

			for(i = 0; i < coordsOfLayer2.size(); ++i){
				sendBufGeoms[2 * initSendBufGeomsCounter + 2 * i] = coordsOfLayer2[i].x;
				sendBufGeoms[2 * initSendBufGeomsCounter + 2 * i + 1] = coordsOfLayer2[i].y;
			}

			initSendBufGeomsCounter += coordsOfLayer2.size();

			sendBufSizes[initSendBufSizesCounter + j] = coordsOfLayer2.size();
			++j;
		}

		initSendBufSizesCounter += (1 + tempPair->second->size());
	}


}

//Recv four numbers for sending geometries. 0 # of geometries from layer1; 1 size of the array of sizes of geometries from layer1;
// 2 # of geometries from layer2; 3 size of the array of sizes of geometries from layer2;
// TODO: geometry types getGeometryTypeId()
// 
// recvBufGeoms [Pair0.geoms. x,y,x,y....][Pair0.vect. x,y,x,y...][Pair1...
// recvBufSizes [sizeof geoms of Pair0][sizeof geom0 in vect of Pair0][sizeof geom1 in vect of Pair0]...
// 			MPI_Recv(recvBufGeoms, (recvBuf0[1] + recvBuf0[3])*2, 
//				MPI_DOUBLE, target, myWorldRank, MPI_COMM_WORLD, &status);
//			MPI_Recv(recvBufSizes, recvBuf0[0] + recvBuf0[2], 
//				MPI_UNSIGNED_LONG, target, myWorldRank + 1, MPI_COMM_WORLD, &status);


void extractGeomsFromRecvBuf(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, 
	long unsigned int *recvBuf0, double* recvBufGeoms, long unsigned int* recvBufSizes, long unsigned int* recvMappingArray)
{
	long unsigned int i, j, numGeoms, sizeGeom1, sizeGeom2, numMapGeoms, mappingArrayCounter, geomsArrayStartLoc;
	double *geomArray1, *geomArray2;
	Geometry *geom1, *geom2;
	pair<Geometry*, vector<Geometry *>*>* tempPair = NULL;
	geos::geom::GeometryFactory::Ptr gf = GeometryFactory::create();
	

	if(queue == NULL || recvBuf0 == NULL || recvBufGeoms == NULL 
			|| recvBufSizes == NULL || recvMappingArray == NULL)
		return;

	numGeoms = recvBuf0[0] + recvBuf0[2];

	i = j = mappingArrayCounter = geomsArrayStartLoc = 0;


	for(; i < numGeoms; ){
		sizeGeom1 = recvBufSizes[i];
		geomArray1 = (double*)malloc(sizeGeom1 * 2 * sizeof(double));
		if(geomArray1 == NULL){
			std::cerr << "Malloc [geomArray1] failed!" << std::endl;
			exit(0);
		}

		tempPair = new pair<Geometry*, vector<Geometry *>*>();

		memcpy(geomArray1, recvBufGeoms + geomsArrayStartLoc * 2 * sizeof(double), sizeGeom1 * 2 * sizeof(double));

		geom1 = arrayToGeom(geomArray1, sizeGeom1, gf->getDefaultInstance());
		tempPair->first = geom1;
		geomsArrayStartLoc += sizeGeom1;

		numMapGeoms = recvMappingArray[mappingArrayCounter];
		++mappingArrayCounter;

		tempPair->second->resize(numMapGeoms);
		for(j = i+1; j < i+1+numMapGeoms; ++j){
			sizeGeom2 = recvBufSizes[j];
			geomArray2 = (double*)malloc(sizeGeom2 * 2 * sizeof(double));
			if(geomArray2 == NULL){
				std::cerr << "Malloc [geomArray1] failed!" << std::endl;
				exit(0);
			}

			memcpy(geomArray2, recvBufGeoms + geomsArrayStartLoc * 2 * sizeof(double), sizeGeom2 * 2 * sizeof(double));

			geom2 = arrayToGeom(geomArray2, sizeGeom2, gf->getDefaultInstance());
			tempPair->second->at(j - i - 1) = geom2;
			geomsArrayStartLoc += sizeGeom2;
		}

		i += (1+numMapGeoms);
		queue->push(tempPair);
	}
	
}

Geometry* arrayToGeom(double* geomArray, long unsigned int numVertices, const GeometryFactory* gf){
	long unsigned int i;
	geos::geom::Geometry* geo = NULL;
	geos::geom::Polygon* poly = NULL;
	geos::geom::CoordinateArraySequence* coordsArray = NULL;
	geos::geom::LinearRing* exterior = NULL;
	std::vector< Coordinate > *coords = NULL;

	coords = new std::vector< Coordinate >(numVertices);

	for(i = 0; i < numVertices; ++i){
		coords->at(i).x = geomArray[2*i];
		coords->at(i).y = geomArray[2*i + 1];
	}

	coordsArray = new geos::geom::CoordinateArraySequence(coords);

	try {
        // Create non-empty LinearRing instance
        geos::geom::LinearRing ring(coordsArray, gf);

        // Exterior (clone is required here because Polygon takes ownership)
        geo = ring.clone().release();
        exterior = dynamic_cast<geos::geom::LinearRing*>(geo);

        poly = 	gf->createPolygon(exterior, NULL);
    }
    catch(exception &e) {
        //skip
    }
    return poly;
}
