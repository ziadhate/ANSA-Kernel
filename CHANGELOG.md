# Changelog - ANSA RTOS Kernel

## [1.0.0-Production] - Phase 9 Release

### Added
- Centralized production header (`Inc/kernel_config.h`).
- Automated build & static analysis GitHub Workflow (`.github/workflows/rtos-ci.yml`).
- Verification hardware test suites (`Hardware_Tests/Test_01_GPIO` to `Test_09_Full_Kernel`).

### Hardened
- **Priority Inheritance Protocol**: Fixed priority restoration and task pop on mutex release via `task_queue_pop_task()`.
- **Timeout Handling**: Dynamic TCB unlinking with `task_queue_remove()`.
- **ISR Safety**: Primary interrupt masking for queue operations from Handler mode.
