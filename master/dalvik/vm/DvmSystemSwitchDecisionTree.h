/*
 * DvmSystemSwitchDecisionTree.h
 *
 *  Created on: March 19, 2014
 *      Author: salma
 */
#ifndef DALVIK_DVMSYSTEMSWITCHDECISIONTREE_H_
#define DALVIK_DVMSYSTEMSWITCHDECISIONTREE_H_

#include "DvmSystemCoreValues.h"


u4 dvmSystemReturnMethod(DvmDex* pDvmDex, string className, key curClassMethodIds);
u4 dvmSystemReturnMethod(DvmDex* pDvmDex, string className, key curClassMethodIds, bool tag);

#endif //DALVIK_DVMSYSTEMSWITCHDECISIONTREE_H_
