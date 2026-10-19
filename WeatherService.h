#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <cJSON.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <time.h>
#include "Config.h"

class WeatherService
{
public:
    struct Reading {
        bool valid = false, lastAttemptOk = false;
        float temperature = 0, humidity = 0;
        double sourceTime = 0;
        int64_t receivedUs = 0;
        double ageAtReceipt = 0;
    };

private:
    QueueHandle_t readings = nullptr;
    TaskHandle_t worker = nullptr;

    // HTTPClient decodifica o corpo, mas nao pode alocar uma resposta ilimitada.
    class Response : public Stream {
    public:
        char body[2048]{};
        size_t length = 0;
        int available() override { return 0; }
        int read() override { return -1; }
        int peek() override { return -1; }
        void flush() override {}
        size_t write(uint8_t value) override { return write(&value, 1); }
        size_t write(const uint8_t *data, size_t count) override {
            const size_t room = sizeof(body) - 1 - length;
            if (count > room) return 0;
            memcpy(body + length, data, count);
            length += count;
            body[length] = '\0';
            return count;
        }
    };

    static bool fetch(Reading &out)
    {
        NetworkClientSecure client;
        client.useBuiltinCACertBundle();
        client.setHandshakeTimeout(8);
        HTTPClient http;
        http.setConnectTimeout(5000);
        http.setTimeout(5000);
        http.setReuse(false);
        http.useHTTP10(true);
        if (!http.begin(client, Config::WEATHER_URL)) return false;
        const int status = http.GET();
        Response response;
        const bool received = status == HTTP_CODE_OK && http.getSize() < int(sizeof(response.body)) &&
                              http.writeToStream(&response) > 0;
        http.end();
        if (!received) return false;

        cJSON *root = cJSON_Parse(response.body);
        if (!root) return false;
        const cJSON *current = cJSON_GetObjectItemCaseSensitive(root, "current");
        const cJSON *units = cJSON_GetObjectItemCaseSensitive(root, "current_units");
        const cJSON *temp = cJSON_GetObjectItemCaseSensitive(current, "temperature_2m");
        const cJSON *humidity = cJSON_GetObjectItemCaseSensitive(current, "relative_humidity_2m");
        const cJSON *stamp = cJSON_GetObjectItemCaseSensitive(current, "time");
        const cJSON *tempUnit = cJSON_GetObjectItemCaseSensitive(units, "temperature_2m");
        const cJSON *humidityUnit = cJSON_GetObjectItemCaseSensitive(units, "relative_humidity_2m");
        const cJSON *timeUnit = cJSON_GetObjectItemCaseSensitive(units, "time");
        const double now = double(time(nullptr));
        const bool valid = cJSON_IsNumber(temp) && cJSON_IsNumber(humidity) && cJSON_IsNumber(stamp) &&
            isfinite(temp->valuedouble) && temp->valuedouble >= -90 && temp->valuedouble <= 65 &&
            isfinite(humidity->valuedouble) && humidity->valuedouble >= 0 && humidity->valuedouble <= 100 &&
            isfinite(stamp->valuedouble) && stamp->valuedouble > 1700000000 &&
            stamp->valuedouble <= now + 60 && now - stamp->valuedouble <= Config::WEATHER_MAX_AGE_S &&
            cJSON_IsString(tempUnit) && strcmp(tempUnit->valuestring, "\xC2\xB0" "C") == 0 &&
            cJSON_IsString(humidityUnit) && strcmp(humidityUnit->valuestring, "%") == 0 &&
            cJSON_IsString(timeUnit) && strcmp(timeUnit->valuestring, "unixtime") == 0;
        if (valid) {
            out.temperature = temp->valuedouble;
            out.humidity = humidity->valuedouble;
            out.sourceTime = stamp->valuedouble;
            out.ageAtReceipt = now > out.sourceTime ? now - out.sourceTime : 0;
            out.receivedUs = esp_timer_get_time();
            out.valid = true;
        }
        cJSON_Delete(root);
        return valid;
    }

    static void run(void *context)
    {
        auto &self = *static_cast<WeatherService *>(context);
        Reading cached;
        for (;;) {
            // TLS precisa de relogio sincronizado para validar os certificados.
            if (WiFi.status() != WL_CONNECTED || time(nullptr) < 1700000000) {
                cached.lastAttemptOk = false;
                xQueueOverwrite(self.readings, &cached);
                vTaskDelay(max(TickType_t(1), pdMS_TO_TICKS(5000)));
                continue;
            }
            cached.lastAttemptOk = fetch(cached);
            xQueueOverwrite(self.readings, &cached);
            if (!cached.lastAttemptOk) Serial.println("[CLIMA] Consulta Open-Meteo falhou; cache preservado");
            const uint32_t pause = cached.lastAttemptOk ? Config::WEATHER_INTERVAL_MS : 60000UL;
            vTaskDelay(max(TickType_t(1), pdMS_TO_TICKS(pause)));
        }
    }

public:
    void begin()
    {
        if (readings) return;
        readings = xQueueCreate(1, sizeof(Reading));
        if (!readings) {
            Serial.println("[CLIMA] Falha ao criar cache");
            return;
        }
        Reading empty;
        xQueueOverwrite(readings, &empty);
        if (xTaskCreate(run, "ClimaPOA", 8192, this, 1, &worker) != pdPASS) {
            vQueueDelete(readings);
            readings = nullptr;
            Serial.println("[CLIMA] Falha ao criar tarefa");
        }
    }
    Reading snapshot() const
    {
        Reading value;
        if (readings) xQueuePeek(readings, &value, 0);
        return value;
    }
    static double ageSeconds(const Reading &value)
    {
        return value.ageAtReceipt + (esp_timer_get_time() - value.receivedUs) / 1000000.0;
    }
};
