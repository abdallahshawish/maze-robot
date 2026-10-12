#include <Arduino.h> /* أو #include "freertos/FreeRTOS.h" لبيئة ESP-IDF النقية */
#include "robot_app.h"

/*
 * في بيئة Arduino-ESP32 نستخدم setup و loop.
 * في بيئة ESP-IDF نستخدم void app_main(void).
 */

void setup() {
    /* يمكنك تهيئة الاتصال التسلسلي (UART) للـ Debugging هنا */
    Serial.begin(115200);
    Serial.println("Micromouse System Booting...");

    /* تهيئة التطبيق وربط الكائنات */
    if (robot_app_init() != APP_OK) {
        Serial.println("CRITICAL ERROR: Robot Initialization Failed!");
        while(1) { /* تجميد النظام في حالة فشل العتاد */ }
    }
    Serial.println("Initialization OK. Starting RTOS Tasks...");

    /* إطلاق مهام النظام التشغيلي (FreeRTOS) */
    robot_app_start_tasks();
}

void loop() {
    /*
     * MISRA-C / FreeRTOS Best Practice:
     * بما أن جميع المهام تعمل في الـ Tasks التي أطلقناها،
     * نترك حلقة loop فارغة تماماً ونحذف مهمتها لتوفير الموارد.
     */
    vTaskDelete(NULL);
}