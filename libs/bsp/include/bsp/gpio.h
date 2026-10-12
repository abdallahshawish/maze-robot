#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MISRA-C: تعريف الحالات بوضوح لمنع استخدام الأرقام السحرية (Magic Numbers) */
typedef enum {
    GPIO_DIR_INPUT = 0U,
    GPIO_DIR_OUTPUT,
    GPIO_DIR_INPUT_PULLUP
} gpio_direction_t;

typedef enum {
    GPIO_STATE_LOW = 0U,
    GPIO_STATE_HIGH
} gpio_state_t;

typedef enum {
    GPIO_OK = 0U,
    GPIO_ERR_INVALID_PIN,
    GPIO_ERR_NOT_INITIALIZED
} gpio_error_t;

/*
 * OOP in C: واجهة GPIO (Interface)
 * تعتمد على مؤشرات الدوال (Function Pointers) لإنشاء طبقة تجريد العتاد (HAL).
 */
typedef struct {
    gpio_error_t (*init)(uint8_t pin, gpio_direction_t dir);
    gpio_error_t (*write)(uint8_t pin, gpio_state_t state);
    gpio_error_t (*read)(uint8_t pin, gpio_state_t *out_state);
    gpio_error_t (*toggle)(uint8_t pin);
} gpio_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */