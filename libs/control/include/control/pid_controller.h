#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PID_OK = 0U,
    PID_ERR_NULL_PTR,
    PID_ERR_INVALID_TIME
} pid_error_t;

/*
 * OOP in C: تغليف حالة متحكم الـ PID
 */
typedef struct {
    /* المعاملات الرياضية (Tuning Parameters) */
    float kp;
    float ki;
    float kd;

    /* حدود الإخراج والتراكم لمنع ظاهرة (Integral Windup) */
    float out_max;
    float out_min;
    float integral_max;
    float integral_min;

    /* الذاكرة الداخلية للمتحكم */
    float integral_sum;
    float previous_error;
} pid_controller_t;

/* ==========================================
 * PID API
 * ========================================== */

/* تهيئة المتحكم بالمعاملات والحدود القصوى */
pid_error_t pid_init(pid_controller_t *pid,
                     float kp, float ki, float kd,
                     float out_max, float out_min,
                     float integral_max, float integral_min);

/*
 * حساب الخرج اللحظي:
 * setpoint: الهدف المطلوب (مثلاً السرعة المطلوبة أو الزاوية 0)
 * measured: القراءة الفعلية من الحساس (مثلاً سرعة الإنكودر الحالية)
 * dt_seconds: الفارق الزمني منذ آخر استدعاء بالثواني
 * out_output: مؤشر لتخزين النتيجة (قيمة تصحيح المحرك)
 */
pid_error_t pid_compute(pid_controller_t *pid,
                        float setpoint,
                        float measured,
                        float dt_seconds,
                        float *out_output);

/* تصفير الذاكرة الداخلية (مهم جداً عند تغيير حالة الروبوت أو بدء حركة جديدة) */
pid_error_t pid_reset(pid_controller_t *pid);

#ifdef __cplusplus
}
#endif

#endif /* PID_CONTROLLER_H */