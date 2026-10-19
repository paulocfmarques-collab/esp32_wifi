#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>
#include "Config.h"
#include "NTPUtil.h"
#include "NetworkMonitor.h"
#include "StateLock.h"
class DisplayManager
{
    Adafruit_SSD1306 display;
    SemaphoreHandle_t stateMutex = nullptr;
    String lines[6], trying;
    uint8_t count = 0, page = 0;
    bool ready = false, automatic = true;
    uint32_t rendered = 0, pageAt = 0, consoleUntil = 0;
    const NetworkMonitor *monitor = nullptr;
    void line(uint8_t row, const String &text)
    {
        display.setCursor(0, 14 + row * 10);
        display.print(text.substring(0, 21));
    }
    uint32_t ultimaInteracao = 0;

    void desenharCoracao(int x, int y)
    {
        display.fillCircle(x - 7, y - 4, 8, SSD1306_WHITE);
        display.fillCircle(x + 7, y - 4, 8, SSD1306_WHITE);
        display.fillTriangle(x - 15, y - 2, x + 15, y - 2, x, y + 16, SSD1306_WHITE);
    }

    void desenharCurva(int x, int y, bool sorriso, int largura)
    {
        const int ponta = sorriso ? -4 : 4;
        display.drawLine(x - largura, y + ponta, x - largura / 2, y, SSD1306_WHITE);
        display.drawFastHLine(x - largura / 2, y, largura + 1, SSD1306_WHITE);
        display.drawLine(x + largura / 2, y, x + largura, y + ponta, SSD1306_WHITE);
    }

    void desenharCarinha(int x, int y, uint8_t expressao, bool piscar, int olhar)
    {
        // Olhos e boca ampliados para ocupar a tela inteira, sem moldura.
        const int eyeY = y - 10;
        for (uint8_t i = 0; i < 2; ++i) {
            const int eyeX = x + (i == 0 ? -30 : 30) + olhar;
            if (piscar || expressao == 7 || (expressao == 1 && i == 0)) {
                display.fillRoundRect(eyeX - 14, eyeY - 2, 28, 4, 2, SSD1306_WHITE);
            } else if (expressao == 5) {
                desenharCoracao(eyeX, eyeY);
            } else if (expressao == 8) {
                desenharCurva(eyeX, eyeY - 3, false, 14);
            } else {
                display.fillRoundRect(eyeX - 9, eyeY - 11, 19, 24, 8, SSD1306_WHITE);
                if (expressao == 3) {
                    // Sobrancelhas inclinadas para dentro: bravo.
                    display.drawLine(eyeX - 14, eyeY - (i == 0 ? 18 : 12),
                                     eyeX + 14, eyeY - (i == 0 ? 12 : 18), SSD1306_WHITE);
                } else if (expressao == 2 || expressao == 9) {
                    display.drawLine(eyeX - 13, eyeY - (i == 0 ? 12 : 18),
                                     eyeX + 13, eyeY - (i == 0 ? 18 : 12), SSD1306_WHITE);
                }
            }
        }

        if (expressao == 6) {
            display.drawCircle(x, y + 17, 7, SSD1306_WHITE);
        } else if (expressao == 4) {
            // Expressao neutra da referencia: apenas os olhos.
        } else if (expressao == 7) {
            display.drawFastHLine(x - 15, y + 18, 31, SSD1306_WHITE);
        } else {
            const bool triste = expressao == 2 || expressao == 3 || expressao == 9;
            desenharCurva(x, y + 18, !triste, 18);
        }
    }

    void desenharProtecao(uint32_t elapsed)
    {
        const uint32_t frame = elapsed / 100;
        // Deslocamento curto mantem os desenhos ampliados dentro dos 128x64 pixels.
        const int phaseX = (frame / 3) % 16;
        const int phaseY = (frame / 5) % 12;
        const int x = Config::SCREEN_WIDTH / 2 - 4 + (phaseX <= 8 ? phaseX : 16 - phaseX);
        const int y = Config::SCREEN_HEIGHT / 2 - 3 + (phaseY <= 6 ? phaseY : 12 - phaseY);
        const int olhar = 2 * (int((frame / 12) % 5) - 2);
        // Ordem das dez expressoes da imagem, com piscada na troca de cara.
        const bool piscar = elapsed % 4000 < 200 || frame % 31 == 0;
        desenharCarinha(x, y, (elapsed / 4000) % 10, piscar, olhar);
    }

public:
    DisplayManager() : display(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, &Wire, -1) {}
    void begin()
    {
        stateMutex = xSemaphoreCreateRecursiveMutex();
        if (!stateMutex) {
            Serial.println("[OLED] Falha ao criar mutex");
            return;
        }
        Wire.begin(Config::PIN_SDA, Config::PIN_SCL);
        ready = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        if (ready)
        {
            display.setTextWrap(false);
            display.clearDisplay();
            display.display();
        }
        adicionarLinha(ready ? "OLED pronto" : "OLED indisponivel");
        registrarInteracao();
    }
    bool isPronto() const { return ready; }
    void setMonitor(const NetworkMonitor *m) {
        StateLock lock(stateMutex); if (!lock) return;
        monitor = m;
    }
    void setConnecting(const String &ssid) {
        StateLock lock(stateMutex); if (!lock) return;
        trying = ssid;
    }
    void selecionarPagina(uint8_t p)
    {
        StateLock lock(stateMutex); if (!lock) return;
        registrarInteracao();
        page = p % Config::PAGE_COUNT;
        automatic = false;
        consoleUntil = 0;
        rendered = millis() - 1000;
    }
    void avancarPagina() {
        StateLock lock(stateMutex); if (!lock) return;
        selecionarPagina((page + 1) % Config::PAGE_COUNT);
    }
    void paginasAutomaticas()
    {
        StateLock lock(stateMutex); if (!lock) return;
        registrarInteracao();
        automatic = true;
        pageAt = millis();
        consoleUntil = 0;
        rendered = millis() - 1000;
    }
    void setModoRelogioGrande(bool on) { selecionarPagina(on ? 0 : 1); }
    bool getModoRelogioGrande() const {
        StateLock lock(stateMutex); return lock && page == 0;
    }
    void clearLog()
    {
        StateLock lock(stateMutex); if (!lock) return;
        registrarInteracao();
        for (auto &s : lines)
            s = "";
        count = 0;
        consoleUntil = millis() + 5000;
        rendered = millis() - 1000;
    }
    void adicionarLinha(const String &text)
    {
        StateLock lock(stateMutex); if (!lock) return;
        Serial.println("[OLED] " + text);
        if (count == 6)
        {
            for (int i = 0; i < 5; i++)
                lines[i] = lines[i + 1];
            lines[5] = text;
        }
        else
            lines[count++] = text;
        consoleUntil = millis() + 5000;
        rendered = millis() - 1000;
    }

    void registrarInteracao() {
        StateLock lock(stateMutex); if (!lock) return;
        ultimaInteracao = millis();
        pageAt = ultimaInteracao;
        consoleUntil = 0;
        rendered = ultimaInteracao - 1000;
    }

void atualizarTela(NTPUtil& ntpService) {
    if (!ready) return;
    // Se a rede estiver alterando o estado, tenta novamente no proximo loop.
    StateLock lock(stateMutex, 0); if (!lock) return;
    uint32_t now = millis();
    const uint32_t idle = now - ultimaInteracao;
    const bool protecao = idle >= Config::OLED_TIMEOUT_MS;

    // Limita o trafego I2C para manter o loop livre para ler o botao.
    if (now - rendered < (protecao ? 100U : 1000U)) return;
    rendered = now;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    if (protecao) {
        desenharProtecao(idle - Config::OLED_TIMEOUT_MS);
        display.display();
        return;
    }

    String dt; ntpService.getDateTime(dt, 0); bool valid = dt.length() == 19 && dt.indexOf("Erro") < 0;

    if (int32_t(consoleUntil - now) > 0) {
        display.setCursor(0, 0); display.print(valid ? dt : String("RELOGIO SEM SINCRONIA"));
        display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
        for (int i = 0; i < count; i++) { display.setCursor(0, 14 + i * 8); display.print(lines[i].substring(0, 21)); }
    } else {
        if (automatic && now - pageAt >= Config::PAGE_INTERVAL_MS) {
            page = (page + 1) % Config::PAGE_COUNT;
            pageAt = now;
        }
        
        // DECLARAÇÃO DA VARIÁVEL ADICIONADA AQUI (Resolve o erro do online):
        bool online = WiFi.status() == WL_CONNECTED; 

        const char* titles[] = {"RELOGIO", "REDE WI-FI", "SISTEMA", "PING GATEWAY", "HISTORICO"};
        display.setCursor(0, 0); display.print(titles[page]); display.setCursor(110, 0); display.printf("%u/5", page + 1); display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
        
        NetworkMonitor::Snapshot snapshot;
        const bool hasSnapshot = (page == 3 || page == 4) && monitor && monitor->snapshot(snapshot);
        if (page == 0) {
            display.setTextSize(2); display.setCursor(16, 20); display.print(valid ? dt.substring(11) : String("--:--:--")); display.setTextSize(1); line(3, valid ? dt.substring(0, 10) : String("Aguardando NTP")); line(4, online ? WiFi.SSID() : String("AP ESP32_CONFIG"));
        }
        else if (page == 1) {
            line(0, online ? WiFi.SSID() : trying.length() ? "Busca: " + trying : String("AP ESP32_CONFIG")); line(1, "IP: " + (online ? WiFi.localIP() : WiFi.softAPIP()).toString()); line(2, "GW: " + WiFi.gatewayIP().toString()); line(3, online ? "RSSI: " + String(WiFi.RSSI()) + " dBm" : String("WiFi desconectado")); line(4, "Canal: " + String(WiFi.channel()));
            int bars = online ? (WiFi.RSSI() >= -65 ? 4 : WiFi.RSSI() >= -75 ? 3 : WiFi.RSSI() >= -85 ? 2 : 1) : 0;
            for (int i = 0; i < 4; i++) { int h = 3 + i * 2, x = 104 + i * 6; display.drawRect(x, 61 - h, 4, h, SSD1306_WHITE); if (i < bars) display.fillRect(x, 61 - h, 4, h, SSD1306_WHITE); }
        } else if (page == 2) {
            line(0, String(ESP.getChipModel()) + " " + String(ESP.getCpuFreqMHz()) + "MHz"); line(1, "Heap: " + String(ESP.getFreeHeap() / 1024) + " KB"); line(2, "Min heap: " + String(ESP.getMinFreeHeap() / 1024) + " KB"); line(3, "Flash: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB"); line(4, "Ligado: " + String(now / 1000) + " s");
        }
        else if (page == 3 && hasSnapshot) {
            const auto& m = snapshot;
            line(0, !online ? String("Sem rede") : !m.hasResult ? String("Aguardando ping") : m.replied ? "RTT: " + String(m.rtt) + " ms" : String("Gateway sem resposta"));
            line(1, "Quedas: " + String(m.drops) + " Off:" + String((unsigned long)m.offlineSeconds) + "s");
            line(2, "Falhas: " + String(m.attempts ? uint32_t(uint64_t(m.failures) * 100 / m.attempts) : 0) + "%");
            int peak = 10; for (int v : m.history) if (v > peak) peak = v;
            for (int i = 0; i < 24; i++) { int v = m.history[(m.head + i) % 24], x = 4 + i * 5; if (v == -1) { display.drawLine(x, 48, x + 3, 51, SSD1306_WHITE); display.drawLine(x + 3, 48, x, 51, SSD1306_WHITE); } else if (v >= 0) { int h = max(1, v * 15 / peak); display.drawFastVLine(x, 63 - h, h, SSD1306_WHITE); } }
        } else if (page == 4 && hasSnapshot) {
            for (int i = 0; i < 3; i++) line(i, snapshot.events[i]); line(4, "Historico em RAM");
        } else if (page == 3 || page == 4) {
            line(0, "Atualizando monitor");
        }
    }
    display.display();
}
};
extern DisplayManager oled;
