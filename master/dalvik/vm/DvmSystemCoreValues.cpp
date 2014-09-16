/*
 * DvmSystemCoreValues.cpp
 *
 *  Created on: Nov 14, 2013
 *      Author: salma
*/


#include "DvmSystemCoreValues.h"
#include "jni.h"
#include <time.h>
#include <cutils/properties.h>
#include "stdlib.h"
#include "DvmSystemCoreValuesAccelerometer.h"
#include "DvmSystemCoreValuesLocation.h"

//#include "DvmSystemCoreValuesSensorService.h"
//#include "android_runtime/AndroidRuntime.h"
//#include "DvmSystemActivityDetection.h"

/******* Global ******/
bool SWITCH_FLAG; //set by the android LOG class // just for now. It can be change later
//DvmSystemBatteryService gdvmBatteryService;
//DvmSystemConnectivity gdvmConnectivity;
OperatingPoint gCurOperatingpoint;
int64_t CURRENT_TIME = 0;
//JNIEnv* gCURRENT_ENV;


/** Helper **/
static int dvmSystemReadFromFile(const char* path, char* buf, size_t size)
{
    if (!path)
        return -1;
    int fd = open(path, O_RDONLY, 0);
    if (fd == -1) {
        ALOGE("Could not open '%s'", path);
        return -1;
    }

    ssize_t count = read(fd, buf, size);
    if (count > 0) {
        while (count > 0 && buf[count-1] == '\n')
            count--;
        buf[count] = '\0';
    } else {
        buf[0] = '\0';
    }

    close(fd);
    return count;
}

static int calculateSignalLevel(int rssi) {
        if (rssi <= -100) {
            return 0;
        } else if (rssi >= -55) {
            return 4;
        } else {
            float inputRange = (-100 - (-50));
            float outputRange = 4;
            return (int)((float)(rssi - (-50)) * outputRange / inputRange);
        }
    }

/**************** Set The Current JNIEnv **************************/
//void dvmSystemCoreValuesSetJniEnv(JNIEnv* env){
//	gCURRENT_ENV = env;
//}
/*********** Return address to the global saved Operating Point ******* */
OperatingPoint* dvmGetCurOperatingPoint(){
	return &gCurOperatingpoint;
}



/*********** Generate Operating Point and Update the global one ******* */
/* ******************************************************************** */
void dvmSystemUpdateBatteryService()//(JNIEnv* env, char* batterycap)
{
    char    path[PATH_MAX];
    struct dirent* entry;
   // int success = 0;

   // char battstatus[20];
   // char batthealth[20];
    char battvolt[20];
    char battcap[20];
  //  char batttech[20];
    char batttemp[20];


    DIR* dir = opendir(POWER_SUPPLY_PATH);
    if (dir == NULL) {
        ALOGE("Could not open %s\n", POWER_SUPPLY_PATH);
    } else {
        while ((entry = readdir(dir))) {
            const char* name = entry->d_name;

            // ignore "." and ".."
            if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
                continue;
            }

            char buf[20];
            // Look for "type" file in each subdirectory
            snprintf(path, sizeof(path), "%s/%s/type", POWER_SUPPLY_PATH, name);
            int length = dvmSystemReadFromFile(path, buf, sizeof(buf));
            if (length > 0) {
                if (buf[length - 1] == '\n')
                    buf[length - 1] = 0;

//                if (strcmp(buf, "Mains") == 0) {
//                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	gdvmBatteryService.batteryType = BATTERY_PLUGGED_AC;
//                    	//   gPaths.acOnlinePath = strdup(path);
//                    }
//                }
//                else if (strcmp(buf, "USB") == 0) {
//                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	gdvmBatteryService.batteryType = BATTERY_PLUGGED_USB;
//                    	 //  gPaths.usbOnlinePath = strdup(path);
//                    }
//
//                }
//                else if (strcmp(buf, "Wireless") == 0) {
//                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	gdvmBatteryService.batteryType = BATTERY_PLUGGED_WIRELESS;
//                    	 // gPaths.wirelessOnlinePath = strdup(path);
//                    }
//                }
               /* else*/ if (strcmp(buf, "Battery") == 0) {
//                	gdvmBatteryService.batteryType = BATTERY_ON_BATTERY;
//                	snprintf(path, sizeof(path), "%s/%s/status", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	int length = dvmSystemReadFromFile(path,battstatus,sizeof(battstatus));
//                    	if(length>0){
//                    		//ALOGD("battery status: %s", battstatus);
//                    		gdvmBatteryService.batteryStatus = atoi(battstatus);
//                    	}
//                    	 // gPaths.batteryStatusPath = strdup(path);
//                    }
//                    snprintf(path, sizeof(path), "%s/%s/health", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	int length = dvmSystemReadFromFile(path,batthealth,sizeof(batthealth));
//                    	if(length>0){
//                    	//	ALOGD("battery health: %s", batthealth);
//                    		gdvmBatteryService.batteryHealth = atoi(batthealth);
//                    	}
//                    	// gPaths.batteryHealthPath = strdup(path);
//                    }
//
//                    snprintf(path, sizeof(path), "%s/%s/present", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	gdvmBatteryService.batteryPresent = true;
//                    	// gPaths.batteryPresentPath = strdup(path);
//                    }
                    snprintf(path, sizeof(path), "%s/%s/capacity", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0){
                    	//int length = dvmSystemReadFromFile(path,batterycap,sizeof(batterycap));
                    	int length = dvmSystemReadFromFile(path,battcap,sizeof(battcap));
                    	if (length > 0){
                    		//ALOGD("batterycap: %s",battcap);
                    		gCurOperatingpoint.batt.batteryCapacity = atoi(battcap);
                    		//success = 1;
                    	}
                    	 // gPaths.batteryCapacityPath = strdup(path);
                    }
                    snprintf(path, sizeof(path), "%s/%s/voltage_now", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0) {
                    	int length = dvmSystemReadFromFile(path,battvolt,sizeof(battvolt));
                    	if(length>0){
                    	 // ALOGD("battery voltage: %s", battvolt);
                    	  // voltage_now is in microvolts, not millivolts
                    		gCurOperatingpoint.batt.batteryVoltage = atoi(battvolt) / 1000000.0; // to volt
                    	 }
                       // gPaths.batteryVoltagePath = strdup(path);
                       // // voltage_now is in microvolts, not millivolts
                       // gVoltageDivisor = 1000;
                    } else {
                        snprintf(path, sizeof(path), "%s/%s/batt_vol", POWER_SUPPLY_PATH, name);
                        if (access(path, R_OK) == 0){
                        	int length = dvmSystemReadFromFile(path,battvolt,sizeof(battvolt));
                        	if(length>0){
                        	    // ALOGD("battery voltage: %s", battvolt);
                        	     // voltage_now is in microvolts, not millivolts
                        		gCurOperatingpoint.batt.batteryVoltage = atoi(battvolt) / 1000000.0; // to volt
                        	}
                        }
                          //  gPaths.batteryVoltagePath = strdup(path);
                    }

                    snprintf(path, sizeof(path), "%s/%s/temp", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0) {
                    	int length = dvmSystemReadFromFile(path,batttemp,sizeof(batttemp));
                    	if(length>0){
                    	   	   //	  ALOGD("battery temp: %s", batttemp);
                    		gCurOperatingpoint.batt.batteryTemperature = atoi(batttemp) / 10.0; // change it to degree Celcius
                    	}
                       // gPaths.batteryTemperaturePath = strdup(path);
                    } else {
                        snprintf(path, sizeof(path), "%s/%s/batt_temp", POWER_SUPPLY_PATH, name);
                        if (access(path, R_OK) == 0){
                        	int length = dvmSystemReadFromFile(path,batttemp,sizeof(batttemp));
                        	if(length>0){
                        	   	  // 	  ALOGD("battery temp: %s", batttemp);
                        		gCurOperatingpoint.batt.batteryTemperature = atoi(batttemp) / 10.0; // change it to degree Celcius
                        	}
                        	//   gPaths.batteryTemperaturePath = strdup(path);
                        }
                    }

//                    snprintf(path, sizeof(path), "%s/%s/technology", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	int length = dvmSystemReadFromFile(path,batttech,sizeof(batttech));
//                    	if(length>0){
//                    	   	   	 // ALOGD("battery temp: %s", batttech);
//                    	   	 	  gdvmBatteryService.bateryTecnhology = batttech;
//                    	}
//                    	//gPaths.batteryTechnologyPath = strdup(path);
//                    }
                }
            }
        }
        closedir(dir);
    }

   // gCurOperatingpoint.batt = gdvmBatteryService;
    //return success;
}

void dvmSystemUpdateConnectivityState(){
	/*************** Wifi connection **************************/
	/*access /sys/class/wlan0/operstate to know if it is up or down*/
	char    path[PATH_MAX];
	char    wifi_operstate[10];

	DIR* dir = opendir(WLAN_PATH);
	    if (dir == NULL) {
	      //  ALOGE("Could not open %s\n", WLAN_PATH);
	        gCurOperatingpoint.connect.wifi_state = 0;
	    } else {
	    	snprintf(path, sizeof(path), "%s/operstate", WLAN_PATH);
	    	if (access(path, R_OK) == 0){
	    		//ALOGD("Path to WIFI is accessable");
	    		int length = dvmSystemReadFromFile(path,wifi_operstate,sizeof(wifi_operstate));
	    		if(length>0){
	    			if(strstr ( wifi_operstate, "up" ) != NULL ){
	    				 gCurOperatingpoint.connect.wifi_state = 1;
	    				//ALOGD("WIFI is up");
	    			}
	    			else if(strstr (wifi_operstate, "down") != NULL )
	    				gCurOperatingpoint.connect.wifi_state = 0;
	    			else {
	    				 gCurOperatingpoint.connect.wifi_state = 0;
	    				//ALOGD("Wifi state is Unknown set to default false");
	    			}
	    		}
	    	}else{
	    		//ALOGD("WIFI is not accessed");
	    		 gCurOperatingpoint.connect.wifi_state = 0;
	    	}
	    	closedir(dir);
	    }

	  //  ALOGD("Wifi connection is %d",gdvmConnectivity.wifi_state);

	    /****************** Link Quality and Signal Level RSSI************************************/
	    //execute a shell command in C

	    if( gCurOperatingpoint.connect.wifi_state == 1){
			FILE *fp;
			char pathcommand[110];
			char quality[3];
			char rssi[5];
			//open command for reading
			fp = popen("/system/bin/cat /proc/net/wireless", "r");
			if(fp==NULL){
				ALOGE("Failed to run the command");
			}
			while(fgets(pathcommand, sizeof(pathcommand)-1, fp)!=NULL){
				//ALOGD("%s", pathcommand);
				if(strstr(pathcommand, "wlan0")){
					strncpy(quality, pathcommand+15, 2);
					quality[2] ='\0'; // null character manually added;
				//	ALOGD("quality%s",quality);
					gCurOperatingpoint.connect.wifi_link_quality= atoi(quality);
				//	ALOGD("gCurOperatingpoint.connect.wifi_link_quality=%d",gCurOperatingpoint.connect.wifi_link_quality);

					strncpy(rssi,pathcommand+19, 4);
					rssi[4]='\0';
					//ALOGD("rssi%s",rssi);
					gCurOperatingpoint.connect.wifi_signal_rssi= atoi(rssi);
					gCurOperatingpoint.connect.wifi_signal_level =  calculateSignalLevel(gCurOperatingpoint.connect.wifi_signal_rssi);
					//ALOGD("gCurOperatingpoint.connect.wifi_signal_level=%d",gCurOperatingpoint.connect.wifi_signal_level);
					//for(unsigned int i =0; i< sizeof(pathcommand)-1; i++){
//						ALOGD("[%d] = %c", i,pathcommand[i]);
					//}
					}
				}
			pclose(fp);
	    }
	    else{
	    	gCurOperatingpoint.connect.wifi_link_quality= -1; // -1 means not available or N/A
	    	gCurOperatingpoint.connect.wifi_signal_level= 0;
	    }


	    /*********************GPS***********************************************/
          /* use the code of dumpsys  (didn't work)*/
	//    if (getLocation() == 1){};

	    /* use the the property buffer (didn't work)*/
	    //Always gives gps.disabled = 0
//	    char propBuf[PROPERTY_VALUE_MAX];
////
//	    // check to see if GPS should be disabled
//	    property_get("gps.disable", propBuf, "");
//	    if (propBuf[0] == '1')
//	    {
//	        ALOGD("gps.disable=1");
//	    } else {
//	    	ALOGD("gps.disable=0");
//	    }



	    /*Use jni (stupid ... didn't work)*/
	    /* That's what I want to do
	     * LocationManager mlocManager = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);Context.LOCATION_SERVICE = location
    		boolean enabled = mlocManager.isProviderEnabled(LocationManager.GPS_PROVIDER);LocationManager.GPS_PROVIDER="gps"
	     * */



    	//jclass clazz = gCURRENT_ENV->FindClass("com/android/server/location/GpsLocationProvider");
    	//jclass clazz = gCURRENT_ENV->FindClass("android/location/LocationManager");



//	    	jclass location_manger_class = gCURRENT_ENV->FindClass("android/location/LocationManager");

//	    if(gCURRENT_ENV == NULL )return;
//	    else{
//	    	//jclass context_class = gCURRENT_ENV->FindClass("android/content/Context");
//	    	jclass activity_class = gCURRENT_ENV->FindClass("android/app/Activity");
//	    	jmethodID midGetSystemService = gCURRENT_ENV->GetMethodID(activity_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
//
//	    	char* buf = (char*) malloc(9);
//	    	strcpy(buf,"location");
//	    	jstring StringArg = gCURRENT_ENV->NewStringUTF(buf);
//	    	if(StringArg != NULL )ALOGD("location string is not null");
//	    	jobject jSystemServiceObj = gCURRENT_ENV->CallObjectMethod(activity_class, midGetSystemService, StringArg);
//
//	    	if(jSystemServiceObj != NULL) ALOGD("found system object");
//
//	    	if(activity_class !=NULL && midGetSystemService !=NULL /*&& jSystemServiceObj !=NULL */)  ALOGD("Context class != NULL");
//
////	    	if(location_manger_class != NULL ){
////	    		method_reportStatus = gCURRENT_ENV->GetMethodID(location_manger_class, "isProviderEnabled", "(Ljava/lang/String;)Z");
////
////				if(method_reportStatus == NULL )ALOGD("method report status is NULL ");
////				else{
////					ALOGD("Found method report status");
////
////				}
////    		}
////	    	else{
////	    		ALOGD("location_manger_class provider is NULL");
////
////	    	}
//	    }

}

void dvmSystemUpdateSensorService(){

//	DalvikSensorData mSensorData;
//	double x, y, z;
//	unsigned long timestamp;
//	if(mSensorData.getAccelerometerData(x, y, z, timestamp) == 0 ){
//		ALOGD("%lu\t%8f\t%8f\t%8f\t",timestamp, x, y,z);
//	}
//	mSensorData.getAccelerometerData(x, y, z, timestamp);
//	ALOGD("%lu\t%8f\t%8f\t%8f\t",timestamp, x, y,z);

	//Check activity;
    ALOGD("Current Activity %s", getCurrentActivity().c_str());

}

void  dvmSystemCoreValuesUpdate(){

     struct timespec tm;
	 clock_gettime(CLOCK_MONOTONIC, &tm);
    // ALOGD("time clock = %ld", tm.tv_sec);
     if(tm.tv_sec - CURRENT_TIME < 0 ){
    	 // clock reset
    	 CURRENT_TIME = 0;
     }
	 if (  (tm.tv_sec - CURRENT_TIME) >= UPDATE_INTERVAL ){ // update each second
		 ALOGD("Update Values");
		 CURRENT_TIME = tm.tv_sec;
		// ALOGD("Current time = %lld", CURRENT_TIME);
		 if(CURRENT_TIME == 61) CURRENT_TIME = 0; //reset the time

		 dvmSystemUpdateConnectivityState(); //this already update the gCurOperatingpoint
		 dvmSystemUpdateBatteryService();
		 dvmSystemUpdateSensorService();

//		 optpoint->batt->first = (u4)gdvmBatteryService.batteryCapacity;
//		 optpoint->batt->second = (u4)gdvmBatteryService.batteryTemperature;
//		 optpoint->batt->third = (u4)gdvmBatteryService.batteryVoltage;
//		 optpoint->connect->wifi_state = gdvmConnectivity.wifi_state;
//         optpoint->connect->gps = gdvmConnectivity.gps;

//         gCurOperatingpoint.batt->first = (u4)gdvmBatteryService.batteryCapacity;
//         gCurOperatingpoint.batt->second = (u4)gdvmBatteryService.batteryTemperature;
//         gCurOperatingpoint.batt->third = (u4)gdvmBatteryService.batteryVoltage;
//         gCurOperatingpoint.connect->wifi_state = gdvmConnectivity.wifi_state;
//         gCurOperatingpoint.connect->gps = gdvmConnectivity.gps;

        // optpoint = &gCurOperatingpoint;

		// return true;
	 }
	 else{
		 // do nothing
		 //optpoint = &gCurOperatingpoint; //the saved one before update
		 //return false;
	 }

	 //return gCurOperatingpoint;
}
/* ********************************************************************* */





