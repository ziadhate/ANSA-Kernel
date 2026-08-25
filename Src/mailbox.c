#include "mailbox.h"
#include <string.h>
#include "../Port/Cortex_M4/Inc/port.h"

mailbox_status_t kernel_mailbox_init(kernel_mailbox_t *mb, uint32_t item_size)
{
	if((mb==NULL) || (item_size<=0)){
		return MAILBOX_INIT_FAILED;
	}
    mb->item_size = item_size;
    mb->has_data  = false;
    
    return MAILBOX_INIT_SUCCEED;
}

void kernel_mailbox_send(kernel_mailbox_t *mb, const void *item)
{
    port_critical_enter();
    memcpy(mb->buffer, item, mb->item_size);
    mb->has_data = true;
    port_critical_exit();
}

mailbox_status_t kernel_mailbox_receive(kernel_mailbox_t *mb, void *item)
{
    mailbox_status_t result = MAILBOX_EMPTY;

    port_critical_enter();
    if (mb->has_data)
    {
        memcpy(item, mb->buffer, mb->item_size);
        result = MAILBOX_OK;
    }
    port_critical_exit();

    return result;
}
