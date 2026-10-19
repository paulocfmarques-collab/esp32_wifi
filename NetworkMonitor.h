#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "ping/ping_sock.h"
#include "StateLock.h"

// Callbacks publicam resultados; TaskRede atualiza e o OLED le um snapshot protegido.
class NetworkMonitor
{
public:
    struct Snapshot {
        bool hasResult = false, replied = false;
        uint32_t drops = 0, attempts = 0, failures = 0, rtt = 0;
        uint64_t offlineSeconds = 0;
        String events[3];
        int history[24]{};
        uint8_t head = 0;
    };
    bool snapshot(Snapshot &out) const;
    bool connected = false, initialized = false, hasResult = false, replied = false;
    uint32_t drops = 0, attempts = 0, failures = 0, rtt = 0, lastDown = 0, measuredAt = 0, received = 0, minimum = 0, maximum = 0;
    uint64_t totalRtt = 0;
    uint64_t offlineMs = 0;
    String events[3];
    int history[24];
    uint8_t head = 0;
    void begin();
    void update();
    uint64_t offlineSeconds() const { return (offlineMs + (!connected ? uint32_t(millis() - offlineAt) : 0)) / 1000; }

private:
    SemaphoreHandle_t stateMutex = nullptr;
    uint32_t offlineAt = 0, tickAt = 0, nextAt = 0, epoch = 0, sessionEpoch = 0;
    esp_ping_handle_t session = nullptr;
    IPAddress target;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    bool done = false, success = false;
    uint32_t elapsed = 0;
    static void onSuccess(esp_ping_handle_t, void *);
    static void onEnd(esp_ping_handle_t, void *);
    void event(const String &text);
};
