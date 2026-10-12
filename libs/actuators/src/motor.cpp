#include "motor.h"
#include <stddef.h> /* من أجل NULL */

/* تردد الـ PWM للروبوتات يجب أن يكون خارج النطاق المسموع للبشر (>20kHz) لمنع الطنين المزعج */
#define MOTOR_PWM_FREQUENCY_HZ (20000U)
#define MOTOR_PWM_RESOLUTION   (8U) /* دقة 8-بت تعني 0-255 */

motor_error_t motor_init(motor_t *motor,
                         const gpio_interface_t *gpio,
                         const pwm_interface_t *pwm,
                         uint8_t pwm_pin,
                         uint8_t dir_pin1,
                         uint8_t dir_pin2,
                         uint8_t is_inverted)
{
    /* MISRA-C: الحماية الصارمة ضد المؤشرات الفارغة (Null Pointers) */
    if ((motor == NULL) || (gpio == NULL) || (pwm == NULL)) {
        return MOTOR_ERR_NULL_PTR;
    }

    /* حقن التبعيات والبيانات داخل الكائن */
    motor->gpio = gpio;
    motor->pwm  = pwm;
    motor->pwm_pin = pwm_pin;
    motor->dir_pin1 = dir_pin1;
    motor->dir_pin2 = dir_pin2;
    motor->is_inverted = is_inverted;

    /* تهيئة منافذ الاتجاه كمخارج باستخدام الواجهة المجردة */
    if (motor->gpio->init(motor->dir_pin1, GPIO_DIR_OUTPUT) != GPIO_OK) {
        return MOTOR_ERR_HAL_INIT_FAILED;
    }
    if (motor->gpio->init(motor->dir_pin2, GPIO_DIR_OUTPUT) != GPIO_OK) {
        return MOTOR_ERR_HAL_INIT_FAILED;
    }

    /* تهيئة منفذ الـ PWM */
    if (motor->pwm->init(motor->pwm_pin, MOTOR_PWM_FREQUENCY_HZ, MOTOR_PWM_RESOLUTION) != PWM_OK) {
        return MOTOR_ERR_HAL_INIT_FAILED;
    }

    /* التأكد من توقف المحرك كحالة ابتدائية آمنة */
    (void)motor_stop(motor); /* تحويل لـ void لإسكات تحذيرات المترجم لأننا لا نحتاج لفحص الخطأ هنا */

    return MOTOR_OK;
}

motor_error_t motor_set_speed(motor_t *motor, float speed_percent) {
    if (motor == NULL) {
        return MOTOR_ERR_NULL_PTR;
    }

    /* 1. الحماية الكينماتيكية (Clamping): منع القيم المتطرفة */
    if (speed_percent > 100.0f) { speed_percent = 100.0f; }
    if (speed_percent < -100.0f) { speed_percent = -100.0f; }

    /* 2. تحديد الاتجاه (Forward vs Backward) */
    uint8_t is_forward = (speed_percent >= 0.0f) ? 1U : 0U;

    /* 3. تطبيق الانعكاس الميكانيكي (إذا كان المحرك الأيمن راكباً بالعكس مثلاً) */
    if (motor->is_inverted == 1U) {
        is_forward = (is_forward == 1U) ? 0U : 1U; /* XOR Logic */
    }

    /* 4. إرسال أوامر الـ H-Bridge */
    if (is_forward == 1U) {
        motor->gpio->write(motor->dir_pin1, GPIO_STATE_HIGH);
        motor->gpio->write(motor->dir_pin2, GPIO_STATE_LOW);
    } else {
        motor->gpio->write(motor->dir_pin1, GPIO_STATE_LOW);
        motor->gpio->write(motor->dir_pin2, GPIO_STATE_HIGH);
    }

    /* 5. حساب القيمة المطلقة للسرعة وإرسالها للـ PWM */
    float absolute_speed = (speed_percent < 0.0f) ? -speed_percent : speed_percent;
    motor->pwm->set_duty(motor->pwm_pin, absolute_speed);

    return MOTOR_OK;
}

motor_error_t motor_stop(motor_t *motor) {
    if (motor == NULL) {
        return MOTOR_ERR_NULL_PTR;
    }
    /* الإيقاف الحر (Fast Decay): إطفاء إشارات الاتجاه والـ PWM */
    motor->gpio->write(motor->dir_pin1, GPIO_STATE_LOW);
    motor->gpio->write(motor->dir_pin2, GPIO_STATE_LOW);
    motor->pwm->stop(motor->pwm_pin);

    return MOTOR_OK;
}

motor_error_t motor_brake(motor_t *motor) {
    if (motor == NULL) {
        return MOTOR_ERR_NULL_PTR;
    }
    /* الكبح السريع (Slow Decay): قصر أقطاب المحرك بوضع HIGH على كلا الدبوسين و PWM على 0 */
    /* ملاحظة: يعتمد هذا على نوع الـ H-Bridge، هذا الإعداد يناسب شريحة TB6612FNG */
    motor->gpio->write(motor->dir_pin1, GPIO_STATE_HIGH);
    motor->gpio->write(motor->dir_pin2, GPIO_STATE_HIGH);
    motor->pwm->stop(motor->pwm_pin);

    return MOTOR_OK;
}