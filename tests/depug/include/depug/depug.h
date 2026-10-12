#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stdio.h> /* لاستخدام snprintf لاحقاً إذا احتجنا */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ==========================================
 * مستويات التشخيص (Debug Levels)
 * ==========================================
 */
typedef enum {
    DEBUG_LEVEL_ERROR = 0U, /* أخطاء حرجة توقف النظام */
    DEBUG_LEVEL_WARN,       /* تحذيرات لا توقف النظام لكن تحتاج انتباه */
    DEBUG_LEVEL_INFO,       /* معلومات عامة (مثل تغيير حالة الروبوت) */
    DEBUG_LEVEL_TRACE       /* طباعة تفصيلية (مثل قراءات الحساسات اللحظية - قد تبطئ النظام) */
} debug_level_t;

/*
 * المفتاح الرئيسي: قم بتغيير هذا الماكرو إلى 0 عند سباق المتاهة
 * لتعطيل جميع المطبوعات وتوفير دورات المعالج.
 */
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED (1U)
#endif

/* مستوى التشخيص الحالي للنظام */
#ifndef CURRENT_DEBUG_LEVEL
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

/* ==========================================
 * Debug API
 * ========================================== */

/* تهيئة واجهة التشخيص (مثال: بدء Serial.begin في ESP32) */
void debug_init(uint32_t baud_rate);

/*
 * دوال الطباعة الأساسية.
 * في لغة C، نستخدم هذه الدوال إذا كان DEBUG_ENABLED = 1
 */
#if DEBUG_ENABLED == 1U

    void debug_print(debug_level_t level, const char *module, const char *message);
    void debug_print_value(debug_level_t level, const char *module, const char *label, float value);
    void debug_print_error_code(const char *module, const char *function, int error_code);

    /*
     * Macros متقدمة تستخدم في الكود لتسهيل القراءة وتوضيح المصدر
     * مثال: LOG_ERROR("MOTOR", "Initialization Failed");
     */
    #define LOG_ERROR(module, msg) do { if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_ERROR) debug_print(DEBUG_LEVEL_ERROR, module, msg); } while(0)
    #define LOG_WARN(module, msg)  do { if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_WARN)  debug_print(DEBUG_LEVEL_WARN,  module, msg); } while(0)
    #define LOG_INFO(module, msg)  do { if (CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_INFO)  debug_print(DEBUG_LEVEL_INFO,  module, msg); } while(0)

    /* مخصصة لقراءات الحساسات الكثيفة */
    #define LOG_TRACE_VAL(module, label, val) do { if (CURRENT_DEBUG_LEVEL >=