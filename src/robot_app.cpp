#include "robot_app.h"
#include "pin_definitions.h" /* يحتوي على جميع تعريفات العتاد */
#include "bsp_esp32.h"       /* واجهات GPIO و PWM الخاصة بالـ ESP32 */
/* تضمين باقي المكتبات التي بنيناها */
#include "motor.h"
#include "laser_sensor.h"
#include "encoder_sensor.h"
#include "pid_controller.h"
#include "robot_state_machine.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ==========================================
 * MISRA-C: Static Allocation
 * تعريف جميع الكائنات كمتغيرات ثابتة (Static) لحماية الذاكرة (Heap)
 * ومنع تسريب الذاكرة (Memory Leak). لا نستخدم malloc إطلاقاً.
 * ========================================== */

static motor_t          motor_left;
static motor_t          motor_right;

static laser_sensor_t   laser_front;
static laser_sensor_t   laser_left;
static laser_sensor_t   laser_right;

static encoder_sensor_t encoder_left;
static encoder_sensor_t encoder_right;

static pid_controller_t pid_speed;
static pid_controller_t pid_heading;

static robot_context_t  robot_ctx;

/* ملاحظة: نفترض وجود كائن يمثل واجهة i2c_interface_t للـ ESP32 */
extern const i2c_interface_t esp32_i2c_interface;

/* ==========================================
 * RTOS Tasks Definitions
 * ========================================== */

/* 1. مهمة التحكم اللحظي (High Priority - Core 1) - 500Hz */
static void task_control_loop(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(2); /* 2ms = 500Hz */
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        /* قراءة الإنكودرات السريعة عبر I2C0 */
        encoder_update(&encoder_left);
        encoder_update(&encoder_right);

        /*
         * هنا نضع معادلات الكينماتيكا لحساب السرعة الخطية والزاوية
         * وتمريرها لمتحكمات الـ PID، ثم إرسال الخرج للمحركات.
         */

        vTaskDelayUntil(&xLastWakeTime, xFrequency); /* توقيت حتمي (Deterministic) */
    }
}

/* 2. مهمة الاستشعار (Medium Priority - Core 1) - 50Hz */
static void task_sensor_update(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(20); /* 20ms = 50Hz */
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        /* قراءة حساسات الليزر عبر I2C1 بشكل دوري لعدم إرهاق الناقل */
        uint16_t dist;
        laser_sensor_read_mm(&laser_front, &dist);
        laser_sensor_read_mm(&laser_left, &dist);
        laser_sensor_read_mm(&laser_right, &dist);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/* 3. مهمة الذكاء الاصطناعي (Low Priority - Core 0) - 20Hz */
static void task_fsm_logic(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(50); /* 50ms = 20Hz */
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        /* تحديث آلة الحالة واستراتيجية المتاهة (ديكسترا / الفيضان) */
        fsm_update(&robot_ctx);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/* ==========================================
 * Application API Implementation
 * ========================================== */

app_error_t robot_app_init(void) {

    /* 1. تهيئة المحركات (Dependency Injection) */
    /* نمرر واجهات ESP32 للـ GPIO والـ PWM إلى المحركات. المعامل الأخير هو is_inverted */
    motor_init(&motor_left, &esp32_gpio_interface, &esp32_pwm_interface,
               PIN_MOTOR_L_PWM, PIN_MOTOR_L_DIR1, PIN_MOTOR_L_DIR2, 0U);

    motor_init(&motor_right, &esp32_gpio_interface, &esp32_pwm_interface,
               PIN_MOTOR_R_PWM, PIN_MOTOR_R_DIR1, PIN_MOTOR_R_DIR2, 1U); /* المحرك الأيمن معكوس ميكانيكياً غالباً */

    /* 2. تهيئة حساسات الليزر (على الناقل I2C_BUS_1) */
    laser_sensor_init(&laser_front, &esp32_i2c_interface, I2C_BUS_1, I2C_ADDR_TOF_FRONT);
    laser_sensor_init(&laser_left,  &esp32_i2c_interface, I2C_BUS_1, I2C_ADDR_TOF_LEFT);
    laser_sensor_init(&laser_right, &esp32_i2c_interface, I2C_BUS_1, I2C_ADDR_TOF_RIGHT);

    /* 3. تهيئة الإنكودر (على الناقل السريع I2C_BUS_0) */
    encoder_init(&encoder_left,  &esp32_i2c_interface, I2C_BUS_0, I2C_ADDR_ENCODER_L, 0U);
    encoder_init(&encoder_right, &esp32_i2c_interface, I2C_BUS_0, I2C_ADDR_ENCODER_R, 1U); /* معكوس أيضاً */

    /* 4. تهيئة متحكمات الـ PID (القيم تحتاج لعملية Tuning لاحقاً) */
    /* kp, ki, kd, out_max, out_min, int_max, int_min */
    pid_init(&pid_speed,   1.5f, 0.1f, 0.05f, 100.0f, -100.0f, 50.0f, -50.0f);
    pid_init(&pid_heading, 2.0f, 0.0f, 0.1f,  50.0f,  -50.0f,  10.0f, -10.0f);

    /* 5. ربط كل هذه الكائنات بسياق الروبوت (Context) */
    robot_ctx.motor_left    = &motor_left;
    robot_ctx.motor_right   = &motor_right;
    robot_ctx.laser_front   = &laser_front;
    robot_ctx.laser_left    = &laser_left;
    robot_ctx.laser_right   = &laser_right;
    robot_ctx.encoder_left  = &encoder_left;
    robot_ctx.encoder_right = &encoder_right;
    robot_ctx.pid_speed     = &pid_speed;
    robot_ctx.pid_heading   = &pid_heading;

    /* 6. تهيئة آلة الحالة */
    if (fsm_init(&robot_ctx) != FSM_OK) {
        return APP_ERR_INIT_FAILED;
    }

    return APP_OK;
}

void robot_app_start_tasks(void) {
    /*
     * توزيع المهام على أنوية المعالج (Core 0 و Core 1)
     * xTaskCreatePinnedToCore (Function, Name, Stack Size, Param, Priority, Handle, Core ID)
     */

    /* المهام الحركية والفيزيائية على النواة 1 (أعلى أولوية للـ PID) */
    xTaskCreatePinnedToCore(task_control_loop,  "Ctrl_Task",   4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_sensor_update, "Sensor_Task", 4096, NULL, 3, NULL, 1);

    /* المهام الرياضية وخوارزمية المتاهة ديكسترا على النواة 0 (حتى لا تعيق المحركات) */
    xTaskCreatePinnedToCore(task_fsm_logic,     "Algo_Task",   8192, NULL, 2, NULL, 0);
}