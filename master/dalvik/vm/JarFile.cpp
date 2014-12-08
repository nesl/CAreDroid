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
 * Access the contents of a Jar file.
 *
 * This isn't actually concerned with any of the Jar-like elements; it
 * just wants a zip archive with "classes.dex" inside.  In Android the
 * most common example is ".apk".
 */

#include "Dalvik.h"
#include "libdex/OptInvocation.h"

#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <fcntl.h>
#include <errno.h>

//salma
//#include "../external/astl/include/stdio_filebuf.h"
//#include <ext/stdio_filebuf.h> //from libastl
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
using namespace std;
//endsalma


static const char* kDexInJarName = "classes.dex";
//salma//
static const char* ConfigInJarName = "assets/config.xml";
//endsalma
/*
 * Attempt to open a file whose name is similar to <fileName>,
 * but with the supplied suffix.  E.g.,
 * openAlternateSuffix("Home.apk", "dex", O_RDONLY) will attempt
 * to open "Home.dex".  If the open succeeds, a pointer to a
 * malloc()ed copy of the opened file name will be put in <*pCachedName>.
 *
 * <flags> is passed directly to open(). O_CREAT is not supported.
 */
static int openAlternateSuffix(const char *fileName, const char *suffix,
    int flags, char **pCachedName)
{
    char *buf, *c;
    size_t fileNameLen = strlen(fileName);
    size_t suffixLen = strlen(suffix);
    size_t bufLen = fileNameLen + suffixLen + 1;
    int fd = -1;

    buf = (char*)malloc(bufLen);
    if (buf == NULL) {
        errno = ENOMEM;
        return -1;
    }

    /* Copy the original filename into the buffer, find
     * the last dot, and copy the suffix to just after it.
     */
    memcpy(buf, fileName, fileNameLen + 1);
    c = strrchr(buf, '.');
    if (c == NULL) {
        errno = ENOENT;
        goto bail;
    }
    memcpy(c + 1, suffix, suffixLen + 1);

    fd = open(buf, flags);
    if (fd >= 0) {
        *pCachedName = buf;
        return fd;
    }
    ALOGV("Couldn't open %s: %s", buf, strerror(errno));
bail:
    free(buf);
    return -1;
}

/*
 * Checks the dependencies of the dex cache file corresponding
 * to the jar file at the absolute path "fileName".
 */
DexCacheStatus dvmDexCacheStatus(const char *fileName)
{
    ZipArchive archive;
    char* cachedName = NULL;
    int fd;
    DexCacheStatus result = DEX_CACHE_ERROR;
    ZipEntry entry;

    /* Always treat elements of the bootclasspath as up-to-date.
     * The fact that interpreted code is running at all means that this
     * should be true.
     */
    if (dvmClassPathContains(gDvm.bootClassPath, fileName)) {
        return DEX_CACHE_OK;
    }

    //TODO: match dvmJarFileOpen()'s logic.  Not super-important
    //      (the odex-first logic is only necessary for dexpreopt)
    //      but it would be nice to be consistent.

    /* Try to find the dex file inside of the archive.
     */
    if (dexZipOpenArchive(fileName, &archive) != 0) {
        return DEX_CACHE_BAD_ARCHIVE;
    }
    entry = dexZipFindEntry(&archive, kDexInJarName);
    if (entry != NULL) {
        bool newFile = false;

        /*
         * See if there's an up-to-date copy of the optimized dex
         * in the cache, but don't create one if there isn't.
         */
        ALOGV("dvmDexCacheStatus: Checking cache for %s", fileName);
        cachedName = dexOptGenerateCacheFileName(fileName, kDexInJarName);
        if (cachedName == NULL)
            return DEX_CACHE_BAD_ARCHIVE;

        fd = dvmOpenCachedDexFile(fileName, cachedName,
                dexGetZipEntryModTime(&archive, entry),
                dexGetZipEntryCrc32(&archive, entry),
                /*isBootstrap=*/false, &newFile, /*createIfMissing=*/false);
        ALOGV("dvmOpenCachedDexFile returned fd %d", fd);
        if (fd < 0) {
            result = DEX_CACHE_STALE;
            goto bail;
        }

        /* dvmOpenCachedDexFile locks the file as a side-effect.
         * Unlock and close it.
         */
        if (!dvmUnlockCachedDexFile(fd)) {
            /* uh oh -- this process needs to exit or we'll wedge the system */
            ALOGE("Unable to unlock DEX file");
            goto bail;
        }

        /* When createIfMissing is false, dvmOpenCachedDexFile() only
         * returns a valid fd if the cache file is up-to-date.
         */
    } else {
        /*
         * There's no dex file in the jar file.  See if there's an
         * optimized dex file living alongside the jar.
         */
        fd = openAlternateSuffix(fileName, "odex", O_RDONLY, &cachedName);
        if (fd < 0) {
            ALOGI("Zip is good, but no %s inside, and no .odex "
                    "file in the same directory", kDexInJarName);
            result = DEX_CACHE_BAD_ARCHIVE;
            goto bail;
        }

        ALOGV("Using alternate file (odex) for %s ...", fileName);
        if (!dvmCheckOptHeaderAndDependencies(fd, false, 0, 0, true, true)) {
            ALOGE("%s odex has stale dependencies", fileName);
            ALOGE("odex source not available -- failing");
            result = DEX_CACHE_STALE_ODEX;
            goto bail;
        } else {
            ALOGV("%s odex has good dependencies", fileName);
        }
    }
    result = DEX_CACHE_OK;

bail:
    dexZipCloseArchive(&archive);
    free(cachedName);
    if (fd >= 0) {
        close(fd);
    }
    return result;
}

/*
 * Open a Jar file.  It's okay if it's just a Zip archive without all of
 * the Jar trimmings, but we do insist on finding "classes.dex" inside
 * or an appropriately-named ".odex" file alongside.
 *
 * If "isBootstrap" is not set, the optimizer/verifier regards this DEX as
 * being part of a different class loader.
 */
int dvmJarFileOpen(const char* fileName, const char* odexOutputName,
    JarFile** ppJarFile, bool isBootstrap)
{
    /*
     * TODO: This function has been duplicated and modified to become
     * dvmRawDexFileOpen() in RawDexFile.c. This should be refactored.
     */
//salma
	//ALOGD("JarFile.cpp: dvmJarFileOpen %s", fileName);
//endsalma
    ZipArchive archive;
    DvmDex* pDvmDex = NULL;
    char* cachedName = NULL;
    bool archiveOpen = false;
    bool locked = false;
    int fd = -1;
    int result = -1;
//salma
    bool configresult = false; //for configfile
	DvmConfigFile* pDvmConfigFile = NULL;
	//DvmConfigMap* pDvmConfigMap; // priority only : cleaned
	//pDvmConfigMap =  dvmConfigFileGetMapAddress(); // global map // cleaned priority only
	DvmConfigMethodMap* pDvmConfigMethodMap;
	pDvmConfigMethodMap = dvmConfigFileGetMapMethodAddress(); //global map
 //endsalma
    /* Even if we're not going to look at the archive, we need to
     * open it so we can stuff it into ppJarFile.
     */
    if (dexZipOpenArchive(fileName, &archive) != 0)
        goto bail;
    archiveOpen = true;

    /* If we fork/exec into dexopt, don't let it inherit the archive's fd.
     */
    dvmSetCloseOnExec(dexZipGetArchiveFd(&archive));

    /* First, look for a ".odex" alongside the jar file.  It will
     * have the same name/path except for the extension.
     */
    fd = openAlternateSuffix(fileName, "odex", O_RDONLY, &cachedName);

    if (fd >= 0) {
        ALOGV("Using alternate file (odex) for %s ...", fileName);
        if (!dvmCheckOptHeaderAndDependencies(fd, false, 0, 0, true, true)) {
            ALOGE("%s odex has stale dependencies", fileName);
            free(cachedName);
            cachedName = NULL;
            close(fd);
            fd = -1;
            goto tryArchive;
        } else {
            ALOGV("%s odex has good dependencies", fileName);
            //TODO: make sure that the .odex actually corresponds
            //      to the classes.dex inside the archive (if present).
            //      For typical use there will be no classes.dex.
        }
    } else {
        ZipEntry entry;

tryArchive:
        /*
         * Pre-created .odex absent or stale.  Look inside the jar for a
         * "classes.dex".
         */
        entry = dexZipFindEntry(&archive, kDexInJarName);
        //salma
        //ALOGD("JarFile.cpp: KdexInJarName: %s", kDexInJarName);
        //endsalma
        if (entry != NULL) {
            bool newFile = false;

            /*
             * We've found the one we want.  See if there's an up-to-date copy
             * in the cache.
             *
             * On return, "fd" will be seeked just past the "opt" header.
             *
             * If a stale .odex file is present and classes.dex exists in
             * the archive, this will *not* return an fd pointing to the
             * .odex file; the fd will point into dalvik-cache like any
             * other jar.
             */
            if (odexOutputName == NULL) {
                cachedName = dexOptGenerateCacheFileName(fileName,
                                kDexInJarName);
                if (cachedName == NULL)
                    goto bail;
            } else {
                cachedName = strdup(odexOutputName);
            }
            ALOGV("dvmJarFileOpen: Checking cache for %s (%s)",
                fileName, cachedName);
            fd = dvmOpenCachedDexFile(fileName, cachedName,
                    dexGetZipEntryModTime(&archive, entry),
                    dexGetZipEntryCrc32(&archive, entry),
                    isBootstrap, &newFile, /*createIfMissing=*/true);
            if (fd < 0) {
                ALOGI("Unable to open or create cache for %s (%s)",
                    fileName, cachedName);
                goto bail;
            }
            locked = true;

            /*
             * If fd points to a new file (because there was no cached version,
             * or the cached version was stale), generate the optimized DEX.
             * The file descriptor returned is still locked, and is positioned
             * just past the optimization header.
             */
            if (newFile) {
                u8 startWhen, extractWhen, endWhen;
                bool result;
                off_t dexOffset;

                dexOffset = lseek(fd, 0, SEEK_CUR);
                result = (dexOffset > 0);

                if (result) {
                    startWhen = dvmGetRelativeTimeUsec();
                    result = dexZipExtractEntryToFile(&archive, entry, fd) == 0;
                    extractWhen = dvmGetRelativeTimeUsec();
                }
                if (result) {
                    result = dvmOptimizeDexFile(fd, dexOffset,
                                dexGetZipEntryUncompLen(&archive, entry),
                                fileName,
                                dexGetZipEntryModTime(&archive, entry),
                                dexGetZipEntryCrc32(&archive, entry),
                                isBootstrap);
                }

                if (!result) {
                    ALOGE("Unable to extract+optimize DEX from '%s'",
                        fileName);
                    goto bail;
                }

                endWhen = dvmGetRelativeTimeUsec();
                ALOGD("DEX prep '%s': unzip in %dms, rewrite %dms",
                    fileName,
                    (int) (extractWhen - startWhen) / 1000,
                    (int) (endWhen - extractWhen) / 1000);
            }
        } else {
            ALOGI("Zip is good, but no %s inside, and no valid .odex "
                    "file in the same directory", kDexInJarName);
            goto bail;
        }
    }

    //salma
    /* look for the config file inside the archive */
    /* assets/config.xml works with the emulator only!! */
     ZipEntry configentry;
  //   ALOGD("JarFile.cpp: Before Calling dexZipEntry for configfile");
     configentry = dexZipFindEntry(&archive, ConfigInJarName);
     if(configentry != NULL){
    	// __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp: configentry is not null");
         char* configcachedName = NULL;
    	 int fdconfig = -1;
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

         configcachedName = dexOptGenerateCacheFileName(fileName, "assets@config.xml");
        //__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp: cache name = %s", configcachedName);

        /* Form the cache directory of the configFile */
        lastPartStr = "/data/"+ lastPartStr + "/cache";
        //ALOGD("lastPartStr = %s", lastPartStr.c_str());
        string configcachedName2str = string(configcachedName);
        	//remove the directory /dalvik-cache from the cache file path
        configcachedName2str.replace(configcachedName2str.find("/dalvik-cache"), sizeof("/dalvik-cache")-1, lastPartStr);
//       __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp: cache name = %s", cachedName2str.c_str());

        /* Get a file descriptor for the name of the cache config file*/
       	fdconfig = open(configcachedName2str.c_str(), O_CREAT|O_RDWR, 0644);
//       __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG","open fdconfig %d", fdconfig);
         if (fdconfig < 0) {
        	 const std::string errnoString(strerror(errno));
             ALOGE("Can't open  cache file '%s': %s", configcachedName2str.c_str(), errnoString.c_str());
             fdconfig = open(configcachedName2str.c_str(), O_RDONLY, 0);
             if (fdconfig < 0){
                  const std::string errnoString(strerror(errno));
                  ALOGE("Can't open  cache file '%s': %s", configcachedName2str.c_str(), errnoString.c_str());
             }
         }
         else{ //fdconfig is captured
        	 /* Copy the contents of the config file into the cache file under the library cache folder */
        	 configresult = dexZipExtractEntryToFile(&archive, configentry, fdconfig) == 0;
        	 //success copy for the file
        	 //__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp: copy asset into cache file: result2: %d",result2);
        }
       	if(configresult){
       		pDvmConfigFile = dvmConfigFileParse(configcachedName2str.c_str());

       	}
     }
     else
     {
    	// __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp: configentry is NULL");

     }
     //endsalma

    /*
     * Map the cached version.  This immediately rewinds the fd, so it
     * doesn't have to be seeked anywhere in particular.
     */
    if (dvmDexFileOpenFromFd(fd, &pDvmDex) != 0) {
        ALOGI("Unable to map %s in %s", kDexInJarName, fileName);
        goto bail;
    }
//salma
    /* Assign the config file to the allocated pDvmDex*/
	if(pDvmConfigFile!=NULL){
		pDvmDex->isConfigFile = true;
		pDvmDex->pConfigFile = pDvmConfigFile;
//DEBUG
		//dvmConfigFileDebug(pDvmDex->pConfigFile);
		//Set auxiliary parameters foe sensitive methods;
		if(pDvmDex->pDexFile != NULL) {
			// Map Priority only
			//dvmDexConfigMap(pDvmDex, pDvmConfigMap);
			//pDvmDex->pconfigMap =  pDvmConfigMap; //commented out in structure DvmDex
			// Map all method param
			dvmDexConfigMethodMap(pDvmDex, pDvmConfigMethodMap);
			pDvmDex->pconfigMethodMap =  pDvmConfigMethodMap;
		}


	}
//endsalma

    if (locked) {
        /* unlock the fd */
        if (!dvmUnlockCachedDexFile(fd)) {
            /* uh oh -- this process needs to exit or we'll wedge the system */
            ALOGE("Unable to unlock DEX file");
            goto bail;
        }
        locked = false;
    }

   // ALOGV("Successfully opened '%s' in '%s'", kDexInJarName, fileName);
    //salma
   // __android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG", "JarFile.cpp:Successfully opened '%s' in '%s'", kDexInJarName, fileName);
    //endsalma
    *ppJarFile = (JarFile*) calloc(1, sizeof(JarFile));
    (*ppJarFile)->archive = archive;
    (*ppJarFile)->cacheFileName = cachedName;
    (*ppJarFile)->pDvmDex = pDvmDex;
    cachedName = NULL;      // don't free it below
    result = 0;

bail:
    /* clean up, closing the open file */
    if (archiveOpen && result != 0)
        dexZipCloseArchive(&archive);
    free(cachedName);
    if (fd >= 0) {
        if (locked)
            (void) dvmUnlockCachedDexFile(fd);
        close(fd);
    }
    return result;
}

/*
 * Close a Jar file and free the struct.
 */
void dvmJarFileFree(JarFile* pJarFile)
{
    if (pJarFile == NULL)
        return;

    dvmDexFileFree(pJarFile->pDvmDex);
    dexZipCloseArchive(&pJarFile->archive);
    free(pJarFile->cacheFileName);
    free(pJarFile);
}
