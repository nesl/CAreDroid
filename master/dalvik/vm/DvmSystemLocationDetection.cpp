/*
 * DvmSystemLocationDetection.cpp
 *
 *  Created on:  Dec 1, 2014
 *      Author: salma
 * Description: Location Detection Thread
*/
#include "DvmSystemLocationDetection.h"
#include "DvmSystemCoreValuesLocation.h" //contains reportLocation()
#include <pthread.h>



/**** Thread Handling ****/
pthread_t locationDetectionHandle;
bool locationthreadflag = false;

static void* locationCatcherThreadStart(void* arg)
{
    Thread* self = dvmThreadSelf();

    UNUSED_PARAMETER(arg);

    while (true)  { //maybe have to put break somewhere
        dvmChangeStatus(self, THREAD_VMWAIT);

        dvmChangeStatus(self, THREAD_RUNNING);

        reportLocation();


    	// more than the queue event rate for accelerometer data to make sure that when the thread runs the queue already has data available
    	dvmThreadSleep(100, 0);
    }

    return NULL;
}


bool dvmSystemLocationGetLocationThreadFlag(){
	return locationthreadflag;
}

bool dvmSystemLocationDetectionStartup(){

	if(locationthreadflag == false)
		if (!dvmCreateInternalThread(&locationDetectionHandle,
                "CAreDroid Location Detection", locationCatcherThreadStart, NULL))
			return false;

	locationthreadflag = true;
    return true;
}

void dvmSystemLocationDetectionShutdown()
{

    if (locationDetectionHandle == 0)      // not started yet
        return;

    pthread_kill(locationDetectionHandle, SIGQUIT);

    pthread_join(locationDetectionHandle, NULL);
    ALOGV("CAreDroid Location Detection has shut down");
}



