#ifndef __READ_GLOBAL_HEADER_H_INCLUDE__
#define __READ_GLOBAL_HEADER_H_INCLUDE__

//c c++ headers
#include <vector>
#include <fstream>
#include <list>
#include <vector>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <ctime>
#include <cmath>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unistd.h>
#include <utility>
#include <sys/stat.h>
#include <sys/types.h>

//External libs
#include "mpi.h"
#include <geos/geom/Coordinate.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/index/strtree/STRtree.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/LinearRing.h>
#include "../include/wsq.hpp"

//Internal headers
#include "../include/readStruct.h"
#include "../include/quadTreeGrid.h"

#define USE_UNSTABLE_GEOS_CPP_API

#define ELEM_PER_PROC 2
#define ITERATIONS_PER_CHECK_JOB_REQUEST 100
#define NUM_JOBS_TO_MARK_FINISHED 128
#define NUM_JOBS_TO_SEND 128
#define ALL_LOCAL_JOB_NOT_COMPLETED 0xFFFF0001
#define STARVE_PROCESS_WAITING 0xFFFF0002
#define ALL_LOCAL_JOB_COMPLETED 0xFFFF0003
#define THRESH_HOLD_FOR_CHECK_JOB_REQUEST 100
#define SEND_BUF_0_SIZE 4
#define NUM_THIVES 4

typedef std::unique_ptr<geos::geom::Geometry> GeometryAutoPtr;
typedef std::unique_ptr<geos::geom::Polygon> PolygonAutoPtr;

void generateSendBuf(list<pair<Geometry*, vector<Geometry *>*> *> * listOfGeometriesToSend, 
		long unsigned int *sendBuf0, double* &sendBufGeoms, long unsigned int* &sendBufSizes, long unsigned int* &sendMappingArray);

void extractGeomsFromRecvBuf(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, 
		long unsigned int *recvBuf0, double* recvBufGeoms, long unsigned int* recvBufSizes, long unsigned int* recvMappingArray);

void ownerThread(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, MPI_Win *win,
		std::list<Geometry* > *lGeoms1, std::list<Geometry* > *lGeoms2);

void thiefThread(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue);

void triggerSpatialJoin(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, MPI_Win *win, 
		std::list<Geometry* > *lGeoms1, std::list<Geometry* > *lGeoms2);

void triggerSpatialJoinWithoutOwner(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue);

int getMostNearNode(bool &isAllGlobalJobDone, MPI_Win &win);

void checkOtherProcessForJob(WorkStealingQueue<pair<Geometry*, vector<Geometry *>*>*>* queue, MPI_Win &win);

int spatialJoin(list<Geometry* > *lGeoms1, list<Geometry* > *lGeoms2);

Geometry* arrayToGeom(double* geomArray, long unsigned int numVertices, const GeometryFactory* gf);

#endif
