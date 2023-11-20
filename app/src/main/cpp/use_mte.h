#pragma once

#define MTE_SYNC 1
#define MTE_ASYNC 2
#define MTE_NONE 0




#define SIGNAL_RUN_ON_ALL_THREADS 41

//对本线程、以及本线程clone出的新线程生效
int set_mte(int level);

int set_mte_for_all_threads(int level);

void print_current_mte_level(const char* context);