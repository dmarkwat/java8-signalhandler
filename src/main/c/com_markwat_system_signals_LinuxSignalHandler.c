#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "com_markwat_system_signals_LinuxSignalHandler.h"

typedef struct cell list;

struct cell {
    jobject obj;
    list *head;
};

typedef struct chained sigchain;

struct chained {
    list *chain;
    struct sigaction *oldaction;
};

// index 0 unused--pay 1 jobject pointer to save on off-by-one errors? yes, please
sigchain *handlers [ 32 ] = { NULL };

JavaVM *jvm;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;

    // setup global refs

    return JNI_VERSION_1_8;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if ((*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL) != 0) {
        return;
    }

    // delete global refs

    for (int k = 0; k < 32; k++) {
        sigchain *next = handlers[k];
        if (next == NULL) continue;

        list *c = next->chain;
        while (c != NULL) {
            if (c->obj != NULL) (*env)->DeleteGlobalRef(env, c->obj);
            c = c->head;
        }
    }
}

void handle(int signo, siginfo_t *info, void *ucontext) {
    if (handlers[signo] == NULL) return;

    sigchain *next = handlers[signo];
    list *c = next->chain;

    JNIEnv *env;
    if ((*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL) != 0) {
        return;
    }

    while (c != NULL) {
        char *sigcode_field_name;
        switch (info->si_code) {
            case SI_USER:
                sigcode_field_name = "USER";
                break;
            case SI_KERNEL:
                sigcode_field_name = "KERNEL";
                break;
            case SI_QUEUE:
                sigcode_field_name = "QUEUE";
                break;
            case SI_TKILL:
                sigcode_field_name = "TKILL";
                break;
            default:
                sigcode_field_name = "UNKNOWN";
        }

        jthrowable exc;

        jclass sigcode_clazz = (*env)->FindClass(env, "com/markwat/system/signals/LinuxSigCode");
        jfieldID sigcode_field = (*env)->GetStaticFieldID(env, sigcode_clazz, sigcode_field_name, "Lcom/markwat/system/signals/LinuxSigCode;");
        jobject sigcode_obj = (*env)->GetStaticObjectField(env, sigcode_clazz, sigcode_field);
        exc = (*env)->ExceptionOccurred(env);
        if (exc != NULL) (*env)->FatalError(env, "oops on sigcode");

        jclass signo_clazz = (*env)->FindClass(env, "com/markwat/system/signals/LinuxSigNo");
        jmethodID signo_value_of = (*env)->GetStaticMethodID(env, signo_clazz, "valueOf", "(I)Lcom/markwat/system/signals/LinuxSigNo;");
        jobject signo_obj = (*env)->CallStaticObjectMethod(env, signo_clazz, signo_value_of, signo);
        exc = (*env)->ExceptionOccurred(env);
        if (exc != NULL) (*env)->FatalError(env, "oops on signo");

        jclass siginfo_clazz = (*env)->FindClass(env, "com/markwat/system/signals/LinuxSigInfo");
        jmethodID siginfo_init = (*env)->GetMethodID(env, siginfo_clazz, "<init>", "(Lcom/markwat/system/signals/LinuxSigCode;Lcom/markwat/system/signals/LinuxSigNo;)V");

        jobject siginfo_obj = (*env)->NewObject(env, siginfo_clazz, siginfo_init, sigcode_obj, signo_obj);
        exc = (*env)->ExceptionOccurred(env);
        if (exc != NULL) (*env)->FatalError(env, "oops on new object");

        jclass handle_clazz = (*env)->FindClass(env, "com/markwat/system/signals/SignalHandler");
        jmethodID handle_fn = (*env)->GetMethodID(env, handle_clazz, "handle", "(Lcom/markwat/system/signals/LinuxSigInfo;)V");

        // todo handle errors better
        (*env)->CallVoidMethod(env, c->obj, handle_fn, siginfo_obj);
        exc = (*env)->ExceptionOccurred(env);
        (*env)->ExceptionClear(env);

        c = c->head;
    }

    if ((*jvm)->DetachCurrentThread(jvm) != JNI_OK) {
        // todo ???
    }

    // call the chained sa_sigaction or sa_handler (man7 suggests both shouldn't be set)
    // http://man7.org/linux/man-pages/man2/sigaction.2.html
    if (next->oldaction != NULL && next->oldaction->sa_handler != NULL) {
        next->oldaction->sa_handler(signo);
    } else if (next->oldaction != NULL && next->oldaction->sa_sigaction != NULL) {
        next->oldaction->sa_sigaction(signo, info, ucontext);
    }
}

JNIEXPORT void JNICALL Java_com_markwat_system_signals_LinuxSignalHandler_handle
  (JNIEnv *env, jclass clazz, jobject j_signo, jobject j_handler) {

    jclass signo_clazz = (*env)->GetObjectClass(env, j_signo);
    jmethodID get_signo = (*env)->GetMethodID(env, signo_clazz, "getSigno", "()I");

    jint signo = (*env)->CallIntMethod(env, j_signo, get_signo);
    jthrowable exc = (*env)->ExceptionOccurred(env);
    if (exc != NULL) (*env)->FatalError(env, "error getting signo");

    if (signo > 31) {
        // todo throw better exception
        jclass oom_exc_clazz = (*env)->FindClass(env, "java/lang/RuntimeException");
        (*env)->ThrowNew(env, oom_exc_clazz, "don't support realtime signals");
    }

    if (handlers[signo] == NULL) {
        sigchain *chain_link = malloc(sizeof(sigchain));
        struct sigaction *act = malloc(sizeof(struct sigaction));
        struct sigaction *oldact = calloc(1, sizeof(struct sigaction));

        if (chain_link == NULL || act == NULL || oldact == NULL) {
            if (chain_link != NULL) free(chain_link);
            if (act != NULL) free(act);
            if (oldact != NULL) free(oldact);

            jclass oom_exc_clazz = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
            (*env)->ThrowNew(env, oom_exc_clazz, "couldn't allocate signal chain or handlers");
        }

        act->sa_sigaction = &handle;
        sigemptyset(&(act->sa_mask));
        act->sa_flags = SA_SIGINFO | SA_RESTART;

        if (0 != sigaction(signo, act, oldact)) {
            free(act);
            free(chain_link);
            free(oldact);

            if (errno == EINVAL) {
                // An invalid signal was specified.  This will also be generated if an attempt is made to change the action for SIGKILL or SIGSTOP, which cannot be caught or ignored.
                // todo throw better exception
                jclass oom_exc_clazz = (*env)->FindClass(env, "java/lang/RuntimeException");
                (*env)->ThrowNew(env, oom_exc_clazz, "invalid signal specified");
            } else if (errno == EFAULT) {
                // act or oldact points to memory which is not a valid part of the process address space.
                (*env)->FatalError(env, "one of the signal handlers refers to bad address space (EFAULT)");
            } else {
                (*env)->FatalError(env, "unspecified error calling sigaction");
            }
        } else {
            // start the chain with NULL
            chain_link->chain = NULL;
            chain_link->oldaction = oldact;
            handlers[signo] = chain_link;
        }
    }

    list *next_cell = malloc(sizeof(list));
    if (next_cell == NULL) {
        jclass oom_exc_clazz = (*env)->FindClass(env, "java/lang/OutOfMemoryError");
        (*env)->ThrowNew(env, oom_exc_clazz, "couldn't allocate list cell");
    }

    next_cell->obj = (*env)->NewGlobalRef(env, j_handler);
    next_cell->head = handlers[signo]->chain;

    handlers[signo]->chain = next_cell;
}

