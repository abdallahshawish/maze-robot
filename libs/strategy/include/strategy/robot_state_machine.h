#ifndef ROBOT_STATE_MACHINE_H
#define ROBOT_STATE_MACHINE_H

#include <stdint.h>
#include "motor.h"
#include "laser_sensor.h"
#include "encoder_sensor.h"
#include "pid_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MISRA-C: تعريف جميع الحالات الممكنة للروبوت بوضوح */
typedef enum {
    STATE_INIT = 0U,       /* تهيئة العتاد والانتظار */
    STATE_IDLE,            /* انتظار إشارة البدء (زر أو حساس) */
    STATE_EXPLORE,         /* استكشاف المتاهة (الفيضان) */
    STATE_PLANNING,        /* توقف لحساب مسار ديكسترا */
    STATE_FAST_RUN,        /* الجري السريع نحو الهدف */
    STATE_ERROR            /* حالة الطوارئ (توقف المحركات بأمان) */
} robot_state_t;

typedef enum {
    FSM_OK = 0U,
    FSM_ERR_NULL_PTR,
    FSM_ERR_INVALID_TRANSITION
} fsm_error_t;

/*
 * سياق الروبوت (Robot Context)
 * هذا الهيكل يجمع كل المكونات الفيزيائية للروبوت ويمرر لآلة الحالة كمرجع (Reference)
 */
typedef struct {
    /* المحركات */
    motor_t *motor_left;
    motor_t *motor_right;

    /* الحساسات */
    laser_sensor_t *laser_front;
    laser_sensor_t *laser_left;
    laser_sensor_t *laser_right;
    encoder_sensor_t *encoder_left;
    encoder_sensor_t *encoder_right;

    /* متحكمات الـ PID */
    pid_controller_t *pid_speed;
    pid_controller_t *pid_heading;

    /* متغيرات النظام */
    robot_state_t current_state;
    uint8_t target_reached;
} robot_context_t;

/* ==========================================
 * FSM API
 * ========================================== */

/* تهيئة آلة الحالة وربطها بسياق الروبوت */
fsm_error_t fsm_init(robot_context_t *ctx);

/*
 * الدالة المركزية: يجب استدعاؤها بتردد ثابت (مثلاً 50Hz) داخل مهمة (RTOS Task).
 * هي المسؤولة عن اتخاذ القرارات بناءً على الحالة الحالية.
 */
fsm_error_t fsm_update(robot_context_t *ctx);

/* إجبار النظام على الانتقال لحالة معينة (يستخدم للطوارئ) */
fsm_error_t fsm_set_state(robot_context_t *ctx, robot_state_t new_state);

#ifdef __cplusplus
}
#endif

#endif /* ROBOT_STATE_MACHINE_H */