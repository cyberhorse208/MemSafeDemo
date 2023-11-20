#include <jni.h>
#include <string>

#include <sys/prctl.h>

#include <sys/types.h>
#include "mte_header.h"
#include "ubsan-demo.h"

#include "log.h"



int getMTELevel(){
    int tagged_addr_ctrl = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
    if (tagged_addr_ctrl < 0) {
        return -1;
    }
    tagged_addr_ctrl = (tagged_addr_ctrl & PR_MTE_TCF_MASK);
    return tagged_addr_ctrl;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_memsafe_demo_MainActivity_getMteLevel(
        JNIEnv* env,
        jobject /* this */) {
    int tagged_addr_ctrl = getMTELevel();
    if(tagged_addr_ctrl == PR_MTE_TCF_SYNC)
    {
        return env->NewStringUTF("SYNC");
    }
    if(tagged_addr_ctrl == PR_MTE_TCF_ASYNC)
    {
        return env->NewStringUTF("ASYNC");
    }
    if(tagged_addr_ctrl == PR_MTE_TCF_NONE)
    {
        return env->NewStringUTF("NONE");
    }
    return env->NewStringUTF("UNKNOWN");
}


int buff_overflow(){
    char *tmp = (char *)malloc(3);
    return tmp[30];
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_memsafe_demo_MainActivity_stringFromJNIOOB(
        JNIEnv* env,
        jobject /* this */) {
    int tagLevel = getMTELevel();
    char tmp[1024];
    sprintf(tmp, "app use getprogname at %p\n", getprogname);
    std::string result =std::string (tmp) +  "in " + std::string(getprogname()) + ", current mte level:";
    if(tagLevel == PR_MTE_TCF_NONE){
        result += "none, no mte error will find, no tombstone";
    } else if(tagLevel == PR_MTE_TCF_ASYNC){
        result += "async, no mte error this time";
    } else if(tagLevel == PR_MTE_TCF_SYNC){
        result += "sync, mte will trigger error and generate tombstone";
    } else {
        result += "unknown, no mte error will find, no tombstone";
    }
    int i = buff_overflow();
    return env->NewStringUTF((result + "\naccess 30, bound 3, result: " + std::to_string(i)).c_str());
}


int use_after_free(){
    char *tmp = (char *)malloc(3);
    tmp[1] = 'a';
    free(tmp);
    return tmp[2];
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_memsafe_demo_MainActivity_stringFromJNIUAF(
        JNIEnv* env,
        jobject /* this */) {
    int tagLevel = getMTELevel();
    std::string result = "in " + std::string(getprogname()) + ", current mte level:";
    if(tagLevel == PR_MTE_TCF_NONE){
        result += "none, no mte error will find, no tombstone";
    } else if(tagLevel == PR_MTE_TCF_ASYNC){
        result += "async, no mte error this time";
    } else if(tagLevel == PR_MTE_TCF_SYNC){
        result += "sync, mte will trigger error and generate tombstone";
    } else {
        result += "unknown, no mte error will find, no tombstone";
    }
    int i = use_after_free();
    return env->NewStringUTF((result + "\na[2] after free a, result: " + std::to_string(i)).c_str());
}



int use_after_free2(){
    char *tmp = (char *)malloc(3);
    tmp[1] = 'a';
    free(tmp);
    return strlen(tmp);
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_memsafe_demo_MainActivity_stringFromJNIUAF2(
        JNIEnv* env,
        jobject /* this */) {
    int tagLevel = getMTELevel();
    std::string result = "in " + std::string(getprogname()) + ", current mte level:";
    if(tagLevel == PR_MTE_TCF_NONE){
        result += "none, no mte error will find, no tombstone";
    } else if(tagLevel == PR_MTE_TCF_ASYNC){
        result += "async, no mte error this time";
    } else if(tagLevel == PR_MTE_TCF_SYNC){
        result += "sync, mte will trigger error and generate tombstone";
    } else {
        result += "unknown, no mte error will find, no tombstone";
    }
    int i = use_after_free2();
    return env->NewStringUTF((result + "\nstrlen(a) after free a, result: " + std::to_string(i)).c_str());
}

int signal_count = 0;
struct sigaction oldaction;
bool handler_registed = false;

void handler(int signo, siginfo_t* info, void* ucontext_raw){
    //just count and pass to old action
    signal_count++;
    LOGD("after count, call old action");
    oldaction.sa_sigaction(signo, info, ucontext_raw);
}


void reg_handler()
{
    if(handler_registed)
        return;
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;
    sigaction(SIGSEGV, &action, &oldaction);
    handler_registed = true;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_memsafe_demo_MainActivity_handleSignalMyself(
        JNIEnv* env,
        jobject /* this */) {
    LOGD("in handler");
    reg_handler();
    LOGD("after reg_handler");
    std::string result = "reg a signal handler for sigsegv\nuse OOB to trigger sigsegv...\n";
    buff_overflow();
    LOGD("after oob");
    return env->NewStringUTF((result + "\nafter mte error, current signal count: " + std::to_string(signal_count)).c_str());
}

int LEAK_COUNT = 100000;
int justLeak(){
    for(int i = 0; i < LEAK_COUNT; i++){
        int *a = (int *) malloc(sizeof(int));
        *a = 100;
    }
    return LEAK_COUNT * sizeof (int );
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_memsafe_demo_ConsumeNativeMem_leakSomeNativeMem(JNIEnv *env, jclass clazz) {
    return justLeak();
}

int i=0;
extern "C"
JNIEXPORT jstring JNICALL
Java_com_memsafe_demo_MainActivity_ubsanDemo(JNIEnv *env, jobject thiz) {
    srand(time(NULL));
    i = rand()% 6;
    execute(i);
    return env->NewStringUTF("ubsan-demo已执行，请在logcat中查找日志");
}