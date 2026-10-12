#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#include <stdint.h>

/*
 * MISRA-C: استخدام اللاحقة 'U' للأرقام غير السالبة (Unsigned)
 * تم تخصيص منافذ ESP32 بناءً على أفضل الممارسات لتجنب منافذ الـ Booting Strapping
 */

/* ==========================================
 * Motor PWM & Direction Pins (ESP32)
 * ========================================== */
#define PIN_MOTOR_L_PWM     (25U)
#define PIN_MOTOR_L_DIR1    (26U)
#define PIN_MOTOR_L_DIR2    (27U)

#define PIN_MOTOR_R_PWM     (14U)
#define PIN_MOTOR_R_DIR1    (12U)
#define PIN_MOTOR_R_DIR2    (13U)

/* ==========================================
 * I2C0 Bus (High Speed 400kHz - Encoders)
 * ========================================== */
#define I2C0_SDA_PIN        (21U)
#define I2C0_SCL_PIN        (22U)

/* ==========================================
 * I2C1 Bus (Standard/Fast Mode - ToF Lasers)
 * ========================================== */
#define I2C1_SDA_PIN        (32U)
#define I2C1_SCL_PIN        (33U)

/* ==========================================
 * I2C Device Addresses
 * ========================================== */
#define I2C_ADDR_ENCODER_L  (0x36U) /* مثال: حساس مغناطيسي AS5600 */
#define I2C_ADDR_ENCODER_R  (0x37U)

#define I2C_ADDR_TOF_FRONT  (0x29U) /* العنوان الافتراضي لـ VL53L0X */
#define I2C_ADDR_TOF_LEFT   (0x30U) /* العناوين المعدلة برمجياً */
#define I2C_ADDR_TOF_RIGHT  (0x31U)

#endif /* PIN_DEFINITIONS_H */