# ISR Rules

## Purpose

This document defines rules for using kernel services from interrupt
service routines.

## General Rules

ISR code must remain short and deterministic.

ISR handlers must not perform operations that can block indefinitely.

## ISR-Safe APIs

Only APIs explicitly implemented and documented as ISR-safe may be
called from interrupt context.

Examples may include:

- ISR notification
- ISR queue operations
- ISR mailbox operations
- ISR semaphore operations

The exact available APIs are defined by the source headers.

## Scheduling From ISR

If an ISR operation wakes a higher-priority task, the implementation
may request a context switch according to the port and scheduler rules.

## Critical Sections

Interrupt masking and critical-section behavior must follow the
architecture-specific implementation.
