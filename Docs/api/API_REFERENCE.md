# ANSA Kernel API Reference

## Purpose

This document provides a high-level reference for the public ANSA Kernel
interfaces.

The source headers under `Inc/` are the authoritative API definition.

## Public API Areas

- Kernel initialization
- Task management
- Scheduler interaction
- Mutex
- Semaphore
- Queue
- Notification
- Mailbox
- Watchdog
- Kernel configuration

## Task APIs

Task APIs are responsible for:

- Task creation
- Task lifecycle management
- Task scheduling interaction
- Task delay and blocking
- Task state management

## Synchronization APIs

Synchronization primitives include:

- Mutex
- Semaphore

Blocking and timeout behavior must follow the implementation in the
corresponding source files.

## IPC APIs

IPC mechanisms include:

- Queue
- Notification
- Mailbox

ISR-safe APIs must only be used according to the ISR rules documented
in the architecture documentation.

## Authoritative Source

For exact function prototypes and types, refer to:

    Inc/*.h

and the corresponding implementation files under:

    Src/*.c
