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
    bool modoRelogioGrande; // <- Controla o estado de exibição

public:
    DisplayManager() : display(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, &Wire, -1), 
                       totalLinhas(0), oledInicializado(false), ultimaAtualizacaoRelogio(0),
                       modoRelogioGrande(false) {}

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

    void setModoRelogioGrande(bool ativar) {
        modoRelogioGrande = ativar;
        renderizar();
    }

    bool getModoRelogioGrande() const {
        return modoRelogioGrande;
    }

    void adicionarLinha(const String& novoTexto) {
        Serial.println("[OLED] " + novoTexto);
        if (!oledInicializado) return;

        if (totalLinhas >= (Config::MAX_LINHAS - 1)) {
            for (int i = 0; i < (Config::MAX_LINHAS - 2); i++) {
                historicoLinhas[i] = historicoLinhas[i + 1];
            }
            historicoLinhas[Config::MAX_LINHAS - 2] = novoTexto;
        } else {
            historicoLinhas[totalLinhas] = novoTexto;
            totalLinhas++;
        }
        
        // Se estiver no modo relógio grande, não renderiza o log imediatamente na tela
        if (!modoRelogioGrande) {
            renderizar();
        }
    }

    void atualizarTela(NTPUtil& ntpService) {
        if (!oledInicializado) return;

        unsigned long agora = millis();
        if (agora - ultimaAtualizacaoRelogio >= 1000) {
            ultimaAtualizacaoRelogio = agora;
            renderizar();
        }
    }

private:
    void renderizar() {
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);

        if (modoRelogioGrande) {
            renderizarRelogioGrande();
        } else {
            renderizarLogs();
        }
        
        display.display();
    }

    void renderizarLogs() {
        display.setTextSize(1);
        String dataHoraCompleta;
        ntp.getDateTime(dataHoraCompleta, 50);
        display.setCursor(0, 0);
        display.print("[" + (dataHoraCompleta.indexOf("Erro") == -1 && dataHoraCompleta.length() >= 19 ? dataHoraCompleta : "00/00/0000 00:00:00") + "]");
        
        display.drawFastHLine(0, 9, Config::SCREEN_WIDTH, SSD1306_WHITE);

        for (int i = 0; i < totalLinhas; i++) {
            display.setCursor(0, 12 + (i * 8)); 
            display.println(historicoLinhas[i]);
        }
    }

    void renderizarRelogioGrande() {
        String dataHoraCompleta;
        ntp.getDateTime(dataHoraCompleta, 50);
        
        String dataStr = "--/--/----";
        String horaStr = "--:--:--";
        
        if (dataHoraCompleta.length() >= 19) {
            dataStr = dataHoraCompleta.substring(0, 10); // DD/MM/AAAA
            horaStr = dataHoraCompleta.substring(11, 19); // HH:MM:SS
        }

        // Desenha a Hora Grande (Tamanho de fonte 2)
        display.setTextSize(2);
        // Centralização aproximada para fonte tam 2 (cada caractere tem 12px de largura, 8 caracteres = 96px)
        int16_t xHora = (Config::SCREEN_WIDTH - 96) / 2; 
        display.setCursor(xHora, 15);
        display.print(horaStr);

        // Desenha a Data Média (Tamanho de fonte 1)
        display.setTextSize(1);
        // Centralização aproximada para fonte tam 1 (cada caractere tem 6px de largura, 10 caracteres = 60px)
        int16_t xData = (Config::SCREEN_WIDTH - 60) / 2;
        display.setCursor(xData, 42);
        display.print(dataStr);
    }
};

extern DisplayManager oled;

#endif
