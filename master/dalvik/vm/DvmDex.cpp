/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * VM-specific state associated with a DEX file.
 */
#include "Dalvik.h"
#include <sys/mman.h>
//salma
//#include <fcntl.h>
//#include "DvmConfig.h"
#include <vector>

//endsalma

/*
 * Create auxillary data structures.
 *
 * We need a 4-byte pointer for every reference to a class, method, field,
 * or string constant.  Summed up over all loaded DEX files (including the
 * whoppers in the boostrap class path), this adds up to be quite a bit
 * of native memory.
 *
 * For more traditional VMs these values could be stuffed into the loaded
 * class file constant pool area, but we don't have that luxury since our
 * classes are memory-mapped read-only.
 *
 * The DEX optimizer will remove the need for some of these (e.g. we won't
 * use the entry for virtual methods that are only called through
 * invoke-virtual-quick), creating the possibility of some space reduction
 * at dexopt time.
 */

static DvmDex* allocateAuxStructures(DexFile* pDexFile)
{
    DvmDex* pDvmDex;
    const DexHeader* pHeader;
    u4 stringSize, classSize, methodSize, fieldSize;

    pHeader = pDexFile->pHeader;

    stringSize = pHeader->stringIdsSize * sizeof(struct StringObject*);
    classSize  = pHeader->typeIdsSize * sizeof(struct ClassObject*);
    methodSize = pHeader->methodIdsSize * sizeof(struct Method*);
    fieldSize  = pHeader->fieldIdsSize * sizeof(struct Field*);

    u4 totalSize = sizeof(DvmDex) +
                   stringSize + classSize + methodSize + fieldSize;

    u1 *blob = (u1 *)dvmAllocRegion(totalSize,
                              PROT_READ | PROT_WRITE, "dalvik-aux-structure");
    if ((void *)blob == MAP_FAILED)
        return NULL;

    pDvmDex = (DvmDex*)blob;
    blob += sizeof(DvmDex);

    pDvmDex->pDexFile = pDexFile;
    pDvmDex->pHeader = pHeader;

    pDvmDex->pResStrings = (struct StringObject**)blob;
    blob += stringSize;
    pDvmDex->pResClasses = (struct ClassObject**)blob;
    blob += classSize;
    pDvmDex->pResMethods = (struct Method**)blob;
    blob += methodSize;
    pDvmDex->pResFields = (struct Field**)blob;

    ALOGV("+++ DEX %p: allocateAux (%d+%d+%d+%d)*4 = %d bytes",
        pDvmDex, stringSize/4, classSize/4, methodSize/4, fieldSize/4,
        stringSize + classSize + methodSize + fieldSize);

    pDvmDex->pInterfaceCache = dvmAllocAtomicCache(DEX_INTERFACE_CACHE_SIZE);

    dvmInitMutex(&pDvmDex->modLock);

    return pDvmDex;
}

/*
 * Given an open optimized DEX file, map it into read-only shared memory and
 * parse the contents.
 *
 * Returns nonzero on error.
 */
int dvmDexFileOpenFromFd(int fd, DvmDex** ppDvmDex)
{
    DvmDex* pDvmDex;
    DexFile* pDexFile;
    MemMapping memMap;
    int parseFlags = kDexParseDefault;
    int result = -1;

//    //salma //Success here
    // All the file manipulation moved to the JarFile.cpp

    //DvmConfigFile* pDvmConfigFile;
//    int configFd = -1;
//    __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmDex.cpp: Try to open config.xml");
//    configFd = open("/sdcard/config.xml", O_RDONLY);
//    if(configFd < 0)
//    	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmDex.cpp: FAIL Try to open config.xml");
//    else{
//    	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmDex.cpp: SUCCESS Try to open config.xml");
//    	//success
//    	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmDex.cpp: SUCCESS  Start Parse config.xml");
//    	pDvmConfigFile = dvmConfigFileParse(configFd);
//    	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmDex.cpp: SUCCESS  Finish Parse config.xml");
//    	dvmConfigFileDebug(pDvmConfigFile);
//    	__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "DvmDex.cpp: SUCCESS  Finish Debug config.xml");
//    }
////endsalma


    if (gDvm.verifyDexChecksum)
        parseFlags |= kDexParseVerifyChecksum;

    if (lseek(fd, 0, SEEK_SET) < 0) {
        ALOGE("lseek rewind failed");
        goto bail;
    }

    if (sysMapFileInShmemWritableReadOnly(fd, &memMap) != 0) {
        ALOGE("Unable to map file");
        goto bail;
    }

    pDexFile = dexFileParse((u1*)memMap.addr, memMap.length, parseFlags);
    if (pDexFile == NULL) {
        ALOGE("DEX parse failed");
        sysReleaseShmem(&memMap);
        goto bail;
    }

    pDvmDex = allocateAuxStructures(pDexFile);
    if (pDvmDex == NULL) {
        dexFileFree(pDexFile);
        sysReleaseShmem(&memMap);
        goto bail;
    }

    /* tuck this into the DexFile so it gets released later */
    sysCopyMap(&pDvmDex->memMap, &memMap);
    pDvmDex->isMappedReadOnly = true;
    *ppDvmDex = pDvmDex;
    result = 0;

bail:
    return result;
}

/*
 * Create a DexFile structure for a "partial" DEX.  This is one that is in
 * the process of being optimized.  The optimization header isn't finished
 * and we won't have any of the auxillary data tables, so we have to do
 * the initialization slightly differently.
 *
 * Returns nonzero on error.
 */
int dvmDexFileOpenPartial(const void* addr, int len, DvmDex** ppDvmDex)
{
    DvmDex* pDvmDex;
    DexFile* pDexFile;
    int parseFlags = kDexParseDefault;
    int result = -1;

    /* -- file is incomplete, new checksum has not yet been calculated
    if (gDvm.verifyDexChecksum)
        parseFlags |= kDexParseVerifyChecksum;
    */

    pDexFile = dexFileParse((u1*)addr, len, parseFlags);
    if (pDexFile == NULL) {
        ALOGE("DEX parse failed");
        goto bail;
    }
    pDvmDex = allocateAuxStructures(pDexFile);
    if (pDvmDex == NULL) {
        dexFileFree(pDexFile);
        goto bail;
    }

    pDvmDex->isMappedReadOnly = false;
    *ppDvmDex = pDvmDex;
    result = 0;

bail:
    return result;
}

/*
 * Free up the DexFile and any associated data structures.
 *
 * Note we may be called with a partially-initialized DvmDex.
 */
void dvmDexFileFree(DvmDex* pDvmDex)
{
    u4 totalSize;

    if (pDvmDex == NULL)
        return;

    dvmDestroyMutex(&pDvmDex->modLock);

    totalSize  = pDvmDex->pHeader->stringIdsSize * sizeof(struct StringObject*);
    totalSize += pDvmDex->pHeader->typeIdsSize * sizeof(struct ClassObject*);
    totalSize += pDvmDex->pHeader->methodIdsSize * sizeof(struct Method*);
    totalSize += pDvmDex->pHeader->fieldIdsSize * sizeof(struct Field*);
    totalSize += sizeof(DvmDex);

    dexFileFree(pDvmDex->pDexFile);

    ALOGV("+++ DEX %p: freeing aux structs", pDvmDex);
    dvmFreeAtomicCache(pDvmDex->pInterfaceCache);
    sysReleaseShmem(&pDvmDex->memMap);
    munmap(pDvmDex, totalSize);
}


/*
 * Change the byte at the specified address to a new value.  If the location
 * already has the new value, do nothing.
 *
 * This requires changing the access permissions to read-write, updating
 * the value, and then resetting the permissions.
 *
 * We need to ensure mutual exclusion at a page granularity to avoid a race
 * where one threads sets read-write, another thread sets read-only, and
 * then the first thread does a write.  Since we don't do a lot of updates,
 * and the window is small, we just use a lock across the entire DvmDex.
 * We're only trying to make the page state change atomic; it's up to the
 * caller to ensure that multiple threads aren't stomping on the same
 * location (e.g. breakpoints and verifier/optimizer changes happening
 * simultaneously).
 *
 * TODO: if we're back to the original state of the page, use
 * madvise(MADV_DONTNEED) to release the private/dirty copy.
 *
 * Returns "true" on success.
 */
bool dvmDexChangeDex1(DvmDex* pDvmDex, u1* addr, u1 newVal)
{
    if (*addr == newVal) {
        ALOGV("+++ byte at %p is already 0x%02x", addr, newVal);
        return true;
    }

    /*
     * We're not holding this for long, so we don't bother with switching
     * to VMWAIT.
     */
    dvmLockMutex(&pDvmDex->modLock);

    ALOGV("+++ change byte at %p from 0x%02x to 0x%02x", addr, *addr, newVal);
    if (sysChangeMapAccess(addr, 1, true, &pDvmDex->memMap) != 0) {
        ALOGD("NOTE: DEX page access change (->RW) failed");
        /* expected on files mounted from FAT; keep going (may crash) */
    }

    *addr = newVal;

    if (sysChangeMapAccess(addr, 1, false, &pDvmDex->memMap) != 0) {
        ALOGD("NOTE: DEX page access change (->RO) failed");
        /* expected on files mounted from FAT; keep going */
    }

    dvmUnlockMutex(&pDvmDex->modLock);

    return true;
}

/*
 * Change the 2-byte value at the specified address to a new value.  If the
 * location already has the new value, do nothing.
 *
 * Otherwise works like dvmDexChangeDex1.
 */
bool dvmDexChangeDex2(DvmDex* pDvmDex, u2* addr, u2 newVal)
{
    if (*addr == newVal) {
        ALOGV("+++ value at %p is already 0x%04x", addr, newVal);
        return true;
    }

    /*
     * We're not holding this for long, so we don't bother with switching
     * to VMWAIT.
     */
    dvmLockMutex(&pDvmDex->modLock);

    ALOGV("+++ change 2byte at %p from 0x%04x to 0x%04x", addr, *addr, newVal);
    if (sysChangeMapAccess(addr, 2, true, &pDvmDex->memMap) != 0) {
        ALOGD("NOTE: DEX page access change (->RW) failed");
        /* expected on files mounted from FAT; keep going (may crash) */
    }

    *addr = newVal;

    if (sysChangeMapAccess(addr, 2, false, &pDvmDex->memMap) != 0) {
        ALOGD("NOTE: DEX page access change (->RO) failed");
        /* expected on files mounted from FAT; keep going */
    }

    dvmUnlockMutex(&pDvmDex->modLock);

    return true;
}


//salma
/* Fill in the configuration Map from config file priority only*/
void dvmDexConfigMap(DvmDex* pDvmDex, DvmConfigMap* pconfigMap){

	//ToDo: Handle this exception later
	if(pDvmDex==NULL) return;
	if(!pDvmDex->isConfigFile) return;

	const char* curMethodName =NULL;
	const char* sensitiveMethodName =NULL;
	unsigned int priority = 0;
    const DexClassDef* pDexClassDef = NULL;
    const DexMethodId* pDexMethodId = NULL;
	unsigned int methodIdsSize = pDvmDex->pDexFile->pHeader->methodIdsSize;
	DvmConfigFile* pConfigFile = pDvmDex->pConfigFile;

	//DvmConfigMap configMap; // it is already set in the previous function so we will access the pointer directly

	for(int i =0; i<pConfigFile->numSensitiveClasses; i++){
		 pDexClassDef = dexFindClass(pDvmDex->pDexFile,pConfigFile->sensitiveClasses.at(i).className.c_str() );
		 if(pDexClassDef==NULL){
			 ALOGE("dvmDexConfigMap: pDexClassDef == NULL");
			 return;
		 }
		unsigned int classIdx = dexGetIndexForClassDef(pDvmDex->pDexFile,pDexClassDef);
		// ALOGD("dvmDexConfigMap: classIdx:%d",classIdx); //this is correct // 256
		 //sensitiveClassIdx.push_back(classIdx);

		//((DexClassDef)(pDvmDex->pDexFile->pClassDefs[classIdx])).isSensitiveClass = 1;
		// pDvmDex->pDexFile->pClassDefs[classIdx].isSensitiveClass = 1;

		 for(int j=0; j< pConfigFile->sensitiveClasses.at(i).numSensitiveMethods; j++){
			 sensitiveMethodName = pConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodName.c_str();
			// ALOGD("sensitiveMethodName = %s",sensitiveMethodName);
			 priority =  pConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodparam.priority;
			// ALOGD("priority =%d",priority );
			 for(unsigned int methodIdx=0; methodIdx<methodIdsSize; methodIdx++){
				 pDexMethodId = dexGetMethodId(pDvmDex->pDexFile,methodIdx);
				 curMethodName = dexStringById(pDvmDex->pDexFile, pDexMethodId->nameIdx);
				//if(curMethodName) ALOGD("curMethodName = %s",curMethodName);
				 if(curMethodName == NULL) return;
				 if(strcmp(curMethodName,sensitiveMethodName) == 0){

					 pconfigMap->insert(std::make_pair(key(classIdx,methodIdx),priority));
					// ALOGD("After mapping");
				 }
			 }
		 }
	}

}


//salma
/* Fill in the configuration Map from config file*/
/* Is called from vm/JarFile.cpp*/
void dvmDexConfigMethodMap(DvmDex* pDvmDex, DvmConfigMethodMap* pconfigMethodMap){

	//ToDo: Handle this exception later
	if(pDvmDex==NULL) return;
	if(!pDvmDex->isConfigFile) return;

	const char* curMethodName =NULL;
	const char* sensitiveMethodName =NULL;
    const DexClassDef* pDexClassDef = NULL;
    const DexMethodId* pDexMethodId = NULL;
	unsigned int methodIdsSize = pDvmDex->pDexFile->pHeader->methodIdsSize;
	DvmConfigFile* pConfigFile = pDvmDex->pConfigFile;

	for(int i =0; i<pConfigFile->numSensitiveClasses; i++){
		 pDexClassDef = dexFindClass(pDvmDex->pDexFile,pConfigFile->sensitiveClasses.at(i).className.c_str() );
		 if(pDexClassDef==NULL){
			 ALOGE("dvmDexConfigMap: pDexClassDef == NULL");
			 return;
		 }
		 unsigned int classIdx = dexGetIndexForClassDef(pDvmDex->pDexFile,pDexClassDef);
		 for(int j=0; j< pConfigFile->sensitiveClasses.at(i).numSensitiveMethods; j++){
			 sensitiveMethodName = pConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodName.c_str();
			 for(unsigned int methodIdx=0; methodIdx<methodIdsSize; methodIdx++){
				 pDexMethodId = dexGetMethodId(pDvmDex->pDexFile,methodIdx);
				 curMethodName = dexStringById(pDvmDex->pDexFile, pDexMethodId->nameIdx);
				 if(curMethodName == NULL) return;
				 if(strcmp(curMethodName,sensitiveMethodName) == 0){
					 pconfigMethodMap->insert(std::make_pair(key(classIdx,methodIdx), pConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodparam));
				 }
			 }
		 }
	}

	//DEBUG
//	if(pDvmDex->pconfigMethodMap!=NULL){
//		ALOGD("dvmDexConfigMapMethodDebug");
//		if(pDvmDex->pconfigMethodMap==NULL) return;
//		if(pDvmDex->pconfigMethodMap->size() ==0 ) return;
//
//		ALOGD("dvmDexConfigMapDebug: pDvmConfigMap exists and size = %d",pDvmDex->pconfigMethodMap->size() );
//
//		for(DvmConfigMethodMapIter it=pDvmDex->pconfigMethodMap->begin(); it!=pDvmDex->pconfigMethodMap->end(); ++it){
//			ALOGD("ClassID:%d MethodID:%d ==> tag:%d, priority:%d, powerindex:%d, tempindex:%d, voltindex:%d ",it->first.first, it->first.second, it->second.tag, it->second.priority, it->second.ids.first, it->second.ids.second, it->second.ids.third);
//		}
//	}

}


//endsalma
//
////void dvmDexConfigMapDebug(DvmConfigMap* pDvmConfigMap){
////
////	//ALOGD("dvmDexConfigMapDebug");
////	if(pDvmConfigMap==NULL) return;
////	if(pDvmConfigMap->size() ==0 ) return;
////
////	//ALOGD("dvmDexConfigMapDebug: pDvmConfigMap exists and size = %d",pDvmConfigMap->size() );
////
////	for(DvmConfigMapIter it=pDvmConfigMap->begin(); it!=pDvmConfigMap->end(); ++it){
////		for(class_method_mapIter it2=it->second.begin(); it2!=it->second.end(); ++it2){
////			ALOGD(" %d ==> %d, %d", it->first, it2->first, it2->second );
////		}
////	}
////
////
////}
//
//
//void dvmDexConfigMapDebug(DvmConfigMap* pDvmConfigMap){
//
//	//ALOGD("dvmDexConfigMapDebug");
//	if(pDvmConfigMap==NULL) return;
//	if(pDvmConfigMap->size() ==0 ) return;
//
//	//ALOGD("dvmDexConfigMapDebug: pDvmConfigMap exists and size = %d",pDvmConfigMap->size() );
//
//	for(DvmConfigMapIter it=pDvmConfigMap->begin(); it!=pDvmConfigMap->end(); ++it){
//		ALOGD("ClassID:%d MethodID:%d ==> Priority:%d ",it->first.first, it->first.second, it->second);
//	}
//
//
//}
///*return true means replacement took place
// * return false means replacement didn't happen
// * */
////bool dvmDexUpdateConfigMapMethodID(DvmConfigMap* pDvmConfigMap, u4 classId, u4 previousId, u4 updatedId){
////
////	if(pDvmConfigMap == NULL || pDvmConfigMap->size() ==0 ){
////		//ALOGE("pDvmConfigMap = NULL dvmDexUpdateConfigMapMethodID");
////		return false;
////	}
////
////	//ALOGD("dvmDexUpdateConfigMapMethodIDL passed parameters: classId: %d, previousId: %d, updatedId: %d",classId, previousId, updatedId );
////	for(DvmConfigMapIter it=pDvmConfigMap->begin(); it!=pDvmConfigMap->end(); ++it){
////		for(class_method_mapIter it2=it->second.begin(); it2!=it->second.end(); ++it2){
////			//Check the pair <classID, MethodID> for updating
////			if(it2->first == classId && it2->second == previousId){
////				//ALOGD(" before replacing: priority %d ==> ClassId %d, MethodId %d", it->first, it2->first, it2->second );
////				it2->second = updatedId;
////				//ALOGD(" after replacing: priority %d ==> ClassId %d, MethodId %d", it->first, it2->first, it2->second );
////				return true; // MethodId is found so just return and don't continue the loop
////			}
////
////		}
////	}
////
////	return false;
////}
//bool dvmDexUpdateConfigMapMethodID(DvmConfigMap* pDvmConfigMap, u4 classId, u4 previousId, u4 updatedId){
//
//	if(pDvmConfigMap == NULL || pDvmConfigMap->size() ==0 ){
//		//ALOGE("pDvmConfigMap = NULL dvmDexUpdateConfigMapMethodID");
//		return false;
//	}
//
//	DvmConfigMapIter it;
//	it = pDvmConfigMap->find(key(classId,previousId));
//	if(it != pDvmConfigMap->end()){
//		//keys are not modifiable
//		//erase this specific entry and reinsert a new entry with the updated key
//		//keeping in mind that priority should be maintained
//		u4 priority = it->second;
//		pDvmConfigMap->erase(it);
//		pDvmConfigMap->insert(std::make_pair(key(classId,updatedId),priority));
//		return true;
//	}
//
//
//	return false;
//}
//
//
//
//
//
///*retrieve priority of a method from the config map*/
///*method ID could be the flat one or the vtable index it doesn't matter but the whole map should be consistent*/
///*return 0 means that this method doesn't have an assigned priority
// * i.e. default priority = 0
// * */
////u4 dvmDexGetMethodPriority(DvmConfigMap* pDvmConfigMap, u4 classId, u4 methodId){
////
////	u4 defaultPriority = 0;
////	if(pDvmConfigMap == NULL || pDvmConfigMap->size() ==0 ){
////		//ALOGE("pDvmConfigMap = NULL dvmDexUpdateConfigMapMethodID");
////		return defaultPriority;
////	}
////
////	/*search through the map for the specific class id and method id pair to get the priority*/
////	for(DvmConfigMapIter it=pDvmConfigMap->begin(); it!=pDvmConfigMap->end(); ++it){
////			for(class_method_mapIter it2=it->second.begin(); it2!=it->second.end(); ++it2){
////				//Check the pair <classID, MethodID> for updating
////				if(it2->first == classId && it2->second == methodId){
////					return it->first; // MethodId is found so just return the mapped priority
////				}
////
////			}
////		}
////
////		return defaultPriority;
////
////}
//
//
//u4 dvmDexGetMethodPriority(DvmConfigMap* pDvmConfigMap, u4 classId, u4 methodId){
//
//	u4 defaultPriority = 0;
//	if(pDvmConfigMap == NULL || pDvmConfigMap->size() ==0 ){
//		//ALOGE("pDvmConfigMap = NULL dvmDexUpdateConfigMapMethodID");
//		return defaultPriority;
//	}
//
//	/*search through the map for the specific class id and method id pair to get the priority*/
//	DvmConfigMapIter it;
//	it = pDvmConfigMap->find(key(classId,methodId));
//	if(it != pDvmConfigMap->end())
//			return it->second;
//
//	return defaultPriority;
//
//}
//
//

//salma
//
//void dvmDexConfigMap(DvmDex* pDvmDex, DvmConfigMap* pconfigMap){
//
//	//ToDo: Handle this exception later
//	if(pDvmDex==NULL) return;
//	if(!pDvmDex->isConfigFile) return;
//
//
//	const char* curMethodName =NULL;
//	const char* sensitiveMethodName =NULL;
//	unsigned int priority = 0;
//    const DexClassDef* pDexClassDef = NULL;
//    const DexMethodId* pDexMethodId = NULL;
//	unsigned int methodIdsSize = pDvmDex->pDexFile->pHeader->methodIdsSize;
//	DvmConfigFile* pConfigFile = pDvmDex->pConfigFile;
//
//	//DvmConfigMap configMap; // it is already set in the previous function so we will access the pointer directly
//
//	for(int i =0; i<pConfigFile->numSensitiveClasses; i++){
//		 pDexClassDef = dexFindClass(pDvmDex->pDexFile,pConfigFile->sensitiveClasses.at(i).className.c_str() );
//		 if(pDexClassDef==NULL){
//			 ALOGD("dvmDexConfigMap: pDexClassDef == NULL");
//			 return;
//		 }
//		 unsigned int classIdx = dexGetIndexForClassDef(pDvmDex->pDexFile,pDexClassDef);
//		// ALOGD("dvmDexConfigMap: classIdx:%d",classIdx); //this is correct // 256
//		 //sensitiveClassIdx.push_back(classIdx);
//
//		//((DexClassDef)(pDvmDex->pDexFile->pClassDefs[classIdx])).isSensitiveClass = 1;
//		// pDvmDex->pDexFile->pClassDefs[classIdx].isSensitiveClass = 1;
//
//		 for(int j=0; j< pConfigFile->sensitiveClasses.at(i).numSensitiveMethods; j++){
//			 sensitiveMethodName = pConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(j).methodName.c_str();
//			// ALOGD("sensitiveMethodName = %s",sensitiveMethodName);
//			 priority =  pConfigFile->sensitiveClasses.at(i).sensitiveMethods.at(j).priorityLevel;
//			// ALOGD("priority =%d",priority );
//			 for(unsigned int methodIdx=0; methodIdx<methodIdsSize; methodIdx++){
//				 pDexMethodId = dexGetMethodId(pDvmDex->pDexFile,methodIdx);
//				 curMethodName = dexStringById(pDvmDex->pDexFile, pDexMethodId->nameIdx);
//				//if(curMethodName) ALOGD("curMethodName = %s",curMethodName);
//				 if(curMethodName == NULL) return;
//				 if(strcmp(curMethodName,sensitiveMethodName) == 0){
//					// ((DexMethodId)(pDvmDex->pDexFile->pMethodIds[idx])).isReplaceable=1;
//					 //pDvmDex->pDexFile->pMethodIds[idx].isReplaceable=1;
//					 //sensitiveMethodIdx.push_back(methodIdx);
//					// ALOGD("start to fill the map classidx: %d, methodsIdx: %d, priority=%d",classIdx,methodIdx,priority );
//
//					// pDvmDex->class_method_priority_map[std::make_pair(classIdx,methodIdx)] = priority; //causes SEGV
//					//try dual map
//					 //pDvmDex->configMap[classIdx][methodIdx] = priority;
//					// configMap[classIdx][methodIdx] = priority;
//
//
//					 class_method_map curClassMethodMap;
//					 curClassMethodMap.insert( std::pair<u4,u4>(classIdx, methodIdx) );
//					 pconfigMap->insert( std::pair<u4,class_method_map>(priority,curClassMethodMap) );
//
//					// ALOGD("After mapping");
//
//				 }
//			 }
//		 }
//
//
//
//	}
//
//}


////endsalma
