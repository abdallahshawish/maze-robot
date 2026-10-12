#include "depug.hpp"
#include <Arduino.h> /* لبيئة Arduino-ESP32 (استخدام Serial) */

/*
 * نقوم بتضمين التنفيذ فقط إذا كان التشخيص مفعلاً.
 * هذا يوفر مساحة كبيرة في الـ Flash Memory في وضع الإنتاج.
 */
#if DEBUG_ENABLED == 1U

static const char* level_to_string(debug_level_t level) {
    switch (level) {
        case DEBUG_LEVEL_ERROR: return "[ERROR]";
        case DEBUG_LEVEL_WARN:  return "[WARN] ";
        case DEBUG_LEVEL_INFO:  return "[INFO] ";
        case DEBUG_LEVEL_TRACE: return "[TRACE]";
        default:                return "[UNKN] ";
    }
}

void debug_init(uint32_t baud_rate) {
    /* تهيئة واجهة UART */
    Serial.begin(baud_rate);

    /* انتظار بسيط لضمان استقرار الاتصال التسلسلي */
    delay(100);

    Serial.println("\n==================================");
    Serial.println(" Micromouse Debug System Started");
    Serial.println("==================================");
}

void debug_print(debug_level_t level, const char *module, const char *message) {
    if (module == NULL || message == NULL) return;

    /*
     * MISRA-C / CERT-C: استخدام snprintf بدلاً من sprintf لتجنب فيضان الذاكرة
     */
    char buffer[128];

    /* طباعة التوقيت اللحظي للنظام (يساعد في تتبع الأخطاء الزمنية RTOS) */
    unsigned long timestamp = millis();

    snprintf(buffer, sizeof(buffer), "%s [%lu ms] <%s> : %s",
             level_to_string(level), timestamp, module, message);

    Serial.println(buffer);
}

void debug_print_value(debug_level_t level, const char *module, const char *label, float value) {
    if (module == NULL || label == NULL) return;

    char buffer[128];
    unsigned long timestamp = millis();

    /*
     * طباعة القيم العائمة (Float) مع دمجها بشكل آمن.
     * استخدام %.3f لطباعة 3 منازل عشرية.
     */
    snprintf(buffer, sizeof(buffer), "%s [%lu ms] <%s> : %s = %.3f",
             level_to_string(level), timestamp, module, label, value);

    Serial.println(buffer);
}

void debug_print_error_code(const char *module, const char *function, int error_code) {
    if (module == NULL || function == NULL) return;

    char buffer[128];
    unsigned long timestamp = millis();

    snprintf(buffer, sizeof(buffer), "[ERROR] [%lu ms] <%s> : %s() failed with code %d",
             timestamp, module, function, error_code);

    Serial.println(buffer);
}

#else

/*
 * Implementation stub when debugging is disabled
 * دالة فارغة في حال استدعاء التهيئة في الـ Release
 */
void debug_init(uint32_t baud_rate) {
    (void)baud_rate; /* إسكات تحذير (Unused Parameter) */
}

#endif /* DEBUG_ENABLED */