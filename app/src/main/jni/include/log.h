#ifndef _CAMERA_LOG_H
#define _CAMERA_LOG_H

#define DEBUG
#include <android/log.h>

#ifdef DEBUG
#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, "LOGI: " __FILE__, __VA_ARGS__))
#define LOGD(...) (__android_log_print(ANDROID_LOG_DEBUG, "LOGD: " __FILE__, __VA_ARGS__))
#else
#define LOGI(...) (0);
#define LOGD(...) (0);
#endif

#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, "LOG_TAG", fmt, ##__VA_ARGS__); \
  }

#endif