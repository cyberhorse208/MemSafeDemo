//
// Created by 11135250 on 2023/10/31.
//

#ifndef NATIVETEST_LOG_H
#define NATIVETEST_LOG_H

#include <android/log.h>
#ifdef NDEBUG
#define RELEASE_BUILD
#else
#define DEBUG_BUILD
#endif

#ifdef DEBUG_BUILD
#define TAG "_V_NativeTest"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG ,__VA_ARGS__)
#else
#define LOGI(...)
#define LOGD(...)
#define LOGE(...)
#define LOGF(...)
#define LOGW(...)
#endif

#endif //NATIVETEST_LOG_H
