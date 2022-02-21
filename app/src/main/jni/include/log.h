#ifndef _CAMERA_LOG_H
#define _CAMERA_LOG_H

#include <android/log.h>

//#define DEBUG

#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, "LOGI: " __FILE__, __VA_ARGS__));

#ifdef DEBUG
#define LOGD(...) (__android_log_print(ANDROID_LOG_DEBUG, "LOGD: " __FILE__, __VA_ARGS__));
#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, "ASSERT", fmt, ##__VA_ARGS__); \
  }
#define CALL(func) status = func; \
        ASSERT(status == 0, "%d error code returned by %s", status, #func)
#else
#define LOGD(...);
#define ASSERT(cond, fmt, ...);
#define CALL(func) status = func;
#endif



#endif