#ifndef NTP_UTIL_H
#define NTP_UTIL_H

#include <Arduino.h>
#include <time.h>

class NTPUtil {
public:
    bool initNTP(int fusoInicial = -3, bool dstAtivo = false) {
        Serial.println(F("--- Inicializando NTP ---"));
        configurarRelogio(fusoInicial, dstAtivo);
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

    void configurarRelogio(int fuso, bool dstAtivo) {
        char tzString[64];
        int effectiveOffset = fuso + (dstAtivo ? 1 : 0);
        snprintf(tzString,sizeof(tzString),"UTC%d",-effectiveOffset);
        Serial.printf("[NTP] Aplicando String POSIX: %s\n", tzString);
        
        // Aplica o fuso e aponta para os servidores NTP
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
            char buffer[25];
            strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
            dateTime = String(buffer);
        } else {
            dateTime = "Erro ao obter data e hora";
        }
    }

    bool isSincronizado() 
    {
        struct tm timeinfo;
        return getLocalTime(&timeinfo, 10);
    }
};

extern NTPUtil ntp;

#endif
