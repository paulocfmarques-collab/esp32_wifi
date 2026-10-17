# ESP32 Wi‑Fi / OLED — version 2.0.0

A polished ESP32 Wi‑Fi experience for the SSD1306 128×64 OLED. This release brings a modern web portal, support for five persistent networks, background reconnection, live network monitoring, five display screens, and a clean, standardized command set — all while preserving the original project pinout.

It also adds built-in Prometheus metrics with persistent configuration directly from the web interface.

## Hardware and dependencies

| Feature | Configuration |
|---|---|
| LED | GPIO 2 |
| Button / BOOT | GPIO 0, connected to GND, INPUT_PULLUP |
| OLED SDA / SCL | GPIO 21 / GPIO 22 |
| Display | SSD1306 128×64, I2C 0x3C, monochrome |
| UDP | Port 4210 |
| Web interface | Fixed port 80 |
| Prometheus metrics | Configurable port, default 9120 |
| Default timezone | UTC -3 |

Use the same ESP32 board selection as the original application, along with Arduino-ESP32 3.x, Adafruit SSD1306, and Adafruit GFX. Ping uses native `ping/ping_sock.h` from ESP-IDF.
No Ethernet peripherals or SD card support have been added.

## Installation

1. Extract the ZIP file while keeping the `wifi` folder and its files together.
2. Open `wifi/wifi.ino` in the Arduino IDE.
3. Select your board, port, and the same options used in the original project.
4. Compile and upload.
5. Open Serial at 115200 baud to follow the startup process.

The monitor `.cpp` file must also remain alongside the sketch.

## First access and portal

When starting without a connection, the ESP32 creates the **ESP32_CONFIG** AP at **192.168.4.1** while it searches for saved networks. Open `/wifi` to register up to five profiles. After connecting, the AP is turned off and the OLED shows the assigned IP address; access that IP to use the portal. If the connection drops, the AP comes back during the next retry cycle.

- `/info`: network, IP, RSSI, memory, uptime, and connection monitor. Updates every ten seconds.
- `/wifi`: five profiles, connected network indication, show/hide password, and explicit open-network selection.
- `/prometheus`: server address, metrics port, and collection interval, saved in Preferences/NVS.
- `/prometheus.yml`: collection configuration to download and apply on the Prometheus server.

## Prometheus

Open `http://ESP32_IP/prometheus` on port **80**. Enter the IPv4 address or DNS name of the Prometheus server, the **ESP32 metrics port** (1..65535, default 9120), and the collection interval (5..3600 seconds, default 15). Save and apply the YAML shown to the Prometheus server.

The model is **pull-based**: Prometheus scrapes `http://ESP32_IP:PORT/metrics`.
The ESP32 does not push data actively; the server address is stored only as a reference. The configured port belongs to the ESP32 endpoint, not the Prometheus server web UI. The interval only takes effect after the YAML is applied.

The dashboard, terminal, and configuration remain on **port 80**, with no redirect.
The new metrics port is applied about one second after saving, without a reboot.
If you choose 80, metrics share the web server; on other ports, the metrics server serves only `/metrics`. This also applies in AP mode.

Only real, available metrics are exposed. Power, battery, environmental sensors, relays, ADC, and MQTT are not integrated and do not generate fake series.
Details, full inventory, limitations, and examples are in
[PROMETHEUS.md](PROMETHEUS.md).

The portal works without Internet access and does not load external fonts, styles, or scripts.
Saved passwords are never sent to the browser. An empty password keeps the previous one only when the SSID does not change; enable Open network to save without a password.
Clearing the SSID removes the profile.

The server validates all profiles, verifies writes by reading the data back, and only confirms success before scheduling the reboot.
If something fails, it attempts to restore the previous data and reports the result. Multi-field writes are not atomic against power loss.
Legacy credentials are migrated to profile 1 when possible.

## Network search and recovery

Scanning is asynchronous. After scanning, profiles are tried sequentially, starting with the last used network, with up to ten seconds per profile. All saved profiles are attempted, including hidden SSIDs. If none connect, the AP remains available and a new cycle begins after thirty seconds. The loop keeps handling the portal, commands, LED, button, and OLED while retries continue. Retry attempts show the SSID on the OLED console and on the network page.

`wifi_add` stores profiles in the circular list; see `COMANDOS.md` for details.
UDP also works in AP mode. Command names are case-insensitive; arguments preserve case. Packets larger than 255 bytes are rejected.

## Screens and button

Five pages: clock, Wi‑Fi network, system, gateway ping, and history.
One-second refresh, eight-second rotation, and buffered OLED drawing.
The display is monochrome: bars change quantity, not color.

A short press on GPIO 0 advances the screen. Hold for five seconds and release to clear networks, cursor, timezone, and DST, then restart while preserving the firmware. Prometheus preferences and boot/watchdog counters are not cleared. The loop keeps running while the button is held. Pressing BOOT during reset or power-up may trigger the board flashing mode; EN remains the reset line.

`tela:0..4`, `tela:next`, `tela:auto`, and `clear_log` control the display.

After three minutes without interaction, six animated faces move across the screen, blink, and alternate between smile, wink, surprise, and sleep while keeping the OLED on. The animation updates every 250 ms. A short button press exits idle protection and advances the page; commands also exit protection and refresh the inactivity timer. Automatic network logs do not refresh this timer. Button input continues during protection mode.

## Network monitor and time

ICMP ping to the gateway runs about every five seconds, with a one-second timeout in a separate task. The graph shows the last 24 responses/timeouts.
The report includes minimum/average/maximum RTT, measurement age, drops, offline time, and three recent events in minutes since boot. Timeouts are not included in the average. Everything stays in RAM and resets with the board. Ping does not test the Internet; routers may block ICMP even when Wi‑Fi is working.

NTP is configured at boot without waiting for synchronization, using the saved parameters.
Integer timezone -12..14, with manual fixed one-hour DST. Without synchronization, screens show waiting status. A valid time does not prove current reachability of the servers.

## Commands and validation

The full command list is in `COMANDOS.md`, also available through the `help` command.
Continuous LED and finite sequences run without blocking; `led_on`/`led_off` cancel any sequence. Credentials are hidden from logs.

Static checks were performed for structure, local includes, integration, command preservation, documentation, and ZIP packaging.
Compilation and on-device testing still need to be done. Please verify first boot, profile saving, reconnection, AP/UDP, all screens, LED, button, and NTP with your board.

References: [Wi‑Fi Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
and [ICMP Echo Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/icmp_echo.html).
