/*
 * DvmSystemCoreValuesAccelerometer.h
 *
 *  Created on: Sep 15, 2014
 *      Author: salma
 */
#ifndef DALVIK_SYSTEM_CORE_ACCELEROMETER_H_
#define DALVIK_SYSTEM_CORE_ACCELEROMETER_H_


#include <math.h>
#include <string>
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


void getAccelerometerData();
std::string getCurrentActivity();

#endif
