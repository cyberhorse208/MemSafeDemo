#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "use_mte.h"
#include <time.h>

#include <vector>
#include <string>

int dumy = 0;

char *getbuff() {
    char *tmp = new char[14];
    tmp[16] = rand() % 10 + 'A';   //buffer overflow, not the same block
    return tmp;
}

int buff_overflow_lite() {
    char *tmp = new char[14];

    int n = 0;
    for (int i = 0; i < 14; i++)
        tmp[i] = rand() % 10;
    for (int i = 0; i < 17; i++)
        n += tmp[i];
    printf("OOB happend: %s\n", tmp);
    delete[] tmp;
    //确保访问tmp[16]的操作不会被编译器优化掉
    return n;
}


void use_after_free_lite() {
    char *tmp = new char[32];
    strcpy(tmp, "AAAAABBBBB");
    printf("%s\n", tmp);
    delete[] tmp;
    tmp[0] = 'A';
}

void *open_close_repeat_mte(void *) {
    printf("open_close_repeat_mte test begine, you shoud set mte in permissive mode\n");
    int count = 0;
    while (count < 10) {
        printf("*******round %d\n", count);
        print_current_mte_level("before buff_overflow_lite");
        int n = buff_overflow_lite();
        //mte will disabled if in perssive mode, no error report for next func
        printf("n=%d\n", n);
        print_current_mte_level("after buff_overflow_lite");
        //set mte level sync
        printf("set mte syc\n");
        set_mte(MTE_SYNC);

        count++;
    }
    printf("open_close_repeat_mte test end\n");

    return NULL;
}


void *failover_mte_test(void *) {
    printf("failover_mte_test test begine, you shoud set mte in failover mode\n");

    print_current_mte_level("before buff_overflow_lite");
    int n = buff_overflow_lite();
    //mte will disabled if in failover mode, no error report for next func
    printf("cause mte error, n=%d\n", n);
    print_current_mte_level("after buff_overflow_lite");
    n = buff_overflow_lite();
    printf("cause no mte error, n=%d\n", n);
    printf("sleep 10 seconds, wait reset mte to sync\n");
    sleep(10);
    n = buff_overflow_lite();
    printf("cause mte error, n=%d\n", n);
    printf("give debuggerd_handler sometime to send mte mode signal, logcat should have some info\n");
    sleep(5);
    print_current_mte_level("after buff_overflow_lite");
    printf("failover_mte_test test end\n");
    return NULL;
}


void *buff_overflow_thread(void *) {
    printf("buff_overflow_thread test begine\n");

    printf("1.should see mte error\n");
    int n = buff_overflow_lite();
    //mte will disabled if in perssive mode, no error report for next func
    printf("2.should not see mte error in perssive mode\n");
    n += buff_overflow_lite();
    //set mte level sync
    set_mte(MTE_SYNC);
    //will get mte error again
    printf("3.should see mte error\n");
    use_after_free_lite();
    n += buff_overflow_lite();
    //mte will disabled if in perssive mode, no error report for next func
    printf("4.should not see mte error in perssive mode\n");
    n += buff_overflow_lite();
    dumy = n;
    printf("buff_overflow_thread test end\n");

    return NULL;
}

void *use_after_free_thread(void *) {
    printf("use_after_free_thread test begine\n");
    bool mteopen = true;
    if (rand() % 2 == 0) {
        set_mte(MTE_NONE);
        mteopen = false;
    } else {
        set_mte(MTE_SYNC);
        mteopen = true;
    }

    sleep(3);
    printf("UAF1.should %s see mte error\n", mteopen ? "" : "not");
    use_after_free_lite();

    printf("use_after_free_thread test end\n");
    return NULL;
}

void buff_overflow() {
    printf("buff_overflow test begine\n");
    char *tmp = new char[14];
    printf("tmp[14]\n");
    tmp[15] = 'A';   //buffer overflow,but can't detect it because in the same 16B block
    printf("OOB in same block(16B), mte can't detect: tmp[15]\n");
    tmp[16] = 'B';   //buffer overflow, not the same block
    printf("OOB not the same block(16B), mte can detect(if we in permissive or failover mode, mte should be colsed now): tmp[16]\n");

    tmp[33] = 'C';   //buffer overflow, not the same block
    printf("OOB not the same block(16B), mte can detect(not permissive or failover mode): tmp[33]\n");

    sleep(2);
    printf("sleeped 2 seconds, mte should enabled again\n");

    tmp[55] = 'D';   //buffer overflow, not the same block
    printf("OOB not the same block(16B), mte can detect(if we in failover mode, mte should be sync now): tmp[55]\n");

    printf("OOB printf, mte can detect: %s\n", tmp);
    delete[] tmp;


    printf("buff_overflow test end\n");
}


void use_after_free() {

    printf("use_after_free test begine\n");
    char *tmp = new char[32];
    printf("tmp[32]\n");
    strcpy(tmp, "AAAAABBBBB");
    printf("%s\n", tmp);
    printf("***free tmp\n");
    delete[] tmp;

    tmp[0] = 'A';
    printf("UAF, mte can detect(if we in permissive or failover mode, mte should be colsed now): tmp[0]\n");
    tmp[1] = 'B';
    printf("UAF, mte can detect(not permissive or failover mode): tmp[1]\n");
    sleep(2);
    printf("sleeped 2 seconds, mte should enabled again\n");

    tmp[2] = 'C';
    printf("UAF, mte can detect(if we in failover mode, mte should be sync now): tmp[2]\n");

    printf("**printf tmp so compiler won't optimize code\n");
    printf("%s\n", tmp);

    printf("use_after_free test end\n");
}

void uaf_strlen_test() {
    char *tmp = new char[32];
    strcpy(tmp, "AAAAABBBBB");
    printf("tmp: %s\n", tmp);
    printf("***free tmp\n");
    delete[] tmp;

    printf("***put a char into tmp[0], get mte error\n");
    tmp[0] = 'A';
    print_current_mte_level("after tmp[0]");

    printf("***strlen of tmp, triger close_mte in nextinstruction mode\n");
    printf("%d\n", strlen(tmp));
    print_current_mte_level("after strlen");

    printf("**printf tmp, no mte error\n");
    printf("%s\n", tmp);
}

#include <android/log.h>

#define TAG "_V_NativeTest"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

void buff_overflow_xj() {
    char *tmp = (char *) malloc(3);
    //buffer overflow, cause sigsegv
    //signal 11 (SIGSEGV), code 9 (SEGV_MTESERR), fault addr 0x04000075a400054e
    //Cause: [MTE]: Buffer Overflow, 27 bytes right of a 3-byte allocation at 0x75a4000530
    tmp[30] = 'A';

    //this will cause sigabrt, not sigsegv
    //signal 6 (SIGABRT), code -1 (SI_QUEUE), fault addr --------
    //Abort message: 'FORTIFY: memcpy: prevented 27-byte write into 3-byte buffer'
    //strcpy(tmp,"AAAAABBBBBCCCDDDDDDEEEEEEE");

    LOGD("%s", tmp);
}

bool g_exit = false;

void *dumy_thread(void *) {
    printf("dumy thread begine, just to get mte level\n");
    while (1) {
        sleep(1);
        if (g_exit)
            break;
    }
    printf("dumy thread exit\n");
    return NULL;
}

using std::string;
using std::vector;



int main(int argc, char *argv[]) {
    vector<vector<string>> vvs;
    for (int i = 0; i < 10; i++) {
        vector<string> tmp;
        vvs.push_back(tmp);
    }
    vvs[2].push_back("jfjj");
    vvs[2].push_back("jfjj");
    vvs[2].push_back("jfjj");
    vvs[4].push_back("jfjj");
    vvs[5].push_back("jfjj");

    for (int i = 0; i < 10; i++) {
        printf("%d %d\n", i, vvs[i].size());
    }
//    setprogname(argv[0]);
    printf("getprogname@%p, name: %s\n", getprogname, getprogname());
    srand(time(NULL));
    if (argc == 2 && (strcmp(argv[1], "1") == 0)) {
        pthread_t ptid1;
        pthread_create(&ptid1, NULL, &buff_overflow_thread, NULL);

        pthread_t ptid2;
        pthread_create(&ptid2, NULL, &use_after_free_thread, NULL);
        pthread_join(ptid1, NULL);
        pthread_join(ptid2, NULL);
        printf("dumy: %d\n", dumy);
    } else if (argc == 2 && (strcmp(argv[1], "2") == 0)) {
        printf("open_close_repeat_mte test\n");
        open_close_repeat_mte(NULL);
    } else if (argc == 2 && (strcmp(argv[1], "3") == 0)) {
        printf("uaf_strlen_test test\n");
        uaf_strlen_test();
    } else if (argc == 2 && (strcmp(argv[1], "4") == 0)) {
        printf("buff_overflow_xj test\n");
        buff_overflow_xj();
    } else if (argc == 2 && (strcmp(argv[1], "5") == 0)) {
        printf("failover_mte_test test\n");
        failover_mte_test(NULL);
    } else if (argc == 2 && (strcmp(argv[1], "6") == 0)) {
        printf("run_all_thread test begin, use 10 threads\n");
        pthread_t ptid[10];
        for (int i = 0; i < 10; i++)
            pthread_create(ptid + i, NULL, &dumy_thread, NULL);

        sleep(5);
        printf("try to set all thread %d\n", MTE_SYNC);
        set_mte_for_all_threads(MTE_SYNC);
        sleep(5);
        printf("try to set all thread %d\n", MTE_ASYNC);
        set_mte_for_all_threads(MTE_ASYNC);
        sleep(5);
        printf("try to set all thread %d\n", MTE_SYNC);
        set_mte_for_all_threads(MTE_SYNC);
        sleep(5);
        printf("try to set all thread %d\n", MTE_NONE);
        set_mte_for_all_threads(MTE_NONE);

        g_exit = true;
        for (int i = 0; i < 10; ++i) {
            pthread_join(ptid[i], NULL);
        }
        printf("run_all_thread test exit\n");
    } else {
        printf("use_after_free_thread test\n");
        use_after_free_thread(NULL);
    }

    return 0;
}
