#ifndef __PROJECTS_PROJECT2_MAPDATA_H__
#define __PROJECTS_PROJECT2_MAPDATA_H__

#include "projects/crossroads/position.h"  // struct position 정의
#include "projects/crossroads/vehicle.h"   // struct vehicle_info 정의
#include "threads/thread.h"                // struct thread
#include "lib/kernel/list.h"
#include <stdbool.h>

extern int crossroads_step;
extern const struct position vehicle_path[4][4][12];

struct priority_semaphore {
    unsigned value;
    struct list waiters;
};

struct priority_lock {
    struct thread *holder;
    struct priority_semaphore sema;
};

void parse_vehicles(struct vehicle_info *vehicle_info, char *input);
void map_draw(void);
void map_draw_vehicle(char id, int row, int col);
void map_draw_reset(void);

void priority_sema_init(struct priority_semaphore *sema, unsigned value);
void priority_sema_down(struct priority_semaphore *sema);
void priority_sema_up(struct priority_semaphore *sema);

void priority_lock_init(struct priority_lock *lock);
void priority_lock_acquire(struct priority_lock *lock);
void priority_lock_release(struct priority_lock *lock);
bool priority_lock_held_by_current_thread(const struct priority_lock *lock);
void init_priority_locks(struct priority_lock ***map_locks);


#endif /* __PROJECTS_PROJECT2_MAPDATA_H__ */