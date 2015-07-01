/*
 * DvmSystemSwitchDecisionTree.cpp
 *
 *  Created on: March 19, 2014
 *      Author: salma
 */

#include "DvmSystemSwitchDecisionTree.h"

#include <binder/IServiceManager.h>
#include <utils/TextOutput.h>
#include <utils/Vector.h>
#include <binder/IBinder.h>
#include <binder/PermissionCache.h>
using namespace android;


/******  Globals ******/
u4 TIME_STAMP = 0;
DvmLUTMap gDvmLUTMap;

// just to test if permission can be checked during the decision
// just needs to put them attached to the thread of the class not the thread of the interpreter
// the right permissions will not be captured in this place because the UID is not right in this place
static bool check_permissions(){

	if(checkCallingPermission(String16("android.permission.DUMP"))){
		    ALOGD("Permission Dump is granted");

	}
	else{
		ALOGD("Permission Dump is not granted");

	}

	if(checkCallingPermission(String16("android.permission.WIFI_STATE"))){
		    ALOGD("Permission wifi state is granted");

	}
	else{
		ALOGD("Permission wifi state is not granted");

	}



	return true;
	//android.permission.ACCESS_WIFI_STATE
	//android.permission.WRITE_EXTERNAL_STORAGE
	//android.permission.ACCESS_FINE_LOCATION
	//android.hardware.sensor.accelerometer
}

/*********************** The Decision Tree ***************************** */
/* The switch policy method */
/* State Machine for the decision tree to decide which method to execute */
// take the operating point and return the key to the method <classID, methodID>
u4 dvmSystemReturnMethod(DvmDex* pDvmDex, string className, key curClassMethodIds){

	key ClassMethodIds;
	string methodName ="";
	DvmConfigClass* curClass = new DvmConfigClass; //&(pDvmDex->pConfigFile->sensitiveClasses.at(1)); // just initialization
	vector<unsigned int> powerindex;
	vector<unsigned int> tempindex;
	vector<unsigned int> voltindex;
	vector<DvmConfigMethodMapIter> powerIter;
	vector<DvmConfigMethodMapIter> tempIter;
	vector<DvmConfigMethodMapIter> voltIter;
	vector<DvmConfigMethodMapIter> wifiIter;
	vector<DvmConfigMethodMapIter> tagIter;

	OperatingPoint* opt = dvmGetCurOperatingPoint(); //gCurOperatingpoint is defined in "DvmSystemCoreValues.h"

	if(check_permissions() == true) ALOGD("We checked permission");



	/*default initialization*/
	//int preference = POWER_PREFERENCE;
	e_preference_t preference = power_preference;
	e_policy_t curpolicy = bestfit;
	int wifi_connect = 0;
	unsigned short current_activity = ACTIVITY_MASK_STILL | ACTIVITY_MASK_WALK | ACTIVITY_MASK_RUN;
	unsigned short current_location = LOCATION_MASK_HOME | LOCATION_MASK_WORK | LOCATION_MASK_MALL | LOCATION_MASK_ANYWHERE;
	//flags
	//bool Check_Temp_All = false;
	bool Check_Volt_All = false;


	/****************************** Check the Cache ***************************************** */

	if(dvmSystemCoreValuesCheckLUT(curClassMethodIds, *opt, ClassMethodIds.second)) {
		return ClassMethodIds.second;
	}
	/*************************************************************************** */


	if(pDvmDex==NULL){
		ALOGE("DvmDex = Null in dvmSystemReturnMethod");
	}
	if(!pDvmDex->isConfigFile){
		ALOGE("DvmDex = Null in dvmSystemReturnMethod");
	}



	/**************************** Get Class Preference Handler and Connectivity and Policy  ************************ */
	for(int i=0; i< pDvmDex->pConfigFile->numSensitiveClasses; i++){
		if(strcmp(pDvmDex->pConfigFile->sensitiveClasses.at(i).className.c_str(), className.c_str()) == 0){
			curClass = &(pDvmDex->pConfigFile->sensitiveClasses.at(i));
			preference = curClass->preference;
			curpolicy = curClass->policy;
		}
	}
	wifi_connect = opt->connect.wifi_state;
	current_activity = opt->curActivity;
	current_location = opt->curLocation.locationMask;
	/**************************************************************************************** */


	/**************************************************************************************** */
	/*			 						Best Fit Policy 									  */
	/**************************************************************************************** */
	if(curpolicy == bestfit){
		ALOGD("Best Fit");
		/******************************* Handle Power Preference first ************************** */
		if(preference == power_preference){
			ALOGD("Power Preference");
			//ALOGD("Power range size %d\n", curClass->powRange.size());
			for (u4 p = 0; p < curClass->powRange.size(); p++){
				// small rangeof power <= operating power point <= large rangeofPower
				//ALOGD("Current power range: %d, %d\n",curClass->powRange.at(p).first, curClass->powRange.at(p).second);
				if(opt->batt.batteryCapacity >= curClass->powRange.at(p).first && opt->batt.batteryCapacity <= curClass->powRange.at(p).second ){
					//ALOGD("operating point lies in the ranges of power at index %d\n",p);
					powerindex.push_back(p);
				}
			}

			if(powerindex.size()==0){
				//raise flag to check all temperature ranges
				//Check_Temp_All =true;
				//ALOGD("power index size =0 all ranges with u\n");
				//meaning all ranges should be checked again so add all the indices in the powerindex
				for(u4 p=0; p < curClass->powRange.size(); p++)
					powerindex.push_back(p);
			}
			//go to second level only if there were more than 1 index from power
			//if power index size == 1 then you get the method and return it.
			if(powerindex.size()==1){
				//for(int m=0; m<pDvmConfigFile->sensitiveClasses.at(i).numSensitiveMethods; m++){
				 for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end(); ++it){
					//if (powerindex.at(0) == pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodparm.ids.first){

					 if (powerindex.at(0) == it->second.ids.first){
						 //methodName = pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodName;
						 ClassMethodIds = it->first;
						// ALOGD("one power index method id = %d", ClassMethodIds.second);
						break;
					}
					else {
					 ALOGD("sth went wrong in power there must be a method");
					}
				}
			}
			else{//more than one power range
				// check the second level which is the temperature only with the power index
				//get temperature operating point index
				//1- get the index(iterator) in the map with these power indices to be only accessed later
				//if(Check_Temp_All==false){ //only if power index was found before
					for(u4 p=0; p<powerindex.size();p++){
						for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
							if(powerindex.at(p) == it->second.ids.first){
								//ALOGD("find a match for power index\t");
								powerIter.push_back(it);
								//printf("size of poweriter = %d\n", powerIter.size());
								break;//found it, don't continue iterate in the map for this power index
							}
						}
					}
				//}
				// 2- Get temperature index
				for(u4 t=0; t< curClass->tempRange.size(); t++){
					if(opt->batt.batteryTemperature >= curClass->tempRange.at(t).first && opt->batt.batteryTemperature <= curClass->tempRange.at(t).second){
						tempindex.push_back(t);
					//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
					}
				}
				if(tempindex.size()==0){
					//meaning nothing map to temperature so check the voltage with the power iteration indices instead
					Check_Volt_All = true;
				}
				if(tempindex.size()==1){
					//3-Access the map with the power iterators to check this temp and return the method,
					for(u4 piter =0; piter < powerIter.size(); piter++ ){
						if(powerIter.at(piter)->second.ids.second == tempindex.at(0)){
							ClassMethodIds = powerIter.at(piter)->first;
							break; // method name found break;
						}
						else if(piter == powerIter.size()-1){
							//reach the end with no temperature matching then go and check all voltage
							Check_Volt_All = true;
						}
					}
				}
				else if(tempindex.size() > 1){
					//more than one in temp range
					//put in the temperature iterator only the ranges that lie under the tree of power iterators
					for(u4 piter=0; piter < powerIter.size(); piter++){
						for(u4 tindex =0; tindex<tempindex.size();tindex++){
							if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
								tempIter.push_back(powerIter.at(piter));
								//ALOGD("find a match for temp index\t");
								//printf("size of tempiter = %d\n", tempIter.size());
							}
							}
							if(piter == powerIter.size()-1 && tempIter.size()==0){
								//no matching
								Check_Volt_All= true;
							}
					}
				}

				if(Check_Volt_All || tempIter.size()>0){
					if(tempIter.size()==1){
						//return the method here
						ClassMethodIds = tempIter.at(0)->first;
						//break;
					}
					else{
						//3- get the voltage indices
						for(u4 v=0; v< curClass->voltRange.size(); v++){
							if(opt->batt.batteryVoltage >= curClass->voltRange.at(v).first && opt->batt.batteryVoltage <= curClass->voltRange.at(v).second){
								voltindex.push_back(v);
								//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
							}
						}

						if(voltindex.size()==0){ // no volt match
							if(Check_Volt_All){ // no temp match
								//return the highest priority of power iterators with opt connectivity required
								u4 priority =10000;
								bool tag_method_found_with_conn = false;
								bool tag_method_found_with_activity = false;
								bool tag_method_found_with_location = false;
								key candidateMethod;
								for(u4 piter =0; piter < powerIter.size(); piter++){
									if ((powerIter.at(piter)->second.priority < priority)){
										priority = powerIter.at(piter)->second.priority;
										candidateMethod = powerIter.at(piter)->first;
										if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
											ClassMethodIds = powerIter.at(piter)->first;
											tag_method_found_with_conn = true;
										}
										// wifi and activity are checked independently
										if ((current_activity & powerIter.at(piter)->second.activitymask) != 0){
											ClassMethodIds =  powerIter.at(piter)->first;
											tag_method_found_with_activity = true;
										}

										// wifi and activity and location are checked independently
										if ((current_location & powerIter.at(piter)->second.locationmask) != 0){
											ClassMethodIds =  powerIter.at(piter)->first;
											tag_method_found_with_location = true;
										}
									}
								}
								if(tag_method_found_with_conn == false && tag_method_found_with_activity == false && tag_method_found_with_location == false)  // no match method with required conn is found
									ClassMethodIds = candidateMethod; 	//take the highest priority in power regardless the conn;
							}
							else {
								// return highest priority of temp iterators with the opt connectivity required
								u4 priority =10000; //any big number
								bool tag_method_found_with_conn = false;
								bool tag_method_found_with_activity = false;
								bool tag_method_found_with_location = false;
								key candidateMethod;
								for (u4 titer =0; titer < tempIter.size();titer++){
									if(tempIter.at(titer)->second.priority < priority){
										priority = tempIter.at(titer)->second.priority;
										candidateMethod = tempIter.at(titer)->first;
										if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
											ClassMethodIds = tempIter.at(titer)->first;
											tag_method_found_with_conn = true;
										}
										// wifi and activity are checked independently
										if ((current_activity & tempIter.at(titer)->second.activitymask) != 0){
											ClassMethodIds =  tempIter.at(titer)->first;
											tag_method_found_with_activity = true;
										}
										//TODO: comment this for now wifi and activity and location are checked independently
										if ((current_location & tempIter.at(titer)->second.locationmask) != 0){
											ClassMethodIds =  tempIter.at(titer)->first;
											tag_method_found_with_location = true;
										}
									}
								}
								if(tag_method_found_with_conn == false && tag_method_found_with_activity == false && tag_method_found_with_location == false) // no match method with required conn is found
									ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
							}
						}

						if(voltindex.size() == 1){
							if(Check_Volt_All){
								for(u4 piter =0; piter < powerIter.size(); piter++){
									if(powerIter.at(piter)->second.ids.third == voltindex.at(0)){
										voltIter.push_back(powerIter.at(piter));
									}
									else if(piter == powerIter.size()-1 && voltIter.size() == 0 ){
										//reach the end of power iteration with no matching to this volt index
										//return method of highest priority from power iteration
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										bool tag_method_found_with_activity = false;
										bool tag_method_found_with_location = false;
										key candidateMethod;
										for(u4 piter =0; piter < powerIter.size(); piter++){
											if (powerIter.at(piter)->second.priority < priority){
												priority = powerIter.at(piter)->second.priority;
												candidateMethod = powerIter.at(piter)->first;
												if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
													ClassMethodIds = powerIter.at(piter)->first;
													tag_method_found_with_conn = true;
												}
												// wifi and activity are checked independently
												if ((current_activity & powerIter.at(piter)->second.activitymask) != 0){
													ClassMethodIds =  powerIter.at(piter)->first;
													tag_method_found_with_activity = true;
												}
												//TODO:comment this for now wifi and activity and location are checked independently
												if ((current_location & powerIter.at(piter)->second.locationmask) != 0){
													ClassMethodIds =  powerIter.at(piter)->first;
													tag_method_found_with_location = true;
												}
											}
										}
										if(tag_method_found_with_conn == false && tag_method_found_with_activity == false && tag_method_found_with_location == false ) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
							else{
								//check if this volt index lie under the temp iterator tree
								for(u4 titer=0; titer < tempIter.size(); titer++){
									if(tempIter.at(titer)->second.ids.third == voltindex.at(0)){
										voltIter.push_back(tempIter.at(titer));
									}
									if(titer == powerIter.size()-1 && voltIter.size()==0){
										//no matching
										//return the highest priority of the temp iterartion
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										bool tag_method_found_with_activity = false;
										bool tag_method_found_with_location = false;
										key candidateMethod;
										for(u4 titer =0; titer < tempIter.size(); titer++){
											if (tempIter.at(titer)->second.priority < priority){
												priority = tempIter.at(titer)->second.priority;
												candidateMethod = tempIter.at(titer)->first;
												if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
													ClassMethodIds = tempIter.at(titer)->first;
													tag_method_found_with_conn = true;
												}
												// wifi and activity are checked independently
												if ((current_activity & tempIter.at(titer)->second.activitymask) != 0){
													ClassMethodIds =  tempIter.at(titer)->first;
													tag_method_found_with_activity = true;
												}
												//TODO: comment this for now wifi and activity and location are checked independently
												if ((current_location & tempIter.at(titer)->second.locationmask) != 0){
													ClassMethodIds =  tempIter.at(titer)->first;
													tag_method_found_with_location = true;
												}
											}
										}
										if(tag_method_found_with_conn == false && tag_method_found_with_activity == false && tag_method_found_with_location == false) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
						}
						else if(voltindex.size()>1){
							if(Check_Volt_All){
								//check only those lie under the power tree iterator
								for(u4 piter=0; piter < powerIter.size(); piter++){
									for(u4 vindex =0; vindex<voltindex.size();vindex++){
										if(powerIter.at(piter)->second.ids.third == voltindex.at(vindex)){
											voltIter.push_back(powerIter.at(piter));
										}
									}
									if(piter == powerIter.size()-1 && voltIter.size()==0){
										//no matching // return highest priority of power iter
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										bool tag_method_found_with_activity = false;
										bool tag_method_found_with_location = false;
										key candidateMethod;
										for(u4 piter =0; piter < powerIter.size(); piter++){
											if (powerIter.at(piter)->second.priority < priority){
												priority = powerIter.at(piter)->second.priority;
												candidateMethod = powerIter.at(piter)->first;
												if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
													ClassMethodIds = powerIter.at(piter)->first;
													tag_method_found_with_conn = true;
												}
												// wifi and activity are checked independently
												if ((current_activity & powerIter.at(piter)->second.activitymask) != 0){
													ClassMethodIds =  powerIter.at(piter)->first;
													tag_method_found_with_activity = true;
												}
												//TODO: comment this for now wifi and activity and location are checked independently
												if ((current_location & powerIter.at(piter)->second.locationmask) != 0){
													ClassMethodIds =  powerIter.at(piter)->first;
													tag_method_found_with_location = true;
												}
											}
										}
										if(tag_method_found_with_conn == false && tag_method_found_with_activity == false && tag_method_found_with_location == false) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
							//check only the volt index lie under the temperature tree iterator
							else {
								for(u4 titer=0; titer < tempIter.size(); titer++){
									for(u4 vindex =0; vindex<voltindex.size();vindex++){
										if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
											voltIter.push_back(tempIter.at(titer));
										}
									}
									if(titer == tempIter.size()-1 && voltIter.size()==0){
										//no matching // return highest priority of temp iteration
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										bool tag_method_found_with_activity = false;
										bool tag_method_found_with_location = false;
										key candidateMethod;
										for(u4 titer =0; titer < tempIter.size(); titer++){
											if (tempIter.at(titer)->second.priority < priority){
												priority = tempIter.at(titer)->second.priority;
												candidateMethod = tempIter.at(titer)->first;
												if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
													ClassMethodIds = tempIter.at(titer)->first;
													tag_method_found_with_conn = true;
												}
												// wifi and activity are checked independently
												if ((current_activity & tempIter.at(titer)->second.activitymask) != 0){
													ClassMethodIds =  tempIter.at(titer)->first;
													tag_method_found_with_activity = true;
												}
												//TODO: comment this for now wifi and activity and location are checked independently
												if ((current_location & tempIter.at(titer)->second.locationmask) != 0){
													ClassMethodIds =  tempIter.at(titer)->first;
													tag_method_found_with_location = true;
												}
											}
										}
										if(tag_method_found_with_conn == false && tag_method_found_with_activity == false && tag_method_found_with_location == false) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
						}

						// At this point we have either the voltIter size = 1 or > 1 but can't be zero
						if(voltIter.size() == 1){
							// return this method
							ClassMethodIds = voltIter.at(0)->first; // We don't need to check the connectivity here
							//break;
						}
						else if(voltIter.size() > 1){
							//get the highest priority of the volt iter
							u4 priority =10000;
							bool tag_method_found_with_conn = false;
							bool tag_method_found_with_activity = false;
							bool tag_method_found_with_location = false;
							key candidateMethod;
							for(u4 viter =0; viter < voltIter.size(); viter++){
								if (voltIter.at(viter)->second.priority < priority){
									priority = voltIter.at(viter)->second.priority;
									candidateMethod = voltIter.at(viter)->first;
									// handle wifi connectivity
									if(wifi_connect == voltIter.at(viter)->second.conn.wifi_state || voltIter.at(viter)->second.conn.wifi_state == 2 /*don't care*/){
										ClassMethodIds = voltIter.at(viter)->first;
										tag_method_found_with_conn = true;
									}
									// wifi and activity are checked independently
									if ((current_activity & voltIter.at(viter)->second.activitymask) != 0){
										ClassMethodIds =  voltIter.at(viter)->first;
										tag_method_found_with_activity = true;
									}
									//TODO: comment this for now wifi and activity and location are checked independently
									if ((current_location & voltIter.at(viter)->second.locationmask) != 0){
										ClassMethodIds =  voltIter.at(viter)->first;
										tag_method_found_with_location = true;
									}
								}
							}
							if(tag_method_found_with_conn == false && tag_method_found_with_activity == false && tag_method_found_with_location == false) // no match method with required conn is found
								ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
						}
					}
				}
			}
			//ALOGD("Return MethodID = %d", ClassMethodIds.second);
		} /*POWER Preference tree */


		/* ****************************** Handle Connectivity Preference First ******************* */
		else if (preference == connectivity_preference){
			/*	Get the indices for the wifi connectivity first */
			//initialize the Check
			Check_Volt_All = false;
			ALOGD("Connectivity Preference");
			for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
				if(it->second.conn.wifi_state == wifi_connect || it->second.conn.wifi_state == 2 /*don't care*/){
					wifiIter.push_back(it);
				//	ALOGD("wifi it pushed back %d", it->first.second );
				}
			}

			if(wifiIter.size() == 0) {
				//push back in the wifi iter everything
				for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
					wifiIter.push_back(it);
				}
				//ALOGD("wifi iter was all zero");
			}

			if (wifiIter.size() == 1){
				 ClassMethodIds = wifiIter.at(0)->first; //stop here
			//	 ALOGD("found only one wifi that matches %d", wifiIter.at(0)->first.second);
			}
			else{
				for (u4 p = 0; p < curClass->powRange.size(); p++){
					// small rangeof power <= operating power point <= large rangeofPower
					//printf("Current power range: %d, %d\n",pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).first, pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).second);
					if(opt->batt.batteryCapacity >= curClass->powRange.at(p).first && opt->batt.batteryCapacity <= curClass->powRange.at(p).second ){
					//	ALOGD("operating point lies in the ranges of power at index %d\n",p);
						powerindex.push_back(p);
					}
				}

				if(powerindex.size()==0){
					//raise flag to check all temperature ranges
					//Check_Temp_All =true;
					//ALOGD("power index size =0 all ranges with you\n");
					//meaning all ranges should be checked again so add all the indices in the powerindex
					for(u4 p=0; p < curClass->powRange.size(); p++)
						powerindex.push_back(p);
				}
				//go to second level only if there were more than 1 index from power
				//if power index size == 1 then you get the method and return it.
				if(powerindex.size()==1){
					//for(int m=0; m<pDvmConfigFile->sensitiveClasses.at(i).numSensitiveMethods; m++){
					 bool tag_method_found = false;
					 for(u4 witer =0; witer < wifiIter.size(); witer++){
						 if (powerindex.at(0) == wifiIter.at(witer)->second.ids.first){
							 //methodName = pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodName;
							 ClassMethodIds = wifiIter.at(witer)->first;
							 tag_method_found = true;
						//	 ALOGD("power index just one and found the wifi iter");
							 break;
						}

					 }
					if(tag_method_found == false) {
						//Power index found does not lie in the list of wifi iterations.
						//return the highest priority method for the wifi that matches activity
						u4 priority =10000;
						bool tag_method_found_with_activity = false;
						bool tag_method_found_with_location = false;
						key candidateMethod;
						for(u4 witer =0; witer < wifiIter.size(); witer++){
							if (wifiIter.at(witer)->second.priority < priority){
								priority = wifiIter.at(witer)->second.priority;
								candidateMethod = wifiIter.at(witer)->first;
								if ((current_activity & wifiIter.at(witer)->second.activitymask) != 0){
									tag_method_found_with_activity = true;
									ClassMethodIds = wifiIter.at(witer)->first;
								}
								// activity and location are checked independently
								if ((current_location & wifiIter.at(witer)->second.activitymask) != 0){
									tag_method_found_with_location = true;
									ClassMethodIds = wifiIter.at(witer)->first;
								}
							}
						}

						if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
							ClassMethodIds = candidateMethod;
						}
					}
				}
				else{//more than one power range
					// check the second level which is the temperature only with the power index
					//get temperature operating point index
					//1- get the index(iterator) in the map with these power indices to be only accessed later
					//if(Check_Temp_All==false){ //only if power index was found before
				//	ALOGD("More than one power index");
						for(u4 p=0; p<powerindex.size();p++){
							for(u4 witer =0; witer < wifiIter.size(); witer++){
								if(powerindex.at(p) == wifiIter.at(witer)->second.ids.first){
								//	ALOGD("find a match for power index\t");
									powerIter.push_back(wifiIter.at(witer));
								//	printf("size of poweriter = %d\n", powerIter.size());
									break;//found it, don't continue iterate in the map for this power index
								}
							}
						}
					//}
					// 2- Get temperature index
					for(u4 t=0; t< curClass->tempRange.size(); t++){
						if(opt->batt.batteryTemperature >= curClass->tempRange.at(t).first && opt->batt.batteryTemperature <= curClass->tempRange.at(t).second){
							tempindex.push_back(t);
						//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
						}
					}
					if(tempindex.size()==0){
						//meaning nothing map to temperature so check the voltage with the power iteration indices instead
						Check_Volt_All = true;
					}
					if(tempindex.size()==1){
						//3-Access the map with the power iterators to check this temp and return the method,
						for(u4 piter =0; piter < powerIter.size(); piter++ ){
							if(powerIter.at(piter)->second.ids.second == tempindex.at(0)){
								ClassMethodIds = powerIter.at(piter)->first;
								break; // method name found break;
							}
							else if(piter == powerIter.size()-1){
								//reach the end with no temperature matching then go and check all voltage
								Check_Volt_All = true;
							}
						}
					}
					else if(tempindex.size() > 1){
						//more than one in temp range
						//put in the temperature iterator only the ranges that lie under the tree of power iterators
						for(u4 piter=0; piter < powerIter.size(); piter++){
							for(u4 tindex =0; tindex<tempindex.size();tindex++){
								if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
									tempIter.push_back(powerIter.at(piter));
								//	ALOGD("find a match for temp index\t");
									//printf("size of tempiter = %d\n", tempIter.size());
								}
							}
							if(piter == powerIter.size()-1 && tempIter.size()==0){
								//no matching
								Check_Volt_All= true;
							}
						}
					}

					if(Check_Volt_All || tempIter.size()>0){
						if(tempIter.size()==1){
							//return the method here
							ClassMethodIds = tempIter.at(0)->first;
							//break;
						}
						else{
							//3- get the voltage indices
							for(u4 v=0; v< curClass->voltRange.size(); v++){
								if(opt->batt.batteryVoltage >= curClass->voltRange.at(v).first && opt->batt.batteryVoltage <= curClass->voltRange.at(v).second){
									voltindex.push_back(v);
									//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
								}
							}

							if(voltindex.size()==0){ // no volt match
								if(Check_Volt_All){ // no temp match
									//return the highest priority of power iterators with opt connectivity required that matches activity
									u4 priority =10000;
									bool tag_method_found_with_activity = false;
									bool tag_method_found_with_location = false;
									key candidateMethod;
									for(u4 piter =0; piter < powerIter.size(); piter++){
										if ((powerIter.at(piter)->second.priority < priority)){
											priority = powerIter.at(piter)->second.priority;
											candidateMethod = powerIter.at(piter)->first;
//salma removed a brackets here											}
											if ((current_activity & powerIter.at(piter)->second.activitymask) != 0){
												tag_method_found_with_activity = true;
												ClassMethodIds = powerIter.at(piter)->first;
											}
											// activity and location are checked independently
											if ((current_location & powerIter.at(piter)->second.locationmask) != 0){
												tag_method_found_with_location = true;
												ClassMethodIds = powerIter.at(piter)->first;
											}
										}
									}
									if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
										ClassMethodIds = candidateMethod;
									}

								}
								else {
									// return highest priority of temp iterators with the opt connectivity required
									u4 priority =10000; //any big number
									bool tag_method_found_with_activity = false;
									bool tag_method_found_with_location = false;
									key candidateMethod;
									for (u4 titer =0; titer < tempIter.size();titer++){
										if(tempIter.at(titer)->second.priority < priority){
											priority = tempIter.at(titer)->second.priority;
											candidateMethod = tempIter.at(titer)->first;
											if ((current_activity & tempIter.at(titer)->second.activitymask) != 0){
												tag_method_found_with_activity = true;
												ClassMethodIds = tempIter.at(titer)->first;
											}
											if ((current_location & tempIter.at(titer)->second.locationmask) != 0){
												tag_method_found_with_location = true;
												ClassMethodIds = tempIter.at(titer)->first;
											}
										}
									}
									if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
										ClassMethodIds = candidateMethod;
									}
								}
							}

							if(voltindex.size() == 1){
								if(Check_Volt_All){
									for(u4 piter =0; piter < powerIter.size(); piter++){
										if(powerIter.at(piter)->second.ids.third == voltindex.at(0)){
											voltIter.push_back(powerIter.at(piter));
										}
										else if(piter == powerIter.size()-1 && voltIter.size() == 0 ){
											//reach the end of power iteration with no matching to this volt index
											//return method of highest priority from power iteration
											u4 priority =10000;
											bool tag_method_found_with_activity = false;
											bool tag_method_found_with_location = false;

											key candidateMethod;
											for(u4 piter =0; piter < powerIter.size(); piter++){
												if (powerIter.at(piter)->second.priority < priority){
													priority = powerIter.at(piter)->second.priority;
													candidateMethod = powerIter.at(piter)->first;
													if ((current_activity & powerIter.at(piter)->second.activitymask) != 0){
														tag_method_found_with_activity = true;
														ClassMethodIds = powerIter.at(piter)->first;
													}
													if ((current_location & powerIter.at(piter)->second.locationmask) != 0){
														tag_method_found_with_location = true;
														ClassMethodIds = powerIter.at(piter)->first;
													}
												}
											}
											if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
												ClassMethodIds = candidateMethod;
											}
										}
									}
								}
								else{
									//check if this volt index lie under the temp iterator tree
									for(u4 titer=0; titer < tempIter.size(); titer++){
										if(tempIter.at(titer)->second.ids.third == voltindex.at(0)){
											voltIter.push_back(tempIter.at(titer));
										}
										if(titer == powerIter.size()-1 && voltIter.size()==0){
											//no matching
											//return the highest priority of the temp iterartion
											u4 priority =10000;
											bool tag_method_found_with_activity = false;
											bool tag_method_found_with_location = false;
											key candidateMethod;
											for(u4 titer =0; titer < tempIter.size(); titer++){
												if (tempIter.at(titer)->second.priority < priority){
													priority = tempIter.at(titer)->second.priority;
													candidateMethod = tempIter.at(titer)->first;
													if ((current_activity & tempIter.at(titer)->second.activitymask) != 0){
														tag_method_found_with_activity = true;
														ClassMethodIds = tempIter.at(titer)->first;
													}
													// location and activity are checked independently
													if ((current_location & tempIter.at(titer)->second.locationmask) != 0){
														tag_method_found_with_location = true;
														ClassMethodIds = tempIter.at(titer)->first;
													}
												}
											}
											if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
												ClassMethodIds = candidateMethod;
											}
										}
									}
								}
							}
							else if(voltindex.size()>1){
								if(Check_Volt_All){
									//check only those lie under the power tree iterator
									for(u4 piter=0; piter < powerIter.size(); piter++){
										for(u4 vindex =0; vindex<voltindex.size();vindex++){
											if(powerIter.at(piter)->second.ids.third == voltindex.at(vindex)){
												voltIter.push_back(powerIter.at(piter));
											}
										}
										if(piter == powerIter.size()-1 && voltIter.size()==0){
											//no matching // return highest priority of power iter
											u4 priority =10000;
											bool tag_method_found_with_activity = false;
											bool tag_method_found_with_location = false;
											key candidateMethod;
											for(u4 piter =0; piter < powerIter.size(); piter++){
												if (powerIter.at(piter)->second.priority < priority){
													priority = powerIter.at(piter)->second.priority;
													candidateMethod = powerIter.at(piter)->first;
													if ((current_activity & powerIter.at(piter)->second.activitymask) != 0){
														tag_method_found_with_activity = true;
														ClassMethodIds = powerIter.at(piter)->first;
													}
													// activity and location are checked independently
													if ((current_location & powerIter.at(piter)->second.locationmask) != 0){
														tag_method_found_with_location = true;
														ClassMethodIds = powerIter.at(piter)->first;
													}
												}
											}
											if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
												ClassMethodIds = candidateMethod;
											}
										}
									}
								}
								//check only the volt index lie under the temperature tree iterator
								else {
									for(u4 titer=0; titer < tempIter.size(); titer++){
										for(u4 vindex =0; vindex<voltindex.size();vindex++){
											if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
												voltIter.push_back(tempIter.at(titer));
											}
										}
										if(titer == tempIter.size()-1 && voltIter.size()==0){
											//no matching // return highest priority of temp iteration
											u4 priority =10000;
											bool tag_method_found_with_activity = false;
											bool tag_method_found_with_location = false;
											key candidateMethod;
											for(u4 titer =0; titer < tempIter.size(); titer++){
												if (tempIter.at(titer)->second.priority < priority){
													priority = tempIter.at(titer)->second.priority;
													candidateMethod = tempIter.at(titer)->first;
													if ((current_activity & tempIter.at(titer)->second.activitymask) != 0){
														tag_method_found_with_activity = true;
														ClassMethodIds = tempIter.at(titer)->first;
													}
													// activity and location are checked independently
													if ((current_location & tempIter.at(titer)->second.locationmask) != 0){
														tag_method_found_with_location = true;
														ClassMethodIds = tempIter.at(titer)->first;
													}
												}
											}
											if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
												ClassMethodIds = candidateMethod;
											}
										}
									}
								}
							}

							// At this point we have either the voltIter size = 1 or > 1 but can't be zero
							if(voltIter.size() == 1){
								// return this method
								ClassMethodIds = voltIter.at(0)->first; // We don't need to check the connectivity here
								//break;
							}
							else if(voltIter.size() > 1){
								//get the highest priority of the volt iter
								u4 priority =10000;
								bool tag_method_found_with_activity = false;
								bool tag_method_found_with_location = false;
								key candidateMethod;
								for(u4 viter =0; viter < voltIter.size(); viter++){
									if (voltIter.at(viter)->second.priority < priority){
										priority = voltIter.at(viter)->second.priority;
										candidateMethod = voltIter.at(viter)->first;
										if ((current_activity & voltIter.at(viter)->second.activitymask) != 0){
											tag_method_found_with_activity = true;
											ClassMethodIds = voltIter.at(viter)->first;
										}
										// location and activity are checked independently
										if ((current_location & voltIter.at(viter)->second.locationmask) != 0){
											tag_method_found_with_location = true;
											ClassMethodIds = voltIter.at(viter)->first;
										}
									}
								}
								if(tag_method_found_with_activity == false && tag_method_found_with_location == false){
									ClassMethodIds = candidateMethod;
								}
							}
						}
					}
				}
			}
		}/*Connectivity Preference tree */

	} /* Most fit Policy */


	/**************************************************************************************** */
	/*			 						Must Fit Policy 									  */
	/**************************************************************************************** */
	else if(curpolicy == mustfit){
		ALOGD("must Fit");
		/******************************* Handle Power Preference first ************************** */
		if(preference == power_preference){

			ALOGD("Power Preference");
			for (u4 p = 0; p < curClass->powRange.size(); p++){
				// small rangeof power <= operating power point <= large rangeofPower
				//printf("Current power range: %d, %d\n",pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).first, pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).second);
				if(opt->batt.batteryCapacity >= curClass->powRange.at(p).first && opt->batt.batteryCapacity <= curClass->powRange.at(p).second ){
					//ALOGD("operating point lies in the ranges of power at index %d\n",p);
					powerindex.push_back(p);
				}
			}

			if(powerindex.size()==0){
				// return the default // the one that has been called with
				ClassMethodIds = curClassMethodIds;

			}
			//go to second level only if there were more than 1 index from power
			else{//more than one power range
				// check the second level which is the temperature only with the power index
				//get temperature operating point index
				//1- get the index(iterator) in the map with these power indices to be only accessed later
				//if(Check_Temp_All==false){ //only if power index was found before
					for(u4 p=0; p<powerindex.size();p++){
						for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
							if(powerindex.at(p) == it->second.ids.first){
								//ALOGD("find a match for power index\t");
								powerIter.push_back(it);
								//printf("size of poweriter = %d\n", powerIter.size());
								break;//found it, don't continue iterate in the map for this power index
							}
						}
					}
				//}
				// 2- Get temperature index
				for(u4 t=0; t< curClass->tempRange.size(); t++){
					if(opt->batt.batteryTemperature >= curClass->tempRange.at(t).first && opt->batt.batteryTemperature <= curClass->tempRange.at(t).second){
						tempindex.push_back(t);
					//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
					}
				}

				if(tempindex.size()==0){
					ClassMethodIds = curClassMethodIds;
				}
				else {
					//more than one in temp range
					//put in the temperature iterator only the ranges that lie under the tree of power iterators
					for(u4 piter=0; piter < powerIter.size(); piter++){
						for(u4 tindex =0; tindex<tempindex.size();tindex++){
							if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
								tempIter.push_back(powerIter.at(piter));
								//ALOGD("find a match for temp index\t");
								//printf("size of tempiter = %d\n", tempIter.size());
							}
						}
						if(piter == powerIter.size()-1 && tempIter.size()==0){
							//no matching //return default
							ClassMethodIds = curClassMethodIds;
						}
					}
				}

				if(tempIter.size()>0){
						//3- get the voltage indices
						for(u4 v=0; v< curClass->voltRange.size(); v++){
							if(opt->batt.batteryVoltage >= curClass->voltRange.at(v).first && opt->batt.batteryVoltage <= curClass->voltRange.at(v).second){
								voltindex.push_back(v);
								//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
							}
						}

						if(voltindex.size()==0){ // no volt match
							ClassMethodIds = curClassMethodIds;
						}
						else {
							//check only the volt index lie under the temperature tree iterator
							for(u4 titer=0; titer < tempIter.size(); titer++){
								for(u4 vindex =0; vindex<voltindex.size();vindex++){
									if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
										voltIter.push_back(tempIter.at(titer));
									}
								}
								if(titer == tempIter.size()-1 && voltIter.size()==0){
									//no match
									ClassMethodIds =  curClassMethodIds;
								}
							}
						}

						// At this point we have either the voltIter size = 1 or > 1 but can't be zero
						if(voltIter.size() == 1){
							//  check the connectivity here
							if(wifi_connect == voltIter.at(0)->second.conn.wifi_state || voltIter.at(0)->second.conn.wifi_state == 2 /*don't care*/)
								ClassMethodIds = voltIter.at(0)->first;
							else //no match for connectivity return default
								ClassMethodIds =  curClassMethodIds;
							//break;
						}
						else if(voltIter.size() > 1){
							//get the highest priority of the volt iter
							// get the one that fit the connectivity
							u4 priority =10000;
							bool tag_method_found = false;
							//key candidateMethod;
							for(u4 viter =0; viter < voltIter.size(); viter++){
								if (voltIter.at(viter)->second.priority < priority){
									priority = voltIter.at(viter)->second.priority;
								//	candidateMethod = voltIter.at(viter)->first;
									//wifi and activity and location are checked together because it is must fit
									if( (wifi_connect == voltIter.at(viter)->second.conn.wifi_state || voltIter.at(viter)->second.conn.wifi_state == 2 /*don't care*/)
											&& ((current_activity & voltIter.at(viter)->second.activitymask) != 0)  && ((current_location & voltIter.at(viter)->second.locationmask) != 0)  ){
										//ClassMethodIds = voltIter.at(viter)->first;
										tag_method_found = true;
										ClassMethodIds = voltIter.at(viter)->first;

									}

								}
							}
							if(tag_method_found == false) // no match method with required conn is found
								ClassMethodIds = curClassMethodIds; 	//take the default
						}

				}
			}
			//ALOGD("Return MethodID = %d", ClassMethodIds.second);
		} /*POWER Preference tree */


		/* ****************************** Handle Connectivity Preference First ******************* */

		else if (preference == connectivity_preference){
			/*	Get the indices for the wifi connectivity first */

			ALOGD("Connectivity Preference");
			for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
				if(it->second.conn.wifi_state == wifi_connect || it->second.conn.wifi_state == 2 /*don't care*/){
					wifiIter.push_back(it);
					//ALOGD("wifi it pushed back %d", it->first.second );
				}
			}

			if(wifiIter.size() == 0) {
				//return default
				ClassMethodIds = curClassMethodIds;
			}
			else{
				for (u4 p = 0; p < curClass->powRange.size(); p++){
					if(opt->batt.batteryCapacity >= curClass->powRange.at(p).first && opt->batt.batteryCapacity <= curClass->powRange.at(p).second ){
						//ALOGD("operating point lies in the ranges of power at index %d\n",p);
						powerindex.push_back(p);
					}
				}

				if(powerindex.size()==0){
					// return default
					ClassMethodIds = curClassMethodIds;
				}
				else{//more than one power range
					for(u4 p=0; p<powerindex.size();p++){
						for(u4 witer =0; witer < wifiIter.size(); witer++){
							if(powerindex.at(p) == wifiIter.at(witer)->second.ids.first){
							//	ALOGD("find a match for power index\t");
								powerIter.push_back(wifiIter.at(witer));
							}
							if(witer == wifiIter.size()-1 && powerIter.size()==0){
								//no matching //return default
								ClassMethodIds = curClassMethodIds;
							}
						}
					}
					if(powerIter.size() > 0){
						// 2- Get temperature index
						for(u4 t=0; t< curClass->tempRange.size(); t++){
							if(opt->batt.batteryTemperature >= curClass->tempRange.at(t).first && opt->batt.batteryTemperature <= curClass->tempRange.at(t).second){
								tempindex.push_back(t);
							//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
							}
						}
						if(tempindex.size()==0){
							//no matching //return default
							ClassMethodIds = curClassMethodIds;
						}
						else{
							//more than one in temp range
							//put in the temperature iterator only the ranges that lie under the tree of power iterators
							for(u4 piter=0; piter < powerIter.size(); piter++){
								for(u4 tindex =0; tindex<tempindex.size();tindex++){
									if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
										tempIter.push_back(powerIter.at(piter));
										//ALOGD("find a match for temp index\t");
										//printf("size of tempiter = %d\n", tempIter.size());
									}
								}
								if(piter == powerIter.size()-1 && tempIter.size()==0){
									//no matching //return default
									ClassMethodIds = curClassMethodIds;
								}
							}
						}

						if(tempIter.size()>0){
							//3- get the voltage indices
							for(u4 v=0; v< curClass->voltRange.size(); v++){
								if(opt->batt.batteryVoltage >= curClass->voltRange.at(v).first && opt->batt.batteryVoltage <= curClass->voltRange.at(v).second){
									voltindex.push_back(v);
									//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
								}
							}
							if(voltindex.size()==0){ // no volt match
								//no matching //return default
								ClassMethodIds = curClassMethodIds;
							}
							else {
								//check only the volt index lie under the temperature tree iterator
								for(u4 titer=0; titer < tempIter.size(); titer++){
									for(u4 vindex =0; vindex<voltindex.size();vindex++){
											if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
												voltIter.push_back(tempIter.at(titer));
											}
									}
									if(titer == tempIter.size()-1 && voltIter.size()==0){
										//no matching //return default
										ClassMethodIds = curClassMethodIds;
									}
								}
								// At this point we have either the voltIter size = 1 or > 1 but can't be zero
								if(voltIter.size() > 0){
									//get the highest priority of the volt iter
									u4 priority =10000;
									bool tag_method_found = false;
									//key candidateMethod;
									for(u4 viter =0; viter < voltIter.size(); viter++){
										if (voltIter.at(viter)->second.priority < priority){
											priority = voltIter.at(viter)->second.priority;
											//candidateMethod = voltIter.at(viter)->first;
											//wifi and activity and location are checked together because it is must fit
											if( (wifi_connect == voltIter.at(viter)->second.conn.wifi_state || voltIter.at(viter)->second.conn.wifi_state == 2 /*don't care*/)
													&& ((current_activity & voltIter.at(viter)->second.activitymask) != 0) && ((current_location & voltIter.at(viter)->second.locationmask) != 0)){
												//ClassMethodIds = voltIter.at(viter)->first;
												tag_method_found = true;
												ClassMethodIds = voltIter.at(viter)->first;

											}

										}
									}
									if(tag_method_found == false) // no match method with required conn is found
										ClassMethodIds = curClassMethodIds; 	//take the default

								}
							}
						}
					}
				}
			}
		}/*Connectivity Preference tree */


	} /* Best fit policy*/


	/************************************** Update the LUT ********************************* */
	OperatingPoint curopt = *opt;
	LUTkey lutk = { curClassMethodIds, curopt};
	keyTimeStamp kts = { ClassMethodIds, TIME_STAMP};
	if (gDvmLUTMap.size() != MAX_LUT_SIZE ){
		gDvmLUTMap.insert(make_pair(lutk,kts));
		//ALOGD("Insert in the LUT, @%d", TIME_STAMP);
		TIME_STAMP++;
	}
	else{ //erase and insert
		//ALOGD("Clear an element in the LUT");
		DvmLUTMapIter it;
		//find the element with the oldest time stamp
		bool eraseFlag = true; // to avoid multiple erase
		for (it = gDvmLUTMap.begin(); it != gDvmLUTMap.end(); ++it ){
			if (it->second.time_stamp == 0 && eraseFlag ){
				gDvmLUTMap.erase(it); //erase first element
				eraseFlag = false;
			}
			else
				it->second.time_stamp = it->second.time_stamp - 1;
		}
		TIME_STAMP = MAX_LUT_SIZE - 1;
		kts.time_stamp = TIME_STAMP;
		gDvmLUTMap.insert(make_pair(lutk,kts));
	}
	/**************************************************************************************** */


	return ClassMethodIds.second;
}
/* ********************************************************************* */



/**********************  Decision Tree Cache *************************** */
bool dvmSystemCoreValuesCheckLUT(key curClassMethodIds, OperatingPoint optpoint, u4& MethodIds){
	/*Before starting the decision tree check the LUT*/
	if(gDvmLUTMap.size() != 0){
		DvmLUTMapIter it;
		LUTkey lutk = {curClassMethodIds, optpoint};
		it = gDvmLUTMap.find(lutk);
		if(it != gDvmLUTMap.end()){
			MethodIds =  it->second.ClassMethod_ids.second; // it->second = keyTimeStamp --> ClasMethod_ids=the key<ClassID, newMethodID>
			//ALOGD("Method found in the LUT");
			return true;
		}
		else{
			//ALOGD("Method is not in the LUT");
			return false;
		}
	}
	else return false;
}
/*override the less than operator for the insert in LUT map*/
bool operator <(const LUTkey&lhs, const  LUTkey& rhs){
	//Compare the Class id first then the method id second
	if(lhs.ClassMethod_ids.first != rhs.ClassMethod_ids.first){
		//ALOGD("lhs.ClassMethod_ids.first != rhs.ClassMethod_ids.first");
		return lhs.ClassMethod_ids.first < rhs.ClassMethod_ids.first;
	}

	else if (lhs.ClassMethod_ids.second !=  rhs.ClassMethod_ids.second ){
		//ALOGD("lhs.ClassMethod_ids.second !=  rhs.ClassMethod_ids.second");
		return lhs.ClassMethod_ids.second <  rhs.ClassMethod_ids.second;
	}

	else if (lhs.opt.batt.batteryCapacity != rhs.opt.batt.batteryCapacity){
		//ALOGD("lhs.opt.batt->batteryCapacity != rhs.opt.batt->batteryCapacity");
		return lhs.opt.batt.batteryCapacity < rhs.opt.batt.batteryCapacity;
	}

	else if (lhs.opt.batt.batteryTemperature != rhs.opt.batt.batteryTemperature){
		//ALOGD("lhs.opt.batt->batteryTemperature != rhs.opt.batt->batteryTemperature");
		return lhs.opt.batt.batteryTemperature < rhs.opt.batt.batteryTemperature;
	}

	else if (lhs.opt.batt.batteryVoltage != rhs.opt.batt.batteryVoltage){
		//ALOGD("lhs.opt.batt->batteryVoltage != rhs.opt.batt->batteryVoltage");
		return lhs.opt.batt.batteryVoltage < rhs.opt.batt.batteryVoltage;
	}

	else if(lhs.opt.connect.wifi_state != rhs.opt.connect.wifi_state){
		//ALOGD("lhs.opt.connect->wifi_state != rhs.opt.connect->wifi_state");
		return lhs.opt.connect.wifi_state < rhs.opt.connect.wifi_state;
	}
	else{
		//ALOGD("lhs.opt.connect->wifi_state= %d, rhs.opt.connect->wifi_state= %d",  lhs.opt.connect.wifi_state, rhs.opt.connect.wifi_state);
		return lhs.opt.connect.wifi_state < rhs.opt.connect.wifi_state;
	}
}

void dvmSystemCoreValuesDebugLUT(){
	DvmLUTMapIter it;
	for (it = gDvmLUTMap.begin(); it != gDvmLUTMap.end(); ++it ){
		ALOGD("%d, %d, %d, %d, %d, %d --> %d, %d, %d", it->first.ClassMethod_ids.first,
													   it->first.ClassMethod_ids.second,
													   it->first.opt.batt.batteryCapacity,
													   it->first.opt.batt.batteryTemperature,
													   it->first.opt.batt.batteryVoltage,
													   it->first.opt.connect.wifi_state,
													   it->second.ClassMethod_ids.first,
													   it->second.ClassMethod_ids.second,
													   it->second.time_stamp);

    }
}
/* ********************************************************************* */






/*********************** The Decision Tree ***************************** */
/* The switch policy method using tag*/
//FIXME: location is still not handled here
/* State Machine for the decision tree to decide which method to execute */
// take the operating point and return the key to the method <classID, methodID>
u4 dvmSystemReturnMethod(DvmDex* pDvmDex, string className, key curClassMethodIds, bool tag){

	if (tag == false)
		return dvmSystemReturnMethod(pDvmDex,  className, curClassMethodIds);



	key ClassMethodIds;
	string methodName ="";
	DvmConfigClass* curClass = new DvmConfigClass; //&(pDvmDex->pConfigFile->sensitiveClasses.at(1)); // just initialization
	vector<unsigned int> powerindex;
	vector<unsigned int> tempindex;
	vector<unsigned int> voltindex;
	vector<DvmConfigMethodMapIter> powerIter;
	vector<DvmConfigMethodMapIter> tempIter;
	vector<DvmConfigMethodMapIter> voltIter;
	vector<DvmConfigMethodMapIter> wifiIter;
	vector<DvmConfigMethodMapIter> tagIter;

	OperatingPoint* opt = dvmGetCurOperatingPoint(); //gCurOperatingpoint is defined in "DvmSystemCoreValues.h"


	/*default initialization*/
	//int preference = POWER_PREFERENCE;
	e_preference_t preference = power_preference;
	e_policy_t curpolicy = bestfit;
	int wifi_connect = 0;
	unsigned short current_activity = ACTIVITY_MASK_STILL | ACTIVITY_MASK_WALK | ACTIVITY_MASK_RUN;


	unsigned int methodTag;
	//flags
	//bool Check_Temp_All = false;
	bool Check_Volt_All = false;


	/****************************** Check the Cache ***************************************** */
	if(dvmSystemCoreValuesCheckLUT(curClassMethodIds, *opt, ClassMethodIds.second)) return ClassMethodIds.second;
	/*************************************************************************** */

	if(pDvmDex==NULL){
		ALOGE("DvmDex = Null in dvmSystemReturnMethod");
	}
	if(!pDvmDex->isConfigFile){
		ALOGE("DvmDex = Null in dvmSystemReturnMethod");
	}



	/**************************** Get Class Preference Handler and Connectivity and Policy  ************************ */
	for(int i=0; i< pDvmDex->pConfigFile->numSensitiveClasses; i++){
		if(strcmp(pDvmDex->pConfigFile->sensitiveClasses.at(i).className.c_str(), className.c_str()) == 0){
			curClass = &(pDvmDex->pConfigFile->sensitiveClasses.at(i));
			preference = curClass->preference;
			curpolicy = curClass->policy;
		}
	}
	wifi_connect = opt->connect.wifi_state;
	current_activity = opt->curActivity;
	/**************************************************************************************** */

	/************************ Tag Handler (Not Finished)*************************************** */
	DvmConfigMethodMapIter it = pDvmDex->pconfigMethodMap->find(curClassMethodIds);
	methodTag = it->second.tag;

// 	Collect the Method IDs with the same TAG
//	for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end(); ++it){
//		if(it->second.tag == methodTag){
//		/// Solve the Tag issue here
//			tagIter.push_back(it);
//		}
//	}


	 vector<pair<u4,u4> >* curtempRange = &curClass->tagRanges[methodTag].tempRange;
	 vector<pair<u4,u4> >* curvoltRange = &curClass->tagRanges[methodTag].voltRange;
	 vector<pair<u4,u4> >* curpowRange = &curClass->tagRanges[methodTag].powRange;
	/**************************************************************************************** */

	/**************************************************************************************** */
	/*			 						best Fit Policy 									  */
	/**************************************************************************************** */
	if(curpolicy == bestfit){
		ALOGD("Best Fit");
		/******************************* Handle Power Preference first ************************** */
		if(preference == power_preference){
			ALOGD("Power Preference");
			for (u4 p = 0; p < curpowRange->size(); p++){
				// small rangeof power <= operating power point <= large rangeofPower
				//printf("Current power range: %d, %d\n",pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).first, pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).second);
				if(opt->batt.batteryCapacity >= curpowRange->at(p).first && opt->batt.batteryCapacity <= curpowRange->at(p).second ){
					//ALOGD("operating point lies in the ranges of power at index %d\n",p);
					powerindex.push_back(p);
				}
			}

			if(powerindex.size()==0){
				//raise flag to check all temperature ranges
				//Check_Temp_All =true;
				//ALOGD("power index size =0 all ranges with u\n");
				//meaning all ranges should be checked again so add all the indices in the powerindex
				for(u4 p=0; p < curpowRange->size(); p++)
					powerindex.push_back(p);
			}
			//go to second level only if there were more than 1 index from power
			//if power index size == 1 then you get the method and return it.
			if(powerindex.size()==1){
				//for(int m=0; m<pDvmConfigFile->sensitiveClasses.at(i).numSensitiveMethods; m++){
				 for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end(); ++it){
					//if (powerindex.at(0) == pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodparm.ids.first){
					 if (powerindex.at(0) == it->second.ids.first  && it->second.tag == methodTag){
						 //methodName = pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodName;
						 ClassMethodIds = it->first;
						// ALOGD("one power index method id = %d", ClassMethodIds.second);
						break;
					}
					else {
					 ALOGD("sth went wrong in power there must be a method");
					}
				}
			}
			else{//more than one power range
				// check the second level which is the temperature only with the power index
				//get temperature operating point index
				//1- get the index(iterator) in the map with these power indices to be only accessed later
				//if(Check_Temp_All==false){ //only if power index was found before
					for(u4 p=0; p<powerindex.size();p++){
						for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
							if(powerindex.at(p) == it->second.ids.first  && it->second.tag == methodTag){
								//ALOGD("find a match for power index\t");
								powerIter.push_back(it);
								//printf("size of poweriter = %d\n", powerIter.size());
								break;//found it, don't continue iterate in the map for this power index
							}
						}
					}
				//}
				// 2- Get temperature index
				for(u4 t=0; t< curtempRange->size(); t++){
					if(opt->batt.batteryTemperature >= curtempRange->at(t).first && opt->batt.batteryTemperature <= curtempRange->at(t).second){
						tempindex.push_back(t);
					//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
					}
				}
				if(tempindex.size()==0){
					//meaning nothing map to temperature so check the voltage with the power iteration indices instead
					Check_Volt_All = true;
				}
				if(tempindex.size()==1){
					//3-Access the map with the power iterators to check this temp and return the method,
					for(u4 piter =0; piter < powerIter.size(); piter++ ){
						if(powerIter.at(piter)->second.ids.second == tempindex.at(0)){
							ClassMethodIds = powerIter.at(piter)->first;
							break; // method name found break;
						}
						else if(piter == powerIter.size()-1){
							//reach the end with no temperature matching then go and check all voltage
							Check_Volt_All = true;
						}
					}
				}
				else if(tempindex.size() > 1){
					//more than one in temp range
					//put in the temperature iterator only the ranges that lie under the tree of power iterators
					for(u4 piter=0; piter < powerIter.size(); piter++){
						for(u4 tindex =0; tindex<tempindex.size();tindex++){
							if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
								tempIter.push_back(powerIter.at(piter));
								//ALOGD("find a match for temp index\t");
								//printf("size of tempiter = %d\n", tempIter.size());
							}
							}
							if(piter == powerIter.size()-1 && tempIter.size()==0){
								//no matching
								Check_Volt_All= true;
							}
					}
				}

				if(Check_Volt_All || tempIter.size()>0){
					if(tempIter.size()==1){
						//return the method here
						ClassMethodIds = tempIter.at(0)->first;
						//break;
					}
					else{
						//3- get the voltage indices
						for(u4 v=0; v< curvoltRange->size(); v++){
							if(opt->batt.batteryVoltage >= curvoltRange->at(v).first && opt->batt.batteryVoltage <= curvoltRange->at(v).second){
								voltindex.push_back(v);
								//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
							}
						}

						if(voltindex.size()==0){ // no volt match
							if(Check_Volt_All){ // no temp match
								//return the highest priority of power iterators with opt connectivity required
								u4 priority =10000;
								bool tag_method_found_with_conn = false;
								key candidateMethod;
								for(u4 piter =0; piter < powerIter.size(); piter++){
									if ((powerIter.at(piter)->second.priority < priority)){
										priority = powerIter.at(piter)->second.priority;
										candidateMethod = powerIter.at(piter)->first;
										if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
											ClassMethodIds = powerIter.at(piter)->first;
											tag_method_found_with_conn = true;
										}
									}
								}
								if(tag_method_found_with_conn == false) // no match method with required conn is found
									ClassMethodIds = candidateMethod; 	//take the highest priority in power regardless the conn;
							}
							else {
								// return highest priority of temp iterators with the opt connectivity required
								u4 priority =10000; //any big number
								bool tag_method_found_with_conn = false;
								key candidateMethod;
								for (u4 titer =0; titer < tempIter.size();titer++){
									if(tempIter.at(titer)->second.priority < priority){
										priority = tempIter.at(titer)->second.priority;
										candidateMethod = tempIter.at(titer)->first;
										if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
											ClassMethodIds = tempIter.at(titer)->first;
											tag_method_found_with_conn = true;
										}
									}
								}
								if(tag_method_found_with_conn == false) // no match method with required conn is found
									ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
							}
						}

						if(voltindex.size() == 1){
							if(Check_Volt_All){
								for(u4 piter =0; piter < powerIter.size(); piter++){
									if(powerIter.at(piter)->second.ids.third == voltindex.at(0)){
										voltIter.push_back(powerIter.at(piter));
									}
									else if(piter == powerIter.size()-1 && voltIter.size() == 0 ){
										//reach the end of power iteration with no matching to this volt index
										//return method of highest priority from power iteration
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										key candidateMethod;
										for(u4 piter =0; piter < powerIter.size(); piter++){
											if (powerIter.at(piter)->second.priority < priority){
												priority = powerIter.at(piter)->second.priority;
												candidateMethod = powerIter.at(piter)->first;
												if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
													ClassMethodIds = powerIter.at(piter)->first;
													tag_method_found_with_conn = true;
												}
											}
										}
										if(tag_method_found_with_conn == false) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
							else{
								//check if this volt index lie under the temp iterator tree
								for(u4 titer=0; titer < tempIter.size(); titer++){
									if(tempIter.at(titer)->second.ids.third == voltindex.at(0)){
										voltIter.push_back(tempIter.at(titer));
									}
									if(titer == powerIter.size()-1 && voltIter.size()==0){
										//no matching
										//return the highest priority of the temp iterartion
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										key candidateMethod;
										for(u4 titer =0; titer < tempIter.size(); titer++){
											if (tempIter.at(titer)->second.priority < priority){
												priority = tempIter.at(titer)->second.priority;
												candidateMethod = tempIter.at(titer)->first;
												if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
													ClassMethodIds = tempIter.at(titer)->first;
													tag_method_found_with_conn = true;
												}
											}
										}
										if(tag_method_found_with_conn == false) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
						}
						else if(voltindex.size()>1){
							if(Check_Volt_All){
								//check only those lie under the power tree iterator
								for(u4 piter=0; piter < powerIter.size(); piter++){
									for(u4 vindex =0; vindex<voltindex.size();vindex++){
										if(powerIter.at(piter)->second.ids.third == voltindex.at(vindex)){
											voltIter.push_back(powerIter.at(piter));
										}
									}
									if(piter == powerIter.size()-1 && voltIter.size()==0){
										//no matching // return highest priority of power iter
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										key candidateMethod;
										for(u4 piter =0; piter < powerIter.size(); piter++){
											if (powerIter.at(piter)->second.priority < priority){
												priority = powerIter.at(piter)->second.priority;
												candidateMethod = powerIter.at(piter)->first;
												if(wifi_connect == powerIter.at(piter)->second.conn.wifi_state || powerIter.at(piter)->second.conn.wifi_state == 2 /*don't care*/ ){
													ClassMethodIds = powerIter.at(piter)->first;
													tag_method_found_with_conn = true;
												}
											}
										}
										if(tag_method_found_with_conn == false) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
							//check only the volt index lie under the temperature tree iterator
							else {
								for(u4 titer=0; titer < tempIter.size(); titer++){
									for(u4 vindex =0; vindex<voltindex.size();vindex++){
										if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
											voltIter.push_back(tempIter.at(titer));
										}
									}
									if(titer == tempIter.size()-1 && voltIter.size()==0){
										//no matching // return highest priority of temp iteration
										u4 priority =10000;
										bool tag_method_found_with_conn = false;
										key candidateMethod;
										for(u4 titer =0; titer < tempIter.size(); titer++){
											if (tempIter.at(titer)->second.priority < priority){
												priority = tempIter.at(titer)->second.priority;
												candidateMethod = tempIter.at(titer)->first;
												if(wifi_connect == tempIter.at(titer)->second.conn.wifi_state || tempIter.at(titer)->second.conn.wifi_state == 2 /*don't care*/){
													ClassMethodIds = tempIter.at(titer)->first;
													tag_method_found_with_conn = true;
												}
											}
										}
										if(tag_method_found_with_conn == false) // no match method with required conn is found
											ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
									}
								}
							}
						}

						// At this point we have either the voltIter size = 1 or > 1 but can't be zero
						if(voltIter.size() == 1){
							// return this method
							ClassMethodIds = voltIter.at(0)->first; // We don't need to check the connectivity here
							//break;
						}
						else if(voltIter.size() > 1){
							//get the highest priority of the volt iter
							u4 priority =10000;
							bool tag_method_found_with_conn = false;
							key candidateMethod;
							for(u4 viter =0; viter < voltIter.size(); viter++){
								if (voltIter.at(viter)->second.priority < priority){
									priority = voltIter.at(viter)->second.priority;
									candidateMethod = voltIter.at(viter)->first;
									if(wifi_connect == voltIter.at(viter)->second.conn.wifi_state || voltIter.at(viter)->second.conn.wifi_state == 2 /*don't care*/){
										ClassMethodIds = voltIter.at(viter)->first;
										tag_method_found_with_conn = true;
									}
								}
							}
							if(tag_method_found_with_conn == false) // no match method with required conn is found
								ClassMethodIds = candidateMethod; 	//take the highest priority in temp regardless the conn;
						}
					}
				}
			}
			//ALOGD("Return MethodID = %d", ClassMethodIds.second);
		} /*POWER Preference tree */


		/* ****************************** Handle Connectivity Preference First ******************* */

		else if (preference == connectivity_preference){
			/*	Get the indices for the wifi connectivity first */
			//initialize the Check
			Check_Volt_All = false;
			ALOGD("Connectivity Preference");
			for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
				if((it->second.conn.wifi_state == wifi_connect || it->second.conn.wifi_state == 2 /*don't care*/)  && it->second.tag == methodTag){
					wifiIter.push_back(it);
				//	ALOGD("wifi it pushed back %d", it->first.second );
				}
			}

			if(wifiIter.size() == 0) {
				//push back in the wifi iter everything
				for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
					if(it->second.tag == methodTag)
						wifiIter.push_back(it);
				}
				//ALOGD("wifi iter was all zero");
			}

			if (wifiIter.size() == 1){
				 ClassMethodIds = wifiIter.at(0)->first; //stop here
			//	 ALOGD("found only one wifi that matches %d", wifiIter.at(0)->first.second);
			}
			else{
				for (u4 p = 0; p < curpowRange->size(); p++){
					// small rangeof power <= operating power point <= large rangeofPower
					//printf("Current power range: %d, %d\n",pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).first, pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).second);
					if(opt->batt.batteryCapacity >= curpowRange->at(p).first && opt->batt.batteryCapacity <= curpowRange->at(p).second ){
					//	ALOGD("operating point lies in the ranges of power at index %d\n",p);
						powerindex.push_back(p);
					}
				}

				if(powerindex.size()==0){
					//raise flag to check all temperature ranges
					//Check_Temp_All =true;
					//ALOGD("power index size =0 all ranges with you\n");
					//meaning all ranges should be checked again so add all the indices in the powerindex
					for(u4 p=0; p < curpowRange->size(); p++)
						powerindex.push_back(p);
				}
				//go to second level only if there were more than 1 index from power
				//if power index size == 1 then you get the method and return it.
				if(powerindex.size()==1){
					//for(int m=0; m<pDvmConfigFile->sensitiveClasses.at(i).numSensitiveMethods; m++){
					 bool tag_method_found = false;
					 for(u4 witer =0; witer < wifiIter.size(); witer++){
						 if (powerindex.at(0) == wifiIter.at(witer)->second.ids.first){
							 //methodName = pDvmConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(m).methodName;
							 ClassMethodIds = wifiIter.at(witer)->first;
							 tag_method_found = true;
						//	 ALOGD("power index just one and found the wifi iter");
							 break;
						}
					 }
					if(tag_method_found == false) {
						//Power index found does not lie in the list of wifi iterations.
						//return the highest priority method for the wifi
						u4 priority =10000;
						for(u4 witer =0; witer < wifiIter.size(); witer++){
							if (wifiIter.at(witer)->second.priority < priority){
								priority = wifiIter.at(witer)->second.priority;
								ClassMethodIds = wifiIter.at(witer)->first;
							}
						}
					}
				}
				else{//more than one power range
					// check the second level which is the temperature only with the power index
					//get temperature operating point index
					//1- get the index(iterator) in the map with these power indices to be only accessed later
					//if(Check_Temp_All==false){ //only if power index was found before
				//	ALOGD("More than one power index");
						for(u4 p=0; p<powerindex.size();p++){
							for(u4 witer =0; witer < wifiIter.size(); witer++){
								if(powerindex.at(p) == wifiIter.at(witer)->second.ids.first){
								//	ALOGD("find a match for power index\t");
									powerIter.push_back(wifiIter.at(witer));
								//	printf("size of poweriter = %d\n", powerIter.size());
									break;//found it, don't continue iterate in the map for this power index
								}
							}
						}
					//}
					// 2- Get temperature index
					for(u4 t=0; t< curtempRange->size(); t++){
						if(opt->batt.batteryTemperature >= curtempRange->at(t).first && opt->batt.batteryTemperature <= curtempRange->at(t).second){
							tempindex.push_back(t);
						//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
						}
					}
					if(tempindex.size()==0){
						//meaning nothing map to temperature so check the voltage with the power iteration indices instead
						Check_Volt_All = true;
					}
					if(tempindex.size()==1){
						//3-Access the map with the power iterators to check this temp and return the method,
						for(u4 piter =0; piter < powerIter.size(); piter++ ){
							if(powerIter.at(piter)->second.ids.second == tempindex.at(0)){
								ClassMethodIds = powerIter.at(piter)->first;
								break; // method name found break;
							}
							else if(piter == powerIter.size()-1){
								//reach the end with no temperature matching then go and check all voltage
								Check_Volt_All = true;
							}
						}
					}
					else if(tempindex.size() > 1){
						//more than one in temp range
						//put in the temperature iterator only the ranges that lie under the tree of power iterators
						for(u4 piter=0; piter < powerIter.size(); piter++){
							for(u4 tindex =0; tindex<tempindex.size();tindex++){
								if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
									tempIter.push_back(powerIter.at(piter));
								//	ALOGD("find a match for temp index\t");
									//printf("size of tempiter = %d\n", tempIter.size());
								}
								}
								if(piter == powerIter.size()-1 && tempIter.size()==0){
									//no matching
									Check_Volt_All= true;
								}
						}
					}

					if(Check_Volt_All || tempIter.size()>0){
						if(tempIter.size()==1){
							//return the method here
							ClassMethodIds = tempIter.at(0)->first;
							//break;
						}
						else{
							//3- get the voltage indices
							for(u4 v=0; v< curvoltRange->size(); v++){
								if(opt->batt.batteryVoltage >= curvoltRange->at(v).first && opt->batt.batteryVoltage <= curvoltRange->at(v).second){
									voltindex.push_back(v);
									//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
								}
							}

							if(voltindex.size()==0){ // no volt match
								if(Check_Volt_All){ // no temp match
									//return the highest priority of power iterators with opt connectivity required
									u4 priority =10000;
									for(u4 piter =0; piter < powerIter.size(); piter++){
										if ((powerIter.at(piter)->second.priority < priority)){
											priority = powerIter.at(piter)->second.priority;
											ClassMethodIds = powerIter.at(piter)->first;
											}
										}
									}
								}
								else {
									// return highest priority of temp iterators with the opt connectivity required
									u4 priority =10000; //any big number
									for (u4 titer =0; titer < tempIter.size();titer++){
										if(tempIter.at(titer)->second.priority < priority){
											priority = tempIter.at(titer)->second.priority;
											ClassMethodIds = tempIter.at(titer)->first;
										}
									}
								}
							}

							if(voltindex.size() == 1){
								if(Check_Volt_All){
									for(u4 piter =0; piter < powerIter.size(); piter++){
										if(powerIter.at(piter)->second.ids.third == voltindex.at(0)){
											voltIter.push_back(powerIter.at(piter));
										}
										else if(piter == powerIter.size()-1 && voltIter.size() == 0 ){
											//reach the end of power iteration with no matching to this volt index
											//return method of highest priority from power iteration
											u4 priority =10000;
											for(u4 piter =0; piter < powerIter.size(); piter++){
												if (powerIter.at(piter)->second.priority < priority){
													priority = powerIter.at(piter)->second.priority;
													ClassMethodIds = powerIter.at(piter)->first;
												}
											}
										}
									}
								}
								else{
									//check if this volt index lie under the temp iterator tree
									for(u4 titer=0; titer < tempIter.size(); titer++){
										if(tempIter.at(titer)->second.ids.third == voltindex.at(0)){
											voltIter.push_back(tempIter.at(titer));
										}
										if(titer == powerIter.size()-1 && voltIter.size()==0){
											//no matching
											//return the highest priority of the temp iterartion
											u4 priority =10000;
											for(u4 titer =0; titer < tempIter.size(); titer++){
												if (tempIter.at(titer)->second.priority < priority){
													priority = tempIter.at(titer)->second.priority;
													ClassMethodIds = tempIter.at(titer)->first;
												}
											}
										}
									}
								}
							}
							else if(voltindex.size()>1){
								if(Check_Volt_All){
									//check only those lie under the power tree iterator
									for(u4 piter=0; piter < powerIter.size(); piter++){
										for(u4 vindex =0; vindex<voltindex.size();vindex++){
											if(powerIter.at(piter)->second.ids.third == voltindex.at(vindex)){
												voltIter.push_back(powerIter.at(piter));
											}
										}
										if(piter == powerIter.size()-1 && voltIter.size()==0){
											//no matching // return highest priority of power iter
											u4 priority =10000;
											for(u4 piter =0; piter < powerIter.size(); piter++){
												if (powerIter.at(piter)->second.priority < priority){
													priority = powerIter.at(piter)->second.priority;
													ClassMethodIds = powerIter.at(piter)->first;
												}
											}
										}
									}
								}
								//check only the volt index lie under the temperature tree iterator
								else {
									for(u4 titer=0; titer < tempIter.size(); titer++){
										for(u4 vindex =0; vindex<voltindex.size();vindex++){
											if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
												voltIter.push_back(tempIter.at(titer));
											}
										}
										if(titer == tempIter.size()-1 && voltIter.size()==0){
											//no matching // return highest priority of temp iteration
											u4 priority =10000;
											for(u4 titer =0; titer < tempIter.size(); titer++){
												if (tempIter.at(titer)->second.priority < priority){
													priority = tempIter.at(titer)->second.priority;
													ClassMethodIds = tempIter.at(titer)->first;
												}
											}
										}
									}
								}
							}

							// At this point we have either the voltIter size = 1 or > 1 but can't be zero
							if(voltIter.size() == 1){
								// return this method
								ClassMethodIds = voltIter.at(0)->first; // We don't need to check the connectivity here
								//break;
							}
							else if(voltIter.size() > 1){
								//get the highest priority of the volt iter
								u4 priority =10000;
								for(u4 viter =0; viter < voltIter.size(); viter++){
									if (voltIter.at(viter)->second.priority < priority){
										priority = voltIter.at(viter)->second.priority;
										ClassMethodIds = voltIter.at(viter)->first;
									}
								}
							}
						}
					}
				}
		}/*Connectivity Preference tree */

	} /* Most fit Policy */


	/**************************************************************************************** */
	/*			 						Must Fit Policy 									  */
	/**************************************************************************************** */
	else if(curpolicy == mustfit){
		ALOGD("must Fit");
		/******************************* Handle Power Preference first ************************** */
		if(preference == power_preference){

			ALOGD("Power Preference");
			for (u4 p = 0; p < curpowRange->size(); p++){
				// small rangeof power <= operating power point <= large rangeofPower
				//printf("Current power range: %d, %d\n",pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).first, pDvmConfigFile->sensitiveClasses.at(i).powRange.at(p).second);
				if(opt->batt.batteryCapacity >= curpowRange->at(p).first && opt->batt.batteryCapacity <= curpowRange->at(p).second ){
					//ALOGD("operating point lies in the ranges of power at index %d\n",p);
					powerindex.push_back(p);
				}
			}

			if(powerindex.size()==0){
				// return the default // the one that has been called with
				ClassMethodIds = curClassMethodIds;

			}
			//go to second level only if there were more than 1 index from power
			else{//more than one power range
				// check the second level which is the temperature only with the power index
				//get temperature operating point index
				//1- get the index(iterator) in the map with these power indices to be only accessed later
				//if(Check_Temp_All==false){ //only if power index was found before
					for(u4 p=0; p<powerindex.size();p++){
						for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
							if(powerindex.at(p) == it->second.ids.first  && it->second.tag == methodTag){
								//ALOGD("find a match for power index\t");
								powerIter.push_back(it);
								//printf("size of poweriter = %d\n", powerIter.size());
								break;//found it, don't continue iterate in the map for this power index
							}
						}
					}
				//}
				// 2- Get temperature index
				for(u4 t=0; t< curtempRange->size(); t++){
					if(opt->batt.batteryTemperature >= curtempRange->at(t).first && opt->batt.batteryTemperature <= curtempRange->at(t).second){
						tempindex.push_back(t);
					//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
					}
				}

				if(tempindex.size()==0){
					ClassMethodIds = curClassMethodIds;
				}
				else {
					//more than one in temp range
					//put in the temperature iterator only the ranges that lie under the tree of power iterators
					for(u4 piter=0; piter < powerIter.size(); piter++){
						for(u4 tindex =0; tindex<tempindex.size();tindex++){
							if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
								tempIter.push_back(powerIter.at(piter));
								//ALOGD("find a match for temp index\t");
								//printf("size of tempiter = %d\n", tempIter.size());
							}
						}
						if(piter == powerIter.size()-1 && tempIter.size()==0){
							//no matching //return default
							ClassMethodIds = curClassMethodIds;
						}
					}
				}

				if(tempIter.size()>0){
						//3- get the voltage indices
						for(u4 v=0; v< curvoltRange->size(); v++){
							if(opt->batt.batteryVoltage >= curvoltRange->at(v).first && opt->batt.batteryVoltage <= curvoltRange->at(v).second){
								voltindex.push_back(v);
								//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
							}
						}

						if(voltindex.size()==0){ // no volt match
							ClassMethodIds = curClassMethodIds;
						}
						else {
							//check only the volt index lie under the temperature tree iterator
							for(u4 titer=0; titer < tempIter.size(); titer++){
								for(u4 vindex =0; vindex<voltindex.size();vindex++){
									if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
										voltIter.push_back(tempIter.at(titer));
									}
								}
								if(titer == tempIter.size()-1 && voltIter.size()==0){
									//no match
									ClassMethodIds =  curClassMethodIds;
								}
							}
						}

						// At this point we have either the voltIter size = 1 or > 1 but can't be zero
						if(voltIter.size() == 1){
							//  check the connectivity here
							if(wifi_connect == voltIter.at(0)->second.conn.wifi_state || voltIter.at(0)->second.conn.wifi_state == 2 /*don't care*/)
								ClassMethodIds = voltIter.at(0)->first;
							else //no match for connectivity return default
								ClassMethodIds =  curClassMethodIds;
							//break;
						}
						else if(voltIter.size() > 1){
							//get the highest priority of the volt iter
							// get the one that fit the connectivity
							u4 priority =10000;
							bool tag_method_found_with_conn = false;
							key candidateMethod;
							for(u4 viter =0; viter < voltIter.size(); viter++){
								if (voltIter.at(viter)->second.priority < priority){
									priority = voltIter.at(viter)->second.priority;
									candidateMethod = voltIter.at(viter)->first;
									if(wifi_connect == voltIter.at(viter)->second.conn.wifi_state || voltIter.at(viter)->second.conn.wifi_state == 2 /*don't care*/){
										ClassMethodIds = voltIter.at(viter)->first;
										tag_method_found_with_conn = true;
									}
								}
							}
							if(tag_method_found_with_conn == false) // no match method with required conn is found
								ClassMethodIds = curClassMethodIds; 	//take the default
						}

				}
			}
			//ALOGD("Return MethodID = %d", ClassMethodIds.second);
		} /*POWER Preference tree */


		/* ****************************** Handle Connectivity Preference First ******************* */

		else if (preference == connectivity_preference){
			/*	Get the indices for the wifi connectivity first */

			ALOGD("Connectivity Preference");
			for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end();++it){
				if((it->second.conn.wifi_state == wifi_connect || it->second.conn.wifi_state == 2 /*don't care*/) && it->second.tag == methodTag){
					wifiIter.push_back(it);
					//ALOGD("wifi it pushed back %d", it->first.second );
				}
			}

			if(wifiIter.size() == 0) {
				//return default
				ClassMethodIds = curClassMethodIds;
			}
			else{
				for (u4 p = 0; p < curpowRange->size(); p++){
					if(opt->batt.batteryCapacity >= curpowRange->at(p).first && opt->batt.batteryCapacity <= curpowRange->at(p).second ){
						//ALOGD("operating point lies in the ranges of power at index %d\n",p);
						powerindex.push_back(p);
					}
				}

				if(powerindex.size()==0){
					// return default
					ClassMethodIds = curClassMethodIds;
				}
				else{//more than one power range
					for(u4 p=0; p<powerindex.size();p++){
						for(u4 witer =0; witer < wifiIter.size(); witer++){
							if(powerindex.at(p) == wifiIter.at(witer)->second.ids.first){
							//	ALOGD("find a match for power index\t");
								powerIter.push_back(wifiIter.at(witer));
							}
							if(witer == wifiIter.size()-1 && powerIter.size()==0){
								//no matching //return default
								ClassMethodIds = curClassMethodIds;
							}
						}
					}
					if(powerIter.size() > 0){
						// 2- Get temperature index
						for(u4 t=0; t< curtempRange->size(); t++){
							if(opt->batt.batteryTemperature >= curtempRange->at(t).first && opt->batt.batteryTemperature <= curtempRange->at(t).second){
								tempindex.push_back(t);
							//	ALOGD("opertaing point lies in the ranges of temp at index %d\n",t);
							}
						}
						if(tempindex.size()==0){
							//no matching //return default
							ClassMethodIds = curClassMethodIds;
						}
						else{
							//more than one in temp range
							//put in the temperature iterator only the ranges that lie under the tree of power iterators
							for(u4 piter=0; piter < powerIter.size(); piter++){
								for(u4 tindex =0; tindex<tempindex.size();tindex++){
									if(powerIter.at(piter)->second.ids.second == tempindex.at(tindex)){
										tempIter.push_back(powerIter.at(piter));
										//ALOGD("find a match for temp index\t");
										//printf("size of tempiter = %d\n", tempIter.size());
									}
								}
								if(piter == powerIter.size()-1 && tempIter.size()==0){
									//no matching //return default
									ClassMethodIds = curClassMethodIds;
								}
							}
						}

						if(tempIter.size()>0){
							//3- get the voltage indices
							for(u4 v=0; v< curvoltRange->size(); v++){
								if(opt->batt.batteryVoltage >= curvoltRange->at(v).first && opt->batt.batteryVoltage <= curvoltRange->at(v).second){
									voltindex.push_back(v);
									//ALOGD("opertaing point lies in the ranges of volt at index %d\n",v);
								}
							}
							if(voltindex.size()==0){ // no volt match
								//no matching //return default
								ClassMethodIds = curClassMethodIds;
							}
							else {
								//check only the volt index lie under the temperature tree iterator
								for(u4 titer=0; titer < tempIter.size(); titer++){
									for(u4 vindex =0; vindex<voltindex.size();vindex++){
											if(tempIter.at(titer)->second.ids.third == voltindex.at(vindex)){
												voltIter.push_back(tempIter.at(titer));
											}
									}
									if(titer == tempIter.size()-1 && voltIter.size()==0){
										//no matching //return default
										ClassMethodIds = curClassMethodIds;
									}
								}
								// At this point we have either the voltIter size = 1 or > 1 but can't be zero
								if(voltIter.size() > 0){
									//get the highest priority of the volt iter
									u4 priority =10000;
									for(u4 viter =0; viter < voltIter.size(); viter++){
										if (voltIter.at(viter)->second.priority < priority){
											priority = voltIter.at(viter)->second.priority;
											ClassMethodIds = voltIter.at(viter)->first;
										}
									}
								}
							}
						}
					}
				}
			}
		}/*Connectivity Preference tree */


	} /* Best fit policy*/


	/************************************** Update the LUT ********************************* */
	OperatingPoint curopt = *opt;
	LUTkey lutk = { curClassMethodIds, curopt};
	keyTimeStamp kts = { ClassMethodIds, TIME_STAMP};
	if (gDvmLUTMap.size() != MAX_LUT_SIZE ){
		gDvmLUTMap.insert(make_pair(lutk,kts));
		//ALOGD("Insert in the LUT, @%d", TIME_STAMP);
		TIME_STAMP++;
	}
	else{ //erase and insert
		//ALOGD("Clear an element in the LUT");
		DvmLUTMapIter it;
		//find the element with the oldest time stamp
		bool eraseFlag = true; // to avoid multiple erase
		for (it = gDvmLUTMap.begin(); it != gDvmLUTMap.end(); ++it ){
			if (it->second.time_stamp == 0 && eraseFlag ){
				gDvmLUTMap.erase(it); //erase first element
				eraseFlag = false;
			}
			else
				it->second.time_stamp = it->second.time_stamp - 1;
		}
		TIME_STAMP = MAX_LUT_SIZE - 1;
		kts.time_stamp = TIME_STAMP;
		gDvmLUTMap.insert(make_pair(lutk,kts));
	}
	/**************************************************************************************** */


	return ClassMethodIds.second;
}
/* ********************************************************************* */

