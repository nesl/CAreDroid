/*
 * DvmSystemActivityDetection.h
 *
 *  Created on: Sep 10, 2014
 *      Author: salma
 */

#ifndef DALVIK_SYSTEM_ACTIVITY_DETECTION_H_
#define DALVIK_SYSTEM_ACTIVITY_DETECTION_H_

#include "Dalvik.h"

bool dvmSystemActivityGetActivityThreadFlag();
bool dvmSystemActivityDetectionStartup();
void dvmSystemActivityDetectionShutdown();

#endif
