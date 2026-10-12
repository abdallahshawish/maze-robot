#ifndef PWM_H
#define PWM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PWM_OK = 0U,
    PWM_ERR_INVALID_PIN,
    PWM_ERR_CHANNEL_IN_USE,
    PWM_ERR_INVALID_DUTY
} pwm_error_t;

/*
 * OOP in C: واجهة PWM (Interface)
 */
typedef struct {
    /*
     * frequency_hz: تردد الإشارة (مثلاً 20000 هرتز لتجنب الضوضاء الصوتية للمحرك)
     * resolution_bits: دقة التحكم (مثلاً 8 بت = 0-255)
     */
    pwm_error_t (*init)(uint8_t pin, uint32_t frequency_hz, uint8_t resolution_bits);

    /*
     * duty_cycle_percent: دورة العمل كنسبة مئوية من 0.0 إلى 100.0
     * استخدام float مقبول هنا لوجود FPU عتادي في ESP32
     */
    pwm_error_t (*set_duty)(uint8_t pin, float duty_cycle_percent);

    /* الإيقاف الطارئ / الحر للمحرك */
    pwm_error_t (*stop)(uint8_t pin);
} pwm_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* PWM_H */