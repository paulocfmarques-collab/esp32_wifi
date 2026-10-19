#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Mutex de tarefa: nunca bloqueia interrupcoes durante String, I2C ou rede.
class StateLock
{
    SemaphoreHandle_t mutex;
    bool acquired;
public:
    explicit StateLock(SemaphoreHandle_t value, TickType_t wait = portMAX_DELAY)
        : mutex(value), acquired(value && xSemaphoreTakeRecursive(value, wait) == pdTRUE) {}
    ~StateLock() { if (acquired) xSemaphoreGiveRecursive(mutex); }
    explicit operator bool() const { return acquired; }
    StateLock(const StateLock&) = delete;
    StateLock& operator=(const StateLock&) = delete;
};
