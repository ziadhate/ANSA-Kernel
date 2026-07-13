// test_kernel_task_create.c
#include "unity.h"
#include "scheduler.h"
#include <string.h>

static void dummy_task(void *arg) {
	(void)arg; 
	while(1) {} 
}

void setUp(void)    { kernel_test_reset(); }
void tearDown(void) {}

/* ---- Return codes / registration ---- */

void test_create_success_returns_zero(void) {
    uint8_t rc = kernel_task_create(dummy_task, NULL, 1, 128);
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_UINT8(1, total_tasks);
}

void test_create_fills_correct_slot(void) {
    kernel_task_create(dummy_task, NULL, 1, 128);
    uint8_t id = total_tasks - 1;
    TEST_ASSERT_EQUAL_PTR(dummy_task, task_table[id].task_function);
    TEST_ASSERT_EQUAL(TASK_STATE_READY, task_table[id].state);
    TEST_ASSERT_EQUAL_UINT8(1, task_table[id].priority);
    TEST_ASSERT_EQUAL_UINT32(0, task_table[id].tick_counter);
}

void test_create_rejects_when_max_tasks_reached(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, kernel_task_create(dummy_task, NULL, 1, 64));
    }
    TEST_ASSERT_EQUAL_UINT8(1, kernel_task_create(dummy_task, NULL, 1, 64));
    TEST_ASSERT_EQUAL_UINT8(MAX_TASKS, total_tasks); /* pas incrémenté sur échec */
}

void test_create_rejects_when_pool_exhausted(void) {
    uint8_t rc = kernel_task_create(dummy_task, NULL, 1, KERNEL_STACK_POOL_SIZE + 2);
    TEST_ASSERT_EQUAL_UINT8(2, rc);
    TEST_ASSERT_EQUAL_UINT8(0, total_tasks); /* rien alloué */
    TEST_ASSERT_EQUAL_UINT32(0, next_free_stack_word);
}

/* ---- Allocation odd stack_size -> rounded up (alignement) ---- */

void test_odd_stack_size_rounds_up(void) {
    kernel_task_create(dummy_task, NULL, 1, 63); /* impair */
    TEST_ASSERT_EQUAL_UINT32(64, next_free_stack_word);
}

void test_even_stack_size_unchanged(void) {
    kernel_task_create(dummy_task, NULL, 1, 64);
    TEST_ASSERT_EQUAL_UINT32(64, next_free_stack_word);
}

/* ---- Allocation / non-overlap entre tâches ---- */

void test_two_tasks_get_non_overlapping_slices(void) {
    kernel_task_create(dummy_task, NULL, 1, 32);
    kernel_task_create(dummy_task, NULL, 1, 32);
    /* task 0 sp doit être < task 1 sp (pool grandit vers le haut) */
    TEST_ASSERT_TRUE(task_table[0].sp < task_table[1].sp);
}

/* ---- 8-byte alignment ---- */

void test_sp_is_8byte_aligned(void) {
    kernel_task_create(dummy_task, NULL, 1, 63); /* impair -> forcé pair */
    uint8_t id = total_tasks - 1;
    TEST_ASSERT_EQUAL_UINT32(0, ((uint32_t)task_table[id].sp) & 0x7);
}

/* ---- kernel_stack_init() frame content (test direct, sans passer par create) ---- */

void test_stack_init_frame_layout(void) {
    static uint32_t local_stack[32] __attribute__((aligned(8)));
    uint32_t *stack_top = &local_stack[32];

    uint32_t *sp = kernel_stack_init(stack_top, dummy_task, (void*)0xCAFEBABE);

    TEST_ASSERT_EQUAL_PTR(stack_top - 16, sp);          /* 16 mots poussés */
    TEST_ASSERT_EQUAL_UINT32(0xCAFEBABE, sp[8]);          /* R0 = arg */
    TEST_ASSERT_EQUAL_UINT32(0x01010101, sp[9]);          /* R1 */
    TEST_ASSERT_EQUAL_UINT32(0x02020202, sp[10]);         /* R2 */
    TEST_ASSERT_EQUAL_UINT32(0x03030303, sp[11]);         /* R3 */
    TEST_ASSERT_EQUAL_UINT32(0x12121212, sp[12]);         /* R12 */
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFE, sp[13]);         /* LR/EXC_RETURN */
    TEST_ASSERT_EQUAL_UINT32((uint32_t)dummy_task, sp[14]); /* PC */
    TEST_ASSERT_EQUAL_UINT32(0x01000000, sp[15]);         /* xPSR Thumb bit */
}

void test_stack_init_r4_to_r11_zeroed(void) {
    static uint32_t local_stack[32] __attribute__((aligned(8)));
    uint32_t *stack_top = &local_stack[32];
    uint32_t *sp = kernel_stack_init(stack_top, dummy_task, NULL);
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_UINT32(0, sp[i]);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_success_returns_zero);
    RUN_TEST(test_create_fills_correct_slot);
    RUN_TEST(test_create_rejects_when_max_tasks_reached);
    RUN_TEST(test_create_rejects_when_pool_exhausted);
    RUN_TEST(test_odd_stack_size_rounds_up);
    RUN_TEST(test_even_stack_size_unchanged);
    RUN_TEST(test_two_tasks_get_non_overlapping_slices);
    RUN_TEST(test_sp_is_8byte_aligned);
    RUN_TEST(test_stack_init_frame_layout);
    RUN_TEST(test_stack_init_r4_to_r11_zeroed);
    return UNITY_END();
}
