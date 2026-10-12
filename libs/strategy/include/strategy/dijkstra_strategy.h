#ifndef DIJKSTRA_STRATEGY_H
#define DIJKSTRA_STRATEGY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * إعدادات المتاهة (مثال لمتاهة 16x16 الكلاسيكية في Micromouse)
 */
#define MAZE_WIDTH      (16U)
#define MAZE_HEIGHT     (16U)
#define NUM_CELLS       (MAZE_WIDTH * MAZE_HEIGHT)
#define COST_INFINITY   (0xFFFFU) /* يمثل اللانهاية في حساب التكلفة (16-bit) */

/*
 * تعريف اتجاهات الجدران باستخدام الـ Bitmasks
 * هذا يسهل تخزين كل جدران الخلية في متغير 8-بت واحد.
 */
#define WALL_NORTH  (0x01U) /* 0000 0001 */
#define WALL_EAST   (0x02U) /* 0000 0010 */
#define WALL_SOUTH  (0x04U) /* 0000 0100 */
#define WALL_WEST   (0x08U) /* 0000 1000 */

/*
 * إحداثيات الخلية
 */
typedef struct {
    uint8_t x;
    uint8_t y;
} cell_pos_t;

/*
 * تكاليف الحركة (Weights) لتفضيل المسارات المستقيمة
 */
#define COST_STRAIGHT (10U) /* تكلفة الحركة لمربع واحد للأمام */
#define COST_TURN     (25U) /* تكلفة الانعطاف 90 درجة (وزن أعلى لتقليل المنعطفات) */

/*
 * بنية المتاهة والخوارزمية
 */
typedef struct {
    /*
     * walls: مصفوفة تخزن جدران كل خلية.
     * Index = y * MAZE_WIDTH + x
     */
    uint8_t walls[NUM_CELLS];

    /*
     * costs: مصفوفة تخزن التكلفة التراكمية (g-cost) من نقطة البداية إلى كل خلية.
     */
    uint16_t costs[NUM_CELLS];

    /*
     * path_parent: مصفوفة لحفظ "الآباء" (الخلية السابقة) لكل خلية لإعادة بناء المسار.
     * تخزن الـ Index للخلية السابقة.
     */
    uint16_t path_parent[NUM_CELLS];

    /* حالة الخلية (تمت الزيارة أم لا) */
    bool visited[NUM_CELLS];

} maze_map_t;

/* ==========================================
 * Dijkstra API
 * ========================================== */

/* تهيئة مصفوفات المتاهة والتكاليف */
void dijkstra_init(maze_map_t *maze);

/*
 * تحديث معلومات الجدران لخلية معينة (تُستدعى أثناء مرحلة الاستكشاف)
 */
void dijkstra_update_wall(maze_map_t *maze, cell_pos_t pos, uint8_t wall_mask);

/*
 * تشغيل الخوارزمية لحساب أقصر مسار.
 * يُفضل استدعاؤها في حالة (STATE_PLANNING) عندما يتوقف الروبوت.
 */
bool dijkstra_calculate_path(maze_map_t *maze, cell_pos_t start, cell_pos_t goal);

/*
 * استرجاع الخلية التالية في المسار (تُستدعى أثناء الجري السريع)
 */
bool dijkstra_get_next_cell(const maze_map_t *maze, cell_pos_t current_pos, cell_pos_t goal_pos, cell_pos_t *next_pos);

#ifdef __cplusplus
}
#endif

#endif /* DIJKSTRA_STRATEGY_H */