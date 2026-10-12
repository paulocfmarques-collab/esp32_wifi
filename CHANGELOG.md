# Changelog

## 2.0.1

- Documentation refreshed and expanded with a more polished project presentation.
- README rewritten in English with badges, screenshots placeholders, and a compact quick-start section.
- Added clearer guidance for installation, first boot, portal usage, and recovery flow.
- Changelog aligned to English for consistency across project documentation.

## 2.0.0

- Command inventory synchronized with the network, diagnostic, OLED, and LED handlers.
- Responsive information and five-network editing portal with local-only resources.
- Verified writes, validation, and rollback attempt on failure.
- Asynchronous scanning and automatic reconnection while keeping AP/UDP available.
- Five OLED pages and ICMP monitor with in-memory history.
- Button short press for navigation and long press for cleanup, including offline usage.
- Network, monitor, display, and diagnostic commands; help and full list updated.
- Continuous LED behavior and non-blocking pulses.
- NTP configured at boot without waiting and explicit manual DST adjustment.
- Passwords hidden from logs and oversized UDP packets rejected.
- Original pins and existing commands preserved.
