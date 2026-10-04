# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Professional GitHub README structure and visual upgrade.
- Improved project documentation for easier onboarding and repository presentation.
- Clear sections for features, architecture, quick start, command reference, and troubleshooting.
- Documentation links corrected to match the actual repository content.

### Changed
- Reworked the project landing page to be more polished and presentation-oriented for GitHub users.
- Refined the project description to emphasize ESP32 provisioning, monitoring, and device management.

### Fixed
- Broken documentation links in the README.
- Inaccurate references to documentation files that did not exist in the repository.

## [1.0.0] - 2026-10-04

### Added
- Initial ESP32 Wi-Fi provisioning workflow.
- Remote device management over UDP.
- Command-based hardware control layer for LED, time, and network settings.
- Persistent Wi-Fi credential storage using NVS/Preferences.
- Device monitoring and diagnostics for CPU, RAM, flash, uptime, and network status.
- Modular architecture for WiFi manager, UDP gateway, command processor, display, RGB, and system monitor components.
- GitHub repository structure and project documentation foundation.

### Features
- Access point mode for first-time provisioning.
- Remote command execution via UDP gateway.
- Basic telemetry and status reporting.
- Modular support for RGB LED feedback and display output.
- External integration support for embedded IoT deployments.

---

For more details about the current project status and future improvements, see the repository issues and pull requests on GitHub.
