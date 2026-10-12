# ESP32 Wi-Fi / OLED

[![Release](https://img.shields.io/badge/release-2.0.1-blue.svg)](./CHANGELOG.md)
[![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Language](https://img.shields.io/badge/language-C%2B%2B%20%2F%20C-lightgrey.svg)](https://isocpp.org/)
[![Arduino-ESP32](https://img.shields.io/badge/Arduino-ESP32%203.x-green.svg)](https://github.com/espressif/arduino-esp32)

Firmware for ESP32 boards with a local Wi-Fi setup portal, SSD1306 OLED display, network monitoring, and serial/UDP commands.

> [!IMPORTANT]
> This project is designed to run entirely locally, without relying on external web services, CDNs, or remote assets.

## Screenshots

> Add screenshots or short GIFs here to showcase the portal, OLED pages, and network status views.
>
> Suggested assets:
> - `docs/images/portal.png`
> - `docs/images/oled-home.png`
> - `docs/images/wifi-settings.gif`

| Portal | OLED Status | Network Monitor |
|---|---|---|
| _Screenshot placeholder_ | _Screenshot placeholder_ | _Screenshot placeholder_ |

## Quick start

1. Open `wifi/wifi.ino` in the Arduino IDE.
2. Keep the `wifi` folder and supporting source files together.
3. Select your ESP32 board and the same options used by the original project.
4. Install the required libraries:
   - Arduino-ESP32 3.x
   - Adafruit SSD1306
   - Adafruit GFX
5. Compile, upload, and open the Serial Monitor at 115200 baud.
6. Connect to the **ESP32_CONFIG** access point if the device starts in configuration mode.
7. Open `/wifi` to save up to five Wi-Fi profiles.

## Overview

ESP32 Wi-Fi / OLED provides a self-contained Wi-Fi configuration experience for ESP32 devices. It stores up to five network profiles, offers automatic reconnection, exposes a local portal for status and configuration, and keeps the OLED display and diagnostics active during recovery attempts.

## Key features

- Local web portal for managing up to five Wi-Fi profiles
- Automatic reconnection with AP fallback when the connection drops
- SSD1306 128x64 OLED interface with five status pages
- ICMP gateway monitoring with in-memory history
- Serial and UDP command handling for network, display, LED, and diagnostics
- Offline-friendly portal with no external dependencies

## Hardware requirements

| Component | Configuration |
|---|---|
| Board | ESP32 compatible board using Arduino-ESP32 3.x |
| LED | GPIO 2 |
| Button / BOOT | GPIO 0, tied to GND, using `INPUT_PULLUP` |
| OLED SDA / SCL | GPIO 21 / GPIO 22 |
| Display | SSD1306 128x64, I2C address `0x3C`, monochrome |
| UDP | Port `4210` |
| Default time zone | UTC-3 |

## Dependencies

- Arduino-ESP32 3.x
- Adafruit SSD1306
- Adafruit GFX
- Native `ping/ping_sock.h` support from ESP-IDF

## First boot and portal access

When no connection is available, the ESP32 starts the **ESP32_CONFIG** access point at **192.168.4.1** while it searches for saved networks.

- Visit `/wifi` to configure up to five profiles.
- Once a connection is established, the access point is disabled.
- The OLED shows the assigned IP address, which becomes the primary portal endpoint.
- If the connection drops, the access point is re-enabled while recovery retries continue.

## Portal endpoints

- `/info`: shows network, IP address, RSSI, memory usage, uptime, and connection-monitor status.
- `/wifi`: manages the five Wi-Fi profiles, displays the active network, supports show/hide password, and allows open-network configuration.

### Portal behavior

- The portal works without Internet access.
- No external fonts, styles, or scripts are loaded.
- Stored passwords are never sent back to the browser.
- If the SSID does not change, an empty password keeps the previous one.
- Select **Open network** to save a profile without a password.
- Clearing the SSID removes the profile.

The server validates every profile, verifies writes by reading them back, and only confirms success before scheduling a reboot. If a write fails, it attempts to restore the previous data and reports the outcome.

## Network discovery and recovery

- Network scanning is asynchronous.
- After scanning, profiles are tried sequentially.
- The most recently used network is attempted first.
- Each profile can be tried for up to ten seconds.
- Hidden SSIDs are supported.
- If no network connects, the access point remains available and a new retry cycle begins after thirty seconds.

The main loop continues serving the portal, commands, LED, button, and OLED updates while recovery is in progress.

## OLED pages and button behavior

The firmware includes five main pages:

1. Clock
2. Wi-Fi status
3. System information
4. Gateway ping
5. History

- One-second refresh rate
- Automatic page rotation every eight seconds
- Buffered OLED rendering
- Short press on GPIO 0 advances to the next page
- Five-second long press clears networks, cursor position, time zone, and DST state, then reboots the device

## Network monitoring and time

- ICMP ping to the gateway approximately every five seconds
- One-second timeout handled in a dedicated task
- Graph of the last 24 responses/timeouts
- Minimum, average, and maximum RTT statistics
- NTP configured at boot without blocking startup
- Time zone configurable from UTC-12 to UTC+14
- Manual DST support with a fixed one-hour adjustment

## Commands

The full command list is available in `COMANDOS.md` and through the `help` command.

## Notes

- Credentials are hidden from logs.
- UDP packets larger than 255 bytes are rejected.
- The button remains readable even during the screen-protection mode.
- Hardware validation and real-device testing should still be performed on your target board.

## References

- [Wi-Fi Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
- [ICMP Echo Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/icmp_echo.html)
