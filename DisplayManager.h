#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>
#include "Config.h"
#include "NTPUtil.h"
#include "NetworkMonitor.h"
class DisplayManager
{
    Adafruit_SSD1306 display;
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

    void desenharCarinha(int x, int y, uint8_t expressao, bool piscar)
    {
        display.drawCircle(x, y, 9, SSD1306_WHITE);
        if (piscar || expressao == 3) {
            display.drawFastHLine(x - 5, y - 2, 3, SSD1306_WHITE);
            display.drawFastHLine(x + 3, y - 2, 3, SSD1306_WHITE);
        } else {
            display.fillCircle(x - 4, y - 2, 1, SSD1306_WHITE);
            if (expressao == 1)
                display.drawFastHLine(x + 3, y - 2, 3, SSD1306_WHITE);
            else
                display.fillCircle(x + 4, y - 2, 1, SSD1306_WHITE);
        }

        if (expressao == 2) {
            display.drawCircle(x, y + 4, 2, SSD1306_WHITE);
        } else if (expressao == 3) {
            display.drawFastHLine(x - 2, y + 4, 5, SSD1306_WHITE);
        } else {
            display.drawLine(x - 4, y + 2, x - 2, y + 4, SSD1306_WHITE);
            display.drawFastHLine(x - 2, y + 5, 5, SSD1306_WHITE);
            display.drawLine(x + 2, y + 4, x + 4, y + 2, SSD1306_WHITE);
        }
    }

    void desenharProtecao(uint32_t elapsed)
    {
        const uint32_t frame = elapsed / 250;
        // Seis carinhas em areas separadas, com movimento e expressoes diferentes.
        const int cellW = Config::SCREEN_WIDTH / 3;
        const int cellH = Config::SCREEN_HEIGHT / 2;
        const int rangeX = cellW - 19;
        const int rangeY = cellH - 19;
        for (uint8_t i = 0; i < 6; ++i) {
            const int phaseX = (frame + i * 11) % (2 * rangeX);
            const int phaseY = (frame + i * 7) % (2 * rangeY);
            const int x = (i % 3) * cellW + 9 +
                          (phaseX <= rangeX ? phaseX : 2 * rangeX - phaseX);
            const int y = (i / 3) * cellH + 9 +
                          (phaseY <= rangeY ? phaseY : 2 * rangeY - phaseY);
            desenharCarinha(x, y, (elapsed / 3000 + i) % 4,
                             (frame + i * 3) % 16 == 0);
        }
    }

public:
    DisplayManager() : display(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, &Wire, -1) {}
    void begin()
    {
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
    void setMonitor(const NetworkMonitor *m) { monitor = m; }
    void setConnecting(const String &ssid) { trying = ssid; }
    void selecionarPagina(uint8_t p)
    {
        registrarInteracao();
        page = p % Config::PAGE_COUNT;
        automatic = false;
        consoleUntil = 0;
        rendered = millis() - 1000;
    }
    void avancarPagina() { selecionarPagina((page + 1) % Config::PAGE_COUNT); }
    void paginasAutomaticas()
    {
        registrarInteracao();
        automatic = true;
        pageAt = millis();
        consoleUntil = 0;
        rendered = millis() - 1000;
    }
    void setModoRelogioGrande(bool on) { selecionarPagina(on ? 0 : 1); }
    bool getModoRelogioGrande() const { return page == 0; }
    void clearLog()
    {
        registrarInteracao();
        for (auto &s : lines)
            s = "";
        count = 0;
        consoleUntil = millis() + 5000;
        rendered = millis() - 1000;
    }
    void adicionarLinha(const String &text)
    {
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
        ultimaInteracao = millis();
        pageAt = ultimaInteracao;
        consoleUntil = 0;
        rendered = ultimaInteracao - 1000;
    }

void atualizarTela(NTPUtil& ntpService) {
    if (!ready) return;
    uint32_t now = millis();
    const uint32_t idle = now - ultimaInteracao;
    const bool protecao = idle >= Config::OLED_TIMEOUT_MS;

    // Limita o trafego I2C para manter o loop livre para ler o botao.
    if (now - rendered < (protecao ? 250U : 1000U)) return;
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
        else if (page == 3 && monitor) {
            const auto& m = *monitor;
            line(0, !online ? String("Sem rede") : !m.hasResult ? String("Aguardando ping") : m.replied ? "RTT: " + String(m.rtt) + " ms" : String("Gateway sem resposta"));
            line(1, "Quedas: " + String(m.drops) + " Off:" + String((unsigned long)m.offlineSeconds()) + "s");
            line(2, "Falhas: " + String(m.attempts ? uint32_t(uint64_t(m.failures) * 100 / m.attempts) : 0) + "%");
            int peak = 10; for (int v : m.history) if (v > peak) peak = v;
            for (int i = 0; i < 24; i++) { int v = m.history[(m.head + i) % 24], x = 4 + i * 5; if (v == -1) { display.drawLine(x, 48, x + 3, 51, SSD1306_WHITE); display.drawLine(x + 3, 48, x, 51, SSD1306_WHITE); } else if (v >= 0) { int h = max(1, v * 15 / peak); display.drawFastVLine(x, 63 - h, h, SSD1306_WHITE); } }
        } else if (page == 4 && monitor) {
            for (int i = 0; i < 3; i++) line(i, monitor->events[i]); line(4, "Historico em RAM");
        }
    }
    display.display();
}
};
extern DisplayManager oled;
