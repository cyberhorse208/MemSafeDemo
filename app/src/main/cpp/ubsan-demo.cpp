/*
 * =====================================================================================
 *
 *       Filename:  ubsan-demo.cpp
 *
 *    Description:  ubsan test
 *
 *        Version:  1.0
 *        Created:  07/27/2023 10:22:10 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  yaoping,
 *        Company:  
 *        source: https://blogs.oracle.com/linux/post/improving-application-security-with-undefinedbehaviorsanitizer-ubsan-and-gcc
 * =====================================================================================
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"

int under_flow(){
	int a[10];
	LOGD("under_flow: %d\n", a[rand()%10 - 10]);
	return 0;
}

int over_flow(){
	int a[10];
	LOGD("over_flow: %d\n", a[rand()%2 + 10]);
	return 0;
}

//clang can detect this when compile
// warning: overflow in expression; result is -2147483639 with type 'int' [-Werror,-Winteger-overflow]
int integer_overflow(){
	return INT_MAX + 10;
}


int shift_overflow(){
	unsigned int j = 1 << 31;
    LOGD("shift_overflow: %d\n", j);
	return 0;
}

//clang can detect this when compile
//error: operator '<<' has lower precedence than '+'; '+' will be evaluated first [-Werror,-Wshift-op-parentheses]
int shift_exponent_precedence(){
	unsigned long j = (unsigned long)1 << 63 + 2;
	return j;

}

//only log, not tombstone
//SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior system/core/ubsan-demo/ubsan-demo.cpp:75:22 in
int shift_exponent(){
	unsigned long exponent = 63lu;
	unsigned long j = 1 << exponent;
    LOGD("shift_exponent: %d\n", j);
	return 0;
}

int execute(int idx){
    LOGD("ubsan-demo %d", idx);
	if(idx == 0){
		return under_flow();
	}else if (idx == 1) {
		return over_flow();
	}else if (idx == 2) {
		return integer_overflow();
	}else if (idx == 3) {
		return shift_exponent_precedence();
	}else if (idx == 4) {
		return shift_exponent();
	}else {
		printf("unknown n\n");
		return 2;
	}
}

int main(int argc, char** argv){
	if(argc == 1)
	{
		printf("usage: %s n\n", argv[0]);
		printf("n:\n");
		printf("\t0 under_flow\n");
		printf("\t1 over_flow\n");
		printf("\t2 integer_overflow\n");
		printf("\t3 shift_exponent_precedence\n");
		printf("\t4 shift_exponent\n");
		return 1;
	}
	srand(time(NULL));
	return execute(atoi(argv[1]));
}
