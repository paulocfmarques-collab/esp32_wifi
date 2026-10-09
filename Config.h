#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config {
    constexpr uint8_t PIN_LED = 2;
    constexpr uint8_t PIN_BOTAO_RESET = 0;
    constexpr uint8_t PIN_SDA = 21;
    constexpr uint8_t PIN_SCL = 22;

    constexpr uint8_t SCREEN_WIDTH = 128;
    constexpr uint8_t SCREEN_HEIGHT = 64;
    constexpr uint8_t MAX_LINHAS = 7; // Limite de linhas de log na tela OLED
    
    constexpr uint16_t UDP_PORT = 4210;
    constexpr uint8_t PAGE_COUNT = 5;
    constexpr uint32_t PAGE_INTERVAL_MS = 8000;
    constexpr uint32_t WIFI_ATTEMPT_MS = 10000;
    constexpr uint32_t WIFI_RETRY_MS = 30000;
    constexpr int FUSO_PADRAO = -3; // Fuso de Brasília

    constexpr const float MB = 1024.0 * 1024.0; // Constante para conversão de bytes para megabytes
    constexpr const float KB = 1024.0; // Constante para conversão de bytes para kilobytes

    constexpr const char* OTA_HOSTNAME = "ESP32-Network-Hub";
    constexpr const char* OTA_PASSWORD = ""; // Deixe vazio ou coloque uma senha para proteger o upload
    constexpr uint32_t OLED_TIMEOUT_MS = 180000;
}


#endif
