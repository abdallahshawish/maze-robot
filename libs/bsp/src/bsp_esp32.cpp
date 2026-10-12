#include "bsp_esp32.h"
#include <Arduino.h> /* يمكن استبدالها بـ driver/gpio.h و driver/ledc.h في بيئة ESP-IDF النقية */

/* ==========================================
 * 1. ESP32 GPIO Implementation
 * ========================================== */

/* MISRA-C: تعريف الدوال كـ static لمنع تلوث مساحة الأسماء العالمية (Namespace Pollution) */
static gpio_error_t esp32_gpio_init(uint8_t pin, gpio_direction_t dir) {
    if (pin > 39U) {
        return GPIO_ERR_INVALID_PIN; /* حماية ضد أرقام المنافذ الوهمية */
    }

    switch (dir) {
        case GPIO_DIR_OUTPUT:
            pinMode(pin, OUTPUT);
            break;
        case GPIO_DIR_INPUT:
            pinMode(pin, INPUT);
            break;
        case GPIO_DIR_INPUT_PULLUP:
            pinMode(pin, INPUT_PULLUP);
            break;
        default:
            return GPIO_ERR_INVALID_PIN;
    }
    return GPIO_OK;
}

static gpio_error_t esp32_gpio_write(uint8_t pin, gpio_state_t state) {
    if (pin > 39U) {
        return GPIO_ERR_INVALID_PIN;
    }
    digitalWrite(pin, (state == GPIO_STATE_HIGH) ? HIGH : LOW);
    return GPIO_OK;
}

static gpio_error_t esp32_gpio_read(uint8_t pin, gpio_state_t *out_state) {
    /* MISRA-C: فحص المؤشرات قبل استخدامها (Null Pointer Check) */
    if ((pin > 39U) || (out_state == NULL)) {
        return GPIO_ERR_INVALID_PIN;
    }
    *out_state = (digitalRead(pin) == HIGH) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
    return GPIO_OK;
}

static gpio_error_t esp32_gpio_toggle(uint8_t pin) {
    gpio_state_t current_state;
    gpio_error_t err = esp32_gpio_read(pin, &current_state);

    if (err == GPIO_OK) {
        gpio_state_t new_state = (current_state == GPIO_STATE_HIGH) ? GPIO_STATE_LOW : GPIO_STATE_HIGH;
        err = esp32_gpio_write(pin, new_state);
    }
    return err;
}

/* ربط الدوال بواجهة الـ GPIO */
const gpio_interface_t esp32_gpio_interface = {
    .init   = esp32_gpio_init,
    .write  = esp32_gpio_write,
    .read   = esp32_gpio_read,
    .toggle = esp32_gpio_toggle
};


/* ==========================================
 * 2. ESP32 PWM (LEDC) Implementation
 * ========================================== */

#define MAX_LEDC_CHANNELS (16U)
static uint8_t next_free_channel = 0U;

/* مصفوفة لربط رقم المنفذ (Pin) بقناة الـ PWM (Channel) لتحديث السرعة لاحقاً O(1) */
static uint8_t pin_to_channel_map[40] = { [0 ... 39] = 255U }; /* 255 تعني غير مستخدم */

static pwm_error_t esp32_pwm_init(uint8_t pin, uint32_t frequency_hz, uint8_t resolution_bits) {
    if (pin > 39U) {
        return PWM_ERR_INVALID_PIN;
    }
    if (next_free_channel >= MAX_LEDC_CHANNELS) {
        return PWM_ERR_CHANNEL_IN_USE; /* نفدت القنوات العتادية */
    }
    if (pin_to_channel_map[pin] != 255U) {
        return PWM_ERR_CHANNEL_IN_USE; /* المنفذ تمت تهيئته مسبقاً */
    }

    uint8_t channel = next_free_channel;

    /* دوال ESP32 لتشغيل الـ PWM */
    ledcSetup(channel, (double)frequency_hz, resolution_bits);
    ledcAttachPin(pin, channel);

    /* حفظ القناة في المصفوفة للوصول السريع، وزيادة العداد */
    pin_to_channel_map[pin] = channel;
    next_free_channel++;

    return PWM_OK;
}

static pwm_error_t esp32_pwm_set_duty(uint8_t pin, float duty_cycle_percent) {
    if (pin > 39U) {
        return PWM_ERR_INVALID_PIN;
    }

    uint8_t channel = pin_to_channel_map[pin];
    if (channel == 255U) {
        return PWM_ERR_INVALID_PIN; /* المنفذ لم يتم تهيئته كـ PWM */
    }

    /*
     * حماية رياضية (Clamping): منع القيم المتطرفة من تدمير المحركات
     */
    if (duty_cycle_percent > 100.0f) { duty_cycle_percent = 100.0f; }
    if (duty_cycle_percent < 0.0f)   { duty_cycle_percent = 0.0f; }

    /* تحويل النسبة المئوية إلى قيمة رقمية صالحة لمسجل الـ PWM بافتراض دقة 8 بت (0-255) */
    /* يمكن جعل الدقة ديناميكية لاحقاً بناءً على resolution_bits */
    uint32_t duty_value = (uint32_t)((duty_cycle_percent / 100.0f) * 255.0f);

    ledcWrite(channel, duty_value);

    return PWM_OK;
}

static pwm_error_t esp32_pwm_stop(uint8_t pin) {
    /* الإيقاف هو ببساطة جعل دورة العمل صفراً */
    return esp32_pwm_set_duty(pin, 0.0f);
}

/* ربط الدوال بواجهة الـ PWM */
const pwm_interface_t esp32_pwm_interface = {
    .init     = esp32_pwm_init,
    .set_duty = esp32_pwm_set_duty,
    .stop     = esp32_pwm_stop
};