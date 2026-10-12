#ifndef BSP_ESP32_H
#define BSP_ESP32_H

#include "gpio.h"
#include "pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MISRA-C: استخدام 'const' لوضع الهياكل في ذاكرة القراءة فقط (Flash/ROM).
 * هذا يمنع أي كود خبيث أو خاطئ من تغيير مؤشرات الدوال (Function Pointers) في وقت التشغيل.
 */
extern const gpio_interface_t esp32_gpio_interface;
extern const pwm_interface_t  esp32_pwm_interface;

#ifdef __cplusplus
}
#endif

#endif /* BSP_ESP32_H */