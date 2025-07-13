#ifndef __PROJECTS_PROJECT2_BLINKER_H__
#define __PROJECTS_PROJECT2_BLINKER_H__

#define NUM_BLINKER 4

#include "threads/synch.h"
#include "projects/crossroads/vehicle.h"

struct blinker_info {
    struct lock **map_locks;
    struct vehicle_info *vehicles;
};

void init_blinker(struct blinker_info* blinkers, struct priority_lock **map_locks, struct vehicle_info * vehicle_info);
void start_blinker(void);
void control_node_main(void *aux);  // 이 선언이 꼭 필요!


#endif /* __PROJECTS_PROJECT2_BLINKER_H__ */
