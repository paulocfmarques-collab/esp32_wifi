#ifndef HARDWARE_CONTROLLER_H
#define HARDWARE_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

class HardwareController {
private:
    bool blinkAtivo;
    bool estadoLed;
    unsigned long ultimoToggle;
    unsigned long intervaloBlink;

public:
    HardwareController() : blinkAtivo(false), estadoLed(false), ultimoToggle(0), intervaloBlink(500) {}

    void begin() {
        pinMode(Config::PIN_LED, OUTPUT);
        pinMode(Config::PIN_BOTAO_RESET, INPUT_PULLUP);
        digitalWrite(Config::PIN_LED, LOW);
    }

    void setLed(bool ligar) {
        blinkAtivo = false;
        estadoLed = ligar;
        digitalWrite(Config::PIN_LED, estadoLed);
    }

    void alternarLed() {
        estadoLed = !estadoLed;
        digitalWrite(Config::PIN_LED, estadoLed);
    }

    void iniciarBlinkAsync(unsigned long intervalo) {
        intervaloBlink = intervalo;
        blinkAtivo = true;
    }

    void piscarSincrono(int piscadas, int tempoMs) {
        blinkAtivo = false;
        for (int i = 0; i < piscadas; i++) {
            digitalWrite(Config::PIN_LED, HIGH);  delay(tempoMs);
            digitalWrite(Config::PIN_LED, LOW);   delay(tempoMs);
        }
    }

    bool verificarBotaoReset() {
        return (digitalRead(Config::PIN_BOTAO_RESET) == LOW);
    }

    float lerTemperatura() {
        return temperatureRead();
    }

    void atualizarBlink() {
        if (!blinkAtivo) return;

        unsigned long agora = millis();
        if (agora - ultimoToggle >= intervaloBlink) {
            ultimoToggle = agora;
            estadoLed = !estadoLed;
            digitalWrite(Config::PIN_LED, estadoLed);
        }
    }
};

extern HardwareController hardware;

#endif
