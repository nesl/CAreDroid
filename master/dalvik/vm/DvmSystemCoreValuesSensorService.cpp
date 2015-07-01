/*
 * DvmSystemCoreValuesSensorService.cpp
 *
 *  Created on: August 11, 2014
 *      Author: salma
 */


#include "DvmSystemCoreValuesSensorService.h"
#include "DvmSystemCoreValuesLocation.h"

static nsecs_t sStartTime = 0;



int receiver(int fd, int events, void* data)
{
    sp<SensorEventQueue> q((SensorEventQueue*)data);
    ssize_t n;
    ASensorEvent buffer[8];

    static nsecs_t oldTimeStamp = 0;


    while ((n = q->read(buffer, 8)) > 0) {
        for (int i=0 ; i<n ; i++) {
            float t;
            if (oldTimeStamp) {
                t = float(buffer[i].timestamp - oldTimeStamp) / s2ns(1);
            } else {
                t = float(buffer[i].timestamp - sStartTime) / s2ns(1);
            }
            oldTimeStamp = buffer[i].timestamp;

            if (buffer[i].type == Sensor::TYPE_ACCELEROMETER) {
                ALOGD("%lld\t%8f\t%8f\t%8f\t%f\n",
                        buffer[i].timestamp,
                        buffer[i].data[0], buffer[i].data[1], buffer[i].data[2],
                        1.0/t);

            }

        }
    }

    if (n<0 && n != -EAGAIN) {
        ALOGD("error reading events (%s)\n", strerror(-n));
    }
    return 1;
}



int DalvikSensorData::getAccelerometerData(double &x, double &y, double &z, unsigned long& timestamp)
{
	SensorManager& mgr(SensorManager::getInstance());
	Sensor const* const* list;
	ssize_t count = mgr.getSensorList(&list);
	ALOGD("numSensors=%d\n", int(count));

	sp<SensorEventQueue> q = mgr.createEventQueue();
	ALOGD("queue=%p\n", q.get());

	Sensor const* accelerometer = mgr.getDefaultSensor(Sensor::TYPE_ACCELEROMETER);
	ALOGD("accelerometer=%p (%s)\n",
	accelerometer, accelerometer->getName().string());

	sStartTime = systemTime();

	q->enableSensor(accelerometer);

	q->setEventRate(accelerometer, ms2ns(10));

    ssize_t n;
    ASensorEvent buffer[8];

    static nsecs_t oldTimeStamp = 0;


    while ((n = q->read(buffer, 8)) > 0) {
    	//ALOGD("Inside the while loop");
        for (int i=0 ; i<n ; i++) {
            float t;
            if (oldTimeStamp) {
                t = float(buffer[i].timestamp - oldTimeStamp) / s2ns(1);
            } else {
                t = float(buffer[i].timestamp - sStartTime) / s2ns(1);
            }
            oldTimeStamp = buffer[i].timestamp;

            if (buffer[i].type == Sensor::TYPE_ACCELEROMETER) {
            //	ALOGD("Inside type accelerometer");
                ALOGD("%lld\t%8f\t%8f\t%8f\t%f\n",
                        buffer[i].timestamp,
                        buffer[i].data[0], buffer[i].data[1], buffer[i].data[2],
                        1.0/t);
                x = buffer[i].data[0];
                y = buffer[i].data[1];
                z = buffer[i].data[2];
                timestamp = buffer[i].timestamp;
            }

        }
    }

    if (n<0 && n != -EAGAIN) {
        ALOGD("error reading events (%s)\n", strerror(-n));
    }


//
//	sp<Looper> loop = new Looper(false);
//	loop->addFd(q->getFd(), 0, ALOOPER_EVENT_INPUT, receiver, q.get());
//	    //do {
//	           //printf("about to poll...\n");
//	int32_t ret = loop->pollOnce(-1);
//
//	           switch (ret) {
//	               case ALOOPER_POLL_WAKE:
//	                   ALOGD("ALOOPER_POLL_WAKE\n");
//	                   break;
//	               case ALOOPER_POLL_CALLBACK:
//	                   //ALOGD("ALOOPER_POLL_CALLBACK\n");
//	                   break;
//	               case ALOOPER_POLL_TIMEOUT:
//	                   ALOGD("ALOOPER_POLL_TIMEOUT\n");
//	                   break;
//	               case ALOOPER_POLL_ERROR:
//	                   ALOGD("ALOOPER_POLL_TIMEOUT\n");
//	                   break;
//	               default:
//	                  ALOGD("ugh? poll returned %d\n", ret);
//	                   break;
//	           }
//	       	//void* data;
//	       	//int events;
//	       	//int fd;
//	       	//int ret = loop->pollOnce(-1, &fd,  &events, &data);
//	       //    ALOGD(" %p ~ pollOnce - returning signalled identifier "
//	       //                            "fd=%d, events=0x%x, data=%d",
//	       //                             this,fd, events, (*(int *)data) );
//	//       } while (1);
//


   return 0;
}



//just in case
//TYPE_MAGNETIC_FIELD
//TYPE_GYROSCOPE
//TYPE_LIGHT
//TYPE_PROXIMITY



//using servicemanager (So hard and didn't get the events)
//	const String16 name("sensorservice");
//    sp<IServiceManager> sm = defaultServiceManager();
//    fflush(stdout);
//    if (sm == NULL) {
//		ALOGE("Unable to get default service manager!");
//        return 20;
//    }
//
//    Vector<String16> args;
//    args.add(name);
//
//    ALOGD("Currently running services:");
//    sp<IBinder> service = sm->checkService(name);
//
//
//    if (service != NULL) {
//    	ALOGD("service != NULL");
//    }
//
//
//    // trial 1
//    sp<ISensorServer> mSensorServer;
//    for (int i=0 ; i<4 ; i++) {
//    	status_t err = getService(name, &mSensorServer);
//    	if (err == NAME_NOT_FOUND) {
//    		usleep(250000);
//            continue;
//    	}
//    	if (err != NO_ERROR) {
//    		return err;
//    	}
//    	break;
//    }
//
//
//
//    // trial 2 --> interface cast
//    sp<ISensorServer> mSensorServerCast = interface_cast<ISensorServer>(service);
//
//    Vector<Sensor> mSensors;
//    mSensors = mSensorServerCast->getSensorList();
//    size_t count = mSensors.size();
//
//    sp<ISensorEventConnection> connection =  mSensorServerCast->createSensorEventConnection(); //get queue then read queue
//    sp<BitTube> bt(connection->getSensorChannel());
//    if(bt != 0){}
//
//
//    for (size_t i=0 ; i<count ; i++) {
//        const Sensor& s(mSensors[i]);
//
//
//
//                __android_log_print(ANDROID_LOG_DEBUG, "Sensor Service",   "%-48s| %-32s | 0x%08x | maxRate=%7.2fHz | "
//                        /*"last=<%5.1f,%5.1f,%5.1f>\n"*/,
//                        s.getName().string(),
//                        s.getVendor().string(),
//                        s.getHandle(),
//                        s.getMinDelay() ? (1000000.0f / s.getMinDelay()) : 0.0f);//,
//                       /* e.data[0], e.data[1], e.data[2]);*/
//    }


//failed trials
// const sensors_event_t& e(mLastEventSeen.valueFor(s.getHandle())); //What I want to achieve
 //trial 1  --> failed Sensor class does not have member process
 //        sensors_event_t event;
//        memset(&event, 0, sizeof(event));
//        event.type = s.getType();
//        sensors_event_t outevent;
//        bool result = s.process(event, &outevent);
 //trial 2: SensorDevice -->failed (private)
 //SensorDevice dev(SensorDevice::getInstance());
//trial 3: -->failed (Built but at runtime couldn't open OEM HAL library!)
 //    const size_t numEventMax = 16;
 //    const size_t minBufferSize = numEventMax + numEventMax * 5;//mVirtualSensorList.size();
 //    sensors_event_t buffer[minBufferSize];
 //
//            SensorDevice& device(SensorDevice::getInstance());
//            ssize_t countsens;
//
//                   countsens = device.poll(buffer, minBufferSize);
//                   if (countsens<0) {
//                       ALOGE("sensor poll failed (%s)", strerror(-countsens));
//                   }

//works with only removal of permission

// trial 0 through dump --> stupid
//    FILE* dumpout;
//    dumpout = fopen("/sdcard/tmp/location.txt", "rw");
//    if(dumpout!=NULL){
//    	ALOGD("file is opened");
//    	int fileDescriptor = fileno(dumpout);
//    	ALOGD("file descriptor %d",fileDescriptor);
//    	int err = service->dump(fileDescriptor, args);
//    	if (err != 0) {
//    		ALOGD("Error dumping service info: (");
//    	}
//    	fclose(dumpout);
//    }
// problem with dumpsys. Error: Permission failure: android.permission.DUMP from uid = ...
// It should be called from system app.
//int getAccelerometerData(int& x, int& y, int& z){
//
//	/* System call to dumpsys */
//	ALOGD("Get Accelerometer Data ...");
//	FILE *fp;
//	char pathcommand[110];
//	char x_ch[5];
//	char y_ch[5];
//	char z_ch[5];
//
//	//open command for reading
//	fp = popen("dumpsys sensorservice | grep Accelerometer", "r");
//	if(fp==NULL){
//			ALOGE("Failed to run the command");
//			return 0;
//	}
//	while(fgets(pathcommand, sizeof(pathcommand)-1, fp)!=NULL){
//		ALOGD("%s",pathcommand );
//		if(strstr(pathcommand, "Accelerometer")){
//
//			string str_pathcommand(pathcommand);
//
//			unsigned int posOflast = str_pathcommand.find("last");
//			strncpy(x_ch, pathcommand+posOflast+7, 4 );
//			x_ch[4] ='\0'; // null character manually added;
//			x = atoi(x_ch);
//			ALOGD("x %d", x);
//
//			strncpy(y_ch,pathcommand+posOflast+7+6, 4);
//			y_ch[4]='\0';
//			y = atoi(y_ch);
//			ALOGD("y %d", y);
//
//			strncpy(z_ch,pathcommand+posOflast+7+6+6, 4);
//			z_ch[4]='\0';
//			z = atoi(z_ch);
//			ALOGD("z %d", z);
//		}
//	}
//	pclose(fp);
//	return 1;
//
//}


//static int sort_func(const String16* lhs, const String16* rhs)
//{
//    return lhs->compare(*rhs);
//}
