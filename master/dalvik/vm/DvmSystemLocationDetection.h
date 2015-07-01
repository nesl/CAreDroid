/*
 * DvmSystemLocationDetection.h
 *
 *  Created on: Dec 1, 2014
 *      Author: salma
 */

#ifndef DALVIK_SYSTEM_LOCATION_DETECTION_H_
#define DALVIK_SYSTEM_LOCATION_DETECTION_H_

#include "Dalvik.h"

bool dvmSystemLocationGetLocationThreadFlag();
bool dvmSystemLocationDetectionStartup();
void dvmSystemLocationDetectionShutdown();

#endif
