#include "pid_controller.h"
#include <stddef.h>

pid_error_t pid_init(pid_controller_t *pid,
                     float kp, float ki, float kd,
                     float out_max, float out_min,
                     float integral_max, float integral_min)
{
    /* MISRA-C: فحص المؤشرات */
    if (pid == NULL) {
        return PID_ERR_NULL_PTR;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->out_max = out_max;
    pid->out_min = out_min;
    pid->integral_max = integral_max;
    pid->integral_min = integral_min;

    /* تهيئة الذاكرة الداخلية */
    pid->integral_sum = 0.0f;
    pid->previous_error = 0.0f;

    return PID_OK;
}

pid_error_t pid_compute(pid_controller_t *pid,
                        float setpoint,
                        float measured,
                        float dt_seconds,
                        float *out_output)
{
    if ((pid == NULL) || (out_output == NULL)) {
        return PID_ERR_NULL_PTR;
    }

    /* حماية ضد القسمة على صفر أو الزمن السالب (حرج جداً في الأنظمة اللحظية) */
    if (dt_seconds <= 0.0f) {
        *out_output = 0.0f;
        return PID_ERR_INVALID_TIME;
    }

    /* 1. حساب الخطأ الحالي (Error) */
    float error = setpoint - measured;

    /* 2. الحساب التناسبي (Proportional) */
    float p_term = pid->kp * error;

    /* 3. الحساب التكاملي (Integral) مع حماية ضد التشبع (Anti-Windup) */
    pid->integral_sum += (error * dt_seconds);

    /* Clamping للمجموع التكاملي */
    if (pid->integral_sum > pid->integral_max) {
        pid->integral_sum = pid->integral_max;
    } else if (pid->integral_sum < pid->integral_min) {
        pid->integral_sum = pid->integral_min;
    }

    float i_term = pid->ki * pid->integral_sum;

    /* 4. الحساب التفاضلي (Derivative) */
    float d_term = pid->kd * ((error - pid->previous_error) / dt_seconds);

    /* تحديث الخطأ السابق للدورة القادمة */
    pid->previous_error = error;

    /* 5. حساب الخرج الكلي (Total Output) */
    float output = p_term + i_term + d_term;

    /* 6. حماية الخرج (Output Clamping) لتناسب قدرة المحركات */
    if (output > pid->out_max) {
        output = pid->out_max;
    } else if (output < pid->out_min) {
        output = pid->out_min;
    }

    *out_output = output;

    return PID_OK;
}

pid_error_t pid_reset(pid_controller_t *pid)
{
    if (pid == NULL) {
        return PID_ERR_NULL_PTR;
    }
    pid->integral_sum = 0.0f;
    pid->previous_error = 0.0f;
    return PID_OK;
}