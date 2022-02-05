#ifndef _CAMERA_LOG_H
#define _CAMERA_LOG_H

#define DEBUG
#include <android/log.h>

#ifdef DEBUG
#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, "LOGI: " __FILE__, __VA_ARGS__))
#else
#define LOGI(...) (0);
#endif

#endif