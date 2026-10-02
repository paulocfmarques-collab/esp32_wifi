1| # ESP32 Wi-Fi Provisioning, Remote Monitoring & UDP Control
2| 
3| A complete ESP32 firmware for Wi-Fi provisioning through a captive web portal, persistence of credentials in NVS/Preferences, remote UDP control, hardware supervision, and device diagnostics.
4| 
5| This project turns an ESP32 into a small smart module capable of:
6| 
7| - creating its own configuration access point when no Wi-Fi is stored
8| - saving network credentials in non-volatile memory
9| - reconnecting automatically after boot
10| - serving a web form for Wi-Fi setup
11| - accepting remote UDP commands over port 4210
12| - controlling the onboard LED
13| - monitoring hardware metrics such as temperature, CPU, memory, uptime, and network data
14| - resetting Wi-Fi configuration via hardware button or command
15| 
16| ---
17| 
18| ## Overview
19| 
20| The firmware follows a simple and robust lifecycle:
21| 
22| 1. Boot the ESP32
23| 2. Check whether valid Wi-Fi credentials are stored in Preferences
24| 3. If credentials exist, connect to the network automatically
25| 4. If not, start an access point and serve a configuration page
26| 5. Once configured, the device enters operational mode and listens for UDP commands
27| 6. The system can report status, execute actions, and reset its Wi-Fi configuration when needed
28| 
29| ---
30| 
31| ## System Architecture
32| 
33| ```mermaid
34| flowchart TB
35|     subgraph User
36|         U1[Web Browser]
37|         U2[UDP Client / Remote Controller]
38|     end
39| 
40|     subgraph ESP32 Firmware
41|         A[Boot and Startup]
42|         B{Credentials saved?}
43|         C[Wi-Fi Provisioning Portal]
44|         D[Preferences / NVS Store]
45|         E[Wi-Fi Client Mode]
45|         F[UDP Server on Port 4210]
46|         G[LED Control]
47|         H[System Monitor]
48|         I[OLED Display]
49|     end
50| 
51|     U1 --> C
52|     C --> D
53|     D --> E
54|     E --> F
55|     U2 --> F
56|     F --> G
57|     F --> H
58|     H --> U2
59|     G --> U2
60|     A --> B
61|     B -->|No| C
62|     B -->|Yes| E
63|     E --> I
64|     C --> I
65| ```
66| 
67| ---
68| 
69| ## High-Level Data Flow
70| 
71| ```mermaid
72| sequenceDiagram
73|     autonumber
74|     participant User as User / Client
75|     participant ESP as ESP32
76|     participant NVS as Preferences (NVS)
77|     participant WiFi as Wi-Fi Network
78| 
79|     User->>ESP: Power on / Reset
80|     ESP->>ESP: Check saved SSID and password
81| 
82|     alt Credentials not found
83|         ESP->>User: Start AP: ESP32_CONFIG
84|         User->>ESP: Open web portal
85|         User->>ESP: Submit SSID and password
86|         ESP->>NVS: Save credentials
87|         ESP->>ESP: Restart device
88|     else Credentials found
89|         ESP->>WiFi: Connect to saved SSID
90|         WiFi-->>ESP: Connection status
91|         ESP->>User: Run UDP listener on port 4210
92|     end
93| ```
94| 
95| ---
96| 
97| ## Hardware Wiring Diagram
98| 
99| The project uses a standard ESP32 development board, an SSD1306 OLED display, a status LED, and a reset button.
100| 
101| ```text
102|                        +----------------------+
103|                        |      ESP32 DevKit    |
104|                        |                      |
105|         GPIO2  ─────────┤ LED                 |
106|                        |
107|         GPIO0  ─────────┤ BOOT / RESET BTN    |
108|                        |
109|         GPIO21 ─────────┤ SDA (OLED)          |
110|         GPIO22 ─────────┤ SCL (OLED)          |
111|                        |
112|                  3V3 ──┤ VCC (OLED)          |
113|                  GND ──┤ GND (OLED)          |
114|                        +----------------------+
115| 
116|                   +----------------------------+
117|                   |  SSD1306 128x64 OLED      |
118|                   |  I2C Address: 0x3C        |
119|                   +----------------------------+
120| ```
121| 
122| ### Pin Mapping
123| 
124| | Function | ESP32 Pin | Description |
125| |---|---:|---|
126| | LED | GPIO 2 | Status LED, used for visual feedback |
127| | RESET BUTTON | GPIO 0 | Resets Wi-Fi settings when pressed |
128| | OLED SDA | GPIO 21 | I2C data line |
129| | OLED SCL | GPIO 22 | I2C clock line |
130| | OLED VCC | 3V3 | Power |
131| | OLED GND | GND | Ground |
132| 
133| ---
134| 
135| ## Power and I/O Behavior
136| 
137| ```mermaid
138| flowchart LR
139|     A[ESP32 Power On] --> B[Initialize serial + I2C]
140|     B --> C[Initialize OLED]
141|     C --> D{Wi-Fi credentials stored?}
142|     D -->|No| E[Access Point mode: ESP32_CONFIG]
143|     D -->|Yes| F[Station mode: connect to network]
144|     E --> G[Serve configuration page via HTTP 80]
145|     F --> H[Listen for UDP commands on port 4210]
146|     G --> I[Save SSID + password to NVS]
147|     I --> J[Restart device]
148|     H --> K[Execute action and return response]
149|     K --> H
150| ```
151| 
152| ---
153| 
154| ## Firmware State Machine
155| 
156| ```mermaid
157| stateDiagram-v2
158|     [*] --> Boot
159| 
160|     Boot --> Provisioning : No saved Wi-Fi
161|     Boot --> Connecting : Wi-Fi exists
162| 
163|     Provisioning --> Restart : Save configuration
164|     Restart --> Boot
165| 
166|     Connecting --> Operational : Connected successfully
167|     Connecting --> Provisioning : Connection failed
168| 
169|     Operational --> ResetWiFi : GPIO0 pressed
170|     Operational --> ResetWiFi : RESET_WIFI command
171|     ResetWiFi --> Restart
172| ```
173| 
174| ---
175| 
176| ## Data Storage Model
177| 
178| The firmware stores network parameters in the ESP32 non-volatile memory via `Preferences`:
179| 
180| ```text
181| Namespace: wifi
182| Keys:
183| - ssid
184| - senha
185| ```
186| 
187| This guarantees that the device can reconnect automatically after power cycling without re-entering credentials.
188| 
189| ---
190| 
191| ## Wi-Fi Provisioning Flow
192| 
193| When no saved Wi-Fi information is available, the ESP32 starts an access point named:
194| 
195| ```text
196| ESP32_CONFIG
197| ```
198| 
199| Then the user:
200| 
201| 1. connects to the access point
202| 2. opens the web page on the access point IP
203| 3. enters the SSID and password
204| 4. submits the form
205| 5. the ESP32 writes the values to Preferences
206| 6. the module restarts and connects to the selected Wi-Fi network
207| 
208| ### Provisioning Sequence
209| 
210| ```mermaid
211| sequenceDiagram
212|     participant U as User
213|     participant AP as ESP32 Access Point
214|     participant WEB as HTTP Portal
215|     participant NVS as NVS / Preferences
216| 
217|     U->>AP: Connect to ESP32_CONFIG
218|     U->>WEB: Open http://192.168.4.1
219|     WEB-->>U: Show configuration form
220|     U->>WEB: Send SSID + Password
221|     WEB->>NVS: Save credentials
222|     NVS-->>WEB: Confirmation
223|     WEB->>AP: Restart ESP32
224|     AP-->>U: Device boots into client mode
225| ```
226| 
227| ---
228| 
229| ## UDP Communication
230| 
230| After a successful Wi-Fi connection, the ESP32 opens a UDP server on port 4210.
231| 
232| ```text
233| UDP port: 4210
234| ```
235| 
236| The device receives ASCII commands from a client and replies to the sender address and port with a textual response.
237| 
238| ### UDP Command Flow
239| 
240| ```mermaid
241| flowchart LR
242|     A[UDP Client] --> B[ESP32:4210]
243|     B --> C{Command received}
244|     C --> D[LED_ON]
245|     C --> E[LED_OFF]
246|     C --> F[LED_PISCA]
247|     C --> G[LED_BLINK]
248|     C --> H[TEMP]
249|     C --> I[CPU]
250|     C --> J[RAM]
251|     C --> K[FLASH]
252|     C --> L[INIT]
253|     C --> M[UPTIME]
254|     C --> N[MAC]
255|     C --> O[NET_INFO]
256|     C --> P[RESET_WIFI]
257|     D --> Q[Reply to client]
258|     E --> Q
259|     F --> Q
260|     G --> Q
261|     H --> Q
262|     I --> Q
263|     J --> Q
264|     K --> Q
265|     L --> Q
266|     M --> Q
267|     N --> Q
268|     O --> Q
269|     P --> Q
270| ```
271| 
272| ---
273| 
274| ## Available Commands
275| 
276| The firmware supports the following commands through UDP.
277| 
278| | Command | Description | Example |
279| |---|---|---|
280| | `LED_ON` | Turns the LED ON | `LED_ON` |
281| | `LED_OFF` | Turns the LED OFF | `LED_OFF` |
282| | `LED_PISCA:10:250` | Blinks N times with delay in milliseconds | `LED_PISCA:10:250` |
283| | `LED_BLINK:500` | Continuous blinking with interval in ms | `LED_BLINK:500` |
284| | `TEMP` | Reads internal chip temperature | `TEMP` |
285| | `CPU` | Returns CPU model, revision, core count, frequency, and free RAM | `CPU` |
286| | `RAM` | Returns heap usage and memory statistics | `RAM` |
287| | `FLASH` | Returns flash size, speed, sketch size, and free space | `FLASH` |
288| | `INIT` | Returns the reason of the last reset | `INIT` |
289| | `UPTIME` | Returns device uptime in milliseconds | `UPTIME` |
290| | `MAC` | Returns the Wi-Fi MAC address | `MAC` |
291| | `NET_INFO` | Returns IP, gateway, subnet, SSID, and RSSI | `NET_INFO` |
292| | `RESET_WIFI` | Clears all Wi-Fi settings and restarts into provisioning mode | `RESET_WIFI` |
293| 
294| ### Example UDP Response
295| 
296| ```text
297| Client sends:  LED_ON
298| Device reply:  LED ligado
299| ```
300| 
301| ```text
302| Client sends:  NET_INFO
303| Device reply:
304| IP: 192.168.1.25
305| Gateway: 192.168.1.1
306| Mascara de rede: 255.255.255.0
307| RSSI: -45 dbm
308| Nome da Rede: MinhaRede
309| ```
310| 
311| ---
312| 
313| ## Reset Behavior
314| 
315| The device can erase Wi-Fi configuration in two ways:
316| 
317| ### 1. Hardware reset button
318| 
319| When the physical button connected to GPIO 0 is pressed, the firmware:
320| 
321| - clears Preferences memory
322| - removes SSID and password
323| - flashes the LED as feedback
324| - restarts the ESP32
325| - returns to AP mode for reconfiguration
326| 
327| ### 2. Remote command reset
328| 
329| ```text
330| RESET_WIFI
331| ```
332| 
333| This command triggers the same process, ensuring a quick recovery path if the network settings are lost or invalid.
334| 
335| ```mermaid
336| flowchart TD
337|     A[GPIO0 or RESET_WIFI command] --> B[Clear Preferences]
338|     B --> C[Remove SSID]
339|     C --> D[Remove password]
340|     D --> E[Flash LED]
341|     E --> F[Restart ESP32]
342|     F --> G[Start ESP32_CONFIG Access Point]
343| ```
344| 
345| ---
346| 
347| ## OLED Display Behavior
348| 
349| The SSD1306 display is used as a lightweight diagnostic panel and status output. It logs important events such as:
350| 
351| - system startup
352| - Wi-Fi connection attempts
353| - current IP address
354| - configuration portal activation
355| - memory reset events
356| - command logging
357| 
358| The display is connected via the I2C bus:
359| 
360| ```text
361| SDA = GPIO21
362| SCL = GPIO22
363| Address = 0x3C
364| ```
365| 
366| ---
367| 
368| ## Firmware Structure
369| 
370| The repository contains the following files:
371| 
372| ### Main Application
373| - **`wifi.ino`** - Main firmware sketch containing the application entry point and core logic
374| 
375| ### Header Files (Modular Components)
376| - **`Config.h`** - Configuration constants and settings (pins, network parameters, feature toggles)
377| - **`CommandHandler.h`** - UDP command parsing and execution logic
378| - **`DisplayManager.h`** - OLED display rendering and management
379| - **`HardwareController.h`** - LED control, button handling, and GPIO initialization
380| - **`NetworkManager.h`** - Wi-Fi connection management and network utilities
381| - **`NTPUtil.h`** - Network Time Protocol utilities for time synchronization
382| 
383| ### Documentation
384| - **`README.md`** - This file; project documentation and usage guide
385| 
386| ---
387| 
388| ## Quick Start
389| 
390| ### Requirements
391| 
392| - ESP32 development board
393| - SSD1306 OLED (128x64, I2C)
394| - LED on GPIO 2
395| - Reset switch connected to GPIO 0
396| - Arduino IDE or PlatformIO
397| - ESP32 board package installed
398| 
399| ### Build and Upload
400| 
401| 1. Open `wifi.ino` in Arduino IDE
402| 2. Select the correct ESP32 board and COM port
403| 3. Install the required libraries:
404|    - `WiFi.h`
405|    - `WiFiUdp.h`
406|    - `WebServer.h`
407|    - `Preferences.h`
408|    - `Adafruit_GFX.h`
409|    - `Adafruit_SSD1306.h`
410| 4. Upload the sketch to the ESP32
411| 5. Power cycle the device
412| 
413| ### First Boot
414| 
415| - If no Wi-Fi has been saved, the ESP32 will start an access point called `ESP32_CONFIG`
416| - Connect to it and open the configuration page
417| - Enter the Wi-Fi SSID/password and save
418| 
419| ---
420| 
421| ## Operational Notes
422| 
423| - The module uses NVS/Preferences, which are retained across restarts
424| - The AP is created with a default SSID and can be used as a self-contained provisioning mechanism
425| - The UDP server allows automation and remote control in local networks
426| - The firmware is suitable for home automation, prototyping, remote diagnostics, and device control scenarios
427| 
428| ---
429| 
430| ## Example Use Cases
431| 
432| - remote switching of an onboard LED
433| - network monitoring from a local application
434| - remote device health checks
435| - Wi-Fi reconfiguration without connecting to the UART serial console
436| - embedded diagnostic dashboard for ESP32 systems
437| 
438| ---
439| 
440| ## Conclusion
441| 
442| This project provides a practical, compact, and professional ESP32 firmware for Wi-Fi commissioning and remote control. It combines a local web-based provisioning interface, automatic reconnectio[...]
443| 
444| It is ideal for embedded engineers, IoT prototyping, and remote monitoring applications that need a stable and manageable communication layer.
445| 
446| ---
447| 
448| ## License
449| 
450| This project is distributed as-is for educational, experimental, and prototyping purposes. Please review the repository license before production deployment in commercial environments.
451| 
452| ---
453| 
454| ## Project Summary
455| 
456| ```text
457| ESP32 Wi-Fi Provisioning + OLED Diagnostics + UDP Control
458| - Network provisioning via captive web portal
459| - Persistent Wi-Fi credentials with Preferences
460| - UDP control interface on port 4210
461| - LED and reset handling
462| - CPU, temperature, memory, flash, and uptime reports
463| - Local SSD1306 diagnostics display
464| - Modular architecture with header file components
465| ```
466| 