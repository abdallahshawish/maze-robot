#include "encoder_sensor.h"

/*
 * بافتراض استخدام حساس AS5600:
 * عنوان I2C الافتراضي هو 0x36.
 * مسجل الزاوية الخام (Raw Angle) موجود في العناوين 0x0C (High) و 0x0D (Low).
 * الدقة: 12-بت (القيم من 0 إلى 4095).
 */
#define REG_RAW_ANGLE_HIGH    (0x0CU)
#define ENCODER_RESOLUTION    (4096)
#define ENCODER_HALF_RES      (2048) /* النصف (180 درجة) لاكتشاف الدوران العكسي */

encoder_error_t encoder_init(encoder_sensor_t *encoder,
                             const i2c_interface_t *i2c,
                             i2c_bus_t bus_id,
                             uint8_t device_address,
                             uint8_t is_inverted)
{
    if ((encoder == NULL) || (i2c == NULL)) {
        return ENCODER_ERR_NULL_PTR;
    }

    encoder->i2c = i2c;
    encoder->bus_id = bus_id;
    encoder->device_address = device_address;
    encoder->total_ticks = 0;
    encoder->is_inverted = is_inverted;

    /* قراءة ابتدائية للحصول على الزاوية الصفرية (Baseline) */
    uint8_t data[2] = {0U, 0U};
    i2c_error_t err = encoder->i2c->read_regs(encoder->bus_id, encoder->device_address, REG_RAW_ANGLE_HIGH, data, 2U);

    if (err != I2C_OK) {
        return ENCODER_ERR_COMMUNICATION;
    }

    /* دمج البايتين للحصول على قيمة 12 بت (0-4095) */
    encoder->last_raw_angle = (uint16_t)(((uint16_t)data[0] << 8U) | (uint16_t)data[1]);

    return ENCODER_OK;
}

encoder_error_t encoder_update(encoder_sensor_t *encoder)
{
    if (encoder == NULL) {
        return ENCODER_ERR_NULL_PTR;
    }

    uint8_t data[2] = {0U, 0U};
    i2c_error_t err = encoder->i2c->read_regs(encoder->bus_id, encoder->device_address, REG_RAW_ANGLE_HIGH, data, 2U);

    if (err != I2C_OK) {
        return ENCODER_ERR_COMMUNICATION;
    }

    uint16_t current_angle = (uint16_t)(((uint16_t)data[0] << 8U) | (uint16_t)data[1]);

    /*
     * الرياضيات الحرجة لاكتشاف التجاوز (Wrap-around / Overflow Logic)
     * نحسب الفارق بين القراءة الحالية والسابقة
     */
    int32_t delta = (int32_t)current_angle - (int32_t)encoder->last_raw_angle;

    /*
     * إذا تجاوز الفارق 180 درجة (2048 نبضة)، فهذا يعني أن العجلة دارت وتجاوزت الصفر
     * باتجاه الأمام أو الخلف. نقوم بتصحيح الفارق.
     */
    if (delta > ENCODER_HALF_RES) {
        delta -= ENCODER_RESOLUTION; /* العجلة دارت للخلف ومرت بالصفر */
    } else if (delta < -ENCODER_HALF_RES) {
        delta += ENCODER_RESOLUTION; /* العجلة دارت للأمام ومرت بالصفر */
    }

    /* تطبيق الانعكاس البرمجي إذا كان المحرك معكوساً ميكانيكياً */
    if (encoder->is_inverted == 1U) {
        delta = -delta;
    }

    /* تحديث العداد الكلي والزاوية السابقة */
    encoder->total_ticks += delta;
    encoder->last_raw_angle = current_angle;

    return ENCODER_OK;
}

encoder_error_t encoder_get_total_ticks(const encoder_sensor_t *encoder, int64_t *out_ticks)
{
    if ((encoder == NULL) || (out_ticks == NULL)) {
        return ENCODER_ERR_NULL_PTR;
    }

    /*
     * ملاحظة متعلقة بالأنظمة اللحظية (RTOS):
     * إذا كان المتغير من نوع 64-بت والمتحكم 32-بت (مثل ESP32)،
     * قد تحدث مشكلة (Data Tearing) إذا تم تحديث المتغير في مهمة (Task) وقراءته في مهمة أخرى.
     * يجب حماية القراءة مستقبلاً باستخدام (Mutex) أو إيقاف المقاطعات مؤقتاً (Critical Section).
     */
    *out_ticks = encoder->total_ticks;

    return ENCODER_OK;
}

encoder_error_t encoder_reset(encoder_sensor_t *encoder)
{
    if (encoder == NULL) {
        return ENCODER_ERR_NULL_PTR;
    }
    encoder->total_ticks = 0;
    return ENCODER_OK;
}