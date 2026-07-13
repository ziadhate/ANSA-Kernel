#include "scheduler.h"
#include <stdio.h>
#include <string.h>

// ---- Mini test framework ----
static int g_tests_run = 0;
static int g_tests_failed = 0;

#define CHECK(cond,msg)                                             \
    do{                                                             \
        g_tests_run++;                                               \
        if(!(cond)){                                               \
            g_tests_failed++;                                        \
            printf("[FAIL] %s (line %d): %s\n",__func__,__LINE__,msg); \
        }															\
		else{                                                     \
            printf("[PASS] %s\n",msg);                            \
        }                                                             \
    }while (0)

// ----Global queue for each test----
static uint8_t g_buf[8];
static RingQueue_t g_q;

static void reset_queue(uint16_t capacity){
    memset(g_buf,0,sizeof(g_buf));
    kernel_queue_init(&g_q,g_buf,capacity);
}

// ----Test 1: basic FIFO push/pop----
static void test_fifo_basic(void){
    printf("\n[TEST] test_fifo_basic\n");
    reset_queue(8);

    CHECK(kernel_queue_push(&g_q,45)==1,"push(45) succeeds");
    CHECK(kernel_queue_push(&g_q,99)==1,"push(99) succeeds");
    CHECK(g_q.count==2, "count=2 after two pushes");

    uint8_t val=0;
    CHECK(kernel_queue_pop(&g_q,&val)==1,"pop() succeeds");
    CHECK(val==45,"pop() returns 45 first (FIFO order)");

    CHECK(kernel_queue_pop(&g_q,&val)==1,"pop() succeeds");
    CHECK(val==99,"pop() returns 99 second (FIFO order)");

    CHECK(g_q.count==0,"count=0 after draining queue");
}

// ----Test 2: pop on empty queue must fail cleanly----
static void test_pop_empty(void){
    printf("\n[TEST] test_pop_empty\n");
    reset_queue(8);

    uint8_t val=0xAA; // Fail detection value to detect accidental write
    uint8_t status=kernel_queue_pop(&g_q,&val);

    CHECK(status==0,"pop() on empty queue returns failure status");
    CHECK(val==0xAA, "pop() on empty queue does not touch output var");
    CHECK(g_q.count==0,"count stays 0 after failed pop");
}

// ----Test 3: push on full queue must fail cleanly (overflow)----
static void test_push_full_overflow(void){
    printf("\n[TEST] test_push_full_overflow\n");
    reset_queue(4); // small capacity to hit the limit fast

    CHECK(kernel_queue_push(&g_q,1)==1,"push 1/4 succeeds");
    CHECK(kernel_queue_push(&g_q,2)==1,"push 2/4 succeeds");
    CHECK(kernel_queue_push(&g_q,3)==1,"push 3/4 succeeds");
    CHECK(kernel_queue_push(&g_q,4)==1,"push 4/4 succeeds (queue now full)");

    /* queue is full, this push must fail and set overflow flag */
    uint8_t status=kernel_queue_push(&g_q,99);
    CHECK(status==0,"push on full queue returns failure status");
    CHECK(g_q.overflow_flag==1,"overflow_flag is set after push-on-full");
    CHECK(g_q.count==4,"count stays at capacity, no silent corruption");

    /* data integrity check: the 4 original values must still be intact */
    uint8_t val;
    kernel_queue_pop(&g_q,&val); CHECK(val==1,"data intact: first value still 1");
    kernel_queue_pop(&g_q,&val); CHECK(val==2,"data intact: second value still 2");
    kernel_queue_pop(&g_q,&val); CHECK(val==3,"data intact: third value still 3");
    kernel_queue_pop(&g_q,&val); CHECK(val==4,"data intact: fourth value still 4");
    
    // Checks if the queue is effectively empty
	CHECK(g_q.count==0,"queue empty after draining all 4 original values");
}

// ----Test 4: wrap-around ----
static void test_wraparound(void){
    printf("\n[TEST] test_wraparound\n");
    reset_queue(4); // small capacity so wrap-around happens quickly

    // filling the queue
    kernel_queue_push(&g_q,10);
    kernel_queue_push(&g_q,20);
    kernel_queue_push(&g_q,30);
    kernel_queue_push(&g_q,40);
    CHECK(g_q.count==4,"queue full after 4 pushes");

    // pop 2 meaning head advances, freeing 2 slots near the start of buffer
    uint8_t val;
    kernel_queue_pop(&g_q,&val); CHECK(val==10,"pop returns 10 (oldest)");
    kernel_queue_pop(&g_q,&val); CHECK(val==20,"pop returns 20 (2nd oldest)");
    CHECK(g_q.count==2,"count=2 after two pops");

    // push 2 more meaning tail must WRAP AROUND to the beginning of the buffer
    // since head/tail were already at index 2 and 3 out of 4 slots
    CHECK(kernel_queue_push(&g_q,50)==1,"push(50) succeeds after wrap");
    CHECK(kernel_queue_push(&g_q,60)==1,"push(60) succeeds after wrap");
    CHECK(g_q.count==4,"count=4 again after wrap-around pushes");

    /* the remaining order must still be correct FIFO: 30, 40, 50, 60 */
    kernel_queue_pop(&g_q,&val); CHECK(val==30,"wrap: pop returns 30");
    kernel_queue_pop(&g_q,&val); CHECK(val==40,"wrap: pop returns 40");
    kernel_queue_pop(&g_q,&val); CHECK(val==50,"wrap: pop returns 50 (from wrapped slot)");
    kernel_queue_pop(&g_q,&val); CHECK(val==60,"wrap: pop returns 60 (from wrapped slot)");
    CHECK(g_q.count==0,"queue empty again after full wrap-around cycle");
}

int main(void){
    printf("---Kernel Ring queue unit tests---\n");

    test_fifo_basic();
    test_pop_empty();
    test_push_full_overflow();
    test_wraparound();

    printf("\n--- Summary: %d/%d tests passed ---\n",g_tests_run-g_tests_failed,g_tests_run);

    return g_tests_failed>0 ? 1:0; // non-zero exit code on any failure, that is CI-friendly (related to github)
}
