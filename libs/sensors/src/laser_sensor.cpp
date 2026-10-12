#include "laser_sensor.h"

/*
 * العناوين الافتراضية للمسجلات تعتمد على نوع الحساس (كمثال VL53L0X)
 * سنفترض هنا وجود مسجل قراءة المسافة المباشر (لتبسيط الشرح المعماري)
 */
#define REG_RESULT_RANGE_STATUS      (0x14U) /* مسجل النتيجة */

laser_error_t laser_sensor_init(laser_sensor_t *sensor,
                                const i2c_interface_t *i2c,
                                i2c_bus_t bus_id,
                                uint8_t device_address)
{
    /* MISRA-C: فحص المؤشرات */
    if ((sensor == NULL) || (i2c == NULL)) {
        return LASER_ERR_NULL_PTR;
    }

    sensor->i2c = i2c;
    sensor->bus_id = bus_id;
    sensor->device_address = device_address;
    sensor->last_distance_mm = 0xFFFFU; /* 0xFFFF تعني أن القراءة غير صالحة بعد */

    /*
     * التحقق من وجود الحساس على الناقل عبر قراءة مسجل المعرف (WhoAmI)
     * بافتراض مسجل 0xC0 هو معرف الشريحة في VL53L0X
     */
    uint8_t who_am_i = 0U;
    i2c_error_t err = sensor->i2c->read_regs(sensor->bus_id, sensor->device_address, 0xC0U, &who_am_i, 1U);

    if (err != I2C_OK) {
        return LASER_ERR_COMMUNICATION;
    }

    /* يمكنك هنا إرسال تسلسل التهيئة (Init Sequence) الخاص بالحساس */

    return LASER_OK;
}

laser_error_t laser_sensor_read_mm(laser_sensor_t *sensor, uint16_t *out_distance)
{
    if ((sensor == NULL) || (out_distance == NULL)) {
        return LASER_ERR_NULL_PTR;
    }

    uint8_t data[2] = {0U, 0U};

    /* قراءة بايتين متتاليين يمثلان المسافة (MSB و LSB) */
    i2c_error_t err = sensor->i2c->read_regs(sensor->bus_id, sensor->device_address, REG_RESULT_RANGE_STATUS, data, 2U);

    if (err != I2C_OK) {
        return LASER_ERR_COMMUNICATION;
    }

    /* دمج البايتين (Bitwise Shift) */
    uint16_t distance = (uint16_t)(((uint16_t)data[0] << 8U) | (uint16_t)data[1]);

    /* فلترة القيم الخاطئة (Out of Range) - بافتراض أقصى مسافة للمتاهة 1200 ملم */
    if (distance > 1200U) {
        sensor->last_distance_mm = 8190U; /* قيمة متعارف عليها للـ Out of Range */
        *out_distance = 8190U;
        return LASER_ERR_OUT_OF_RANGE;
    }

    sensor->last_distance_mm = distance;
    *out_distance = distance;

    return LASER_OK;
}