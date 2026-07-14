/*
* Here i did implementation of functions in header file 
*/

#include "kernel_queue.h"


/**
 * Task 2 Implementation: Buffer Allocation 
 * Sets up state tracking inside the provided struct context.
 */

//========================================IMPORTANT==================================================================================
//  The below array must be created right above , where the other developer calls the below init function and pass the address of this 
//  array as 2nd argument of init function below
//  this array is the statically allocated space for the queue elements to live  
//  static uint8_t my_raw_storage[element_size * current_count ] ;                                                                   


queue_status_t kernel_queue_init(kernel_queue_t *queue, uint8_t *static_buf, uint32_t max_elements, uint32_t element_size) {
        
    // Validation guard rails for kernel stability 
    if (
        queue == NULL ||            // this means the  structure has not been initiated so an error should be there
        static_buf == NULL ||       // the array was not initialised
        max_elements == 0 ||        // queue size insufficient
        element_size == 0           // element size insufficient , must be atleast 1 byte
    ) {
        return -1 ; // or rather print an error log
    }

    // this will Bind the statically allocated compile-time buffer to the structure context 
    queue->buffer = static_buf ;
    
    // Configure structural rules for size calculation 
    queue->max_elements = max_elements ;
    queue->element_size = element_size ; // multiplying these two will give the size

    // providing the initial values to the indices to avoid any compile time conflict
    queue->head = 0 ;
    queue->tail = 0 ;
    queue->current_count = 0 ;

    return 0 ;
}

