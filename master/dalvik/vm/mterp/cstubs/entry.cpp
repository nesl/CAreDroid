/*
 * Handler function table, one entry per opcode.
 */
#undef H
#define H(_op) (const void*) dvmMterp_##_op
DEFINE_GOTO_TABLE(gDvmMterpHandlers)

#undef H
#define H(_op) #_op
DEFINE_GOTO_TABLE(gDvmMterpHandlerNames)

#include <setjmp.h>

//salma
//#define LOG_NDEBUG 0
//#include "jni.h"
////fwd
//static jclass findClass(JNIEnv* env, const char name[]);
//static jmethodID findMethod(JNIEnv* env, jclass c, const char method[],
//                                  const char params[]);
//static char* toSlashClassName(const char* className);
//static void  locateResource();
/*
 * C mterp entry point.  This just calls the various C fallbacks, making
 * this a slow but portable interpeter.
 *
 * This is only used for the "allstubs" variant.
 */
void dvmMterpStdRun(Thread* self)
{
	//salma
	//__android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG","entry.cpp/dvmMterpStdRun threadid=%d : %s.%s",
    //dvmThreadSelf()->threadId,
    //self->interpSave.method->clazz->descriptor,
   // self->interpSave.method->name);
	_//_android_log_print(ANDROID_LOG_DEBUG, "DVM DEBUG","Salma: Inside entry.cpp/dvmMterpStdRun");

	//endsalma

	jmp_buf jmpBuf;

    self->interpSave.bailPtr = &jmpBuf;

    /* We exit via a longjmp */
    if (setjmp(jmpBuf)) {
        LOGVV("mterp threadid=%d returning", dvmThreadSelf()->threadId);
        return;
    }

    /* run until somebody longjmp()s out */
    while (true) {
        typedef void (*Handler)(Thread* self);

        u2 inst = /*self->interpSave.*/pc[0];
        /*
         * In mterp, dvmCheckBefore is handled via the altHandlerTable,
         * while in the portable interpreter it is part of the handler
         * FINISH code.  For allstubs, we must do an explicit check
         * in the interpretation loop.
         */
        if (self->interpBreak.ctl.subMode) {
            dvmCheckBefore(pc, fp, self);
        }
        Handler handler = (Handler) gDvmMterpHandlers[inst & 0xff];
        (void) gDvmMterpHandlerNames;   /* avoid gcc "defined but not used" */
        LOGVV("handler %p %s",
            handler, (const char*) gDvmMterpHandlerNames[inst & 0xff]);
        (*handler)(self);
        //salma
    //    locateResource();
        //endsalma
    }
}

/*
 * C mterp exit point.  Call here to bail out of the interpreter.
 */
void dvmMterpStdBail(Thread* self)
{
    jmp_buf* pJmpBuf = (jmp_buf*) self->interpSave.bailPtr;
    longjmp(*pJmpBuf, 1);
}

