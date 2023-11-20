#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include "use_mte.h"
#include "mte_header.h"

#include <semaphore.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdatomic.h>

void print_current_mte_level(const char *context) {
    int tagged_addr_ctrl = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
    if (tagged_addr_ctrl < 0) {
        printf("%s failed to PR_GET_TAGGED_ADDR_CTRL, pid= %ld, tid=%ld\n", context, getpid(),
               gettid());
    }
    tagged_addr_ctrl = (tagged_addr_ctrl & PR_MTE_TCF_MASK);
    if (tagged_addr_ctrl == PR_MTE_TCF_SYNC) {
        printf("%s current level: SYNC, pid= %ld, tid=%ld\n", context, getpid(), gettid());
    }
    if (tagged_addr_ctrl == PR_MTE_TCF_ASYNC) {
        printf("%s current level: ASYNC, pid= %ld, tid=%ld\n", context, getpid(), gettid());
    }
    if (tagged_addr_ctrl == PR_MTE_TCF_NONE) {
        printf("%s current level: OFF, pid= %ld, tid=%ld\n", context, getpid(), gettid());
    }
}


int set_mte(int level) {
    int tagged_addr_ctrl = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
    if (tagged_addr_ctrl < 0) {
        printf("failed to PR_GET_TAGGED_ADDR_CTRL\n");
        return -1;
    }
    if (level == MTE_NONE)
        tagged_addr_ctrl = (tagged_addr_ctrl & ~PR_MTE_TCF_MASK) | PR_MTE_TCF_NONE;
    else if (level == MTE_SYNC)
        tagged_addr_ctrl = (tagged_addr_ctrl & ~PR_MTE_TCF_MASK) | PR_MTE_TCF_SYNC;
    else if (level == MTE_SYNC)
        tagged_addr_ctrl = (tagged_addr_ctrl & ~PR_MTE_TCF_MASK) | PR_MTE_TCF_ASYNC;
    else {
        printf("not supported mte level %d, check arg\n", level);
        return -2;
    }
    if (prctl(PR_SET_TAGGED_ADDR_CTRL, tagged_addr_ctrl, 0, 0, 0) < 0) {
        printf("failed to PR_SET_TAGGED_ADDR_CTRL\n");
        return -3;
    }
    print_current_mte_level("after set_mte");
    return 0;
}

bool set_mte_internal(void *arg) {
    int level = *reinterpret_cast<int *>(arg);
    printf("pid= %ld, tid=%ld set mte internal to %d\n", getpid(), gettid(), level);
    if (set_mte(level) != 0)
        return false;
    return true;
}

static bool (*g_func)(void *);

static void *g_arg;
static sem_t g_sem;
static _Atomic(bool) g_retval;


void handler(int nSignum, siginfo_t *info, void *vcontext) {
    if (!g_func(g_arg)) {
        atomic_store(&g_retval, false);
    }
    sem_post(&g_sem);
}

//like
//https://cs.android.com/android/platform/superproject/+/refs/heads/master:bionic/libc/bionic/pthread_internal.cpp;l=122;drc=88f0990b45a7b992fadd6bc58f6d63c3b44054a2;bpv=0;bpt=1?hl=zh-cn
bool run_on_all_threads(bool (*func)(void *), void *arg) {

    // Call the function directly for the current thread so that we don't need to worry about
    // the consequences of synchronizing with ourselves.
    if (!func(arg)) {
        return false;
    }

    if (sem_init(&g_sem, 0, 0) != 0) {
        return false;
    }
    g_func = func;
    g_arg = arg;
    atomic_init(&g_retval, true);

    struct sigaction act = {}, oldact;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    sigfillset(&act.sa_mask);
    if (sigaction(SIGNAL_RUN_ON_ALL_THREADS, &act, &oldact) != 0) {
        sem_destroy(&g_sem);
        printf("run_on_all_threads sigaction failed\n");
        return false;
    }

    pid_t my_pid = getpid();
    pid_t my_tid = gettid();
    size_t num_tids = 0;
    DIR *proc_dir;
    char dirname[100];
    snprintf(dirname, sizeof dirname, "/proc/self/task");
    proc_dir = opendir(dirname);

    if (proc_dir) {
        /* /proc available, iterate through tasks... */
        struct dirent *entry;
        while ((entry = readdir(proc_dir)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;

            int tid = atoi(entry->d_name);
            if (tid == my_tid)
                continue;

            if (tgkill(my_pid, tid, SIGNAL_RUN_ON_ALL_THREADS) == 0) {
                ++num_tids;
                printf("success send SIGNAL_RUN_ON_ALL_THREADS to thread %d\n", tid);
            } else {
                printf("failed to send SIGNAL_RUN_ON_ALL_THREADS to thread %d\n", tid);
                atomic_store(&g_retval, false);
            }
        }
        closedir(proc_dir);
    }

    printf("run_on_all_threads tgkill done\n");

    for (size_t i = 0; i != num_tids; ++i) {
        if (TEMP_FAILURE_RETRY(sem_wait(&g_sem)) != 0) {
            atomic_store(&g_retval, false);
            break;
        }
    }

    sigaction(SIGNAL_RUN_ON_ALL_THREADS, &oldact, 0);
    sem_destroy(&g_sem);
    return atomic_load(&g_retval);
}


int set_mte_for_all_threads(int level) {
    return run_on_all_threads(&set_mte_internal, &level);
}

