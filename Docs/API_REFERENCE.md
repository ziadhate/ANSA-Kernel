# ANSA RTOS Kernel — Production API Reference & Architecture Rules

## Target Architecture
- **Core**: ARM Cortex-M4 (Ported under `Port/Cortex_M4`)
- **System Clock Tick**: 1000 Hz (1ms SysTick)

## 1. Interrupt Service Routine (ISR) Rules & Guidelines

1. **No Task Blocking in ISR**: Interrupt Handlers MUST NOT invoke APIs that cause task blocking (e.g., `kernel_queue_send` with timeout > 0, `kernel_mutex_lock`, or `task_delay`).
2. **ISR-Safe APIs**: Only APIs suffixed with `_from_isr` (e.g., `kernel_queue_send_from_isr`) are permitted inside Handler Mode / Interrupt context.
3. **Deferred Context Switches**:
   - `_from_isr` functions accept a pointer `bool *pxHigherPriorityTaskWoken`.
   - If set to `true`, the ISR MUST trigger a context switch before exiting using `kernel_yield_from_isr(pxHigherPriorityTaskWoken)`.

## 2. Kernel API Contracts

### A. Queue Management (`Inc/queue.h`)
- `kernel_queue_send_from_isr`: Non-blocking queue post for ISR contexts.
- `kernel_queue_send`: Thread-safe queue post supporting dynamic timeouts.

### B. Mutex Management & Priority Inheritance (`Inc/mutex.h`)
- `kernel_mutex_lock`: Supports PIP to prevent priority inversion. Cleans up wait lists on timeout using `task_queue_remove()`.
- `kernel_mutex_unlock`: Transfers ownership using `task_queue_pop_task()` and restores base priority.

### C. Mailbox & Notifications (`Inc/mailbox.h`, `Inc/Notification.h`)
- Direct task notifications and single-latest-value mailbox IPC semantics.
