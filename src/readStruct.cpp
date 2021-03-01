#include "../include/readStruct.h"

void readStruct ::  ReadStrsFromFile(const string filepath,vector<string> *vStrs){
    ifstream file(filepath.c_str());
	string str;

    while(std::getline(file,str)){
       //omit empty strings and invalid strings
        if(str.size() > 5)
            vStrs->push_back(str);
    }
    file.close();
}

void readStruct ::  ReadGeosFromStrs(vector<string> *vStrs, list<Geometry*> *lGeoms){
    geos::io::WKTReader wktreader;

    for(vector<string>::iterator itr = vStrs->begin();itr != vStrs->end();++itr){
        string tmpStr = *itr;
        Geometry* tmpGeo = NULL; 
        //GeomPtr geom;   
        try{
            //GeomPtr geom(wktreader.read("POINT(-117 33)"));

            tmpGeo = (wktreader.read(tmpStr)).release();
            //tmpGeo = wktreader.read("MULTIPOINT (1 1, 2 2)");
            //tmpGeo = wktreader.read("POINT(-117 33)");
        }catch(exception &e){
        //throw;
/*          cout<<e.what()<<endl;
            int s;
            stringstream ss;
            ss<<*tmpStr;
            ss>>s;
            ss.clear();
            cout<<s<<endl; */
        }
        if(tmpGeo != NULL && tmpGeo->isValid()){
            lGeoms->push_back(tmpGeo);
        }

        //if(geom && geom->isValid()){
          //  lGeoms->push_back(geom);
        //}
    }
}

void readStruct ::  MPIReadStrsSplit(const string filepath,vector<string> *vStrs, MPI_Comm comm){
	/*int nprocs, rank; 

	MPI_Comm_size(comm, &nprocs); 
	MPI_Comm_rank(comm, &rank); 
	
	MPI_Info myinfo;
	MPI_Info_create(&myinfo);
	MPI_Info_set(myinfo, "access_style", "read_once,sequential"); 
	MPI_Info_set(myinfo, "collective_buffering", "true"); 
	MPI_Info_set(myinfo, "romio_cb_read", "enable");
  
	MPI_Offset filesize;
	MPI_Offset localsize;
	MPI_Offset start;
	MPI_Offset end;
		
	MPI_File fh;
	//MPI_File_open(comm, filepath.c_str(), MPI_MODE_RDONLY, myinfo, &fh);
    MPI_File_open(comm, filepath.c_str(), 2, myinfo, &fh);
	MPI_File_get_size(fh, &filesize);	
	
	localsize = filesize/nprocs;
	start = rank * localsize;
	end = start + localsize - 1;
	
    //last process need to reach the end of file
    if(nprocs - 1 == rank){
        end = filesize - 1;
    }
    
    MPI_Offset chunkSize = end - start + 1;
    
    //Allocate memory for main chunk
    char * chunk;
    chunk = (char *)malloc((chunkSize + 1) * sizeof(char));

    if(NULL == chunk){
        cerr<<"Error in malloc for file chunk"<<endl;
        exit(1);
    }
    //To store incomplete strings
    char *frontChunk;
    char *backChunk;

    MPI_File_read_at(fh, start, chunk, chunkSize, MPI_CHAR, MPI_STATUS_IGNORE);

    chunk[chunkSize] = '\0';
    
    //Get last string which should be incomplete
    MPI_Offset validEnd = chunkSize;
    if(nprocs - 1 != rank){
        for(;validEnd >=0; --validEnd){
            if(chunk[validEnd] == '\n'){
                backChunk = (char *)malloc((chunkSize - validEnd)*sizeof(char));
                strncpy(backChunk, chunk+validEnd+1, chunkSize - validEnd);
                //cout<<"Back chunk ::"<<backChunk<<endl;
                break;
            }
        }
    }

    //Get first string which should be incomplete
    MPI_Offset validStart = 0;
    if(0 != rank){
        for(;validStart < chunkSize;++validStart){
            if(chunk[validStart] == '\n'){
                frontChunk = (char*)malloc((validStart+1)*sizeof(char));
                strncpy(frontChunk, chunk, validStart);
                frontChunk[validStart] = '\0';
                ++validStart;
                //cout<<"Front chunk ::"<<frontChunk<<endl;
                break;
            }
        }
    }
	MPI_Offset strStart = validStart;
	MPI_Offset strEnd = validStart;

	for(; validStart <= validEnd; ++validStart){
		if(chunk[validStart] == '\n' || validStart == validEnd){
			strEnd = validStart;
			MPI_Offset strLen = strEnd-strStart;
			char *tmpChunk;
			tmpChunk = (char *)malloc((strLen+1) * sizeof(char));
			strncpy(tmpChunk, chunk+strStart, strLen);
			tmpChunk[strLen] = 	'\0';	
			vStrs->push_back(tmpChunk);

			free(tmpChunk);
			strStart = validStart+1;
		}
}
	
	//cout<<vStrs->size()<<endl;
	char * recvChunk;


	if(0 == rank%2){
		if(nprocs - 1 != rank){
			MPI_Offset sendLen = strlen(backChunk);
			MPI_Send(&sendLen, 1, MPI_OFFSET, rank+1, rank, MPI_COMM_WORLD);
			MPI_Send(backChunk, sendLen, MPI_CHAR, rank+1, rank+1, MPI_COMM_WORLD);
			//printf("Send of rank %d :: %lld :: %s \n", rank, sendLen,backChunk);
		}

 		if(0 != rank){
			MPI_Offset recvLen = 0;
			MPI_Status * status = new MPI_Status();

			MPI_Recv(&recvLen, 1, MPI_OFFSET, rank-1, rank-1,MPI_COMM_WORLD, status);
			recvChunk = (char *)malloc((recvLen) * sizeof(char));
			//printf("Recvlen of rank %d :: %lld \n", rank, recvLen);
			MPI_Recv(recvChunk, recvLen, MPI_CHAR, rank-1, rank,MPI_COMM_WORLD, status);

			//printf("Recv of rank %d :: %s \n", rank, recvChunk);
		}
	}else{
		MPI_Offset recvLen = 0;
		MPI_Status * status = new MPI_Status();

		MPI_Recv(&recvLen, 1, MPI_OFFSET, rank-1, rank-1,MPI_COMM_WORLD, status);
		recvChunk = (char *)malloc((recvLen) * sizeof(char));
		//printf("Recvlen of rank %d :: %lld \n", rank, recvLen);
		MPI_Recv(recvChunk, recvLen, MPI_CHAR, rank-1, rank,MPI_COMM_WORLD, status);

		//printf("Recv of rank %d :: %s \n", rank, recvChunk);

 		if(nprocs - 1 != rank){
			MPI_Offset sendLen = strlen(backChunk);
			MPI_Send(&sendLen, 1, MPI_OFFSET, rank+1, rank, MPI_COMM_WORLD);
			MPI_Send(backChunk, sendLen, MPI_CHAR, rank+1, rank+1, MPI_COMM_WORLD);
			//printf("Send of rank %d ::%lld:: %s \n", rank, sendLen,backChunk);
		}
	}
 	if(rank != 0){
		string tmpStr = string(recvChunk)+string(frontChunk);
		vStrs->push_back(tmpStr);
	}

    MPI_File_close(&fh);*/
}

void readStruct :: ReadMBRsFromStrs(vector<string> *vStrs, vector<MBR *> *vMBRs){
    unsigned int size = vStrs->size();
    for(unsigned int i = 0; i < size; i++){
        string tmpStr = vStrs->at(i);
        int start = 0;
        int end = 0;
        double env[4];
        unsigned int length = tmpStr.size();
        for(unsigned int j = 0; j < length; j++){
            int k = 0;
            if(tmpStr[j] == ' '){
                end = j;
			    //cout<<start<<"::"<<end<<endl;
				//cout<<tmpStr.substr(start, end -start)<<"!!!"<<endl;
                env[k++]=stod(tmpStr.substr(start, end -start));
				start = j+1;
            }
            
        }
        vMBRs->push_back(new MBR(env[0], env[1], env[2], env[3]));   
    }    
}

void readStruct :: ReadEnvsFromStrs(vector<string> *vStrs, vector<Envelope *> *vEnvs){
    unsigned int size = vStrs->size();
    for(unsigned int i = 0; i < size; i++){
        string tmpStr = vStrs->at(i);
        //if(i==0)
        //cout<<tmpStr<<endl;
        int start = 0;
        int end = 0;
        double * env = new double[4];
        int k = 0;
        unsigned int length = tmpStr.size();
        for(unsigned int j = 0; j < length; j++){
            
            if(tmpStr[j] == ' ' && k < 4){
                end = j;
                env[k]=strtod((tmpStr.substr(start, end -start)).c_str(), NULL);
                //if(i==0)
                //cout<<tmpStr.substr(start, end -start)<<endl;
                k++;
                start = j+1;
            }
           if(4 == k)break;  
        }
       // cout<<tmpStr.substr(start, length-start)<<endl;
        //if(i==0)
        //printf("%f :: %f :: %f :: %f :: %d\n", env[0], env[1], env[2], env[3],stoi(tmpStr.substr(start, length-start)));
        //cout<<env[0]<<"::"<<env[1]<<"::"<< env[2]<<"::"<< env[3]<<endl;
        vEnvs->push_back(new Envelope(env[0], env[1], env[2], env[3]));
    }
}

void readStruct :: ReadMBRsWeightsFromStrs(vector<string> *vStrs, vector<pair<Envelope *, int>* > *vEnvs){
    unsigned int size = vStrs->size();
    for(unsigned int i = 0; i < size; i++){
        string tmpStr = vStrs->at(i);
        int start = 0;
        int end = 0;
        double * env = new double[4];
        int k = 0;
        unsigned int length = tmpStr.size();
        for(unsigned int j = 0; j < length; j++){
            
            if(tmpStr[j] == ' ' && k < 4){
                end = j;
                env[k]=atof(tmpStr.substr(start, end -start).c_str());
                k++;
                start = j+1;
            }
            if(4 == k)break;           
        }
        //cout<<tmpStr.substr(start, length-start)<<endl;]
		//if(env[0] == env[1] && env[0] == env[2] && env[0] == env[3])printf("%s while get %f \n", tmpStr.c_str(), env[0]);
		//fflush(stdout);
        vEnvs->push_back(new pair<Envelope *, int>(new Envelope(env[0], env[1], env[2], env[3]), atoi((tmpStr.substr(start, length-start)).c_str())));
       // cout<<tmpStr<<endl;
        //cout<<tmpStr.substr(start, length-start)<<"::"<<atoi((tmpStr.substr(start, length-start)).c_str())<<"   END"<<endl;
   		//if(size-1 == i )cout<<vEnvs->back().first->toString()<<endl;
    }
}
