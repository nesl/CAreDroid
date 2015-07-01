/*
 * DvmSystemActivityDetection.cpp
 *
 *  Created on:  Sept 10, 2014
 *      Author: salma
 * Description: Activity code is taken from Ambulation transportmode.java from NESL github
 * code is transformed to vm internal thread by reading the accelerometer event queue directly
*/
#include "DvmSystemActivityDetection.h"
#include "DvmSystemCoreValuesAccelerometer.h" //contains getAccelerometerData()
#include <pthread.h>


pthread_t activityDetectionHandle;
bool activitythreadflag = false;

static void* activityCatcherThreadStart(void* arg);


bool dvmSystemActivityGetActivityThreadFlag(){
	return activitythreadflag;
}

bool dvmSystemActivityDetectionStartup(){

	if(activitythreadflag == false)
		if (!dvmCreateInternalThread(&activityDetectionHandle,
                "CAreDroid Activity Detection", activityCatcherThreadStart, NULL))
			return false;

    activitythreadflag = true;
    return true;
}

void dvmSystemActivityDetectionShutdown()
{

    if (activityDetectionHandle == 0)      // not started yet
        return;

    pthread_kill(activityDetectionHandle, SIGQUIT);

    pthread_join(activityDetectionHandle, NULL);
    ALOGV("CAreDroid Activity Detection has shut down");
}


static void* activityCatcherThreadStart(void* arg)
{
    Thread* self = dvmThreadSelf();

    UNUSED_PARAMETER(arg);

    while (true)  { //maybe have to put break somewhere
        dvmChangeStatus(self, THREAD_VMWAIT);

        dvmChangeStatus(self, THREAD_RUNNING);

    	getAccelerometerData();
    	// more than the queue event rate for accelerometer data to make sure that when the thread runs the queue already has data available
    	dvmThreadSleep(20, 0);
    }

    return NULL;
}
