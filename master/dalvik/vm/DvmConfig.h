/*
 * DvmConfig.h
 *
 *  Created on: Sep 18, 2013
 *      Author: salma
 */

#ifndef DALVIK_DVMCONFIG_H_
#define DALVIK_DVMCONFIG_H_

//#include "Dalvik.h"
//#include "DvmDex.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "libdex/OptInvocation.h"

using namespace std;

#define BUILD_PLATFORM_PATH "/system/build.prop"

#define ACTIVITY_MASK_STILL	0x1
#define ACTIVITY_MASK_WALK	0x2
#define ACTIVITY_MASK_RUN	0x4

#define LOCATION_MASK_HOME		 0x1
#define LOCATION_MASK_WORK	 	 0x2
#define LOCATION_MASK_MALL		 0x4
#define LOCATION_MASK_ANYWHERE	 0x8

/* fwd declarations */
struct DvmConfigMethod;
struct DvmConfigClass;
struct DvmConfigFile;
struct methodParameter;
struct Ranges;
// some typedefs for the map
//typedef std::map<u4,u4> class_method_map;
//typedef std::map<u4,class_method_map> DvmConfigMap;


//typedef std::map<u4,u4> DvmMethodTagMap;	// methodID and methodtag

typedef std::pair<u4,u4> key;                // Class ID and method ID
typedef std::pair<double, double> locationpair;  // latitude and longitude


//priority only
typedef std::map<key, u4> DvmConfigMap; // key is a pair
typedef std::map<key, u4>::iterator DvmConfigMapIter;

//methodparam
typedef std::map<key, struct methodParameter > DvmConfigMethodMap; // map the classID, methodID with the methodparam
typedef std::map<key, struct methodParameter>::iterator DvmConfigMethodMapIter;

//tag with ranges // to handle multiple tags
typedef int tag;
typedef std::map<tag, struct Ranges > DvmConfigTagRanges;
typedef std::map<tag, struct Ranges >:: iterator DvmConfigTagRangesIter;

//map for locations
typedef std::map<u4, locationpair> DvmConfigLocationMap; // location(work, home, ...) --> latitude | longitude
typedef std::map<u4, locationpair>::iterator DvmConfigLocationMapIter;

//before mapping to methodid and classid // Not used now
//typedef std::map< pair<string,string>, struct methodParameter > DvmMethodTable;
//typedef std::map< pair<string,string>, struct methodParameter >::iterator DvmMethodTableIter;

// Iterators
//typedef std::map<u4, u4>::iterator class_method_mapIter;
//typedef std::map<u4,class_method_map>::iterator DvmConfigMapIter;


/*Switching policy in the decision tree*/
enum e_policy_t{
	bestfit = 0,
	mustfit = 1
};

enum e_preference_t{
	power_preference = 0,
	connectivity_preference = 1,
	dont_care_preference = 2
};



struct batterytuple{
	unsigned int first; //the highest priority parameter = power
	unsigned int second; // the second priority parameter = temperature
	unsigned int third; // the third priority parameter = voltage
};

struct connectivity{
	bool wifi_state; // connected = true, disconnected  = false
	int  wifi_link_quality;
	int wifi_signal_level;
};

struct methodParameter{
	unsigned int tag;
	unsigned int priority;
	batterytuple ids;
	connectivity conn;
	/*Activity  */
	/* | Run | Walk | Still |
	 * |  0	 |	0	|	1	|	1	|	still only		|
	 * |  0	 |	1	|	0	|	2	|	walk only		|
	 * |  0	 |	1	|	1	|	3	|	still or walk 	|
	 * |  1	 |	0	|	0	|	4	|	run only		|
	 * |  1  |	0	|	1	|	5	|	still or run	|
	 * |  1	 |	1	|	0	|	6	|	walk or run 	|
	 * |  1	 |	1	|	1	|	7	|	all				|
	 */
	unsigned short activitymask;
	/*Location*/
	unsigned short locationmask;
};

struct DvmConfigSensitivityItem{
	string itemName; 					/* should be mapped to one of the enum values of the sensitivity list*/
	/* Ranges of operation: value start and value end */
	double vstart;
	double vend;
};

struct DvmConfigMethod{
	DvmConfigClass* pDvmConfigClass; 	/* pointer to class this method is member of*/
	string methodName;
	//unsigned int priorityLevel;
	//unsigned int tag;
	int itemCount;
	//batterytuple ids; //ids of these method in the sensitivity items; now we support 3 ids(power,temp and voltage)
	//connectivity conn;
	methodParameter methodparam; // holds all the info of the method
	//DvmConfigSensitivityList* sensitivityList;
	vector<DvmConfigSensitivityItem> sensitivityListItems;
};

struct Ranges{
	vector<pair<u4,u4> > tempRange;
	vector<pair<u4,u4> > voltRange;
	vector<pair<u4,u4> > powRange;
};

struct DvmConfigClass{
	DvmConfigFile * pDvmConfigFile; 	/* pointer to config file we come from */
	std::string className;
	//int preference; // 0 =power 1 = connectivity
	int numSensitiveMethods;// = 0;
	int numOfTags;
	e_policy_t policy;
	e_preference_t preference;
	vector<pair<u4,u4> > tempRange;
	vector<pair<u4,u4> > voltRange;
	vector<pair<u4,u4> > powRange;
	DvmConfigTagRanges tagRanges;
	vector<DvmConfigMethod> sensitiveMethods;/* number of methods in this class that will undergo replacement */
};

struct DvmConfigFile{
	int numSensitiveClasses;// = 0; 			/* Number of classes declared in the config file */
	//DvmMethodTable * pmethodTable;
	vector<DvmConfigClass> sensitiveClasses;
};


int dvmConfigFileOpen();
char* dvmConfigFileCreatePath(const char* fileName);
void dvmConfigFileDebug(DvmConfigFile* configfile);
int dvmConfigFileClose(int configFilefd);
DvmConfigFile* dvmConfigFileParse(const char* pathToConfigFile);
DvmConfigMap* dvmConfigFileGetMapAddress();
DvmConfigMethodMap* dvmConfigFileGetMapMethodAddress();
DvmConfigLocationMap* dvmConfigFileGetMapLocationAddress();
u4 dvmConfigMapNumOfSensitiveMethodsInClass(DvmConfigMap* pDvmConfigMap, u4 classId);
void dvmConfigMapDebug(DvmConfigMap* pDvmConfigMap);
void dvmConfigMethodMapDebug(DvmConfigMethodMap* pDvmConfigMethodMap);
bool dvmUpdateConfigMapID(DvmConfigMap* pDvmConfigMap,u4 classId, u4 previousMethodId, u4 updatedMethodId);
bool dvmUpdateConfigMapMethodID(DvmConfigMethodMap* pDvmConfigMethodMap, u4 classId, u4 previousId, u4 updatedId);
u4 dvmGetMethodPriority(DvmConfigMap* pDvmConfigMap, u4 classId, u4 methodId);
void dvmGetMethodParam(DvmConfigMethodMap* pDvmConfigMethodMap, u4 classId, u4 methodId, methodParameter* methparam);
/*Platform related*/
//propName could be "ro.buiuld.platform" to know the platform
//and decide about what are the HW resources this platform has
//char* getSystemProperty( const char* propFile, const char* propName );

/*thread location update handling*/
bool dvmConfigLocationUpdateGetThreadFlag();
bool dvmConfigLocationUpdateStartup();
void dvmConfigLocationUpdateShutdown();


//not used
/* Sensitivity list for replacement --> Could be extended */
//enum {
//
//	ConfigSensBatteryLevel = 0x001,
//	ConfigSensAmbientTemperatureLevel = 0x002,
//	ConfigSensIlluminationLevel = 0x004
//	/* TODO: Add variability related sensors  */
////	ConfigSensMask =
////			(ConfigSensBatteryLevel | ConfigSensAmbientTemperatureLevel | ConfigSensIlluminationLevel)
//};

#endif  //DALVIK_DVMCONFIG_H_
