#include "../Inc/queue.h"
#include "../Inc/scheduler.h"

void kernel_queue_init(PtrQueue_t *q, void **buf, uint16_t capacity) {
    CRITICAL_SECTION_ENTER();
    q->buffer = buf;
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->overflow_flag = 0;
    CRITICAL_SECTION_EXIT();
}

uint8_t kernel_queue_push(PtrQueue_t *q, void *item) {
    CRITICAL_SECTION_ENTER();
    if (q->count == q->capacity) {
        q->overflow_flag = 1;
        CRITICAL_SECTION_EXIT();
        return 0;
    }
    q->buffer[q->tail] = item;
    q->tail = (uint16_t)((q->tail + 1) % q->capacity);
    q->count++;
    CRITICAL_SECTION_EXIT();
    return 1;
}

uint8_t kernel_queue_pop(PtrQueue_t *q, void **item) {
    CRITICAL_SECTION_ENTER();
    if (q->count == 0) {
        CRITICAL_SECTION_EXIT();
        return 0;
    }
    *item = q->buffer[q->head];
    q->head = (uint16_t)((q->head + 1) % q->capacity);
    q->count--;
    CRITICAL_SECTION_EXIT();
    return 1;
}

uint8_t kernel_queue_is_empty(PtrQueue_t *q) {
    uint8_t result;
    CRITICAL_SECTION_ENTER();
    result = (q->count == 0);
    CRITICAL_SECTION_EXIT();
    return result;
}

uint8_t kernel_queue_is_full(PtrQueue_t *q) {
    uint8_t result;
    CRITICAL_SECTION_ENTER();
    result = (q->count == q->capacity);
    CRITICAL_SECTION_EXIT();
    return result;
}