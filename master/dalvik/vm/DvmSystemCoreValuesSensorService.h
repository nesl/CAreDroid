
/*
 * DvmSystemCoreValuesSensorService.h
 *
 *  Created on: August 11, 2014
 *      Author: salma
 */
#ifndef DALVIK_SYSTEM_CORE_VALUES_SENSOR_SERVICE_H_
#define DALVIK_SYSTEM_CORE_VALUES_SENSOR_SERVICE_H_
#include "Dalvik.h"
#include <stdlib.h>
#include <utils/Log.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/TextOutput.h>
#include <utils/Vector.h>
#include <utils/Looper.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <SensorService.h>
#include <SensorDevice.h>
#include <android/sensor.h>
#include <gui/Sensor.h>
#include <gui/SensorManager.h>
#include <gui/SensorEventQueue.h>

#include <android/sensor.h>



using namespace android;

class DalvikSensorData {
    nsecs_t sStartTime;
	//friend class BinderService<SensorService>; //the other way round, i.e. the friend class should be this class and put in the SensorService
	public:
		DalvikSensorData(){sStartTime = 0;};
		int getAccelerometerData(	double& maccx,
									double& maccy,
									double& maccz,
									unsigned long& macctimestamp);


	private:
//		size_t maccx;
//		size_t maccy;
//		size_t maccz;
//		size_t macctimestamp;


};
#endif //DALVIK_SYSTEM_CORE_VALUES_LOCATION_H_
