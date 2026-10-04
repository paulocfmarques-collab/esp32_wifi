# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- README aligned with the actual repository structure and real implemented files.
- Command reference updated to match the commands present in `CommandHandler.h` and `wifi.ino`.
- Clear documentation of the Wi‑Fi provisioning flow, UDP protocol, and project architecture.
- Troubleshooting section adjusted to reflect current ESP32 behavior.

### Changed
- Reworked the repository overview to describe the real modular design instead of the old placeholder structure.
- Updated project description to focus on actual provisioning, monitoring, and remote device management capabilities.
- Simplified and corrected the documentation entries for the current codebase.

### Fixed
- Incorrect references to non-existent `src/`, `include/`, `docs/`, and `data/` folders.
- README entries that did not match the actual code structure and command list.
- Documentation mismatches between the project description and the implemented ESP32 firmware.

## [1.0.0] - 2026-10-04

### Added
- Initial ESP32 Wi‑Fi provisioning workflow.
- Remote device management over UDP.
- Command-based hardware control layer for LED, time, and network settings.
- Persistent Wi‑Fi credential storage using Preferences.
- Device monitoring and diagnostics for CPU, RAM, flash, uptime, and network status.
- Modular architecture for Wi‑Fi manager, UDP network layer, display, hardware control, NTP utilities, and command execution.
- GitHub repository structure and foundational project documentation.

### Features
- Access point mode for first-time provisioning.
- Remote command execution via UDP gateway.
- Basic telemetry and status reporting.
- Modular support for LED feedback and display output.
- External integration support for embedded IoT deployments.

---

For more details about the current project status and future improvements, see the repository issues and pull requests on GitHub.
