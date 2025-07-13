#include "threads/thread.h"
#include "devices/timer.h"
#include "projects/crossroads/blinker.h"
#include "projects/crossroads/crossroads.h"
#include "projects/crossroads/vehicle.h"
#include "projects/crossroads/ats.h"
#include "projects/crossroads/map.h"  // vehicle_path 정의 포함된 헤더

/* path. A:0 B:1 C:2 D:3 */
const struct position vehicle_path[4][4][12] = {
	/* from A */ {
		/* to A */
		{{4,0},{4,1},{4,2},{4,3},{4,4},{3,4},{2,4},{2,3},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{4,0},{4,1},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{4,0},{4,1},{4,2},{4,3},{4,4},{3,4},{2,4},{1,4},{0,4},{-1,-1},}
	},
	/* from B */ {
		/* to A */
		{{6,4},{5,4},{4,4},{3,4},{2,4},{2,3},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{6,4},{5,4},{4,4},{3,4},{2,4},{2,3},{2,2},{3,2},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{6,4},{5,4},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{6,4},{5,4},{4,4},{3,4},{2,4},{1,4},{0,4},{-1,-1},}
	},
	/* from C */ {
		/* to A */
		{{2,6},{2,5},{2,4},{2,3},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{2,6},{2,5},{2,4},{2,3},{2,2},{3,2},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{2,6},{2,5},{2,4},{2,3},{2,2},{3,2},{4,2},{4,3},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{2,6},{2,5},{2,4},{1,4},{0,4},{-1,-1},}
	},
	/* from D */ {
		/* to A */
		{{0,2},{1,2},{2,2},{2,1},{2,0},{-1,-1},},
		/* to B */
		{{0,2},{1,2},{2,2},{3,2},{4,2},{5,2},{6,2},{-1,-1},},
		/* to C */
		{{0,2},{1,2},{2,2},{3,2},{4,2},{4,3},{4,4},{4,5},{4,6},{-1,-1},},
		/* to D */
		{{0,2},{1,2},{2,2},{3,2},{4,2},{4,3},{4,4},{3,4},{2,4},{1,4},{0,4},{-1,-1},}
	}
};

void parse_vehicles(struct vehicle_info *vehicle_info, char *input)
{
	int idx = 0;
	char *save_ptr;  // for strtok_r
	char *token = strtok_r(input, ":", &save_ptr);

	while (token != NULL && idx < 10) {
		struct vehicle_info *vi = &vehicle_info[idx];

		vi->id = token[0];
		vi->start = token[1];
		vi->dest = token[2];
		vi->state = VEHICLE_STATUS_READY;
		vi->position.row = -1;
		vi->position.col = -1;
		vi->map_locks = NULL;

		if (strlen(token) == 3) {
			// 일반 차량
			vi->type = VEHICL_TYPE_NORMAL;
			vi->arrival = -1;
			vi->golden_time = -1;
		} else {
			// 앰뷸런스 ex: fAB5.12
			vi->type = VEHICL_TYPE_AMBULANCE;

			// 출발 시점 파싱 (예: 5)
			vi->arrival = atoi(&token[3]);

			// 도착 제한 시점 파싱 (예: 12)
			char *dot = strchr(token, '.');
			if (dot != NULL)
				vi->golden_time = atoi(dot + 1);
			else
				vi->golden_time = vi->arrival + 10;
		}

		idx++;
		token = strtok_r(NULL, ":", &save_ptr);
	}
}

static int is_position_outside(struct position pos)
{
	return (pos.row == -1 || pos.col == -1);
}

/* 이동 시도 */
static int try_move(int start, int dest, int step, struct vehicle_info *vi)
{
	struct position pos_cur = vi->position;
	struct position pos_next = vehicle_path[start][dest][step];

	if (vi->state == VEHICLE_STATUS_RUNNING) {
		if (is_position_outside(pos_next)) {
			priority_lock_release(&vi->map_locks[pos_cur.row][pos_cur.col]);
			vi->position.row = vi->position.col = -1;
			return 0;
		}
	}

	// 🔒 우선순위 락 획득
	priority_lock_acquire(&vi->map_locks[pos_next.row][pos_next.col]);

	if (vi->state == VEHICLE_STATUS_READY) {
		vi->state = VEHICLE_STATUS_RUNNING;
	} else if (!is_position_outside(pos_cur)) {
		priority_lock_release(&vi->map_locks[pos_cur.row][pos_cur.col]);
	}

	vi->position = pos_next;
	return 1;
}

void init_on_mainthread(int thread_cnt) {}

void vehicle_loop(void *_vi)
{
	int res, step = 0;
	struct vehicle_info *vi = _vi;

	int start = vi->start - 'A';
	int dest = vi->dest - 'A';

	vi->position.row = vi->position.col = -1;
	vi->state = VEHICLE_STATUS_READY;

	// 🕑 앰뷸런스 대기
	while (vi->type == VEHICL_TYPE_AMBULANCE && crossroads_step < vi->arrival)
		unitstep_changed();

	while (1) {
		res = try_move(start, dest, step, vi);
		if (res == 1) step++;
		if (res == 0) break;
		unitstep_changed();
	}

	vi->state = VEHICLE_STATUS_FINISHED;
}
