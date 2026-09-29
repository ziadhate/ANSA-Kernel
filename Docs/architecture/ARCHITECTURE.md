# ANSA Kernel Architecture

## Overview

ANSA Kernel is a small embedded real-time kernel designed for
microcontroller-based systems.

The architecture is organized into kernel services, scheduling,
synchronization, IPC, timing, and hardware porting layers.

## Main Components

### Kernel Core

Responsible for:

- Kernel initialization
- Kernel state
- Core service coordination

### Scheduler

Responsible for:

- Task selection
- Priority-based scheduling
- Round-robin scheduling for equal-priority tasks
- Context switching coordination

### Tasks

Tasks represent independently scheduled execution contexts.

Each task has a control structure and an associated stack.

### Synchronization

The kernel provides synchronization primitives such as:

- Mutex
- Semaphore

The mutex implementation includes priority inheritance behavior
where implemented.

### IPC

Inter-process communication mechanisms include:

- Queue
- Notification
- Mailbox

### Timing

System timing is based on the configured system tick.

The scheduler uses the system tick for delays, timeouts, and
time-slice management.

### Port Layer

The port layer provides architecture-specific functionality such as:

- Context switching
- Critical sections
- Interrupt handling
- Stack initialization

## Design Principle

The kernel separates portable kernel logic from architecture-specific
port functionality.

This allows the core kernel to remain independent from the target MCU
while the port layer handles processor-specific operations.
