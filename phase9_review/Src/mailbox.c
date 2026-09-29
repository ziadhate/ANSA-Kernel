#include "mailbox.h"
#include <stddef.h>
#include <string.h>

mailbox_status_t kernel_mailbox_init(kernel_mailbox_t *mb, void *buffer, uint32_t item_size)
{
    if ((mb == NULL) || (item_size == 0U) || (buffer == NULL)) {
        return MAILBOX_FAILED;
    }
    mb->buffer = buffer;
    mb->item_size = item_size;
    mb->has_data = false;
    return MAILBOX_PASSED;
}

mailbox_status_t kernel_mailbox_send(kernel_mailbox_t *mb, const void *item)
{
    if ((mb == NULL) || (item == NULL)) {
        return MAILBOX_FAILED;
    }
    (void)memcpy(mb->buffer, item, mb->item_size);
    mb->has_data = true;
    return MAILBOX_PASSED;
}

mailbox_status_t kernel_mailbox_receive(const kernel_mailbox_t *mb, void *item)
{
    if ((mb == NULL) || (item == NULL) || (!mb->has_data)) {
        return MAILBOX_FAILED;
    }
    (void)memcpy(item, mb->buffer, mb->item_size);
    return MAILBOX_PASSED;
}
