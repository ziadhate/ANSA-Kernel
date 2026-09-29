#!/usr/bin/env bash

set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

echo "============================================================"
echo " ANSA KERNEL - PHASE 9 RELEASE SETUP"
echo "============================================================"
echo "Root: $ROOT"
echo

mkdir -p \
    Docs/api \
    Docs/architecture \
    Docs/development \
    Docs/testing \
    Docs/release \
    tools \
    reports \
    .github/workflows

echo "[OK] Phase 9 directories ready"

VERSION="1.0.0"

if [ -f VERSION ]; then
    VERSION="$(tr -d '[:space:]' < VERSION)"
fi

echo "[INFO] Detected kernel version: $VERSION"

if [ ! -f VERSION ]; then
    echo "$VERSION" > VERSION
    echo "[CREATE] VERSION"
else
    echo "[SKIP] VERSION already exists"
fi

if [ ! -f CHANGELOG.md ]; then
cat > CHANGELOG.md <<'CHANGELOG_EOF'
# Changelog

## 1.0.0

Initial documented release of ANSA Kernel.

### Documentation

- Architecture documentation
- API reference
- Development documentation
- Testing documentation
- Release documentation
CHANGELOG_EOF
    echo "[CREATE] CHANGELOG.md"
else
    echo "[SKIP] CHANGELOG.md already exists"
fi

cat > Docs/README.md <<'DOCS_EOF'
# ANSA Kernel Documentation

This directory contains the technical documentation for ANSA Kernel.

## Documentation Structure

- `api/` - Public API documentation
- `architecture/` - Kernel architecture and subsystem design
- `development/` - Build and porting information
- `testing/` - Verification and testing documentation
- `release/` - Release process and release checklist

## Documentation Status

Documentation must describe the implementation that actually exists
in the source tree.

If an API or feature is not implemented, it must not be documented
as production-ready functionality.

## Authoritative Source

The source code and public headers under `Inc/` and `Src/` are the
authoritative implementation.
DOCS_EOF

echo "[CREATE] Docs/README.md"

cat > Docs/api/API_REFERENCE.md <<'API_EOF'
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
API_EOF

echo "[CREATE] Docs/api/API_REFERENCE.md"

cat > Docs/architecture/ARCHITECTURE.md <<'ARCH_EOF'
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
ARCH_EOF

echo "[CREATE] Docs/architecture/ARCHITECTURE.md"

cat > Docs/architecture/ISR_RULES.md <<'ISR_EOF'
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
ISR_EOF

echo "[CREATE] Docs/architecture/ISR_RULES.md"

cat > Docs/development/BUILD.md <<'BUILD_EOF'
# Build and Development

## Build Requirements

The project requires an ARM embedded GCC toolchain for supported
Cortex-M targets.

Typical tools include:

- arm-none-eabi-gcc
- arm-none-eabi-ar
- arm-none-eabi-objcopy
- make

## Build

The kernel build system is defined by the repository Makefile.

Before building, verify that the ARM toolchain is available:

    arm-none-eabi-gcc --version

Then use the repository build instructions.

## Development Rule

Changes to kernel behavior should be accompanied by appropriate
verification or tests.
BUILD_EOF

echo "[CREATE] Docs/development/BUILD.md"

cat > Docs/testing/TESTING.md <<'TEST_EOF'
# Testing and Verification

## Purpose

Testing verifies kernel behavior on supported targets and simulation
environments.

## Test Areas

Testing may cover:

- GPIO
- Task creation
- Scheduler behavior
- Round-robin scheduling
- Timing
- Synchronization
- IPC
- ISR paths
- Full kernel integration

## Verification Rule

A test is considered valid only when its expected behavior can be
reproduced and its result can be observed.

Test documentation must reflect the tests that actually exist in the
repository.
TEST_EOF

echo "[CREATE] Docs/testing/TESTING.md"

cat > Docs/release/RELEASE_CHECKLIST.md <<'RELEASE_EOF'
# Release Checklist

## Source

- [ ] Source tree is clean
- [ ] Intended changes are committed
- [ ] No temporary files are included

## Build

- [ ] Kernel builds successfully
- [ ] Supported target build verified
- [ ] No unexpected compiler errors or warnings

## Tests

- [ ] Available kernel tests executed
- [ ] Scheduler tests verified
- [ ] IPC tests verified where implemented
- [ ] ISR paths verified where implemented

## Documentation

- [ ] README updated
- [ ] API documentation reviewed
- [ ] Architecture documentation reviewed
- [ ] Testing documentation reviewed
- [ ] Release notes updated

## Version

- [ ] VERSION checked
- [ ] CHANGELOG updated
RELEASE_EOF

echo "[CREATE] Docs/release/RELEASE_CHECKLIST.md"

echo
echo "============================================================"
echo " PHASE 9 RELEASE SETUP COMPLETE"
echo "============================================================"
