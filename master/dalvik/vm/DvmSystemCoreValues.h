/*
 * DvmSystemCoreValues.h
 *
 *  Created on: Nov 14, 2013
 *      Author: salma
 */
#ifndef DALVIK_SYSTEM_CORE_VALUES_H_
#define DALVIK_SYSTEM_CORE_VALUES_H_


#include "Dalvik.h"
#include "jni.h"
#include <unistd.h>
#include <dirent.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <map>



#define POWER_SUPPLY_PATH "/sys/class/power_supply"
#define WLAN_PATH "/sys/class/net/wlan0"
#define PATH_MAX 					4096
/** Max number of Cache lines in LUT*/
#define MAX_LUT_SIZE 				20
/** Clock Update Interval*/
#define UPDATE_INTERVAL 			5
#define POWER_PREFERENCE 			0
#define CONNECTIVITY_PREFERENCE 	1
/*fwd*/
struct DvmSystemBatteryService;
struct DvmSystemConnectivity;
struct OperatingPoint;
struct LUTkey;
struct keyTimeStamp;

// LUTkey is the key to the look up table with key<classid, methodid> and operating point --> key<classid, methodid>
typedef std::map<LUTkey,keyTimeStamp> DvmLUTMap; // map the classID, methodID with the methodparam
typedef std::map<LUTkey,keyTimeStamp>::iterator DvmLUTMapIter;


/*Struct of battery services value*/
struct DvmSystemBatteryService{
	//int batteryStatus;
	//int batteryHealth;
//	bool batteryPresent;             // 1 is present, 0 is not
	unsigned int batteryCapacity;    // %of current remaining power cap. inside the battery
	unsigned int batteryVoltage;     // integer containing the current battery voltage level
	unsigned int batteryTemperature; // integer containing battery current temperature
//	char* bateryTecnhology;          // String containing battery technology
//	int batteryType;
};


struct DvmSystemConnectivity{
	int wifi_state;					 // 1 = up, 0 = down 2 = don't care
	int wifi_link_quality;
	int wifi_signal_rssi; //rssi
	int wifi_signal_level; //0-4
//	bool gps;
};
struct keyTimeStamp{
	key ClassMethod_ids;
	u4 time_stamp;
};

struct DvmSystemLocation{
	double latitude;
	double longitude;
	double locationMask; //home, work, mall, anywhere
};




struct OperatingPoint{
	DvmSystemBatteryService batt;
	DvmSystemConnectivity connect;
	unsigned short curActivity;
	DvmSystemLocation curLocation;
};

struct LUTkey{
	key ClassMethod_ids;
	OperatingPoint opt;
};

//
//enum ebatteryHealth{
//	BATTERY_HEALTH_UNKNOWN = 1,
//	BATTERY_HEALTH_GOOD,	 //2
//	BATTERY_HEALTH_OVERHEAT, //3
//	BATTERY_HEALTH_DEAD,     //4
//	BATTERY_HEALTH_OVER_VOLTAGE, //5
//	BATTERY_HEALTH_UNSPECIFIED_FAILURE, //6
//	BATTERY_HEALTH_COLD     //7
//};
//
//
//enum ebatteryStatus{
//	BATTERY_STATUS_UNKNOWN = 1, //1
//	BATTERY_STATUS_CHARGING, //2
//	BATTERY_STATUS_DISCHARGING, //3
//	BATTERY_STATUS_NOT_CHARGING, //4
//	BATTERY_STATUS_FULL //5
//};
//
//enum ebatteryLevel{ //Values taken from R.java generated from aapt
//	//frameworks/base/core/res/res/values/config.xml
//	CRITICAL_BATTERY_LEVEL    		= 4,
//	LOW_BATTERY_WARNING_LEVEL 		= 15,
//	LOW_BATTERY_CLOSE_WARNING_LEVEL = 20
//};
//
//enum ebatteryType{
//	BATTERY_ON_BATTERY = 0,
//	BATTERY_PLUGGED_AC = 1,
//	BATTERY_PLUGGED_USB = 2,
//	BATTERY_PLUGGED_WIRELESS = 4
//};

/*******************************************************************************/

OperatingPoint* dvmGetCurOperatingPoint();

void dvmSystemUpdateBatteryService();
void dvmSystemUpdateConnectivityState();
void dvmSystemUpdateActivity();
void dvmSystemCoreValuesUpdate();

bool dvmSystemCoreValuesGetThreadFlag();
bool dvmSystemCoreValuesStartup();
void dvmSystemCoreValuesShutdown();



u4 dvmSystemReturnMethod(/*OperatingPoint* optpoint,*/ DvmDex* pDvmDex, string className, key curClassMethodIds);

bool dvmSystemCoreValuesCheckLUT(key curClassMethodIds, OperatingPoint optpoint, u4& MethodIds);
void dvmSystemCoreValuesDebugLUT();
/*override the less than operator for the insert in LUT map*/
bool operator <(const LUTkey&lhs, const  LUTkey& rhs);





/** Not Used Now */
//void dvmSystemSetDefaultSwitchPolicy();
//bool dvmSystemGetDefaultSwitchPolicy();
//bool dvmSystemUpdateSwitchPolicy();
//int dvmSystemGetBatteryLevel(JNIEnv* env, char* buf);
//batterytuple dvmSystemGetOperatingPoint();
//void dvmSystemCoreValuesSetJniEnv(JNIEnv* env);


#endif //DALVIK_SYSTEM_CORE_VALUES_H_
