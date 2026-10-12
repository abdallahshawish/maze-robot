#include "dijkstra_strategy.h"
#include <stddef.h>

/* دالة مساعدة لتحويل الإحداثيات (x, y) إلى فهرس أحادي الأبعاد (Index) */
static inline uint16_t get_index(cell_pos_t pos) {
    return (uint16_t)(pos.y * MAZE_WIDTH + pos.x);
}

/* دالة مساعدة للحصول على الإحداثيات من الفهرس */
static inline cell_pos_t get_pos(uint16_t index) {
    cell_pos_t pos;
    pos.x = (uint8_t)(index % MAZE_WIDTH);
    pos.y = (uint8_t)(index / MAZE_WIDTH);
    return pos;
}

void dijkstra_init(maze_map_t *maze) {
    if (maze == NULL) return;

    for (uint16_t i = 0; i < NUM_CELLS; ++i) {
        maze->walls[i] = 0U; /* افتراض عدم وجود جدران (يمكن تعديله حسب قوانين المسابقة) */
        maze->costs[i] = COST_INFINITY;
        maze->path_parent[i] = COST_INFINITY; /* يعني لا يوجد والد */
        maze->visited[i] = false;
    }
}

void dijkstra_update_wall(maze_map_t *maze, cell_pos_t pos, uint8_t wall_mask) {
    if (maze == NULL) return;
    if (pos.x >= MAZE_WIDTH || pos.y >= MAZE_HEIGHT) return;

    uint16_t idx = get_index(pos);
    maze->walls[idx] |= wall_mask; /* إضافة الجدار باستخدام الـ OR */
}

bool dijkstra_calculate_path(maze_map_t *maze, cell_pos_t start, cell_pos_t goal) {
    if (maze == NULL) return false;

    /* إعادة تهيئة مصفوفات التكلفة والزيارة للحساب الجديد */
    for (uint16_t i = 0; i < NUM_CELLS; ++i) {
        maze->costs[i] = COST_INFINITY;
        maze->path_parent[i] = COST_INFINITY;
        maze->visited[i] = false;
    }

    uint16_t start_idx = get_index(start);
    uint16_t goal_idx = get_index(goal);

    maze->costs[start_idx] = 0;

    /*
     * الحلقة الرئيسية للخوارزمية.
     * بما أن العدد الأقصى للخلايا هو 256، فإن هذا اللوب سيكون سريعاً جداً على ESP32
     */
    for (uint16_t step = 0; step < NUM_CELLS; ++step) {

        /* 1. العثور على الخلية ذات التكلفة الأقل التي لم تتم زيارتها */
        uint16_t min_cost = COST_INFINITY;
        uint16_t current_idx = COST_INFINITY;

        for (uint16_t i = 0; i < NUM_CELLS; ++i) {
            if (!maze->visited[i] && maze->costs[i] < min_cost) {
                min_cost = maze->costs[i];
                current_idx = i;
            }
        }

        /* إذا لم نجد أي خلية أو وصلنا للهدف، ننهي البحث */
        if (current_idx == COST_INFINITY) break;
        if (current_idx == goal_idx) break;

        /* تمييز الخلية كتمت الزيارة */
        maze->visited[current_idx] = true;
        cell_pos_t curr_pos = get_pos(current_idx);

        /*
         * 2. تحديث جيران الخلية الحالية (الاسترخاء - Relaxation)
         * يجب التحقق من الجدران قبل تحديث الجار!
         */
        uint8_t current_walls = maze->walls[current_idx];

        /* التحقق باتجاه الشمال (Y يزيد أو ينقص حسب نظام الإحداثيات، لنفترض Y ينقص للشمال) */
        if (!(current_walls & WALL_NORTH) && curr_pos.y > 0) {
            cell_pos_t neighbor = {curr_pos.x, (uint8_t)(curr_pos.y - 1)};
            uint16_t n_idx = get_index(neighbor);

            /*
             * هنا نضع منطق تكلفة الانعطاف المتقدم:
             * إذا كان الاتجاه السابق ليس شمالاً، نضيف COST_TURN.
             * للتبسيط، نضيف COST_STRAIGHT فقط الآن، لكنك ستدمج منطق الاتجاه هنا.
             */
            uint16_t new_cost = maze->costs[current_idx] + COST_STRAIGHT;

            if (new_cost < maze->costs[n_idx]) {
                maze->costs[n_idx] = new_cost;
                maze->path_parent[n_idx] = current_idx;
            }
        }

        /* التحقق باتجاه الجنوب */
        if (!(current_walls & WALL_SOUTH) && curr_pos.y < (MAZE_HEIGHT - 1)) {
            cell_pos_t neighbor = {curr_pos.x, (uint8_t)(curr_pos.y + 1)};
            uint16_t n_idx = get_index(neighbor);
            uint16_t new_cost = maze->costs[current_idx] + COST_STRAIGHT;
            if (new_cost < maze->costs[n_idx]) {
                maze->costs[n_idx] = new_cost;
                maze->path_parent[n_idx] = current_idx;
            }
        }

        /* التحقق باتجاه الشرق */
        if (!(current_walls & WALL_EAST) && curr_pos.x < (MAZE_WIDTH - 1)) {
            cell_pos_t neighbor = {(uint8_t)(curr_pos.x + 1), curr_pos.y};
            uint16_t n_idx = get_index(neighbor);
            uint16_t new_cost = maze->costs[current_idx] + COST_STRAIGHT;
            if (new_cost < maze->costs[n_idx]) {
                maze->costs[n_idx] = new_cost;
                maze->path_parent[n_idx] = current_idx;
            }
        }

        /* التحقق باتجاه الغرب */
        if (!(current_walls & WALL_WEST) && curr_pos.x > 0) {
            cell_pos_t neighbor = {(uint8_t)(curr_pos.x - 1), curr_pos.y};
            uint16_t n_idx = get_index(neighbor);
            uint16_t new_cost = maze->costs[current_idx] + COST_STRAIGHT;
            if (new_cost < maze->costs[n_idx]) {
                maze->costs[n_idx] = new_cost;
                maze->path_parent[n_idx] = current_idx;
            }
        }
    }

    /* هل وصلنا للهدف فعلاً؟ */
    return (maze->costs[goal_idx] != COST_INFINITY);
}

bool dijkstra_get_next_cell(const maze_map_t *maze, cell_pos_t current_pos, cell_pos_t goal_pos, cell_pos_t *next_pos) {
    if (maze == NULL || next_pos == NULL) return false;

    uint16_t current_idx = get_index(current_pos);
    uint16_t goal_idx = get_index(goal_pos);

    if (current_idx == goal_idx) return false; /* وصلنا للهدف */

    /*
     * يجب تتبع الـ path_parent من الهدف (Goal) رجوعاً إلى البداية (Start)
     * لمعرفة الخطوة التالية من نقطتنا الحالية.
     */
    uint16_t node = goal_idx;
    uint16_t prev_node = node;

    /* تتبع عكسي حتى نصل للخلية التي تسبق الخلية الحالية مباشرة في المسار */
    while (maze->path_parent[node] != current_idx && maze->path_parent[node] != COST_INFINITY) {
        prev_node = node;
        node = maze->path_parent[node];
    }

    if (maze->path_parent[node] == COST_INFINITY && node != current_idx) {
        /* المسار مقطوع (No path found) */
        return false;
    }

    *next_pos = get_pos(node);
    return true;
}