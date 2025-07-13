#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/timer.h"

#include "projects/crossroads/blinker.h"
#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/vehicle.h"
#include "projects/crossroads/ats.h"

#define CONTROL_THREAD_PRIORITY (PRI_DEFAULT + 5)

extern int crossroads_step;

// 전역 vehicle 정보
static struct vehicle_info *global_vehicles;
static int global_vehicle_count;

// 🚦 차량 활성화 여부 판단
static bool is_active(struct vehicle_info *vi) {
	if (vi->state == VEHICLE_STATUS_FINISHED)
		return false;
	if (vi->type == VEHICL_TYPE_AMBULANCE && crossroads_step < vi->arrival)
		return false;
	return true;
}

// ✅ step을 제어하는 제어 스레드
void control_node_main(void *aux UNUSED) {
	while (true) {
		int active_count = 0;

		for (int i = 0; i < global_vehicle_count; i++) {
			if (is_active(&global_vehicles[i]))
				active_count++;
		}

		if (active_count == 0)
			break;

		crossroads_step++;
		unitstep_changed();

	}
}

// 🔧 blinker 초기화: vehicle 정보 등록
void init_blinker(struct blinker_info* blinkers, struct priority_lock **map_locks, struct vehicle_info * vehicle_info) {
	global_vehicles = vehicle_info;

	// vehicle 개수 계산 (도착 시간이 -1이 아니면 유효 차량)
	int count = 0;
	while (vehicle_info[count].start != 0 && vehicle_info[count].dest != 0) {
		count++;
	}
	global_vehicle_count = count;
}

// 🚀 blinker 시작 시 제어 스레드 생성
void start_blinker() {
	thread_create("control", CONTROL_THREAD_PRIORITY, control_node_main, NULL);
}
