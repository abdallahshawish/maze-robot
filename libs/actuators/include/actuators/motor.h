#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "gpio.h"
#include "pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MISRA-C: تعريف رموز الأخطاء الخاصة بطبقة المحركات */
typedef enum {
    MOTOR_OK = 0U,
    MOTOR_ERR_NULL_PTR,
    MOTOR_ERR_HAL_INIT_FAILED
} motor_error_t;

/*
 * OOP in C: تغليف حالة المحرك (Encapsulation)
 * هذا الهيكل (Struct) يمثل "الكائن" (Object) الذي يحتفظ ببيانات وواجهات كل محرك على حدة.
 */
typedef struct {
    /* حقن التبعية: مؤشرات ثابتة لواجهات العتاد (HAL Interfaces) */
    const gpio_interface_t *gpio;
    const pwm_interface_t  *pwm;

    /* منافذ التوصيل الفيزيائية */
    uint8_t pwm_pin;
    uint8_t dir_pin1;
    uint8_t dir_pin2;

    /* متغير كينماتيكي لتصحيح اتجاه الدوران إذا كان المحرك معكوساً ميكانيكياً */
    uint8_t is_inverted;
} motor_t;

/* ==========================================
 * Motor API (الدوال الخاصة بالتحكم)
 * ========================================== */

/* دالة البناء والتهيئة (Constructor & Init) */
motor_error_t motor_init(motor_t *motor,
                         const gpio_interface_t *gpio,
                         const pwm_interface_t *pwm,
                         uint8_t pwm_pin,
                         uint8_t dir_pin1,
                         uint8_t dir_pin2,
                         uint8_t is_inverted);

/* التحكم بالسرعة: تقبل قيم من -100.0 (خلف بأقصى سرعة) إلى 100.0 (أمام بأقصى سرعة) */
motor_error_t motor_set_speed(motor_t *motor, float speed_percent);

/* الإيقاف الحر (Coasting) - قطع الطاقة وترك الروبوت ينزلق ببطء */
motor_error_t motor_stop(motor_t *motor);

/* الكبح السريع (Braking) - قصر أقطاب المحرك للتوقف الفوري */
motor_error_t motor_brake(motor_t *motor);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_H */