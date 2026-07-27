#include "../Inc/queue.h"
#include "../Inc/scheduler.h"

void kernel_queue_init(RingQueue_t *q, uint8_t *buf, uint16_t cap) {
    CRITICAL_SECTION_ENTER();
    q->buffer = buf;
    q->capacity = cap;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->overflow_flag = 0;
    CRITICAL_SECTION_EXIT();
}
 
uint8_t kernel_queue_push(RingQueue_t *q, uint8_t data) {
    CRITICAL_SECTION_ENTER();
    if (q->count == q->capacity) {
        q->overflow_flag = 1;
        CRITICAL_SECTION_EXIT();
        return 0;
    }
    q->buffer[q->tail] = data;
    q->tail = (uint16_t)((q->tail + 1) % q->capacity);
    q->count++;
    CRITICAL_SECTION_EXIT();
    return 1;
}
 
uint8_t kernel_queue_pop(RingQueue_t *q, uint8_t *data) {
    CRITICAL_SECTION_ENTER();
    if (q->count == 0) {
        CRITICAL_SECTION_EXIT();
        return 0;
    }
    *data = q->buffer[q->head];
    q->head = (uint16_t)((q->head + 1) % q->capacity);
    q->count--;
    CRITICAL_SECTION_EXIT();
    return 1;
}
