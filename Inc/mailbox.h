#ifndef KERNEL_MAILBOX_H
#define KERNEL_MAILBOX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Maximum size, in bytes, of any single item stored in the mailbox.
   Must be >= sizeof() of the largest type we plan to post. */
#define MAILBOX_MAX_ITEM_SIZE 64

/* Status type for mailbox operations */
typedef enum
{
	MAILBOX_INIT_SUCCEED=0 	// Creation of mailbox succeed
	MAILBOX_INIT_FAILED, 	// Creation of mailbox failed
    MAILBOX_EMPTY,   		// get() called but no data has ever been posted
    MAILBOX_OK          	// get() returns this when data is present
} mailbox_status_t;

typedef struct
{
    uint8_t buffer[MAILBOX_MAX_ITEM_SIZE];
    uint32_t item_size;
    bool has_data;
} kernel_mailbox_t;

/* Initializes a statically-declared kernel_mailbox_t instance */
kernel_mailbox_t kernel_mailbox_init(kernel_mailbox_t *mb, uint32_t item_size);

/* Copies item into the mailbox, overwriting any previous value
   Never blocks, never fails */
void kernel_mailbox_post(kernel_mailbox_t *mb, const void *item);

/* Copies the current value out of the mailbox into item.
   Returns MAILBOX_OK if data was present, MAILBOX_EMPTY otherwise */
mailbox_status_t kernel_mailbox_get(kernel_mailbox_t *mb, void *item);

#endif /* KERNEL_MAILBOX_H */
