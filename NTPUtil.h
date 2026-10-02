#ifndef NTP_UTIL_H
#define NTP_UTIL_H

#include <Arduino.h>
#include <time.h>

class NTPUtil {
public:
    bool initNTP(int fusoInicial = -3) {
        Serial.println(F("--- Inicializando NTP ---"));
        atualizarFusoHorario(fusoInicial);
        Serial.println(F("[NTP] Serviço iniciado. Aguardando sincronização..."));

        for (int tentativa = 0; tentativa < 20; tentativa++) {
            struct tm timeinfo;
            if (getLocalTime(&timeinfo, 1000)) {
                Serial.println(F("\n[NTP] Sincronização concluída."));
                return true;
            }
            delay(1000);
        }
        return false;
    }

    void atualizarFusoHorario(int novoFuso) {
        char tzString[32];
        int fusoInvertido = -novoFuso;

        if (fusoInvertido >= 0) {
            snprintf(tzString, sizeof(tzString), "GMT%d", fusoInvertido);
        } else {
            snprintf(tzString, sizeof(tzString), "GMT-%d", -fusoInvertido);
        }
        
        Serial.printf("[NTP] Aplicando String de Fuso POSIX Correta: %s\n", tzString);
        
        configTzTime(
            tzString,
            "a.st1.ntp.br",
            "pool.ntp.org",
            "time.nist.gov"
        );
    }

    void getDateTime(String& dateTime, uint32_t timeoutMs = 5000) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, timeoutMs)) {
            char buffer[20];
            strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
            dateTime = String(buffer);
        } else {
            dateTime = "Erro ao obter data e hora";
        }
    }
};

extern NTPUtil ntp;

#endif
