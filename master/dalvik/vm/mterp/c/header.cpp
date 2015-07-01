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

/* common includes */
#include "Dalvik.h"
#include "interp/InterpDefs.h"
#include "mterp/Mterp.h"
#include <math.h>                   // needed for fmod, fmodf
#include "mterp/common/FindInterface.h"
//salma
#include "jni.h"
#include <unistd.h>
#include <dirent.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include "DvmSystemCoreValues.h"
#include "DvmSystemSwitchDecisionTree.h"
#include <ctime>
using namespace std;

//endsalma
/*
 * Configuration defines.  These affect the C implementations, i.e. the
 * portable interpreter(s) and C stubs.
 *
 * Some defines are controlled by the Makefile, e.g.:
 *   WITH_INSTR_CHECKS
 *   WITH_TRACKREF_CHECKS
 *   EASY_GDB
 *   NDEBUG
 */
//salma
//#define SWITCH_ENABLED 1 //now it is a globally defined in the CoreSystemValue Files

//#define POWER_SUPPLY_PATH "/sys/class/power_supply"
//#define PATH_MAX 4096
//fwd
//static jclass findClass(JNIEnv* env, const char name[]);
//static jmethodID findMethod(JNIEnv* env, jclass c, const char method[],
//                                  const char params[]);
//static char* toSlashClassName(const char* className);
//static inline void locateResource(Thread* self);
//endsalma


#ifdef WITH_INSTR_CHECKS            /* instruction-level paranoia (slow!) */
# define CHECK_BRANCH_OFFSETS
# define CHECK_REGISTER_INDICES
#endif

/*
 * Some architectures require 64-bit alignment for access to 64-bit data
 * types.  We can't just use pointers to copy 64-bit values out of our
 * interpreted register set, because gcc may assume the pointer target is
 * aligned and generate invalid code.
 *
 * There are two common approaches:
 *  (1) Use a union that defines a 32-bit pair and a 64-bit value.
 *  (2) Call memcpy().
 *
 * Depending upon what compiler you're using and what options are specified,
 * one may be faster than the other.  For example, the compiler might
 * convert a memcpy() of 8 bytes into a series of instructions and omit
 * the call.  The union version could cause some strange side-effects,
 * e.g. for a while ARM gcc thought it needed separate storage for each
 * inlined instance, and generated instructions to zero out ~700 bytes of
 * stack space at the top of the interpreter.
 *
 * The default is to use memcpy().  The current gcc for ARM seems to do
 * better with the union.
 */
#if defined(__ARM_EABI__)
# define NO_UNALIGN_64__UNION
#endif
/*
 * MIPS ABI requires 64-bit alignment for access to 64-bit data types.
 *
 * Use memcpy() to do the transfer
 */
#if defined(__mips__)
/* # define NO_UNALIGN_64__UNION */
#endif


//#define LOG_INSTR                   /* verbose debugging */
/* set and adjust ANDROID_LOG_TAGS='*:i jdwp:i dalvikvm:i dalvikvmi:i' */

/*
 * Export another copy of the PC on every instruction; this is largely
 * redundant with EXPORT_PC and the debugger code.  This value can be
 * compared against what we have stored on the stack with EXPORT_PC to
 * help ensure that we aren't missing any export calls.
 */
#if WITH_EXTRA_GC_CHECKS > 1
# define EXPORT_EXTRA_PC() (self->currentPc2 = pc)
#else
# define EXPORT_EXTRA_PC()
#endif

/*
 * Adjust the program counter.  "_offset" is a signed int, in 16-bit units.
 *
 * Assumes the existence of "const u2* pc" and "const u2* curMethod->insns".
 *
 * We don't advance the program counter until we finish an instruction or
 * branch, because we do want to have to unroll the PC if there's an
 * exception.
 */
#ifdef CHECK_BRANCH_OFFSETS
# define ADJUST_PC(_offset) do {                                            \
        int myoff = _offset;        /* deref only once */                   \
        if (pc + myoff < curMethod->insns ||                                \
            pc + myoff >= curMethod->insns + dvmGetMethodInsnsSize(curMethod)) \
        {                                                                   \
            char* desc;                                                     \
            desc = dexProtoCopyMethodDescriptor(&curMethod->prototype);     \
            ALOGE("Invalid branch %d at 0x%04x in %s.%s %s",                 \
                myoff, (int) (pc - curMethod->insns),                       \
                curMethod->clazz->descriptor, curMethod->name, desc);       \
            free(desc);                                                     \
            dvmAbort();                                                     \
        }                                                                   \
        pc += myoff;                                                        \
        EXPORT_EXTRA_PC();                                                  \
    } while (false)
#else
# define ADJUST_PC(_offset) do {                                            \
        pc += _offset;                                                      \
        EXPORT_EXTRA_PC();                                                  \
    } while (false)
#endif

/*
 * If enabled, log instructions as we execute them.
 */
#ifdef LOG_INSTR
# define ILOGD(...) ILOG(LOG_DEBUG, __VA_ARGS__)
# define ILOGV(...) ILOG(LOG_VERBOSE, __VA_ARGS__)
# define ILOG(_level, ...) do {                                             \
        char debugStrBuf[128];                                              \
        snprintf(debugStrBuf, sizeof(debugStrBuf), __VA_ARGS__);            \
        if (curMethod != NULL)                                              \
            ALOG(_level, LOG_TAG"i", "%-2d|%04x%s",                          \
                self->threadId, (int)(pc - curMethod->insns), debugStrBuf); \
        else                                                                \
            ALOG(_level, LOG_TAG"i", "%-2d|####%s",                          \
                self->threadId, debugStrBuf);                               \
    } while(false)
void dvmDumpRegs(const Method* method, const u4* framePtr, bool inOnly);
# define DUMP_REGS(_meth, _frame, _inOnly) dvmDumpRegs(_meth, _frame, _inOnly)
static const char kSpacing[] = "            ";
#else
# define ILOGD(...) ((void)0)
# define ILOGV(...) ((void)0)
# define DUMP_REGS(_meth, _frame, _inOnly) ((void)0)
#endif

/* get a long from an array of u4 */
static inline s8 getLongFromArray(const u4* ptr, int idx)
{
#if defined(NO_UNALIGN_64__UNION)
    union { s8 ll; u4 parts[2]; } conv;

    ptr += idx;
    conv.parts[0] = ptr[0];
    conv.parts[1] = ptr[1];
    return conv.ll;
#else
    s8 val;
    memcpy(&val, &ptr[idx], 8);
    return val;
#endif
}

/* store a long into an array of u4 */
static inline void putLongToArray(u4* ptr, int idx, s8 val)
{
#if defined(NO_UNALIGN_64__UNION)
    union { s8 ll; u4 parts[2]; } conv;

    ptr += idx;
    conv.ll = val;
    ptr[0] = conv.parts[0];
    ptr[1] = conv.parts[1];
#else
    memcpy(&ptr[idx], &val, 8);
#endif
}

/* get a double from an array of u4 */
static inline double getDoubleFromArray(const u4* ptr, int idx)
{
#if defined(NO_UNALIGN_64__UNION)
    union { double d; u4 parts[2]; } conv;

    ptr += idx;
    conv.parts[0] = ptr[0];
    conv.parts[1] = ptr[1];
    return conv.d;
#else
    double dval;
    memcpy(&dval, &ptr[idx], 8);
    return dval;
#endif
}

/* store a double into an array of u4 */
static inline void putDoubleToArray(u4* ptr, int idx, double dval)
{
#if defined(NO_UNALIGN_64__UNION)
    union { double d; u4 parts[2]; } conv;

    ptr += idx;
    conv.d = dval;
    ptr[0] = conv.parts[0];
    ptr[1] = conv.parts[1];
#else
    memcpy(&ptr[idx], &dval, 8);
#endif
}

/*
 * If enabled, validate the register number on every access.  Otherwise,
 * just do an array access.
 *
 * Assumes the existence of "u4* fp".
 *
 * "_idx" may be referenced more than once.
 */
#ifdef CHECK_REGISTER_INDICES
# define GET_REGISTER(_idx) \
    ( (_idx) < curMethod->registersSize ? \
        (fp[(_idx)]) : (assert(!"bad reg"),1969) )
# define SET_REGISTER(_idx, _val) \
    ( (_idx) < curMethod->registersSize ? \
        (fp[(_idx)] = (u4)(_val)) : (assert(!"bad reg"),1969) )
# define GET_REGISTER_AS_OBJECT(_idx)       ((Object *)GET_REGISTER(_idx))
# define SET_REGISTER_AS_OBJECT(_idx, _val) SET_REGISTER(_idx, (s4)_val)
# define GET_REGISTER_INT(_idx) ((s4) GET_REGISTER(_idx))
# define SET_REGISTER_INT(_idx, _val) SET_REGISTER(_idx, (s4)_val)
# define GET_REGISTER_WIDE(_idx) \
    ( (_idx) < curMethod->registersSize-1 ? \
        getLongFromArray(fp, (_idx)) : (assert(!"bad reg"),1969) )
# define SET_REGISTER_WIDE(_idx, _val) \
    ( (_idx) < curMethod->registersSize-1 ? \
        (void)putLongToArray(fp, (_idx), (_val)) : assert(!"bad reg") )
# define GET_REGISTER_FLOAT(_idx) \
    ( (_idx) < curMethod->registersSize ? \
        (*((float*) &fp[(_idx)])) : (assert(!"bad reg"),1969.0f) )
# define SET_REGISTER_FLOAT(_idx, _val) \
    ( (_idx) < curMethod->registersSize ? \
        (*((float*) &fp[(_idx)]) = (_val)) : (assert(!"bad reg"),1969.0f) )
# define GET_REGISTER_DOUBLE(_idx) \
    ( (_idx) < curMethod->registersSize-1 ? \
        getDoubleFromArray(fp, (_idx)) : (assert(!"bad reg"),1969.0) )
# define SET_REGISTER_DOUBLE(_idx, _val) \
    ( (_idx) < curMethod->registersSize-1 ? \
        (void)putDoubleToArray(fp, (_idx), (_val)) : assert(!"bad reg") )
#else
# define GET_REGISTER(_idx)                 (fp[(_idx)])
# define SET_REGISTER(_idx, _val)           (fp[(_idx)] = (_val))
# define GET_REGISTER_AS_OBJECT(_idx)       ((Object*) fp[(_idx)])
# define SET_REGISTER_AS_OBJECT(_idx, _val) (fp[(_idx)] = (u4)(_val))
# define GET_REGISTER_INT(_idx)             ((s4)GET_REGISTER(_idx))
# define SET_REGISTER_INT(_idx, _val)       SET_REGISTER(_idx, (s4)_val)
# define GET_REGISTER_WIDE(_idx)            getLongFromArray(fp, (_idx))
# define SET_REGISTER_WIDE(_idx, _val)      putLongToArray(fp, (_idx), (_val))
# define GET_REGISTER_FLOAT(_idx)           (*((float*) &fp[(_idx)]))
# define SET_REGISTER_FLOAT(_idx, _val)     (*((float*) &fp[(_idx)]) = (_val))
# define GET_REGISTER_DOUBLE(_idx)          getDoubleFromArray(fp, (_idx))
# define SET_REGISTER_DOUBLE(_idx, _val)    putDoubleToArray(fp, (_idx), (_val))
#endif

/*
 * Get 16 bits from the specified offset of the program counter.  We always
 * want to load 16 bits at a time from the instruction stream -- it's more
 * efficient than 8 and won't have the alignment problems that 32 might.
 *
 * Assumes existence of "const u2* pc".
 */
#define FETCH(_offset)     (pc[(_offset)])

/*
 * Extract instruction byte from 16-bit fetch (_inst is a u2).
 */
#define INST_INST(_inst)    ((_inst) & 0xff)

/*
 * Replace the opcode (used when handling breakpoints).  _opcode is a u1.
 */
#define INST_REPLACE_OP(_inst, _opcode) (((_inst) & 0xff00) | _opcode)

/*
 * Extract the "vA, vB" 4-bit registers from the instruction word (_inst is u2).
 */
#define INST_A(_inst)       (((_inst) >> 8) & 0x0f)
#define INST_B(_inst)       ((_inst) >> 12)

/*
 * Get the 8-bit "vAA" 8-bit register index from the instruction word.
 * (_inst is u2)
 */
#define INST_AA(_inst)      ((_inst) >> 8)

/*
 * The current PC must be available to Throwable constructors, e.g.
 * those created by the various exception throw routines, so that the
 * exception stack trace can be generated correctly.  If we don't do this,
 * the offset within the current method won't be shown correctly.  See the
 * notes in Exception.c.
 *
 * This is also used to determine the address for precise GC.
 *
 * Assumes existence of "u4* fp" and "const u2* pc".
 */
#define EXPORT_PC()         (SAVEAREA_FROM_FP(fp)->xtra.currentPc = pc)

/*
 * Check to see if "obj" is NULL.  If so, throw an exception.  Assumes the
 * pc has already been exported to the stack.
 *
 * Perform additional checks on debug builds.
 *
 * Use this to check for NULL when the instruction handler calls into
 * something that could throw an exception (so we have already called
 * EXPORT_PC at the top).
 */
static inline bool checkForNull(Object* obj)
{
    if (obj == NULL) {
        dvmThrowNullPointerException(NULL);
        return false;
    }
#ifdef WITH_EXTRA_OBJECT_VALIDATION
    if (!dvmIsHeapAddress(obj)) {
        ALOGE("Invalid object %p", obj);
        dvmAbort();
    }
#endif
#ifndef NDEBUG
    if (obj->clazz == NULL || ((u4) obj->clazz) <= 65536) {
        /* probable heap corruption */
        ALOGE("Invalid object class %p (in %p)", obj->clazz, obj);
        dvmAbort();
    }
#endif
    return true;
}

/*
 * Check to see if "obj" is NULL.  If so, export the PC into the stack
 * frame and throw an exception.
 *
 * Perform additional checks on debug builds.
 *
 * Use this to check for NULL when the instruction handler doesn't do
 * anything else that can throw an exception.
 */
static inline bool checkForNullExportPC(Object* obj, u4* fp, const u2* pc)
{
    if (obj == NULL) {
        EXPORT_PC();
        dvmThrowNullPointerException(NULL);
        return false;
    }
#ifdef WITH_EXTRA_OBJECT_VALIDATION
    if (!dvmIsHeapAddress(obj)) {
        ALOGE("Invalid object %p", obj);
        dvmAbort();
    }
#endif
#ifndef NDEBUG
    if (obj->clazz == NULL || ((u4) obj->clazz) <= 65536) {
        /* probable heap corruption */
        ALOGE("Invalid object class %p (in %p)", obj->clazz, obj);
        dvmAbort();
    }
#endif
    return true;
}

////salma
//static inline int readFromFile(const char* path, char* buf, size_t size)
//{
//    if (!path)
//        return -1;
//    int fd = open(path, O_RDONLY, 0);
//    if (fd == -1) {
//        ALOGE("Could not open '%s'", path);
//        return -1;
//    }
//
//    ssize_t count = read(fd, buf, size);
//    if (count > 0) {
//        while (count > 0 && buf[count-1] == '\n')
//            count--;
//        buf[count] = '\0';
//    } else {
//        buf[0] = '\0';
//    }
//
//    close(fd);
//    return count;
//}
//
//static inline int get_battery_level(JNIEnv* env, char* buf)
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
//            				length = readFromFile("/sys/class/power_supply/battery/capacity", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            					ALOGD("battery capacity = %s", buf);
//            					closedir(dir);
//            					return 1;
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"type")){
//            				length = readFromFile("/sys/class/power_supply/battery/type", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            					ALOGD("battery type = %s", buf);
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"power")){
//            				length = readFromFile("/sys/class/power_supply/battery/power", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            					ALOGD("battery power = %s", buf);
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"status")){
//            				length = readFromFile("/sys/class/power_supply/battery/status", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            					ALOGD("battery status = %s", buf);
//            				}
//            			}
//            			else if(strcmp(nameinsidebatt,"health")){
//            				length = readFromFile("/sys/class/power_supply/battery/health", buf, sizeof(buf));
//            				if(length > 0){
//            					if (buf[length - 1] == '\n')
//            						buf[length - 1] = 0;
//            					ALOGD("battery health = %s", buf);
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
//
//static inline int get_battery_service(JNIEnv* env, char* batterycap)
//{
//    char    path[PATH_MAX];
//    struct dirent* entry;
//    int success = 0;
//
//    DIR* dir = opendir(POWER_SUPPLY_PATH);
//    if (dir == NULL) {
//        ALOGE("Could not open %s\n", POWER_SUPPLY_PATH);
//    } else {
//        while ((entry = readdir(dir))) {
//            const char* name = entry->d_name;
//
//            // ignore "." and ".."
//            if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
//                continue;
//            }
//
//            char buf[20];
//            // Look for "type" file in each subdirectory
//            snprintf(path, sizeof(path), "%s/%s/type", POWER_SUPPLY_PATH, name);
//            int length = readFromFile(path, buf, sizeof(buf));
//            if (length > 0) {
//                if (buf[length - 1] == '\n')
//                    buf[length - 1] = 0;
//
//                if (strcmp(buf, "Mains") == 0) {
//                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){}
//                     //   gPaths.acOnlinePath = strdup(path);
//                }
//                else if (strcmp(buf, "USB") == 0) {
//                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){}
//                      //  gPaths.usbOnlinePath = strdup(path);
//                }
//                else if (strcmp(buf, "Wireless") == 0) {
//                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){}
//                       // gPaths.wirelessOnlinePath = strdup(path);
//                }
//                else if (strcmp(buf, "Battery") == 0) {
//                    snprintf(path, sizeof(path), "%s/%s/status", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){}
//                       // gPaths.batteryStatusPath = strdup(path);
//                    snprintf(path, sizeof(path), "%s/%s/health", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){}
//                       // gPaths.batteryHealthPath = strdup(path);
//                    snprintf(path, sizeof(path), "%s/%s/present", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){}
//                       // gPaths.batteryPresentPath = strdup(path);
//                    snprintf(path, sizeof(path), "%s/%s/capacity", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){
//                    	int length = readFromFile(path,batterycap,sizeof(batterycap));
//                    	if (length > 0){
//                    		ALOGD("batterycap: %s",batterycap);
//                    		success = 1;
//                    	}
//                    }
//                       // gPaths.batteryCapacityPath = strdup(path);
//
//                    snprintf(path, sizeof(path), "%s/%s/voltage_now", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0) {
//                       // gPaths.batteryVoltagePath = strdup(path);
//                       // // voltage_now is in microvolts, not millivolts
//                       // gVoltageDivisor = 1000;
//                    } else {
//                        snprintf(path, sizeof(path), "%s/%s/batt_vol", POWER_SUPPLY_PATH, name);
//                        if (access(path, R_OK) == 0){}
//                          //  gPaths.batteryVoltagePath = strdup(path);
//                    }
//
//                    snprintf(path, sizeof(path), "%s/%s/temp", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0) {
//                       // gPaths.batteryTemperaturePath = strdup(path);
//                    } else {
//                        snprintf(path, sizeof(path), "%s/%s/batt_temp", POWER_SUPPLY_PATH, name);
//                        if (access(path, R_OK) == 0){}
//                         //   gPaths.batteryTemperaturePath = strdup(path);
//                    }
//
//                    snprintf(path, sizeof(path), "%s/%s/technology", POWER_SUPPLY_PATH, name);
//                    if (access(path, R_OK) == 0){}
//                        //gPaths.batteryTechnologyPath = strdup(path);
//                }
//            }
//        }
//        closedir(dir);
//    }
//    return success;
//}

//
////salma
//static inline void  locateResource(Thread* self){
//	ALOGD("mterp/c/header. cpp: inside locateResource");
//	jclass assetManagerClass;
//	jmethodID loadResourceValueID;
//
//	char* slashClassName = toSlashClassName("android.content.res.AssetManager");
//	assetManagerClass = findClass(self->jniEnv, slashClassName);
//	assert (assetManagerClass != NULL);
//	if (assetManagerClass == NULL) {
//        ALOGE("JavaVM unable to locate class '%s'\n", slashClassName);
//        /* keep going */
//    } else {
//        loadResourceValueID = findMethod(self->jniEnv, assetManagerClass, "loadResourceValue",
//        		"(ISLandroid/util/TypedValue;Z)I");
//        if (loadResourceValueID == NULL) {
//            ALOGE("JavaVM unable to find loadResourceValueID in '%s'\n", slashClassName);
//            /* keep going */
//        } else {
//            //self->jniEnv->CallStaticVoidMethod(startClass, startMeth, strArray);
//          }
//    }
//    free(slashClassName);
//}
//
//static jclass findClass(JNIEnv* env, const char name[]) {
//   jclass c = env->FindClass(name);
//   LOG_FATAL_IF(!c, "Unable to find class %s", name);
//   return c;
//}
//
//static jmethodID findMethod(JNIEnv* env, jclass c, const char method[],
//                                     const char params[]) {
//    jmethodID m = env->GetMethodID(c, method, params);
//    LOG_FATAL_IF(!m, "Unable to find method %s", method);
//    return m;
//}
//
//static char* toSlashClassName(const char* className){
//    char* result = strdup(className);
//    for (char* cp = result; *cp != '\0'; cp++) {
//            if (*cp == '.') {
//                *cp = '/';
//            }
//        }
//        return result;
//}
////endsalma
