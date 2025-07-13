#include <stdio.h>                        // printf
#include <stdlib.h>                       // malloc
#include "threads/interrupt.h"           // 🔥 intr_disable(), intr_set_level()
#include "projects/crossroads/map.h"     // header 선언

#define ANSI_NONE "\033[0m"
#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

#define ON_ANSI_BLACK "\033[40m"
#define ON_ANSI_RED "\033[41m"
#define ON_ANSI_GREEN "\033[42m"
#define ON_ANSI_YELLOW "\033[43m"
#define ON_ANSI_BLUE "\033[44m"
#define ON_ANSI_MAGENTA "\033[45m"
#define ON_ANSI_CYAN "\033[46m"
#define ON_ANSI_WHITE "\033[47m"

#define clear() printf("\033[H\033[J")
#define gotoxy(y,x) printf("\033[%d;%dH", (y), (x))


const char map_draw_default[7][7] = {
	{'X', 'X', ' ', 'X', ' ', 'X', 'X'}, 
	{'X', 'X', ' ', 'X', ' ', 'X', 'X'}, 
	{' ', ' ', ' ', '-', ' ', ' ', ' '}, 
	{'-', '-', '-', '-', '-', '-', '-'}, 
	{' ', ' ', ' ', '-', ' ', ' ', ' '}, 
	{'X', 'X', ' ', '-', ' ', 'X', 'X'}, 
	{'X', 'X', ' ', '-', ' ', 'X', 'X'}, 
};


void map_draw(void)
{
	int i, j;

	clear();

	for (i=0; i<7; i++) {
		for (j=0; j<7; j++) {
			printf("%c ", map_draw_default[i][j]);
		}
		printf("\n");
	}
	printf("unit step: %d\n", crossroads_step);
	gotoxy(0, 0);
}

void map_draw_vehicle(char id, int row, int col)
{
	if (row >= 0 && col >= 0) {
		gotoxy(row + 1, col * 2 + 1);
		printf("%c ", id);
		gotoxy(0, 0);
	}
}

void map_draw_reset(void)
{
	clear();
}

/* 🔁 우선순위 비교 함수: thread 우선순위 기준 */
static bool priority_higher(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED) {
    struct thread *ta = list_entry(a, struct thread, elem);
    struct thread *tb = list_entry(b, struct thread, elem);
    return ta->priority > tb->priority;
}


/* ✅ priority_semaphore 초기화 */
void priority_sema_init(struct priority_semaphore *sema, unsigned value) {
    sema->value = value;
    list_init(&sema->waiters);
}

/* 🔽 down (P 연산): value가 0이면 waiters에 삽입 (우선순위 정렬) */
void priority_sema_down(struct priority_semaphore *sema) {
    enum intr_level old_level = intr_disable();

    while (sema->value == 0) {
        list_insert_ordered(&sema->waiters, &thread_current()->elem, priority_higher, NULL);
        thread_block();
    }

    sema->value--;
    intr_set_level(old_level);
}

/* 🔼 up (V 연산): 대기중인 쓰레드 중 가장 높은 우선순위 스레드 unblock */
void priority_sema_up(struct priority_semaphore *sema) {
    enum intr_level old_level = intr_disable();

    if (!list_empty(&sema->waiters)) {
        list_sort(&sema->waiters, priority_higher, NULL);
        struct thread *t = list_entry(list_pop_front(&sema->waiters), struct thread, elem);
        thread_unblock(t);
    }

    sema->value++;
    intr_set_level(old_level);
}

/* ✅ priority_lock 초기화 */
void priority_lock_init(struct priority_lock *lock) {
    lock->holder = NULL;
    priority_sema_init(&lock->sema, 1);
}

/* 🚗 Lock 획득 */
void priority_lock_acquire(struct priority_lock *lock) {
    priority_sema_down(&lock->sema);
    lock->holder = thread_current();
}

/* 🚗 Lock 해제 */
void priority_lock_release(struct priority_lock *lock) {
    lock->holder = NULL;
    priority_sema_up(&lock->sema);
}

/* 현재 스레드가 lock을 보유 중인지 확인 */
bool priority_lock_held_by_current_thread(const struct priority_lock *lock) {
    return lock->holder == thread_current();
}

void init_priority_locks(struct priority_lock ***map_locks)
{
	int i, j;
	struct priority_lock **_map_locks;

	_map_locks = *map_locks = malloc(sizeof(struct priority_lock *) * 7);
	for (i = 0; i < 7; i++) {
		_map_locks[i] = malloc(sizeof(struct priority_lock) * 7);
		for (j = 0; j < 7; j++) {
			priority_lock_init(&_map_locks[i][j]);
		}
	}
}
