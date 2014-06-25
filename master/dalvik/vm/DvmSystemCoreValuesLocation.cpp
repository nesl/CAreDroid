//This file does not work
#include <utils/Log.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/TextOutput.h>
#include <utils/Vector.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "DvmSystemCoreValuesLocation.h"

using namespace android;
//static int sort_func(const String16* lhs, const String16* rhs)
//{
//    return lhs->compare(*rhs);
//}
int GetLocation(char* const servicename)
{
    sp<IServiceManager> sm = defaultServiceManager();
   // fflush(stdout);
    if (sm == NULL) {
		ALOGE("Unable to get default service manager!");
		ALOGE("dumpsys: Unable to get default service manager!");
        return 20;
    }
   // Vector<String16> services;
    Vector<String16> args;
   // if (argc == 1) {
    //    services = sm->listServices();
    //    services.sort(sort_func);
    //    args.add(String16("-a"));
   // } else {
   //     services.add(String16(argv[1]));
   //     for (int i=2; i<argc; i++) {
            args.add(String16(servicename));
    //    }
   // }
   // const size_t N = services.size();
   // if (N > 1) {
        // first print a list of the current services
       ALOGD("Currently running services:");

      //  for (size_t i=0; i<N; i++) {
            sp<IBinder> service = sm->checkService(String16(servicename));
            if (service != NULL) {
                ALOGD("service != NULL");
            }
      //  }
   // }
   // for (size_t i=0; i<N; i++) {
   //     sp<IBinder> service = sm->checkService(services[i]);
   //     if (service != NULL) {
   //         if (N > 1) {
   //            ALOGD( "------------------------------------------------------------"
   //                     "-------------------");
                ALOGD("DUMP OF SERVICE %s :", servicename);
  //          }
            FILE* dumpout;
            dumpout = fopen("/data/tmp/location.txt", "rw");
            if(dumpout!=NULL){
            	ALOGD("file is opened");
            	int fileDesriptor = fileno(dumpout);
            	int err = service->dump(fileDesriptor, args);
            	if (err != 0) {
            	   	ALOGD("Error dumping service info: (");
            	}
            	fclose(dumpout);
            }


    return 0;
}

