#ifndef I2C_INTERFACE_H
#define I2C_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    I2C_OK = 0U,
    I2C_ERR_TIMEOUT,
    I2C_ERR_DEVICE_NOT_FOUND,
    I2C_ERR_BUS_LOCKED,
    I2C_ERR_NULL_PTR
} i2c_error_t;

/*
 * تحديد رقم الناقل (Bus ID)
 * في ESP32 سنستخدم I2C0 للإنكودر (سريع) و I2C1 لليزر (عادي)
 */
typedef enum {
    I2C_BUS_0 = 0U,
    I2C_BUS_1 = 1U
} i2c_bus_t;

/* OOP in C: I2C Interface */
typedef struct {
    /* تهيئة الناقل مع تحديد التردد (مثلاً 400kHz) */
    i2c_error_t (*init)(i2c_bus_t bus, uint8_t sda_pin, uint8_t scl_pin, uint32_t freq_hz);

    /* قراءة مسجل أو عدة مسجلات من جهاز */
    i2c_error_t (*read_regs)(i2c_bus_t bus, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t length);

    /* كتابة بايت أو عدة بايتات إلى مسجل */
    i2c_error_t (*write_regs)(i2c_bus_t bus, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t length);
} i2c_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* I2C_INTERFACE_H */