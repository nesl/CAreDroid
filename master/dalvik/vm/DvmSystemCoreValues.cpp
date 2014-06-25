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
#include "DvmSystemCoreValuesLocation.h"
//#include "android_runtime/AndroidRuntime.h"


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
	    else
	    	{
	    		gCurOperatingpoint.connect.wifi_link_quality= -1; // -1 means not available or N/A
	    		gCurOperatingpoint.connect.wifi_signal_level= 0;
	    	}






	    /*********************GPS***********************************************/
	//    char* const loc= "location";
	//   GetLocation(loc);
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


	    /*That's what I want to do
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




//
///*********************** The Decision Tree ***************************** */
///* The switch policy method */
///* State Machine for the decision tree to decide which method to execute */
//// take the operating point and return the key to the method <classID, methodID>
//u4 dvmSystemReturnMethod(/*OperatingPoint* opt,*/DvmDex* pDvmDex, string className, key curClassMethodIds){
//
//	key ClassMethodIds;
//	string methodName ="";
//	DvmConfigClass curClass;
//	vector<unsigned int> powerindex;
//	vector<unsigned int> tempindex;
//	vector<unsigned int> voltindex;
//	vector<DvmConfigMethodMapIter> powerIter;
//	vector<DvmConfigMethodMapIter> tempIter;
//	vector<DvmConfigMethodMapIter> voltIter;
//	vector<DvmConfigMethodMapIter> wifiIter;
//	vector<DvmConfigMethodMapIter> tagIter;
//
//	OperatingPoint *opt = &gCurOperatingpoint;
//
//	int preference = POWER_PREFERENCE; /*default power is the preference*/
//	unsigned int methodTag; //ignore for now
//	//flags
//	//bool Check_Temp_All = false;
//	bool Check_Volt_All = false;
//
//
//	/****************************** Check the Cache ***************************************** */
//	//dvmSystemCoreValuesDebugLUT();
//	if(dvmSystemCoreValuesCheckLUT(curClassMethodIds, *opt, ClassMethodIds.second)) return ClassMethodIds.second;
//	/************************ Tag Handler *************************************************** */
//
//
//	if(pDvmDex==NULL){
//		ALOGE("DvmDex = Null in dvmSystemReturnMethod");
//	}
//	if(!pDvmDex->isConfigFile){
//		ALOGE("DvmDex = Null in dvmSystemReturnMethod");
//	}
//
//
//	/************************ Tag Handler (Not Finished)*************************************** */
//	DvmConfigMethodMapIter it = pDvmDex->pconfigMethodMap->find(curClassMethodIds);
//	methodTag = it->second.tag;
//	//ALOGD("Current tag in decision tree");
//
//	for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end(); ++it){
//		if(it->second.tag == methodTag){
//		/// Solve the Tag issue here
//			tagIter.push_back(it);
//		}
//	}
//	/**************************************************************************************** */
//
//
//	/**************************** Preference Handler and Connectivity ************************ */
//	for(int i=0; i< pDvmDex->pConfigFile->numSensitiveClasses; i++){
//		if(strcmp(pDvmDex->pConfigFile->sensitiveClasses.at(i).className.c_str(), className.c_str()) == 0){
//			curClass = pDvmDex->pConfigFile->sensitiveClasses.at(i);
//			preference = curClass.preference;
//		}
//	}
//	int wifi_connect = opt->connect.wifi_state;
//	/**************************************************************************************** */
//
//
//	/**************************************************************************************** */
//	/******************************* Handle Power Preference first ************************** */
//	/**************************************************************************************** */
//	if(preference == POWER_PREFERENCE){
//		for (u4 p = 0; p < curClass.powRange.size(); p++){
//			// small rangeof power <= operating power point <= large rangeofPower
//			//printf("Current power range: %d, %d\n",pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).first, pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).second);
//			if(opt->batt.batteryCapacity >= curClass.powRange.at(p).first && opt->batt.batteryCapacity <= curClass.powRange.at(p).second ){
//			//	ALOGD("operating point lies in the ranges of power at index %d\n",p);
//				powerindex.push_back(p);
//			}
//		}
//
//		if(powerindex.size()==0){
//			//raise flag to check all temperature ranges
//			//Check_Temp_All =true;
//			//ALOGD("power index size =0 all ranges with u\n");
//			//meaning all ranges should be checked again so add all the indices in the powerindex
//			for(u4 p=0; p < curClass.powRange.size(); p++)
//				powerindex.push_back(p);
//		}
//		//go to second level only if there were more than 1 index from power
//		//if power index size == 1 then you get the method and return it.
//		if(powerindex.size()==1){
//			//for(int m=0; m<pDvmConfigFile->sensitiveClasses.at(i).numSensitiveMethods; m++){
//			 for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end(); ++it){
//				//if (powerindex.at(0) == pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodparm.ids.first){
//				 if (powerindex.at(0) == it->second.ids.first){
//					 //methodName = pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodName;
//					 ClassMethodIds = it->first;
//					break;
//				}
//				else {
//				 ALOGD("sth went wrong in power there must be a method");
//				}
//			}
//		}
//		else{//more than one power range
//			// check the second level which is the temperature only with the power index
//			//get temperature operating point index
//			//1- get the index(iterator) in the map with these power indices to be only accessed later
//			//if(Check_Temp_All==false){ //only if power index was found before
//				for(u4 p=0; p<powerindex.size();p++){
//					for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
//						if(powerindex.at(p) == it->second.ids.first){
//						//	ALOGD("find a match for power index\t");
//							powerIter.push_back(it);
//						//	printf("size of poweriter = %d\n", powerIter.size());
//							break;//found it, don't continue iterate in the map for this power index
//						}
//					}
//				}
//			//}
//			// 2- Get temperature index
//			for(u4 t=0; t< curClass.tempRange.size(); t++){
//				if(opt->batt.batteryTemperature >= curClass.tempRange.at(t).first && opt->batt.batteryTemperature <= curClass.tempRange.at(t).second){
//					tempindex.push_back(t);
//					ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
//				}
//			}
//			if(tempindex.size()==0){
//				//meaning nothing map to temperature so check the voltage with the power iteration indices instead
//				Check_Volt_All = true;
//			}
//			if(tempindex.size()==1){
//				//3-Access the map with the power iterators to check this temp and return the method,
//				for(u4 piter =0; piter < powerIter.size(); piter++ ){
//					if(powerIter.at(piter)->second.ids.second == tempindex.at(0)){
//						ClassMethodIds = powerIter.at(piter)->first;
//						break; // method name found break;
//					}
//					else if(piter == powerIter.size()-1){
//						//reach the end with no temperature matching then go and check all voltage
//						Check_Volt_All = true;
//					}
//				}
//			}
//			else if(tempindex.size() > 1){
//				//more than one in temp range
//				//put in the temperature iterator only the ranges that lie under the tree of power iterators
//				for(u4 piter=0; piter < powerIter.size(); piter++){
//					for(u4 tindex =0; tindex<tempindex.size();tindex++){
//						if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
//							tempIter.push_back(powerIter.at(piter));
//							//ALOGD("find a match for temp index\t");
//							//printf("size of tempiter = %d\n", tempIter.size());
//						}
//						}
//						if(piter == powerIter.size()-1 && tempIter.size()==0){
//							//no matching
//							Check_Volt_All= true;
//						}
//				}
//			}
//
//			if(Check_Volt_All || tempIter.size()>0){
//				if(tempIter.size()==1){
//					//return the method here
//					ClassMethodIds = tempIter.at(0)->first;
//					//break;
//				}
//				else{
//					//3- get the voltage indices
//					for(u4 v=0; v< curClass.voltRange.size(); v++){
//						if(opt->batt.batteryVoltage >= curClass.voltRange.at(v).first && opt->batt.batteryVoltage <= curClass.voltRange.at(v).second){
//							voltindex.push_back(v);
//							//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
//						}
//					}
//
//					if(voltindex.size()==0){ // no volt match
//						if(Check_Volt_All){ // no temp match
//							//return the highest priority of power iterators with opt connectivity required
//							u4 priority =10000;
//							bool tag_method_found_with_conn = false;
//							key candidateMethod;
//							for(u4 piter =0; piter < powerIter.size(); piter++){
//								if ((powerIter.at(piter)->second.priority < priority)){
//									priority = powerIter.at(piter)->second.priority;
//									candidateMethod = powerIter.at(piter)->first;
//									if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
//										ClassMethodIds = powerIter.at(piter)->first;
//										tag_method_found_with_conn = true;
//									}
//								}
//							}
//							if(tag_method_found_with_conn == false) // no match method with required conn is found
//								ClassMethodIds = candidateMethod; 	//take the highest priority in power regardless the conn;
//						}
//						else {
//							// return highest priority of temp iterators with the opt connectivity required
//							u4 priority =10000; //any big number
//							bool tag_method_found_with_conn = false;
//							key candidateMethod;
//							for (u4 titer =0; titer < tempIter.size();titer++){
//								if(tempIter.at(titer)->second.priority < priority){
//									priority = tempIter.at(titer)->second.priority;
//									candidateMethod = tempIter.at(titer)->first;
//									if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
//										ClassMethodIds = tempIter.at(titer)->first;
//										tag_method_found_with_conn = true;
//									}
//								}
//							}
//							if(tag_method_found_with_conn == false) // no match method with required conn is found
//								ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
//						}
//					}
//
//					if(voltindex.size() == 1){
//						if(Check_Volt_All){
//							for(u4 piter =0; piter < powerIter.size(); piter++){
//								if(powerIter.at(piter)->second.ids.third == voltindex.at(0)){
//									voltIter.push_back(powerIter.at(piter));
//								}
//								else if(piter == powerIter.size()-1 && voltIter.size() == 0 ){
//									//reach the end of power iteration with no matching to this volt index
//									//return method of highest priority from power iteration
//									u4 priority =10000;
//									bool tag_method_found_with_conn = false;
//									key candidateMethod;
//									for(u4 piter =0; piter < powerIter.size(); piter++){
//										if (powerIter.at(piter)->second.priority < priority){
//											priority = powerIter.at(piter)->second.priority;
//											candidateMethod = powerIter.at(piter)->first;
//											if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
//												ClassMethodIds = powerIter.at(piter)->first;
//												tag_method_found_with_conn = true;
//											}
//										}
//									}
//									if(tag_method_found_with_conn == false) // no match method with required conn is found
//										ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
//								}
//							}
//						}
//						else{
//							//check if this volt index lie under the temp iterator tree
//							for(u4 titer=0; titer < tempIter.size(); titer++){
//								if(tempIter.at(titer)->second.ids.third == voltindex.at(0)){
//									voltIter.push_back(tempIter.at(titer));
//								}
//								if(titer == powerIter.size()-1 && voltIter.size()==0){
//									//no matching
//									//return the highest priority of the temp iterartion
//									u4 priority =10000;
//									bool tag_method_found_with_conn = false;
//									key candidateMethod;
//									for(u4 titer =0; titer < tempIter.size(); titer++){
//										if (tempIter.at(titer)->second.priority < priority){
//											priority = tempIter.at(titer)->second.priority;
//											candidateMethod = tempIter.at(titer)->first;
//											if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
//												ClassMethodIds = tempIter.at(titer)->first;
//												tag_method_found_with_conn = true;
//											}
//										}
//									}
//									if(tag_method_found_with_conn == false) // no match method with required conn is found
//										ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
//								}
//							}
//						}
//					}
//					else if(voltindex.size()>1){
//						if(Check_Volt_All){
//							//check only those lie under the power tree iterator
//							for(u4 piter=0; piter < powerIter.size(); piter++){
//								for(u4 vindex =0; vindex<voltindex.size();vindex++){
//									if(powerIter.at(piter)->second.ids.third == voltindex.at(vindex)){
//										voltIter.push_back(powerIter.at(piter));
//									}
//								}
//								if(piter == powerIter.size()-1 && voltIter.size()==0){
//									//no matching // return highest priority of power iter
//									u4 priority =10000;
//									bool tag_method_found_with_conn = false;
//									key candidateMethod;
//									for(u4 piter =0; piter < powerIter.size(); piter++){
//										if (powerIter.at(piter)->second.priority < priority){
//											priority = powerIter.at(piter)->second.priority;
//											candidateMethod = powerIter.at(piter)->first;
//											if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
//												ClassMethodIds = powerIter.at(piter)->first;
//												tag_method_found_with_conn = true;
//											}
//										}
//									}
//									if(tag_method_found_with_conn == false) // no match method with required conn is found
//										ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
//								}
//							}
//						}
//						//check only the volt index lie under the temperature tree iterator
//						else {
//							for(u4 titer=0; titer < tempIter.size(); titer++){
//								for(u4 vindex =0; vindex<voltindex.size();vindex++){
//									if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
//										voltIter.push_back(tempIter.at(titer));
//									}
//								}
//								if(titer == tempIter.size()-1 && voltIter.size()==0){
//									//no matching // return highest priority of temp iteration
//									u4 priority =10000;
//									bool tag_method_found_with_conn = false;
//									key candidateMethod;
//									for(u4 titer =0; titer < tempIter.size(); titer++){
//										if (tempIter.at(titer)->second.priority < priority){
//											priority = tempIter.at(titer)->second.priority;
//											candidateMethod = tempIter.at(titer)->first;
//											if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
//												ClassMethodIds = tempIter.at(titer)->first;
//												tag_method_found_with_conn = true;
//											}
//										}
//									}
//									if(tag_method_found_with_conn == false) // no match method with required conn is found
//										ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
//								}
//							}
//						}
//					}
//
//					// At this point we have either the voltIter size = 1 or > 1 but can't be zero
//					if(voltIter.size() == 1){
//						// return this method
//						ClassMethodIds = voltIter.at(0)->first; // We don't need to check the connectivity here
//						//break;
//					}
//					else if(voltIter.size() > 1){
//						//get the highest priority of the volt iter
//						u4 priority =10000;
//						bool tag_method_found_with_conn = false;
//						key candidateMethod;
//						for(u4 viter =0; viter < voltIter.size(); viter++){
//							if (voltIter.at(viter)->second.priority < priority){
//								priority = voltIter.at(viter)->second.priority;
//								candidateMethod = voltIter.at(viter)->first;
//								if(wifi_connect == voltIter.at(viter)->second.conn.wifi_state || voltIter.at(viter)->second.conn.wifi_state == 2 /*don't care*/){
//									ClassMethodIds = voltIter.at(viter)->first;
//									tag_method_found_with_conn = true;
//								}
//							}
//						}
//						if(tag_method_found_with_conn == false) // no match method with required conn is found
//							ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
//					}
//				}
//			}
//		}
//		//ALOGD("Return MethodID = %d", ClassMethodIds.second);
//	} /*POWER Preference tree */
//
//	/* *************************************************************************************** */
//	/* ****************************** Handle Connectivity Preference First ******************* */
//	/* *************************************************************************************** */
//	else if (preference == CONNECTIVITY_PREFERENCE){
//		/*	Get the indices for the wifi connectivity first */
//		for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
//			if(it->second.conn.wifi_state == wifi_connect || it->second.conn.wifi_state == 2 /*don't care*/){
//				wifiIter.push_back(it);
//				//ALOGD("wifi it pushed back %d", it->first.second );
//			}
//		}
//
//		if(wifiIter.size() == 0) {
//			//push back in the wifi iter everything
//			for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
//				wifiIter.push_back(it);
//			}
//			//ALOGD("wifi iter was all zero");
//		}
//
//		if (wifiIter.size() == 1){
//			 ClassMethodIds = wifiIter.at(0)->first; //stop here
//			// ALOGD("found only one wifi that matches %d", wifiIter.at(0)->first.second);
//		}
//		else{
//			for (u4 p = 0; p < curClass.powRange.size(); p++){
//				// small rangeof power <= operating power point <= large rangeofPower
//				//printf("Current power range: %d, %d\n",pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).first, pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).second);
//				if(opt->batt.batteryCapacity >= curClass.powRange.at(p).first && opt->batt.batteryCapacity <= curClass.powRange.at(p).second ){
//					//ALOGD("operating point lies in the ranges of power at index %d\n",p);
//					powerindex.push_back(p);
//				}
//			}
//
//			if(powerindex.size()==0){
//				//raise flag to check all temperature ranges
//				//Check_Temp_All =true;
//				//ALOGD("power index size =0 all ranges with u\n");
//				//meaning all ranges should be checked again so add all the indices in the powerindex
//				for(u4 p=0; p < curClass.powRange.size(); p++)
//					powerindex.push_back(p);
//			}
//			//go to second level only if there were more than 1 index from power
//			//if power index size == 1 then you get the method and return it.
//			if(powerindex.size()==1){
//				//for(int m=0; m<pDvmConfigFile->sensitiveClasses.at(i).numSensitiveMethods; m++){
//				 bool tag_method_found = false;
//				 for(u4 witer =0; witer < wifiIter.size(); witer++){
//					 if (powerindex.at(0) == wifiIter.at(witer)->second.ids.first){
//						 //methodName = pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodName;
//						 ClassMethodIds = wifiIter.at(witer)->first;
//						 tag_method_found = true;
//						// ALOGD("power index just one and found the wifi iter");
//						 break;
//					}
//				 }
//				if(tag_method_found == false) {
//					//Power index found does not lie in the list of wifi iterations.
//					//return the highest priority method for the wifi
//					u4 priority =10000;
//					for(u4 witer =0; witer < wifiIter.size(); witer++){
//						if (wifiIter.at(witer)->second.priority < priority){
//							priority = wifiIter.at(witer)->second.priority;
//							ClassMethodIds = wifiIter.at(witer)->first;
//						}
//					}
//				}
//			}
//			else{//more than one power range
//				// check the second level which is the temperature only with the power index
//				//get temperature operating point index
//				//1- get the index(iterator) in the map with these power indices to be only accessed later
//				//if(Check_Temp_All==false){ //only if power index was found before
//					for(u4 p=0; p<powerindex.size();p++){
//						for(u4 witer =0; witer < wifiIter.size(); witer++){
//							if(powerindex.at(p) == wifiIter.at(witer)->second.ids.first){
//							//	ALOGD("find a match for power index\t");
//								powerIter.push_back(wifiIter.at(witer));
//							//	printf("size of poweriter = %d\n", powerIter.size());
//								break;//found it, don't continue iterate in the map for this power index
//							}
//						}
//					}
//				//}
//				// 2- Get temperature index
//				for(u4 t=0; t< curClass.tempRange.size(); t++){
//					if(opt->batt.batteryTemperature >= curClass.tempRange.at(t).first && opt->batt.batteryTemperature <= curClass.tempRange.at(t).second){
//						tempindex.push_back(t);
//						ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
//					}
//				}
//				if(tempindex.size()==0){
//					//meaning nothing map to temperature so check the voltage with the power iteration indices instead
//					Check_Volt_All = true;
//				}
//				if(tempindex.size()==1){
//					//3-Access the map with the power iterators to check this temp and return the method,
//					for(u4 piter =0; piter < powerIter.size(); piter++ ){
//						if(powerIter.at(piter)->second.ids.second == tempindex.at(0)){
//							ClassMethodIds = powerIter.at(piter)->first;
//							break; // method name found break;
//						}
//						else if(piter == powerIter.size()-1){
//							//reach the end with no temperature matching then go and check all voltage
//							Check_Volt_All = true;
//						}
//					}
//				}
//				else if(tempindex.size() > 1){
//					//more than one in temp range
//					//put in the temperature iterator only the ranges that lie under the tree of power iterators
//					for(u4 piter=0; piter < powerIter.size(); piter++){
//						for(u4 tindex =0; tindex<tempindex.size();tindex++){
//							if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
//								tempIter.push_back(powerIter.at(piter));
//								//ALOGD("find a match for temp index\t");
//								//printf("size of tempiter = %d\n", tempIter.size());
//							}
//							}
//							if(piter == powerIter.size()-1 && tempIter.size()==0){
//								//no matching
//								Check_Volt_All= true;
//							}
//					}
//				}
//
//				if(Check_Volt_All || tempIter.size()>0){
//					if(tempIter.size()==1){
//						//return the method here
//						ClassMethodIds = tempIter.at(0)->first;
//						//break;
//					}
//					else{
//						//3- get the voltage indices
//						for(u4 v=0; v< curClass.voltRange.size(); v++){
//							if(opt->batt.batteryVoltage >= curClass.voltRange.at(v).first && opt->batt.batteryVoltage <= curClass.voltRange.at(v).second){
//								voltindex.push_back(v);
//								//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
//							}
//						}
//
//						if(voltindex.size()==0){ // no volt match
//							if(Check_Volt_All){ // no temp match
//								//return the highest priority of power iterators with opt connectivity required
//								u4 priority =10000;
//								for(u4 piter =0; piter < powerIter.size(); piter++){
//									if ((powerIter.at(piter)->second.priority < priority)){
//										priority = powerIter.at(piter)->second.priority;
//										ClassMethodIds = powerIter.at(piter)->first;
//										}
//									}
//								}
//							}
//							else {
//								// return highest priority of temp iterators with the opt connectivity required
//								u4 priority =10000; //any big number
//								for (u4 titer =0; titer < tempIter.size();titer++){
//									if(tempIter.at(titer)->second.priority < priority){
//										priority = tempIter.at(titer)->second.priority;
//										ClassMethodIds = tempIter.at(titer)->first;
//									}
//								}
//							}
//						}
//
//						if(voltindex.size() == 1){
//							if(Check_Volt_All){
//								for(u4 piter =0; piter < powerIter.size(); piter++){
//									if(powerIter.at(piter)->second.ids.third == voltindex.at(0)){
//										voltIter.push_back(powerIter.at(piter));
//									}
//									else if(piter == powerIter.size()-1 && voltIter.size() == 0 ){
//										//reach the end of power iteration with no matching to this volt index
//										//return method of highest priority from power iteration
//										u4 priority =10000;
//										for(u4 piter =0; piter < powerIter.size(); piter++){
//											if (powerIter.at(piter)->second.priority < priority){
//												priority = powerIter.at(piter)->second.priority;
//												ClassMethodIds = powerIter.at(piter)->first;
//											}
//										}
//									}
//								}
//							}
//							else{
//								//check if this volt index lie under the temp iterator tree
//								for(u4 titer=0; titer < tempIter.size(); titer++){
//									if(tempIter.at(titer)->second.ids.third == voltindex.at(0)){
//										voltIter.push_back(tempIter.at(titer));
//									}
//									if(titer == powerIter.size()-1 && voltIter.size()==0){
//										//no matching
//										//return the highest priority of the temp iterartion
//										u4 priority =10000;
//										for(u4 titer =0; titer < tempIter.size(); titer++){
//											if (tempIter.at(titer)->second.priority < priority){
//												priority = tempIter.at(titer)->second.priority;
//												ClassMethodIds = tempIter.at(titer)->first;
//											}
//										}
//									}
//								}
//							}
//						}
//						else if(voltindex.size()>1){
//							if(Check_Volt_All){
//								//check only those lie under the power tree iterator
//								for(u4 piter=0; piter < powerIter.size(); piter++){
//									for(u4 vindex =0; vindex<voltindex.size();vindex++){
//										if(powerIter.at(piter)->second.ids.third == voltindex.at(vindex)){
//											voltIter.push_back(powerIter.at(piter));
//										}
//									}
//									if(piter == powerIter.size()-1 && voltIter.size()==0){
//										//no matching // return highest priority of power iter
//										u4 priority =10000;
//										for(u4 piter =0; piter < powerIter.size(); piter++){
//											if (powerIter.at(piter)->second.priority < priority){
//												priority = powerIter.at(piter)->second.priority;
//												ClassMethodIds = powerIter.at(piter)->first;
//											}
//										}
//									}
//								}
//							}
//							//check only the volt index lie under the temperature tree iterator
//							else {
//								for(u4 titer=0; titer < tempIter.size(); titer++){
//									for(u4 vindex =0; vindex<voltindex.size();vindex++){
//										if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
//											voltIter.push_back(tempIter.at(titer));
//										}
//									}
//									if(titer == tempIter.size()-1 && voltIter.size()==0){
//										//no matching // return highest priority of temp iteration
//										u4 priority =10000;
//										for(u4 titer =0; titer < tempIter.size(); titer++){
//											if (tempIter.at(titer)->second.priority < priority){
//												priority = tempIter.at(titer)->second.priority;
//												ClassMethodIds = tempIter.at(titer)->first;
//											}
//										}
//									}
//								}
//							}
//						}
//
//						// At this point we have either the voltIter size = 1 or > 1 but can't be zero
//						if(voltIter.size() == 1){
//							// return this method
//							ClassMethodIds = voltIter.at(0)->first; // We don't need to check the connectivity here
//							//break;
//						}
//						else if(voltIter.size() > 1){
//							//get the highest priority of the volt iter
//							u4 priority =10000;
//							for(u4 viter =0; viter < voltIter.size(); viter++){
//								if (voltIter.at(viter)->second.priority < priority){
//									priority = voltIter.at(viter)->second.priority;
//									ClassMethodIds = voltIter.at(viter)->first;
//								}
//							}
//						}
//					}
//				}
//			}
//	}/*Connectivity Preference tree */
//
//
//
//
//	/************************************** Update the LUT ********************************* */
//	OperatingPoint curopt = *opt;
//	LUTkey lutk = { curClassMethodIds, curopt};
//	keyTimeStamp kts = { ClassMethodIds, TIME_STAMP};
//	if (gDvmLUTMap.size() != MAX_LUT_SIZE ){
//		gDvmLUTMap.insert(make_pair(lutk,kts));
//		//ALOGD("Insert in the LUT, @%d", TIME_STAMP);
//		TIME_STAMP++;
//	}
//	else{ //erase and insert
//		//ALOGD("Clear an element in the LUT");
//		DvmLUTMapIter it;
//		//find the element with the oldest time stamp
//		bool eraseFlag = true; // to avoid multiple erase
//		for (it = gDvmLUTMap.begin(); it != gDvmLUTMap.end(); ++it ){
//			if (it->second.time_stamp == 0 && eraseFlag ){
//				gDvmLUTMap.erase(it); //erase first element
//				eraseFlag = false;
//			}
//			else
//				it->second.time_stamp = it->second.time_stamp - 1;
//		}
//		TIME_STAMP = MAX_LUT_SIZE - 1;
//		kts.time_stamp = TIME_STAMP;
//		gDvmLUTMap.insert(make_pair(lutk,kts));
//	}
//	/**************************************************************************************** */
//
//
//	return ClassMethodIds.second;
//}
///* ********************************************************************* */
//
//
//
///**********************  Decision Tree Cache *************************** */
//bool dvmSystemCoreValuesCheckLUT(key curClassMethodIds, OperatingPoint optpoint, u4& MethodIds){
//	/*Before starting the decision tree check the LUT*/
//	if(gDvmLUTMap.size() != 0){
//		DvmLUTMapIter it;
//		LUTkey lutk = {curClassMethodIds, optpoint};
//		it = gDvmLUTMap.find(lutk);
//		if(it != gDvmLUTMap.end()){
//			MethodIds =  it->second.ClassMethod_ids.second; // it->second = keyTimeStamp --> ClasMethod_ids=the key<ClassID, newMethodID>
//			//ALOGD("Method found in the LUT");
//			return true;
//		}
//		else{
//			//ALOGD("Method is not in the LUT");
//			return false;
//		}
//	}
//	else return false;
//}
///*override the less than operator for the insert in LUT map*/
//bool operator <(const LUTkey&lhs, const  LUTkey& rhs){
//	//Compare the Class id first then the method id second
//	if(lhs.ClassMethod_ids.first != rhs.ClassMethod_ids.first){
//		//ALOGD("lhs.ClassMethod_ids.first != rhs.ClassMethod_ids.first");
//		return lhs.ClassMethod_ids.first < rhs.ClassMethod_ids.first;
//	}
//
//	else if (lhs.ClassMethod_ids.second !=  rhs.ClassMethod_ids.second ){
//		//ALOGD("lhs.ClassMethod_ids.second !=  rhs.ClassMethod_ids.second");
//		return lhs.ClassMethod_ids.second <  rhs.ClassMethod_ids.second;
//	}
//
//	else if (lhs.opt.batt.batteryCapacity != rhs.opt.batt.batteryCapacity){
//		//ALOGD("lhs.opt.batt->batteryCapacity != rhs.opt.batt->batteryCapacity");
//		return lhs.opt.batt.batteryCapacity < rhs.opt.batt.batteryCapacity;
//	}
//
//	else if (lhs.opt.batt.batteryTemperature != rhs.opt.batt.batteryTemperature){
//		//ALOGD("lhs.opt.batt->batteryTemperature != rhs.opt.batt->batteryTemperature");
//		return lhs.opt.batt.batteryTemperature < rhs.opt.batt.batteryTemperature;
//	}
//
//	else if (lhs.opt.batt.batteryVoltage != rhs.opt.batt.batteryVoltage){
//		//ALOGD("lhs.opt.batt->batteryVoltage != rhs.opt.batt->batteryVoltage");
//		return lhs.opt.batt.batteryVoltage < rhs.opt.batt.batteryVoltage;
//	}
//
//	else if(lhs.opt.connect.wifi_state != rhs.opt.connect.wifi_state){
//		//ALOGD("lhs.opt.connect->wifi_state != rhs.opt.connect->wifi_state");
//		return lhs.opt.connect.wifi_state < rhs.opt.connect.wifi_state;
//	}
//	else{
//		//ALOGD("lhs.opt.connect->wifi_state= %d, rhs.opt.connect->wifi_state= %d",  lhs.opt.connect.wifi_state, rhs.opt.connect.wifi_state);
//		return lhs.opt.connect.wifi_state < rhs.opt.connect.wifi_state;
//	}
//}
//
//void dvmSystemCoreValuesDebugLUT(){
//	DvmLUTMapIter it;
//	for (it = gDvmLUTMap.begin(); it != gDvmLUTMap.end(); ++it ){
//		ALOGD("%d, %d, %d, %d, %d, %d --> %d, %d, %d", it->first.ClassMethod_ids.first,
//													   it->first.ClassMethod_ids.second,
//													   it->first.opt.batt.batteryCapacity,
//													   it->first.opt.batt.batteryTemperature,
//													   it->first.opt.batt.batteryVoltage,
//													   it->first.opt.connect.wifi_state,
//													   it->second.ClassMethod_ids.first,
//													   it->second.ClassMethod_ids.second,
//													   it->second.time_stamp);
//
//    }
//}
///* ********************************************************************* */


/*No Used Now*/
//
//void dvmSystemSetDefaultSwitchPolicy(){
//
//	ALOGD("setting policy now ... ");
//
//	switch(gdvmBatteryService.batteryStatus){
//		case BATTERY_STATUS_CHARGING:
//		case BATTERY_STATUS_FULL:
//			SWITCH_FLAG = false; //switch off the flag if it is charging even if the cap is low
//			break;
//		case BATTERY_STATUS_DISCHARGING:
//			if (gdvmBatteryService.batteryCapacity <= LOW_BATTERY_CLOSE_WARNING_LEVEL){
//				SWITCH_FLAG = true;
//				break;
//			}
//			else if (gdvmBatteryService.batteryCapacity > LOW_BATTERY_CLOSE_WARNING_LEVEL){
//				SWITCH_FLAG = false;
//				break;
//			}
//
//	}
//	ALOGD("current switch flag=%d",SWITCH_FLAG);
//
//}
//
//batterytuple dvmSystemGetOperatingPoint(){
//
//	batterytuple optpoint;
//
//	optpoint.first = (u4)gdvmBatteryService.batteryCapacity;
//	optpoint.second = (u4)gdvmBatteryService.batteryTemperature;
//	optpoint.third = (u4)gdvmBatteryService.batteryVoltage;
//
//	return optpoint;
//
//
//}
//
//bool dvmSystemGetDefaultSwitchPolicy(){
//	//ALOGD("Get Switch FLAG");
//	return SWITCH_FLAG;
//}
//
//bool dvmSystemUpdateSwitchPolicy(){
//	//ALOGD("Update Switch Flag");
//	//if(dvmSystemUpdateBatteryService()){
//		dvmSystemUpdateBatteryService();
//		dvmSystemSetDefaultSwitchPolicy();
//	//}
//	return dvmSystemGetDefaultSwitchPolicy();
//}
//
//int dvmSystemGetBatteryLevel(JNIEnv* env, char* buf)
//{
//    //char    path[PATH_MAX];
//    struct dirent* entry;
//    struct dirent* battentry;
//    DIR* dirbatt ;
//    int length;
//
//    DIR* dir = opendir(POWER_SUPPLY_PATH);
//    if (dir == NULL) {
//        ALOGE("Could not open %s\n", POWER_SUPPLY_PATH);
//        return 0;
//    } else {
//        while ((entry = readdir(dir))) {
//            const char* name = entry->d_name;
//
//            // ignore "." and ".."
//            if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
//                continue;
//            }
//           // ALOGD("name in directory = %s",name );
//
//            if(strcmp(name,"battery")==0){
//            	dirbatt = opendir("/sys/class/power_supply/battery");
//            	if(dirbatt == NULL){
//            		ALOGE("could not open battery folder");
//            		return 0;
//            	}
//            	else{
//            		while((battentry = readdir(dirbatt))) {
//            			const char* nameinsidebatt = battentry->d_name;
//            			if (nameinsidebatt[0] == '.' && (nameinsidebatt[1] == 0 || (nameinsidebatt[1] == '.' && nameinsidebatt[2] == 0))) {
//            			   continue;
//            			}
//            			//ALOGD("name in battery directory = %s",nameinsidebatt );
//            			if(strcmp(nameinsidebatt,"capacity")){
//            				length = dvmSystemReadFromFile("/sys/class/power_supply/battery/capacity", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            					//ALOGD("battery capacity = %s", buf);
//            					closedir(dir);
//            					return 1;
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"type")){
//            				length = dvmSystemReadFromFile("/sys/class/power_supply/battery/type", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            					//ALOGD("battery type = %s", buf);
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"power")){
//            				length = dvmSystemReadFromFile("/sys/class/power_supply/battery/power", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            				//	ALOGD("battery power = %s", buf);
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"status")){
//            				length = dvmSystemReadFromFile("/sys/class/power_supply/battery/status", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            				//	ALOGD("battery status = %s", buf);
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"health")){
//            				length = dvmSystemReadFromFile("/sys/class/power_supply/battery/health", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            				//	ALOGD("battery health = %s", buf);
//            				}
//            			}
//            		}
//            	}
//            }
//
//         }
//         closedir(dir);
//        return 0;
//    }
//}
//
