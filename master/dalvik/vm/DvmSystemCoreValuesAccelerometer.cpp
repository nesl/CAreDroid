/*
 * DvmSystemCoreValuesAccelerometer.cpp
 *
 *  Created on:  Sept 15, 2014
 *      Author: salma
 * Description: Activity code is taken from Ambulation transportmode.java from NESL github
 * Code is converted to match running in the vm internal thread
*/

#include "DvmSystemCoreValuesAccelerometer.h"
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>


using namespace android;

# define PI           3.14159265358979323846
// forward declarations
double goertzel(double data[], double freq, double sr);
std::string activity (double var,double avg,double a1,
			double a2,double a3,double a4,double a5,
			double a6,double a7,double a8,double a9,
			double a0);

// Global variables
int sample = 0;
nsecs_t sStartTime;
double accData[100];
std::string curractivity;
bool accSensorisPrepared = false;
sp<SensorEventQueue> q;
//Just for debugging
//const char* activityFile = "/sdcard/ActivityData";
//const char* accDataFile  = "sdcard/AccelerometerData";


/** Helper only used while debugging**/
//static int dvmSystemWriteToFile(const char *file, std::string features)
//{
//
//	std::ofstream ofs (file, std::ofstream::out | std::ofstream::app);
//	ofs << features;
//	ofs.close();
//	return 0;
//
//}



std::string getCurrentActivity(){
	return curractivity;
}

void calculateActivity(){

	double dataSize = sizeof(accData)/sizeof(accData[0]); // 100
	curractivity = "unknown";

	double sum = 0.0, s = 0.0;
	double avg = 0.0, a = 0.0;
	double /*var = 0.0,*/ v = 0.0;
	double /*accFft1, accFft2, accFft3, accFft4, accFft5,*/ a1, a2, a3, a4, a5, a6, a7, a8, a9, a0;

	for (int i = 0; i < dataSize; i++){
		sum += accData[i];
	}

	avg = sum / dataSize;
	sum = 0.0;
	for (int i = 0; i < dataSize; i++){
		sum += pow((accData[i] - avg),2.0);
	}
	//var = sum / dataSize;

//	accFft1 = goertzel(accData, 1., dataSize);
//	accFft2 = goertzel(accData, 2., dataSize);
//	accFft3 = goertzel(accData, 3., dataSize);
//	accFft4 = goertzel(accData, 4., dataSize);
//	accFft5 = goertzel(accData, 5., dataSize);

	for (int i = 0; i < dataSize; i++){
		accData[i] = accData[i] / 310.; // restore to android measurement
		s += accData[i];
	}
	a = s / dataSize;
	s = 0.0;

	a1 = goertzel(accData, 1., dataSize);
	a2 = goertzel(accData, 2., dataSize);
	a3 = goertzel(accData, 3., dataSize);
	a4 = goertzel(accData, 4., dataSize);
	a5 = goertzel(accData, 5., dataSize);
	a6 = goertzel(accData, 6., dataSize);
	a7 = goertzel(accData, 7., dataSize);
	a8 = goertzel(accData, 8., dataSize);
	a9 = goertzel(accData, 9., dataSize);
	a0 = goertzel(accData, 10., dataSize);

	for (int i = 0; i < dataSize; i++){
		s += pow((accData[i] - a),2.0);
	}
	v = s / dataSize;

	curractivity = activity(v, a, a1, a2, a3, a4, a5, a6, a7, a8, a9, a0);
	//ALOGD("currActivity = %s", curractivity.c_str());
	/*******LOGGING ***** BE AWARE IT GROWS THE HEAP SIGNIFICATLLY ***/
	// log the accelerometer data
	/*char datachar [200];
	std::string datastring = "";
	for (unsigned int i = 0; i < dataSize; i++){
			double sample = accData[i];
			sprintf(datachar, ",%2.4f", sample);
			std::string samplestring(datachar);
			datastring.append(samplestring);
	}
	datastring.append("\n");
	dvmSystemWriteToFile(accDataFile,datastring);


	int activityint  = 0;
	if (curractivity.find("still") != std::string::npos) activityint = 0;
	else if (curractivity.find("walk") != std::string::npos) activityint = 1;
	else if (curractivity.find("run") != std::string::npos) activityint = 2;

	char features[100];
	sprintf(features, "%2.4f,%2.4f,%2.4f,%2.4f,%2.4f,%2.4f,%2.4f,%2.4f,%2.4f,%2.4f,%2.4f,%2.4f, %d\n",
				v, a, a1, a2, a3, a4, a5, a6, a7, a8, a9, a0, activityint);
    std::string featuresString(features);
	dvmSystemWriteToFile(activityFile,features);*/

/****************** END OF LOGGING DATA ********/
}

std::string activity (double var,double avg,double a1,double a2,double a3,double a4,double a5,
			double a6,double a7,double a8,double a9,double a0){

	if (var <= 0.0047){
		if (var <= 0.0016) return "still";
		else{
			if (a5 <= 0.1532){
				if (a1 <= 0.5045) return "still";
				else return "walk";
			}
			else return "still";
		}
	}
	else{
		if (a3 <= 60.3539){
			if (var <= 0.0085){
				if (a8 <= 0.0506) return "walk";
				else{
					if (a2 <= 2.8607) return "still";
					else return "walk";
				}
			}
			else{
				if (a2 <= 2.7725){
					if (a1 <= 13.0396) return "walk";
					else return "still";
				}
				else return "walk";
			}
		}
		else return "run";
	}
}

double goertzel(double data[], double freq, double sr){
	double s_prev = 0;
	double s_prev2 = 0;
	double coeff = 2 * cos( (2*PI*freq) / sr);
	double s;
	double dataSize = sizeof(data)/sizeof(data[0]); // 100;
	for (unsigned int i = 0; i < dataSize; i++){
		double sample = data[i];
		s = sample + coeff*s_prev  - s_prev2;
		s_prev2 = s_prev;
		s_prev = s;
	}
	double power = s_prev2*s_prev2 + s_prev*s_prev - coeff*s_prev2*s_prev;
	return power;
}

int classifyMotion(double fft){
	if (fft <= 5654)
		return 0;
	else
		return 1;
}


void prepareAccelerometerSensor(/*sp<SensorEventQueue>& q*/ ){
	accSensorisPrepared = false;
	SensorManager& mgr(SensorManager::getInstance());
	//Sensor const* const* list;
	//ssize_t count = mgr.getSensorList(&list);
	//ALOGD("numSensors=%d\n", int(count));

	q = mgr.createEventQueue();
	//ALOGD("queue=%p\n", q.get());

	Sensor const* accelerometer = mgr.getDefaultSensor(Sensor::TYPE_ACCELEROMETER);

	//ALOGD("accelerometer=%p (%s)\n",
	//accelerometer, accelerometer->getName().string());

	q->enableSensor(accelerometer);
	q->setEventRate(accelerometer, ms2ns(10));
	accSensorisPrepared = true;

	// initiat the accData array
	double dataSize = sizeof(accData)/sizeof(accData[0]);
	for (unsigned int i = 0; i< dataSize; i++ ){
		accData[i] = 0;
	}
}


void getAccelerometerData(){

	if(accSensorisPrepared == false){
		prepareAccelerometerSensor(/*q*/);
	}

	sStartTime = systemTime();

	ssize_t n;
    ASensorEvent buffer[8];

//    static nsecs_t oldTimeStamp = 0;
    double accx, accy, accz;
//    unsigned long timestamp;
    double totalForce = 0.0;
    double grav = /*SensorManager::*/GRAVITY_EARTH;


    while ((n = q->read(buffer, 8)) > 0) {
        for (int i=0 ; i<n ; i++) {
        	//debug
//            float t;
//            if (oldTimeStamp) {
//                t = float(buffer[i].timestamp - oldTimeStamp) / s2ns(1);
//            } else {
//                t = float(buffer[i].timestamp - sStartTime) / s2ns(1);
//            }
//            oldTimeStamp = buffer[i].timestamp;

            if (buffer[i].type == Sensor::TYPE_ACCELEROMETER){
            	totalForce = 0.0;
//                ALOGD("%lld\t%8f\t%8f\t%8f\t%f\t%f\n",
//                        buffer[i].timestamp,
//                        buffer[i].data[0], buffer[i].data[1], buffer[i].data[2],
//                        1.0/t, totalForce);
                accx = buffer[i].data[0];
                accy = buffer[i].data[1];
                accz = buffer[i].data[2];
//                timestamp = buffer[i].timestamp;

                totalForce += pow(accx/grav, 2.0);
                totalForce += pow(accy/grav, 2.0);
                totalForce += pow(accz/grav, 2.0);
                totalForce = sqrt(totalForce);
                totalForce *= 310;

                //debug
//                if (sample < 9){
//                	//ALOGD("Accelerometer Data: Still too few samples");
//                }

	 			/* flush the accData array to avoid buffer overflow and calculate activity */
	 			if (sample == 100){
	 				sample = 0;
	 				calculateActivity();
	 			}
	 			accData[sample] = totalForce;
	 			sample++;
            }
        }
    }


    if (n<0 && n != -EAGAIN) {
        ALOGD("error reading events (%s)\n", strerror(-n));
    }

}

