#include "robot_state_machine.h"
#include <stddef.h>

/* دوال معالجة الحالات (State Handlers) - معرفة كـ Static لعزلها */
static void handle_state_init(robot_context_t *ctx);
static void handle_state_idle(robot_context_t *ctx);
static void handle_state_explore(robot_context_t *ctx);
static void handle_state_planning(robot_context_t *ctx);
static void handle_state_fast_run(robot_context_t *ctx);
static void handle_state_error(robot_context_t *ctx);

fsm_error_t fsm_init(robot_context_t *ctx)
{
    if (ctx == NULL) {
        return FSM_ERR_NULL_PTR;
    }

    ctx->current_state = STATE_INIT;
    ctx->target_reached = 0U;

    return FSM_OK;
}

fsm_error_t fsm_update(robot_context_t *ctx)
{
    if (ctx == NULL) {
        return FSM_ERR_NULL_PTR;
    }

    /*
     * MISRA-C: استخدام Switch-Case صارم بدلاً من سلاسل If-Else المعقدة
     * هذا يضمن تعقيداً زمنياً ثابتاً O(1) ويسهل قراءة الكود وتتبع الأخطاء.
     */
    switch (ctx->current_state) {
        case STATE_INIT:
            handle_state_init(ctx);
            break;

        case STATE_IDLE:
            handle_state_idle(ctx);
            break;

        case STATE_EXPLORE:
            handle_state_explore(ctx);
            break;

        case STATE_PLANNING:
            handle_state_planning(ctx);
            break;

        case STATE_FAST_RUN:
            handle_state_fast_run(ctx);
            break;

        case STATE_ERROR:
        default: /* الحماية ضد أي حالة غير معروفة */
            handle_state_error(ctx);
            break;
    }

    return FSM_OK;
}

fsm_error_t fsm_set_state(robot_context_t *ctx, robot_state_t new_state)
{
    if (ctx == NULL) {
        return FSM_ERR_NULL_PTR;
    }
    ctx->current_state = new_state;
    return FSM_OK;
}

/* ==========================================
 * State Handlers Implementation
 * ========================================== */

static void handle_state_init(robot_context_t *ctx) {
    /*
     * هنا نقوم بتهيئة حساسات الليزر وتصفير الإنكودرات.
     * إذا اكتملت التهيئة بنجاح، ننتقل لحالة الانتظار.
     */
    motor_stop(ctx->motor_left);
    motor_stop(ctx->motor_right);
    encoder_reset(ctx->encoder_left);
    encoder_reset(ctx->encoder_right);

    /* الانتقال للحالة التالية */
    ctx->current_state = STATE_IDLE;
}

static void handle_state_idle(robot_context_t *ctx) {
    /*
     * الروبوت جاهز. ننتظر تمرير يد المستخدم أمام الحساس الأمامي (أو ضغطة زر) للبدء.
     */
    uint16_t front_dist = 0;
    laser_sensor_read_mm(ctx->laser_front, &front_dist);

    if (front_dist < 50U) { /* يد المستخدم على مسافة أقل من 5 سم */
        ctx->current_state = STATE_EXPLORE;
    }
}

static void handle_state_explore(robot_context_t *ctx) {
    /*
     * منطق الاستكشاف:
     * 1. اقرأ الليزر (يمين، يسار، أمام).
     * 2. حدّث خريطة المتاهة (خوارزمية الفيضان ستكون في ملف منفصل).
     * 3. أرسل السرعات المطلوبة لمتغيرات الـ PID (مثلاً: ابقَ في المنتصف وتجنب الجدران).
     */

    if (ctx->target_reached == 1U) {
        /* وصل للمركز، قم بالفرملة فوراً وانتقل للتخطيط */
        motor_brake(ctx->motor_left);
        motor_brake(ctx->motor_right);
        ctx->current_state = STATE_PLANNING;
    } else {
        /* الكود المبدئي: تحرك للأمام بسرعة بطيئة للاستكشاف */
        /* استدعاء استراتيجية الاستكشاف يوضع هنا */
    }
}

static void handle_state_planning(robot_context_t *ctx) {
    /*
     * المحركات متوقفة تماماً هنا.
     * المعالج (ESP32) سيقوم بتشغيل خوارزمية (ديكسترا) ببطء في الخلفية.
     * بمجرد انتهاء الخوارزمية من توليد المسار الأمثل (بدون زوايا حادة لإنقاذ الإنكودر):
     */

    // bool is_path_ready = run_dijkstra_algorithm();
    bool is_path_ready = true; /* مؤقت للتوضيح */

    if (is_path_ready) {
        /* تصفير متحكمات السرعة لتبدأ من جديد */
        pid_reset(ctx->pid_speed);
        pid_reset(ctx->pid_heading);
        ctx->current_state = STATE_FAST_RUN;
    }
}

static void handle_state_fast_run(robot_context_t *ctx) {
    /*
     * هنا يتجلى الذكاء الصناعي الكينماتيكي.
     * يقرأ الروبوت المسار من ديكسترا، ويطبق ملف (S-Curve) للانطلاق بأقصى سرعة.
     * يتم إغلاق حلقة التحكم (Closed-loop) بالاعتماد المكثف على الإنكودرين.
     */
}

static void handle_state_error(robot_context_t *ctx) {
    /*
     * حالة الطوارئ (Fail-Safe State):
     * إذا سحب حساس الـ I2C التيار (Lock-up)، أو قرأ الإنكودر قيمة غير منطقية.
     * نقطع الطاقة عن المحركات فوراً لحماية الروبوت من الاصطدام وتدمير التروس.
     */
    motor_stop(ctx->motor_left);
    motor_stop(ctx->motor_right);

    /* يمكن إضافة كود لإطلاق صوت طنين (Buzzer) لتنبيه المهندس */
}