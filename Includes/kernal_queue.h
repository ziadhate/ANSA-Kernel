/*  First attempt ( DAY1 ) 
*   Date : 12th july , 2026
*   Designed and implemented a modular, memory-safe kernel queue system using a statically allocated ring buffer .
*   Created structural abstractions and pointer-tracking mechanisms to lay the foundation for generic inter-task communication 
*   and scheduling blocks */

#include <stdint.h>


// Status codes for queue operations 
typedef enum {
    QUEUE_EMPTY,
    QUEUE_FULL,
} queue_status_t;



/* 
 * Task 1: Queue Structure Definition
 * This structure tracks a generic, byte-oriented circular ring buffer.
 */
typedef struct {
    uint8_t *buffer;          // Pointer to the statically allocated raw byte array 
    uint32_t head;            // Index pointing to the next item to read/receive 
    uint32_t tail;            // Index pointing to the next empty slot to write/send 
    uint32_t max_elements;    // Maximum capacity of the queue 
    uint32_t element_size;    // Size of a single element in bytes (for generic memcpy) 
    uint32_t current_count;   // Number of active elements currently in the queue 

    // more entries related to remaining tasks
} kernel_queue_t;


/*
*   Task 2 : Buffer allocation
*/
queue_status_t kernel_queue_init(
    kernel_queue_t *queue,              // an instance of struct kernal ( container of info about queue)
    uint8_t *static_buf,                // it stores the address of allocated space 
    uint32_t max_elements,
    uint32_t element_size
);


/* 
 *  Prototypes for remaining Phase tasks.
 *  I have to implement these later 
 */

