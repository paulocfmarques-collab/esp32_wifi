#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "Config.h"
#include "NTPUtil.h"

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    String historicoLinhas[Config::MAX_LINHAS];
    int totalLinhas;
    bool oledInicializado;
    unsigned long ultimaAtualizacaoRelogio;

public:
    DisplayManager() : display(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, &Wire, -1), totalLinhas(0), oledInicializado(false), ultimaAtualizacaoRelogio(0) {}

    void begin() {
        Wire.begin(Config::PIN_SDA, Config::PIN_SCL); 
        if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            oledInicializado = true;
            display.clearDisplay();
            display.display();
            adicionarLinha("OLED Pronto!");
        } else {
            Serial.println(F("[OLED] Falha ao encontrar o Display nos pinos mapeados!"));
        }
    }

    void adicionarLinha(const String& novoTexto) {
        Serial.println("[OLED] " + novoTexto);
        if (!oledInicializado) return;

        // Limita a área inferior de logs (desconta espaço reservado do relógio)
        if (totalLinhas >= (Config::MAX_LINHAS - 1)) {
            for (int i = 0; i < (Config::MAX_LINHAS - 2); i++) {
                historicoLinhas[i] = historicoLinhas[i + 1];
            }
            historicoLinhas[Config::MAX_LINHAS - 2] = novoTexto;
        } else {
            historicoLinhas[totalLinhas] = novoTexto;
            totalLinhas++;
        }
        renderizar();
    }

    void atualizarTela(NTPUtil& ntpService) {
        if (!oledInicializado) return;

        // Atualiza a tela a cada 1 segundo para atualizar o relógio sem travar o loop
        unsigned long agora = millis();
        if (agora - ultimaAtualizacaoRelogio >= 1000) {
            ultimaAtualizacaoRelogio = agora;
            renderizar();
        }
    }

private:
    void renderizar() {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        
        // Exibe o relógio fixo na linha 0
        String dataHoraCompleta;
        ntp.getDateTime(dataHoraCompleta, 50);
        display.setCursor(0, 0);
        display.print("[" + (dataHoraCompleta.indexOf("Erro") == -1 && dataHoraCompleta.length() >= 19 ? dataHoraCompleta : "00/00/0000 00:00:00") + "]");
        
        display.drawFastHLine(0, 9, Config::SCREEN_WIDTH, SSD1306_WHITE);

        // Renderiza os históricos salvos abaixo da linha separadora
        for (int i = 0; i < totalLinhas; i++) {
            display.setCursor(0, 12 + (i * 8)); 
            display.println(historicoLinhas[i]);
        }
        display.display();
    }
};

extern DisplayManager oled;

#endif
