#ifndef LASER_SENSOR_H
#define LASER_SENSOR_H

#include <stdint.h>
#include "i2c_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LASER_OK = 0U,
    LASER_ERR_NULL_PTR,
    LASER_ERR_COMMUNICATION,
    LASER_ERR_OUT_OF_RANGE
} laser_error_t;

/*
 * تغليف حالة حساس الليزر (Encapsulation)
 */
typedef struct {
    const i2c_interface_t *i2c; /* واجهة الـ I2C المحقونة */
    i2c_bus_t  bus_id;          /* رقم الناقل (I2C1) */
    uint8_t    device_address;  /* عنوان الحساس */
    uint16_t   last_distance_mm;/* آخر مسافة مقروءة */
} laser_sensor_t;

/* دوال التحكم بالحساس */
laser_error_t laser_sensor_init(laser_sensor_t *sensor,
                                const i2c_interface_t *i2c,
                                i2c_bus_t bus_id,
                                uint8_t device_address);

laser_error_t laser_sensor_read_mm(laser_sensor_t *sensor, uint16_t *out_distance);

#ifdef __cplusplus
}
#endif

#endif /* LASER_SENSOR_H */