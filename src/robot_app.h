#ifndef ROBOT_APP_H
#define ROBOT_APP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_OK = 0U,
    APP_ERR_INIT_FAILED
} app_error_t;

/*
 * API التطبيق الرئيسي
 * يتم استدعاء هذه الدوال فقط من ملف main.cpp
 */

/* تهيئة جميع مكونات العتاد وربط الكائنات (Dependency Injection) */
app_error_t robot_app_init(void);

/* إطلاق مهام نظام التشغيل اللحظي (RTOS Tasks) وتوزيعها على الأنوية */
void robot_app_start_tasks(void);

#ifdef __cplusplus
}
#endif

#endif /* ROBOT_APP_HPP */