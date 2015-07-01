/*
 * DvmConfig.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: salma
 */


/* Handle the configuration file under asset folder in .pkg */


#include "Dalvik.h"
#include "DvmConfig.h"
#include <stdio.h>
#include <fcntl.h> // file open
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <unistd.h> // file close
#include <inttypes.h> // for intptr_t
#include <algorithm> // for erase and remove of tabs
#include <pthread.h>
/* The file is saved in the file system /sdcard/config.xml */
/* Open the config file, parse it and save VM structures pointers */

/* Global Vector Definitions */
DvmConfigFile gconfigfile;
DvmConfigMap  gconfigmap;  //used for mapping
DvmConfigMethodMap gconfigmethodmap;
DvmConfigLocationMap gconfiglocationmap;



//DvmMethodTagMap gmethodtagmap;
//DvmMethodTable gmethodTable;

//TODO: maps are not used in this code so far ... the implementation is left for usage afterwards
/* Maps: Used since number of vectors are known at runtime */
//static map<int, vector<DvmConfigMethod> > mapClassMethods; 	//clean: not used now		     /*Map class number to the method vector*/
/* Use std::pair<int, int> for the key to the item, the first int is the class key and the second is the method key*/

///static map< pair<int,int>, vector<DvmConfigSensitivityItem> > mapMethodItems;  /*using class number as a key, map method number to the item vector*/// not used // cleaned out

//typedef map<int, vector<DvmConfigMethod> >::iterator ClassMethodIter;
//typedef map< pair<int,int>, vector<DvmConfigSensitivityItem> >::iterator ClassMethodItemIter;
typedef vector<pair<u4,u4> >::iterator VectorPairRangesIter;

//
//static char*
//android_strdup( const char*  str )
//{
//    int    len;
//    char*  copy;
//
//    if (str == NULL)
//        return NULL;
//
//    len  = strlen(str);
//    copy = (char*)malloc(len+1); // I made casting char*
//    memcpy(copy, str, len);
//    copy[len] = 0;
//
//    return copy;
//}

/** Open Configuration File */
int dvmConfigFileOpen(){

	const char* configFileName = "/sdcard/config.xml";
	int configFd = -1;

	// attempt to open the config file

	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp: Try to open config.xml");
	configFd = open(configFileName, O_RDONLY);
	if(configFd < 0)
	 	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp: FAIL Try to open config.xml");
	else
	 	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp: SUCCESS Try to open config.xml");

	return configFd;

}

/** Close Configuration File */
int dvmConfigFileClose(int configFilefd){
	//returns 0 on success
	return close(configFilefd);
}


/*This method update the 3 vector pairs of ranges so that
 * 1- The first value in the smaller and the second is the larger*/
/* 2- The ranges that are not covered are added
 * */
//void DoSanityCheckOfRanges(){
//	for(int i=0; i<gconfigfile.numSensitiveClasses; i++){
//		// Fix Power Ranges
//		for(u4 p =0; p< gconfigfile.sensitiveClasses.at(i).powRange.size(); p++){
//			if(gconfigfile.sensitiveClasses.at(i).powRange.at(p).first > gconfigfile.sensitiveClasses.at(i).powRange.at(p).second){
//				//swap
//				u4 tmp = gconfigfile.sensitiveClasses.at(i).powRange.at(p).first;
//				gconfigfile.sensitiveClasses.at(i).powRange.at(p).first = gconfigfile.sensitiveClasses.at(i).powRange.at(p).second;
//				gconfigfile.sensitiveClasses.at(i).powRange.at(p).second = tmp;
//				//sanity
//				//no need for this bec we have unsigned value
////				if(gconfigfile.sensitiveClasses.at(i).powRange.at(p).first < 0)
////					gconfigfile.sensitiveClasses.at(i).powRange.at(p).first = 0;
//				if(gconfigfile.sensitiveClasses.at(i).powRange.at(p).second > 100)
//					gconfigfile.sensitiveClasses.at(i).powRange.at(p).second = 100;
//			}
//		}
//		// Fix Voltage Ranges
//		for(u4 v =0; v< gconfigfile.sensitiveClasses.at(i).voltRange.size(); v++){
//			if(gconfigfile.sensitiveClasses.at(i).voltRange.at(v).first > gconfigfile.sensitiveClasses.at(i).voltRange.at(v).second){
//				//swap
//				u4 tmp = gconfigfile.sensitiveClasses.at(i).voltRange.at(v).first;
//				gconfigfile.sensitiveClasses.at(i).voltRange.at(v).first = gconfigfile.sensitiveClasses.at(i).voltRange.at(v).second;
//				gconfigfile.sensitiveClasses.at(i).voltRange.at(v).second = tmp;
//				//sanity for lithium ion 3.4/4.2v
//				if(gconfigfile.sensitiveClasses.at(i).voltRange.at(v).first < 3.4)
//					gconfigfile.sensitiveClasses.at(i).voltRange.at(v).first = 3.4;
//				if(gconfigfile.sensitiveClasses.at(i).voltRange.at(v).second > 4.2)
//					gconfigfile.sensitiveClasses.at(i).voltRange.at(v).second = 4.2;
//
//			}
//		}
//		// Fix Temperature Ranges
//		for(u4 t =0; t< gconfigfile.sensitiveClasses.at(i).tempRange.size(); t++){
//			if(gconfigfile.sensitiveClasses.at(i).tempRange.at(t).first > gconfigfile.sensitiveClasses.at(i).tempRange.at(t).second){
//				//swap
//				u4 tmp = gconfigfile.sensitiveClasses.at(i).tempRange.at(t).first;
//				gconfigfile.sensitiveClasses.at(i).tempRange.at(t).first = gconfigfile.sensitiveClasses.at(i).tempRange.at(t).second;
//				gconfigfile.sensitiveClasses.at(i).tempRange.at(t).second = tmp;
//				//sanity for lithium ion
//				if(gconfigfile.sensitiveClasses.at(i).tempRange.at(t).second > 70)
//					gconfigfile.sensitiveClasses.at(i).tempRange.at(t).second = 70;
//			}
//		}
//
//	}
//
//}



/*It is called from JarFile.cpp*/
DvmConfigFile* dvmConfigFileParse(const char* pathToConfigFile){


	//if(configFilefd > 0)
	if(pathToConfigFile != NULL)
	{
		ALOGD("Parsing the config file %s",pathToConfigFile);
		std::string line;
		std::ifstream in(pathToConfigFile);
	//	std::ifstream in("/sdcard/config.xml");

		bool begin_tagClass				= false;
		bool begin_tagClassName			= false;
		bool begin_tagClassPreference   = false;
		bool begin_tagClassPolicy       = false;
		bool begin_tagClassNumOfTags	= false;
		bool begin_tagMethod			= false;
		bool begin_tagMethodName		= false;
		bool begin_tagPriority			= false;
		bool begin_tagMethodtag			= false;
		bool begin_tagWifi				= false;
		bool begin_tagActivity			= false;
		bool begin_tagLocation			= false;
		bool begin_tagListItem			= false;
		bool begin_tagItemName			= false;
		bool begin_tagListStartValue	= false;
		bool begin_tagListEndValue		= false;

		/* Current structs*/
		DvmConfigMethod curDvmConfigMethod;
		DvmConfigClass curDvmConfigClass;
		DvmConfigSensitivityItem curDvmConfigItem;


		/* initialize structs */
		gconfigfile.numSensitiveClasses = 0;
		curDvmConfigMethod.itemCount = 0;
		curDvmConfigClass.numSensitiveMethods = 0;

		/* initialize location map with zeros -- this will be updated at runtime*/
		ALOGD("----Initializing the location map-----");
		gconfiglocationmap.insert(std::make_pair(LOCATION_MASK_HOME, locationpair(0,0)));
		gconfiglocationmap.insert(std::make_pair(LOCATION_MASK_WORK, locationpair(0,0)));
		gconfiglocationmap.insert(std::make_pair(LOCATION_MASK_MALL, locationpair(0,0)));
		gconfiglocationmap.insert(std::make_pair(LOCATION_MASK_ANYWHERE,locationpair(0,0)));



		/* vectors to hold structs */
		vector<DvmConfigClass> curdvmConfigClassVec;
		vector<DvmConfigMethod> curdvmCongfigMethodVec;
		vector<DvmConfigSensitivityItem> curdvmConfigSensitivityItemVec;

		//The tree of ranges
		vector<pair<u4,u4> > curtemperatureRanges;
		vector<pair<u4,u4> > curpowerRanges;
		vector<pair<u4,u4> > curvoltageRanges;
		pair<u4,u4> range;

		u4 curtempindex = 0;
		u4 curvoltindex = 0;
		u4 curpowindex =0;
		//ToDo: Check this//
		gconfigfile.sensitiveClasses = curdvmConfigClassVec;

		string tmp = ""; // = new string(); // strip whitespaces from the beginning

		while (!in.eof()){

				getline(in,line);

		        tmp = line;
		        // remove all occurrences of leading and trailing tabs  and spaces in the string
		        tmp.erase(remove(tmp.begin(), tmp.end(), ' '), tmp.end());
		        tmp.erase(remove(tmp.begin(), tmp.end(), '\t'), tmp.end());

		        // Parse Classes
		        if (tmp == "<Class>"){
		            begin_tagClass = true;
		            gconfigfile.numSensitiveClasses++;
		            continue; // get next line;
		        }
		        else if (tmp == "</Class>"){
		        	//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp: %s", tmp.c_str());
		        	/* at the end of the class copy the method vector there */
		        	//mapClassMethods[gconfigfile.numSensitiveClasses] = curdvmCongfigMethodVec; //clean notused now
		        	//define the ranges for these class
		        	curDvmConfigClass.tempRange = curtemperatureRanges;
		        	curDvmConfigClass.voltRange = curvoltageRanges;
		        	curDvmConfigClass.powRange = curpowerRanges;

		        	curDvmConfigClass.sensitiveMethods=curdvmCongfigMethodVec;
		        	curdvmConfigClassVec.push_back(curDvmConfigClass);

		        	//clear the number of methods because a new class may start
		        	curDvmConfigClass.numSensitiveMethods = 0;
		        	//clear the Method vector because a new class may start
		        	curdvmCongfigMethodVec.clear();
		        	curtemperatureRanges.clear();
		        	curvoltageRanges.clear();
		        	curpowerRanges.clear();
		            begin_tagClass = false;

		        }

		        // --> Parse Class properties
		        if((begin_tagClass==true) && (begin_tagMethod==false)){
		        	if(tmp == "<ClassName>"){
		        		//cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		begin_tagClassName = true;
		        		continue; // get the class name
		        	}
		        	else if (tmp == "</ClassName>"){
		        		//cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		begin_tagClassName = false;
		        	}
		        	else if (tmp == "<Preference>"){
		        		begin_tagClassPreference = true;
		        		continue;
		        	}
		        	else if(tmp == "</Preference>"){
		        		begin_tagClassPreference = false;
		        	}
		        	else if(tmp == "<Policy>"){
		        		begin_tagClassPolicy = true;
		        		continue;
		        	}
		        	else if(tmp == "</Policy>"){
		        		begin_tagClassPolicy = false;
		        	}
		        	else if(tmp == "<NumOfTags>"){
		        		begin_tagClassNumOfTags = true;
		        		continue;
		        	}
		        	else if(tmp == "</NumOfTags>"){
		        		begin_tagClassNumOfTags = false;
		        	}
		        }

		        // parse class name
		        if(begin_tagClassName == true){
		        	//cout << tmp << "\n";
		        	//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		            curDvmConfigClass.className = tmp;
//		        	//debug
//		        	cout << curDvmConfigClass.className << "\n";
		        	curDvmConfigClass.pDvmConfigFile = &gconfigfile;
		        	continue;
		        }

		        //parse class preference
		        if(begin_tagClassPreference == true){
		        	if (tmp.find("power") != std::string::npos)
		        		curDvmConfigClass.preference = power_preference;
		        	else if (tmp.find("connectivity") != std::string::npos)
		        		curDvmConfigClass.preference = connectivity_preference;
		        	else if (tmp.find("don't care") != std::string::npos)
		        		curDvmConfigClass.preference = dont_care_preference;

		        	//curDvmConfigClass.preference =  atoi(tmp.c_str());
		        	begin_tagClassPreference = false;
		        	continue;
		        }

		        //parse class policy
		        if(begin_tagClassPolicy == true){
		        	curDvmConfigClass.policy = static_cast<e_policy_t>(atoi(tmp.c_str()));
		        	begin_tagClassPolicy = false;
		        	continue;
		        }

		        //parse class num of tags
		        if(begin_tagClassNumOfTags == true){
		        	curDvmConfigClass.numOfTags = atoi(tmp.c_str());
		        	begin_tagClassNumOfTags = false;
		        	//create the tagRanges and initialize
		        	for (int tag = 1; tag <= curDvmConfigClass.numOfTags; tag++ ){
		        		Ranges newRange;
		        		curDvmConfigClass.tagRanges.insert(pair<int,Ranges>(tag, newRange));
		        	}
		        	continue;
		        }

		        // Parse Methods
		        if(tmp == "<Method>"){
		        //	cout << tmp << "\n";
		        	//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        	begin_tagMethod = true;
		        	curDvmConfigClass.numSensitiveMethods++;
		        	// initialize the activity mask;
		        	curDvmConfigMethod.methodparam.activitymask = 0;
//		        	//Debug
//		        	cout << "curDvmConfigClass.numSensitiveMethods" << curDvmConfigClass.numSensitiveMethods << "\n";

		        	continue;
		        }
		        else if(tmp == "</Method>"){
		        	//cout << tmp << "\n";
		        	//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        	curDvmConfigMethod.sensitivityListItems = curdvmConfigSensitivityItemVec;
//		        	//Debug
//		        	if(curdvmConfigSensitivityItemVec.size() > 0){
//		        		cout << "curdvmConfigSensitivityItemVec.size()" << curdvmConfigSensitivityItemVec.size() << "\n" ;
//		        	}
	        		//fill the methodParameter here
	        		//curDvmConfigMethod.methodparam.ids = curDvmConfigMethod.ids;
	        		//curDvmConfigMethod.methodparam.priority = curDvmConfigMethod.priorityLevel;
	        		//curDvmConfigMethod.methodparam.tag = curDvmConfigMethod.tag;
	        		//curDvmConfigMethod.methodparam.conn = curDvmConfigMethod.conn;

	        		curdvmCongfigMethodVec.push_back(curDvmConfigMethod);

		        	//mapMethodItems[make_pair(gconfigfile.numSensitiveClasses,curDvmConfigClass.numSensitiveMethods)] = curdvmConfigSensitivityItemVec; //cleaned out

		        	//gmethodTable.insert(make_pair (make_pair(curDvmConfigClass.className, curDvmConfigMethod.methodName), curDvmConfigMethod.methodparam) );
		        	//clear the number of items because a new method will start
		        	curDvmConfigMethod.itemCount = 0;
		        	//clear the curdvmConfigSensitivityItemVec for the next map;
		        	curdvmConfigSensitivityItemVec.clear();
		        	begin_tagMethod = false;
		        }

		        if(begin_tagMethod){
		        	if(tmp == "<MethodName>"){
		        	//	cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		begin_tagMethodName = true;
		        		continue;
		        	}
		        	else if(tmp == "</MethodName>"){
		        		//cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		begin_tagMethodName = false;
		        	}
		        	else if(tmp == "<priority>"){
		        		//cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		begin_tagPriority = true;
		        		continue;
		        	}
		        	else if(tmp == "</priority>"){
		        		//cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		begin_tagPriority = false;
		        	}
		        	else if(tmp == "<tag>"){
		        		begin_tagMethodtag = true;
		        		continue;
		        	}
		        	else if(tmp == "</tag>"){
		        		begin_tagMethodtag = false;
		        	}
		        	else if(tmp == "<wifi>"){
		        		begin_tagWifi = true;
		        		continue;
		        	}
		        	else if(tmp == "</wifi>"){
		        		begin_tagWifi = false;
		        	}
		        	else if(tmp == "<activity>"){
		        		begin_tagActivity = true;
		        		continue;
		        	}
		        	else if(tmp == "</activity>"){
		        		begin_tagActivity = false;
		        	}
		        	else if(tmp == "<location>"){
		        		begin_tagLocation = true;
		        		continue;
		        	}
		        	else if(tmp == "</location>"){
		        		begin_tagActivity = false;
		        	}


		        }
		        /***** Parse Method details ******/

		        //Parse Method Name
		        if(begin_tagMethod && begin_tagMethodName){
		        	//cout << tmp << "\n";
		        	//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        	curDvmConfigMethod.methodName = tmp;
		        	curDvmConfigMethod.pDvmConfigClass = &curDvmConfigClass;
		        	begin_tagMethodName = false;
		        }

		        //Parse Method Priority
		        if(begin_tagMethod && begin_tagPriority){
		        	//cout << tmp << "\n";
		        	//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        	curDvmConfigMethod.methodparam.priority = atoi(tmp.c_str());
		        	begin_tagPriority = false;
		        }

		        //Parse Method Tag
		        if(begin_tagMethod && begin_tagMethodtag){
		        	curDvmConfigMethod.methodparam.tag = atoi(tmp.c_str());
		        	begin_tagMethodtag = false;
		        }

		        //Parse Wifi
		        if(begin_tagMethod && begin_tagWifi){
		           	curDvmConfigMethod.methodparam.conn.wifi_state = atoi(tmp.c_str());
		           	begin_tagWifi = false;
		        }

		        // Parse Activity
		        if(begin_tagMethod && begin_tagActivity){
		        	if (tmp.find("still") != std::string::npos)
		        		curDvmConfigMethod.methodparam.activitymask = curDvmConfigMethod.methodparam.activitymask | ACTIVITY_MASK_STILL;

		        	if (tmp.find("walk") != std::string::npos)
		        		curDvmConfigMethod.methodparam.activitymask = curDvmConfigMethod.methodparam.activitymask | ACTIVITY_MASK_WALK;

		        	if (tmp.find("run") != std::string::npos)
		        		curDvmConfigMethod.methodparam.activitymask = curDvmConfigMethod.methodparam.activitymask | ACTIVITY_MASK_RUN;

		        //	ALOGD("Method %s, Activity Mask = %d",curDvmConfigMethod.methodName.c_str(), curDvmConfigMethod.methodparam.activitymask);
		           	begin_tagActivity = false;
		        }

		        // Parse Location
		        if(begin_tagMethod && begin_tagLocation){
		        //	ALOGD("Parse Location");
		        	if (tmp.find("work") != std::string::npos){
		        		curDvmConfigMethod.methodparam.locationmask = curDvmConfigMethod.methodparam.locationmask | LOCATION_MASK_WORK;

		        	}

		        	if (tmp.find("home") != std::string::npos){
		        		curDvmConfigMethod.methodparam.locationmask = curDvmConfigMethod.methodparam.locationmask | LOCATION_MASK_HOME;
		        	}

		        	if (tmp.find("mall") != std::string::npos){
		        		curDvmConfigMethod.methodparam.locationmask = curDvmConfigMethod.methodparam.locationmask | LOCATION_MASK_MALL;
		        	}

		        	if (tmp.find("anywhere") != std::string::npos){
		        		curDvmConfigMethod.methodparam.locationmask = curDvmConfigMethod.methodparam.locationmask | LOCATION_MASK_ANYWHERE;
		        	}


		        	//	ALOGD("Method %s, Location Mask = %d",curDvmConfigMethod.methodName.c_str(), curDvmConfigMethod.methodparam.locationmask);
		           	begin_tagLocation = false;
		        }

		        // Start parsing the list
		        if(begin_tagMethod){ //&& begin_tagList){

		        	if(tmp == "<item>"){

		        		//cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		begin_tagListItem = true;
		        		curDvmConfigMethod.itemCount++;

		        		continue; // get the name of the item
		        	}
		        	else if(tmp == "</item>"){
		        		//cout << tmp << "\n";
		        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        		curdvmConfigSensitivityItemVec.push_back(curDvmConfigItem);
		        		//Debug
//		        		if(curdvmConfigSensitivityItemVec.size() > 0){
//		        			cout << "curdvmConfigSensitivityItemVec.size()" << curdvmConfigSensitivityItemVec.size() << "\n";
//		        		}

		        		//cout<< curDvmConfigItem.itemName;
		        		begin_tagListItem = false;
		        		continue;
		        	}

		        	if(begin_tagListItem){
			        	if(tmp == "<itemName>"){
			        	//	cout<< tmp << "\n";
			        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
			        		begin_tagItemName = true;
			        		continue;
			        	}
			        	else if (tmp == "</itemName>"){
			        		//cout<< tmp << "\n";
			        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
			        		begin_tagItemName = false;
			        		continue;
			        	}
			        	else if(tmp == "<vstart>"){
		        		//	cout << tmp << "\n";
			        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        			begin_tagListStartValue = true;
		        			continue;
		        		}
		        		else if(tmp == "</vstart>"){
		        			//cout << tmp << "\n";
		        			//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        			begin_tagListStartValue = false;
		        			continue;
		        		}
		        		else if (tmp == "<vend>"){
		        			//cout << tmp << "\n";
		        			//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        			begin_tagListEndValue = true;
		        			continue;
		        		}
		        		else if (tmp == "</vend>"){
		        			//cout << tmp << "\n";
		        			//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        			begin_tagListEndValue = false;
		        			continue;
		        		}



			        	// parse item name
			        	if(begin_tagItemName){
			        		//cout << tmp << "\n";
			        		//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
			        		curDvmConfigItem.itemName = tmp;
			        		begin_tagItemName = false;
			        			continue;
			        	}

		        		//parse the values and indexing them
		        		if(begin_tagListStartValue){
		        			//cout << tmp << "\n";
		        			//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        			// capture it
		        			curDvmConfigItem.vstart = atof(tmp.c_str());
		        			begin_tagListStartValue = false;
		        			range.first = curDvmConfigItem.vstart;
		        		}

		        		if(begin_tagListEndValue){
		        			//cout << tmp << "\n";
		        			//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:%s", tmp.c_str());
		        			//capture it
		        			curDvmConfigItem.vend = atof(tmp.c_str());
		        			begin_tagListEndValue = false;
		        			range.second=curDvmConfigItem.vend;

		        			if(curDvmConfigItem.itemName  == "temperature"){
		        				//printf("vend temp: %d\n", curDvmConfigItem.vend );
		        				//before push the range .. check if it is already existed
		        				VectorPairRangesIter positionOfRangeIteratortag =  std::find(curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].tempRange.begin(),
		        																		  curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].tempRange.end(),
		        																		  range);

		        				if(positionOfRangeIteratortag != curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].tempRange.end()){
		        					curtempindex = positionOfRangeIteratortag - curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].tempRange.begin();
		        				}

		        				VectorPairRangesIter positionOfRangeIterator =  std::find(curtemperatureRanges.begin(), curtemperatureRanges.end(), range);
		        				if(positionOfRangeIterator != curtemperatureRanges.end()){
		        					// it contains the range
		        					curtempindex = positionOfRangeIterator - curtemperatureRanges.begin();
		        					//printf("it contains the range curtempindex = %d\n",curtempindex);
		        				}
		        				else{
		        					//it doesn't contain the range so push it

		        					curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].tempRange.push_back(range);
		        					curtempindex = curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].tempRange.size()-1;

		        					curtemperatureRanges.push_back(range);
		        					curtempindex = curtemperatureRanges.size()-1;

		        					//printf("it doesn't contains the range curtempindex = %d\n",curtempindex);
		        				}
		        				curDvmConfigMethod.methodparam.ids.second = curtempindex;
		        				//printf("curDvmConfigMethod temp id = %d\n",curDvmConfigMethod.ids.second );

		        			}
		        			else if(curDvmConfigItem.itemName  == "voltage"){
		        				//printf("vend voltage: %d\n", curDvmConfigItem.vend );

		        				VectorPairRangesIter positionOfRangeIteratortag =  std::find(curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].voltRange.begin(),
		        						        										  curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].voltRange.end(),
		        						        										  range);
		        				if(positionOfRangeIteratortag != curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].voltRange.end()){
		        					curvoltindex = positionOfRangeIteratortag - curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].voltRange.begin();
		        				}

		        				VectorPairRangesIter positionOfRangeIterator =  std::find(curvoltageRanges.begin(), curvoltageRanges.end(), range);
		        				if(positionOfRangeIterator != curvoltageRanges.end()){
		        					// it contains the range
		        					curvoltindex = positionOfRangeIterator - curvoltageRanges.begin();
		        				}
		        				else{
		        				//it doesn't contain the range so push it

		        					curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].voltRange.push_back(range);
		        					curvoltindex = curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].voltRange.size()-1;

		        					curvoltageRanges.push_back(range);
		        					curvoltindex = curvoltageRanges.size()-1;

		        				}
		        				curDvmConfigMethod.methodparam.ids.third = curvoltindex;
		        			}
		        			else if(curDvmConfigItem.itemName == "battery"){
		        				//printf("vend battery: %d\n", curDvmConfigItem.vend );

		        				VectorPairRangesIter positionOfRangeIteratortag =  std::find(curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].powRange.begin(),
		        																		  curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].powRange.end(),
		        																		  range);
		        				if(positionOfRangeIteratortag != curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].powRange.end()){
		        					curpowindex = positionOfRangeIteratortag - curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].powRange.begin();
		        				}

		        				VectorPairRangesIter positionOfRangeIterator =  std::find(curpowerRanges.begin(), curpowerRanges.end(), range);
		        				if(positionOfRangeIterator != curpowerRanges.end()){
		        				// it contains the range
		        					curpowindex = positionOfRangeIterator - curpowerRanges.begin();
		        				}
		        				else{
		        				//it doesn't contain the range so push it
		        					curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].powRange.push_back(range);
		        					curpowindex = curDvmConfigClass.tagRanges[curDvmConfigMethod.methodparam.tag].powRange.size()-1;

		        					curpowerRanges.push_back(range);
		        					curpowindex = curpowerRanges.size()-1;

		        				}
		        				curDvmConfigMethod.methodparam.ids.first = curpowindex;
		        			}
		        			else{
		        				ALOGE("The Item Name %s is not taken into the account",curDvmConfigItem.itemName.c_str());
		        			}
		        			continue;
		        		}

		        		continue;

		        	}//if(begin_tagListItem)


		        }// if(begin_tagMethod)


		    }// while(!in.eof)

		gconfigfile.sensitiveClasses = curdvmConfigClassVec;
		//gconfigfile.pmethodTable = &gmethodTable;
	}//if(dvmConfigFileOpen() > 0)

	/* Close the configuration file */
	//	dvmConfigFileClose(configFilefd);

	// Ignore the sanity check now to save time
	//DoSanityCheckOfRanges();

	return &gconfigfile;

}

char* dvmConfigFileCreatePath(const char* fileName){

	  char* cachedName2 = NULL;
	   /* get the application name without the trailing ID (-1 or -2)
	    * Note the the dash in invalid identifier for the name of the package so having (-1) in the package name is unlikely to happen
	    * Ex: fileName = /data/app/hidden_dynamic_loading-1.apk
	    * Get the name com.hidden_dynamic_loading which is the name of the lib from the fileName which is /data/app/hidden_dynamic_loading-1.apk
	    */
	   const char* lastPart = strrchr(fileName, '/');
	   if (lastPart != NULL)
	           lastPart++;
	   else
	           lastPart = fileName;
   	  //ALOGD("lastPart = %s", lastPart);
  	  //lastPart = hidden_dynamic_loading-1.apk
   	   const char* tailingPart = strrchr(lastPart, '-');
   	   if(tailingPart != NULL);
   	   else{
		 //if there is no ID
		 tailingPart = strrchr(lastPart, '.');
		 if(tailingPart != NULL);
		 else
		   tailingPart = lastPart;
	    }
	    //ALOGD("tailingPart = %s", tailingPart);
   	 /*Remove from the lastPart the trailingPart*/
   	    string lastPartStr = string(lastPart);
	    string tailingPartStr = string(tailingPart);
	    lastPartStr.erase(lastPartStr.find(tailingPartStr),tailingPartStr.size());
	    //ALOGD("lastPartStr = %s", lastPartStr.c_str());
         cachedName2 = dexOptGenerateCacheFileName(fileName, "assets@config.xml");
	//        __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp: cache name = %s", cachedName2);

     /* Form the cache directory of the configFile */
         lastPartStr = "/data/"+ lastPartStr + "/cache";
	     //ALOGD("lastPartStr = %s", lastPartStr.c_str());
	     string cachedName2str = string(cachedName2);
	     //remove the directory /dalvik-cache from the cache file path
	     cachedName2str.replace(cachedName2str.find("/dalvik-cache"), sizeof("/dalvik-cache")-1, lastPartStr);
	//       __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp: cache name = %s", cachedName2str.c_str());

	     return (char*)cachedName2str.c_str();

}

/* Debug*/
void dvmConfigFileDebug(DvmConfigFile* configfile){

	for(int i =0; i< configfile->numSensitiveClasses; i++){
		__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:Class Name: %s" , configfile->sensitiveClasses.at(i).className.c_str() );
		__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:number of methods: %d" ,configfile->sensitiveClasses.at(i).numSensitiveMethods);
		for(int j=0; j<configfile->sensitiveClasses.at(i).numSensitiveMethods; j++ ){
			__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:Method Name: %s", configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodName.c_str());
			__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:number of items:%d " ,configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).itemCount);
			__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:priority level: %d" ,configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodparam.priority);
			__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:tag: %d" ,configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodparam.tag);
			__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:wifi: %d" ,configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodparam.conn.wifi_state);

			for(int k=0; k< configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).itemCount; k++){
				__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:item name: %s", configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).sensitivityListItems.at(k).itemName.c_str());
				__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:value start: %f", configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).sensitivityListItems.at(k).vstart);
				__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmConfig.cpp:value end: %f",configfile->sensitiveClasses.at(i).sensitiveMethods.at(j).sensitivityListItems.at(k).vend);
			}
		}
	}



}

DvmConfigMap* dvmConfigFileGetMapAddress(){
	return &gconfigmap;
}

DvmConfigMethodMap* dvmConfigFileGetMapMethodAddress(){
	return &gconfigmethodmap;
}

DvmConfigLocationMap* dvmConfigFileGetMapLocationAddress(){
	return &gconfiglocationmap;
}


u4 dvmConfigMapNumOfSensitiveMethodsInClass(DvmConfigMap* pDvmConfigMap, u4 classId){

	if(pDvmConfigMap == NULL) return 0;

	u4 count =0;
	for(DvmConfigMapIter it=pDvmConfigMap->begin(); it!=pDvmConfigMap->end(); ++it){
		if(it->first.first == classId)
			count++;

		if(count!=0 && it->first.first != classId)
			break; //early terminate the loop if class was found and count was incremented bec. map is arranged by default
	}


	return count;
}

void dvmConfigMapDebug(DvmConfigMap* pDvmConfigMap){

	//ALOGD("dvmDexConfigMapDebug");
	if(pDvmConfigMap==NULL) return;
	if(pDvmConfigMap->size() ==0 ) return;

	//ALOGD("dvmDexConfigMapDebug: pDvmConfigMap exists and size = %d",pDvmConfigMap->size() );

	for(DvmConfigMapIter it=pDvmConfigMap->begin(); it!=pDvmConfigMap->end(); ++it){
		ALOGD("ClassID:%d MethodID:%d ==> Priority:%d ",it->first.first, it->first.second, it->second);
	}
}

void dvmConfigLocationMapDebug(DvmConfigLocationMap* pDvmConfigLocationMap){
	if(pDvmConfigLocationMap==NULL) return;
	if(pDvmConfigLocationMap->size() ==0 ) return;
	for(DvmConfigLocationMapIter it=pDvmConfigLocationMap->begin(); it!=pDvmConfigLocationMap->end(); ++it){
			ALOGD("location:%d ==> latitude:%f, longitude=%f ",it->first, it->second.first, it->second.second);
	}
}


void dvmConfigMethodMapDebug(DvmConfigMethodMap* pDvmConfigMethodMap){

	ALOGD("dvmDexConfigMapMethodDebug");
	if(pDvmConfigMethodMap==NULL) return;
	if(pDvmConfigMethodMap->size() ==0 ) return;

	ALOGD("dvmDexConfigMapDebug: pDvmConfigMap exists and size = %d",pDvmConfigMethodMap->size() );

	for(DvmConfigMethodMapIter it=pDvmConfigMethodMap->begin(); it!=pDvmConfigMethodMap->end(); ++it){
		ALOGD("ClassID:%d MethodID:%d ==> tag:%d, priority:%d, powerindex:%d, tempindex:%d, voltindex:%d ",it->first.first, it->first.second, it->second.tag, it->second.priority, it->second.ids.first, it->second.ids.second, it->second.ids.third);
	}
}

/*return true means replacement took place
 * return false means replacement didn't happen
 * */
bool dvmUpdateConfigMapID(DvmConfigMap* pDvmConfigMap, u4 classId, u4 previousId, u4 updatedId){

	if(pDvmConfigMap == NULL || pDvmConfigMap->size() ==0 ){
		//ALOGE("pDvmConfigMap = NULL dvmDexUpdateConfigMapMethodID");
		return false;
	}

	DvmConfigMapIter it;
	it = pDvmConfigMap->find(key(classId,previousId));
	if(it != pDvmConfigMap->end()){
		//keys are not modifiable
		//erase this specific entry and reinsert a new entry with the updated key
		//keeping in mind that priority should be maintained
		u4 priority = it->second;
		pDvmConfigMap->erase(it);
		pDvmConfigMap->insert(std::make_pair(key(classId,updatedId),priority));
		return true;
	}
	return false;
}


bool dvmUpdateConfigMapMethodID(DvmConfigMethodMap* pDvmConfigMethodMap, u4 classId, u4 previousId, u4 updatedId){

	if(pDvmConfigMethodMap == NULL || pDvmConfigMethodMap->size() ==0 ){
		ALOGE("pDvmConfigMap = NULL dvmDexUpdateConfigMapMethodID");
		return false;
	}

	DvmConfigMethodMapIter it;
	it = pDvmConfigMethodMap->find(key(classId,previousId));

	if(it != pDvmConfigMethodMap->end()){
		//keys are not modifiable
		//erase this specific entry and reinsert a new entry with the updated key
		//keeping in mind that priority should be maintained
		methodParameter mp = it->second;
		pDvmConfigMethodMap->erase(it);
		pDvmConfigMethodMap->insert(std::make_pair(key(classId,updatedId),mp));
		return true;
	}
	return false;
}

/*retrieve priority of a method from the config map*/
/*method ID could be the flat one or the vtable index it doesn't matter but the whole map should be consistent*/
/*return 0 means that this method doesn't have an assigned priority
 * i.e. default priority = 0
 * */
u4 dvmGetMethodPriority(DvmConfigMap* pDvmConfigMap, u4 classId, u4 methodId){

	u4 defaultPriority = 0;
	if(pDvmConfigMap == NULL || pDvmConfigMap->size() ==0 ){
		//ALOGE("pDvmConfigMap = NULL dvmDexUpdateConfigMapMethodID");
		return defaultPriority;
	}

	/*search through the map for the specific class id and method id pair to get the priority*/
	DvmConfigMapIter it;
	it = pDvmConfigMap->find(key(classId,methodId));
	if(it != pDvmConfigMap->end())
			return it->second;

	return defaultPriority;
}

void dvmGetMethodParam(DvmConfigMethodMap* pDvmConfigMethodMap, u4 classId, u4 methodId, methodParameter* methparam){
	if(pDvmConfigMethodMap == NULL || pDvmConfigMethodMap->size() == 0){
		methparam->priority = 0; //default
	}
	DvmConfigMethodMapIter it;
	it = pDvmConfigMethodMap->find(key(classId,methodId));
	if(it != pDvmConfigMethodMap->end())
		methparam = & it->second;
}



/**** Thread handler for updating the location map - online change of parameters *****/

pthread_t locationUpdateHandle;
bool locationUpdatethreadflag = false;

static void* locationUpdateCatcherThreadStart(void* arg);


bool dvmConfigLocationUpdateGetThreadFlag(){
	return locationUpdatethreadflag;
}

bool dvmConfigLocationUpdateStartup(){

	if(locationUpdatethreadflag == false)
		if (!dvmCreateInternalThread(&locationUpdateHandle,
                "CAreDroid Location Update", locationUpdateCatcherThreadStart, NULL))
			return false;

	locationUpdatethreadflag = true;
    return true;
}

void dvmConfigLocationUpdateShutdown(){
    if (locationUpdatethreadflag == 0)      // not started yet
        return;

    pthread_kill(locationUpdateHandle, SIGQUIT);

    pthread_join(locationUpdateHandle, NULL);
    ALOGV("CAreDroid Location Update has shut down");
}


static void locationUpdate(){

	/*Access the file system*/
	const char* locationFileName = "/sdcard/MyPrefsFile.txt";
	int locationFileFd = -1;
	//ALOGD("DvmConfig.cpp: Try to open MyPrefsFile.txt");
	locationFileFd = open(locationFileName, O_RDONLY);
	if(locationFileFd < 0){
		// ALOGD("DvmConfig.cpp: FAIL Try to open MyPrefsFile.txt");
		 return;
	}
	else
		 ALOGD("DvmConfig.cpp: SUCCESS Try to open MyPrefsFile.txt");

	/*** Important FIX --- > needed ****/
	//ToDo: check if it has been updated instead of continuously parsing

	/*parse the file to get the value of the map*/
	std::string line;
	std::string tmp;
	std::ifstream in(locationFileName);
	const int MAX_TOKENS_PER_LINE =4; /*512;*/

	// array to store memory addresses of the tokens in buf
	char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
	while (!in.eof()){

			getline(in,line);

	        tmp = line;
	        // remove all occurrences of leading and trailing tabs  and spaces in the string
	        tmp.erase(remove(tmp.begin(), tmp.end(), ' '), tmp.end());
	        tmp.erase(remove(tmp.begin(), tmp.end(), '\t'), tmp.end());

	        if (tmp.find("work") != std::string::npos){
	        	ALOGD("DvmConfig.cpp: found work in the MyPrefsFile");
				char * curline =const_cast<char*>(tmp.c_str());
				ALOGD("%s", curline);
				token[0] = strtok(curline, ":"); // first token
				//ALOGD("first token %s", token[0]);
				if (token[0]) // zero if line is blank
				{
				  for (int n = 1; n < MAX_TOKENS_PER_LINE; n++)
				  {
				     token[n] = strtok(0, ":"); // subsequent tokens
				     //ALOGD("\t %d, %s", n, token[n] );
				     if (!token[n]) break; // no more tokens
				  }
				}

		        //get longitude and latitude
		        char* latitude = token[1];
		        char* longitude = token[2];

		        //ALOGD("longitude = %s and latitude = %s ",longitude, latitude );
		        if(gconfiglocationmap.size() > 0){
		        	gconfiglocationmap[LOCATION_MASK_WORK] =  locationpair(atof(latitude),atof(longitude));
		        	//ALOGD("Config Map Location is updated with MALL");
		        }

		        //ALOGD("Config Map Location is updated with Work");
		        continue;
	        }

	        if (tmp.find("home") != std::string::npos){
	        	ALOGD("DvmConfig.cpp: found home in the MyPrefsFile");
				char * curline =const_cast<char*>(tmp.c_str());
				token[0] = strtok(curline, ":"); // first token
				if (token[0]) // zero if line is blank
				{
				  for (int n = 1; n < MAX_TOKENS_PER_LINE; n++)
				  {
				     token[n] = strtok(0, ":"); // subsequent tokens
				    // ALOGD("\t %d, %s", n, token[n] );
				     if (!token[n]) break; // no more tokens
				  }
				}

		        //get longitude and latitude
		        char* latitude = token[1];
		        char* longitude = token[2];

		        //ALOGD("longitude = %s and latitude = %s ",longitude, latitude );
		        if(gconfiglocationmap.size() > 0){
		        	gconfiglocationmap[LOCATION_MASK_HOME] =  locationpair(atof(latitude),atof(longitude));
		        	//ALOGD("Config Map Location is updated with MALL");
		        }
		       // gconfiglocationmap[LOCATION_MASK_HOME] = locationpair(atof(latitude),atof(longitude));
		       // ALOGD("Config Map Location is updated with Home");
		        continue;
	        }


	        if (tmp.find("mall") != std::string::npos){
	        	ALOGD("DvmConfig.cpp: found mall in the MyPrefsFile");
				char * curline =const_cast<char*>(tmp.c_str());
				token[0] = strtok(curline, ":"); // first token

				if (token[0]) // zero if line is blank
				{
				  for (int n = 1; n < MAX_TOKENS_PER_LINE; n++)
				  {
				     token[n] = strtok(0, ":"); // subsequent tokens
				    // ALOGD("\t %d, %s", n, token[n] );
				     if (!token[n]) break; // no more tokens
				  }
				}

		        //get longitude and latitude
		        char* latitude = token[1];
		        char* longitude = token[2];
		        //char* radius = token[3]; // not used now but should be there
		        //ALOGD("longitude = %s and latitude = %s ",longitude, latitude );
		       // ALOGD("Size of the location map: %d",gconfiglocationmap.size() );

		        if(gconfiglocationmap.size() > 0){
		        	gconfiglocationmap[LOCATION_MASK_MALL] =  locationpair(atof(latitude),atof(longitude));
		        	ALOGD("Config Map Location is updated with MALL");
		        }

		        continue;
	        }
	}

	/* flush the file */
	//ToDO: Flush the file so that the values are not piled up and the loop is small
	// It is OK now due to the fact that the loop is till the end of the file then the last value is the one kept in the map

	//Debug the location map
	//dvmConfigLocationMapDebug(dvmConfigFileGetMapLocationAddress());
}


static void*locationUpdateCatcherThreadStart(void* arg)
{
    Thread* self = dvmThreadSelf();

    UNUSED_PARAMETER(arg);

    while (true)  { //maybe have to put break somewhere
        dvmChangeStatus(self, THREAD_VMWAIT);

        dvmChangeStatus(self, THREAD_RUNNING);

    	// The thread function
        locationUpdate();

    	//Check the file every one second
    	dvmThreadSleep(1000, 0);

    }

    return NULL;
}







//not used
//char* getSystemProperty( const char* propFile, const char* propName )
//{
//    FILE*  file;
//    char   temp[PATH_MAX], *p=temp, *end=p+sizeof(temp);
//    int    propNameLen = strlen(propName);
//    char*  result = NULL;
//
//    file = fopen(propFile, "rb");
//    if (file == NULL) {
//        ALOGD("Could not open file: %s: %s", propFile, strerror(errno));
//        return NULL;
//    }
//
//    while (fgets(temp, sizeof temp, file) != NULL) {
//        /* Trim trailing newlines, if any */
//        p = (char*)memchr(temp, '\0', sizeof temp); // I made the caset char*
//        if (p == NULL)
//            p = end;
//        if (p > temp && p[-1] == '\n') {
//            *--p = '\0';
//        }
//        if (p > temp && p[-1] == '\r') {
//            *--p = '\0';
//        }
//        /* force zero-termination in case of full-buffer */
//        if (p == end)
//            *--p = '\0';
//
//        /* check that the line starts with the property name */
//        if (memcmp(temp, propName, propNameLen) != 0) {
//            continue;
//        }
//        p = temp + propNameLen;
//
//        /* followed by an equal sign */
//        if (p >= end || *p != '=')
//            continue;
//        p++;
//
//        /* followed by something */
//        if (p >= end || !*p)
//            break;
//
//        result = android_strdup(p);
//        break;
//    }
//    fclose(file);
//    return result;
//}

