#ifndef ENCODER_SENSOR_H
#define ENCODER_SENSOR_H

#include <stdint.h>
#include "i2c_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ENCODER_OK = 0U,
    ENCODER_ERR_NULL_PTR,
    ENCODER_ERR_COMMUNICATION
} encoder_error_t;

/*
 * تغليف حالة الإنكودر (Encapsulation)
 */
typedef struct {
    const i2c_interface_t *i2c; /* واجهة الـ I2C المحقونة */
    i2c_bus_t  bus_id;          /* رقم الناقل (I2C0 - سريع) */
    uint8_t    device_address;  /* عنوان الحساس المغناطيسي */

    uint16_t   last_raw_angle;  /* الزاوية الخام السابقة (لحساب الفارق) */
    int64_t    total_ticks;     /* إجمالي الخطوات المقطوعة (يقبل القيمة السالبة للرجوع للخلف) */

    uint8_t    is_inverted;     /* لعكس قراءة الإنكودر برمجياً إذا كان المحرك معكوساً */
} encoder_sensor_t;

/* ==========================================
 * Encoder API
 * ========================================== */

encoder_error_t encoder_init(encoder_sensor_t *encoder,
                             const i2c_interface_t *i2c,
                             i2c_bus_t bus_id,
                             uint8_t device_address,
                             uint8_t is_inverted);

/* يجب استدعاء هذه الدالة بتردد عالٍ (مثلاً 500Hz) في مهمة RTOS */
encoder_error_t encoder_update(encoder_sensor_t *encoder);

/* قراءة المسافة الكلية المقطوعة (بالنبضات) */
encoder_error_t encoder_get_total_ticks(const encoder_sensor_t *encoder, int64_t *out_ticks);

/* تصفير العداد (مهمة جداً عند بداية مرحلة "الجري السريع" أو عند تصحيح الانحراف) */
encoder_error_t encoder_reset(encoder_sensor_t *encoder);

#ifdef __cplusplus
}
#endif

#endif /* ENCODER_SENSOR_H */