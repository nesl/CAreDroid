
#include <utils/Log.h>
//#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/TextOutput.h>
#include <utils/Vector.h>
//#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <binder/IBinder.h>
#include <iostream>
#include <string>
#include <fstream>
#include <pthread.h>
#include "DvmSystemCoreValuesLocation.h"


using namespace android;

static double curlatitude = 0;
static double curlongitude = 0;


void getCurrentLocation(double& latitudeReport, double& longitudeReport){
	latitudeReport = curlatitude;
	longitudeReport = curlongitude;

}



static int parse_location(){

	const int MAX_TOKENS_PER_LINE =3; /*512;*/

	// array to store memory addresses of the tokens in buf
	char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
	std::string line;
	std::ifstream in("/sdcard/location2.txt");
	//int lineno = 0;
	while (!in.eof()){
		getline(in,line);
	//	ALOGD("%d, %s",++lineno, line.c_str());
		if(line.find("Last Known Locations")!= std::string::npos){
			getline(in,line);
			if(line.find("Location[gps") == std::string::npos) {
				//not found
				ALOGD("No Last Known Locations");
				return 1;
			}
		 //	ALOGD("%s",line.c_str());
			 // parse the line
			char * tmp =const_cast<char*>(line.c_str());
			token[0] = strtok(tmp, " "); // first token
			if (token[0]) // zero if line is blank
			{
			  for (int n = 1; n < MAX_TOKENS_PER_LINE; n++)
			  {
			     token[n] = strtok(0, " "); // subsequent tokens
			    // ALOGD("\t %d, %s", n, token[n] );
			     if (!token[n]) break; // no more tokens
			  }
			}

			//get longitude and latitude
			char* location = token[2];
			char* tokenLoc[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
			if (location != NULL){
				tokenLoc[0] = strtok(location, ","); // first token
				if (tokenLoc[0]) // zero if line is blank
				{
				  for (int n = 1; n < MAX_TOKENS_PER_LINE; n++)
				  {
					  tokenLoc[n] = strtok(0, ","); // subsequent tokens
				    // ALOGD("\t %d, %s", n, token[n] );
				     if (!tokenLoc[n]) break; // no more tokens
				  }
				}

				//parse latitude and longitude
				char* latitude = tokenLoc[0];
				char* longitude = tokenLoc[1];

				//ALOGD("Latitude = %s, Longitude = %s",latitude, longitude);

				curlatitude = atof(latitude);
				curlongitude = atof(longitude);


				return 0;
			}
			else{
				ALOGD("Location is no reported");
			}


		}
		else{
			continue;
		}
	}
	return 1;
}


int reportLocation()
{
    sp<IServiceManager> sm = defaultServiceManager();

    if (sm == NULL) {
		ALOGE("Unable to get default service manager!");
		ALOGE("dumpsys: Unable to get default service manager!");
        return UNKNOWN_ERROR;
    }
    Vector<String16> services;
    Vector<String16> args;

    services = sm->listServices();
    args.add(String16("location"));
    sp<IBinder> service = sm->checkService(String16("location"));
    if (service != NULL) {
        FILE* dumpout;
        dumpout = fopen("/sdcard/location.txt", "w");
        if(dumpout!=NULL){
           	//ALOGD("file is opened");
            	int fd = fileno(dumpout);
            	int err = service->dump(fd, args);
            	//ALOGD("Error returns form dump is %d",err);
            	parse_location();
            	fclose(dumpout);
            	return err;
        }
        else{
     	   	ALOGE("file Location can't be opened for dump");
     	   	return 1;
        }
     }
     else {
        ALOGD("location service == null");
        return 1;
        // ALOGD("service location != NULL");
        // ALOGD("Ping Binder %d",service->pingBinder());

        // works but gives null string
        //String16 dumpbinder = get_binder_data(service);
        //if (dumpbinder != NULL){
        //    	ALOGD("%s",good_old_string(dumpbinder).string());
        //}


        // String16 IfName = get_interface_name(service);
        // if(IfName != NULL){
        // 	ALOGD("%s",good_old_string(IfName).string()); // ILocationManager
        // }

        //    String16 IfDescriptor = service->getInterfaceDescriptor();
        //    if(IfDescriptor != NULL){
        //     	ALOGD("%s", good_old_string(IfDescriptor).string());
        //     }

       //works but permission denied
       //Permission Denial: can't dump LocationManagerService from from pid=1568, uid=10044
       // fix this temporarily by disable the permission checking for the dump of location
       // This is done in LocationManager.java

   }
}



/**** Some helpers ****/

//static int sort_func(const String16* lhs, const String16* rhs)
//{
//    return lhs->compare(*rhs);
//}


//static String16 get_binder_data(sp<IBinder> service)
//{
//    if (service != NULL) {
//        Parcel data, reply;
//        //String16 str;
//        //status err_write =  data.writeString16(&str);
//        status_t err = service->transact(IBinder::DUMP_TRANSACTION, data, &reply);
//        if (err == NO_ERROR) {
//        	ALOGD("No Error in reading binder data");
//            //if(reply.read){
//           // 	ALOGD("Reply Parcel is not Null in binder data");
//        	 if (reply.dataSize() < sizeof(status_t)){
//        		ALOGD("NOT_ENOUGH_DATA");
//        		return String16();
//        	 }
//        	 else{
//        		String16 replystring = reply.readString16();
//            	if(replystring != NULL){
//            		ALOGD("Reply String is not null");
//            		return reply.readString16();
//            	}
//            }
//
//        }
//        else{
//        	ALOGD("Error in reading binder data: %d", err);
//        	return String16();
//        }
//    }
//    return String16();
//}

//static String16 ping_binder(sp<IBinder> service)
//{
//    if (service != NULL) {
//        Parcel data, reply;
//        status_t err = service->transact(IBinder::PING_TRANSACTION, data, &reply);
//        if (err == NO_ERROR) {
//            return reply.readString16();
//        }
//    }
//    return String16();
//}

// get the name of the generic interface we hold a reference to
//static String16 get_interface_name(sp<IBinder> service)
//{
//    if (service != NULL) {
//        Parcel data, reply;
//        status_t err = service->transact(IBinder::INTERFACE_TRANSACTION, data, &reply);
//        if (err == NO_ERROR) {
//        	ALOGD("No Error in interface name");
//            return reply.readString16();
//        }
//        else {
//        	ALOGD("Error reading the interface name: %d", err);
//        }
//    }
//    return String16();
//}


//static String8 good_old_string(const String16& src)
//{
//    String8 name8;
//    char ch8[2];
//    ch8[1] = 0;
//    for (unsigned j = 0; j < src.size(); j++) {
//        char16_t ch = src[j];
//        if (ch < 128) ch8[0] = (char)ch;
//        name8.append(ch8);
//    }
//    return name8;
//}
