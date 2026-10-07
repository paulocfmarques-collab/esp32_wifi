#ifndef DEVICE_NETWORK_H
#define DEVICE_NETWORK_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Config.h"
#include "DisplayManager.h"
#include "HardwareController.h"

class DeviceNetwork {
private:
    WebServer server;
    WiFiUDP udp;
    Preferences prefs;
    bool modoAP;
    static DeviceNetwork* instancia;

    static const int MAX_REDES = 5;

    static String esc(String s) {
        s.replace("&", "&amp;"); s.replace("'", "&#39;"); s.replace("<", "&lt;");
        return s;
    }

    static String cabecalho(const char* titulo) {
        return String("<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'>"
               "<meta name='viewport' content='width=device-width, initial-scale=1.0'><title>") + titulo +
               "</title><style>body{font-family:Arial;margin:20px;background:#1a1a1a;color:#fff;text-align:center}"
               ".c{background:#2d2d2d;max-width:360px;margin:auto;padding:20px;border-radius:12px;text-align:left}"
               "input{width:100%;padding:9px;margin:5px 0;box-sizing:border-box;border-radius:6px;border:1px solid #444;background:#222;color:#fff}"
               "input[type=submit]{background:#0a84ff;border:none;font-weight:bold;cursor:pointer}"
               "a{color:#0a84ff}</style></head><body><div class='c'>";
    }

    static void handleConfig() {
        if (!instancia) return;
        DeviceNetwork& n = *instancia;
        n.prefs.begin("wifi", true);
        String h = cabecalho("ESP32 - Configuracao");
        h += "<h2>ESP32 WiFi</h2><p>Ate 5 redes (SSID vazio = remover; senha vazia mantem a atual)</p>"
             "<form action='/salvar' method='POST'>";
        for (int i = 0; i < MAX_REDES; i++) {
            String s = n.prefs.getString(("s" + String(i)).c_str(), "");
            h += "<b>Rede " + String(i + 1) + "</b><input name='ssid" + String(i) + "' value='" + esc(s) +
                 "' placeholder='SSID'><input type='password' name='senha" + String(i) + "' placeholder='Senha'>";
        }
        n.prefs.end();
        h += "<input type='submit' value='Salvar e Reiniciar'></form></div></body></html>";
        n.server.send(200, "text/html", h);
    }

    static void handleInfo() {
        if (!instancia) return;
        DeviceNetwork& n = *instancia;
        unsigned long s = millis() / 1000;
        char up[32];
        snprintf(up, sizeof(up), "%02luh %02lum %02lus", s / 3600, (s % 3600) / 60, s % 60);
        bool ok = WiFi.status() == WL_CONNECTED;
        String h = cabecalho("ESP32 - Informacoes");
        h += "<h2>ESP32 - Informacoes</h2>";
        h += "<p>Chip: " + String(ESP.getChipModel()) + " (" + String(ESP.getChipCores()) + " nucleos, " + String(ESP.getCpuFreqMHz()) + " MHz)</p>";
        h += "<p>Wi-Fi: " + (ok ? esc(WiFi.SSID()) : String("Desconectado")) + "</p>";
        h += "<p>IP: " + (ok ? WiFi.localIP().toString() : WiFi.softAPIP().toString()) + "</p>";
        h += "<p>RSSI: " + String(ok ? WiFi.RSSI() : 0) + " dBm</p>";
        h += "<p>MAC: " + WiFi.macAddress() + "</p>";
        h += "<p>RAM livre: " + String(ESP.getFreeHeap()) + " bytes</p>";
        h += "<p>Uptime: " + String(up) + "</p>";
        h += "<p>Redes salvas:</p><ul>";
        n.prefs.begin("wifi", true);
        for (int i = 0; i < MAX_REDES; i++) {
            String ss = n.prefs.getString(("s" + String(i)).c_str(), "");
            if (ss.length()) h += "<li>" + esc(ss) + "</li>";
        }
        n.prefs.end();
        h += "</ul><p><a href='/wifi'>Configurar redes</a></p></div></body></html>";
        n.server.send(200, "text/html", h);
    }

    static void handleRoot() {
        if (!instancia) return;
        if (instancia->modoAP) handleConfig(); else handleInfo();
    }

    static void handleSalvar() {
        if (!instancia) return;
        DeviceNetwork& n = *instancia;
        n.prefs.begin("wifi", false);
        for (int i = 0; i < MAX_REDES; i++) {
            String k = String(i);
            String ssid = n.server.arg("ssid" + k);
            String senha = n.server.arg("senha" + k);
            ssid.trim();
            if (ssid.length() == 0) {
                n.prefs.remove(("s" + k).c_str());
                n.prefs.remove(("p" + k).c_str());
            } else {
                if (senha.length() == 0 && n.prefs.getString(("s" + k).c_str(), "") == ssid)
                    senha = n.prefs.getString(("p" + k).c_str(), "");
                n.prefs.putString(("s" + k).c_str(), ssid);
                n.prefs.putString(("p" + k).c_str(), senha);
            }
        }
        n.prefs.remove("ssid");
        n.prefs.remove("senha");
        n.prefs.putInt("idx", -1);
        n.prefs.end();
        n.server.send(200, "text/html", "<h2>Configuracao salva! O ESP32 esta reiniciando...</h2>");
        delay(2000);
        ESP.restart();
    }

    void configurarRotas() {
        server.on("/", HTTP_GET, handleRoot);
        server.on("/info", HTTP_GET, handleInfo);
        server.on("/wifi", HTTP_GET, handleConfig);
        server.on("/salvar", HTTP_POST, handleSalvar);
    }

    bool tentarRede(const String& ssid, const String& senha) {
        WiFi.disconnect(true);
        delay(100);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), senha.c_str());
        Serial.print(F("Conectando a rede: ")); Serial.println(ssid);
        oled.adicionarLinha("Conectando a:");
        oled.adicionarLinha(ssid);
        for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
            delay(500);
            Serial.print(".");
            hardware.alternarLed();
        }
        hardware.setLed(false);
        return WiFi.status() == WL_CONNECTED;
    }

public:
    DeviceNetwork() : server(80), modoAP(false) {
        instancia = this;
    }

    bool conectar() {
        prefs.begin("wifi", false);
        String antigo = prefs.getString("ssid", "");
        if (antigo.length() && prefs.getString("s0", "").length() == 0) {
            prefs.putString("s0", antigo);
            prefs.putString("p0", prefs.getString("senha", ""));
        }
        String ssids[MAX_REDES], senhas[MAX_REDES];
        int total = 0;
        for (int i = 0; i < MAX_REDES; i++) {
            ssids[i] = prefs.getString(("s" + String(i)).c_str(), "");
            senhas[i] = prefs.getString(("p" + String(i)).c_str(), "");
            if (ssids[i].length()) total++;
        }
        int ultimo = prefs.getInt("idx", -1);
        prefs.end();

        modoAP = true;
        if (total == 0) return false;

        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        oled.adicionarLinha("Buscando redes...");
        int n = WiFi.scanNetworks();
        for (int k = 1; k <= MAX_REDES && WiFi.status() != WL_CONNECTED; k++) {
            int i = (ultimo + k) % MAX_REDES;
            if (ssids[i].length() == 0) continue;
            bool visivel = false;
            for (int r = 0; r < n; r++) if (WiFi.SSID(r) == ssids[i]) { visivel = true; break; }
            if (!visivel) continue;
            if (tentarRede(ssids[i], senhas[i])) {
                prefs.begin("wifi", false);
                prefs.putInt("idx", i);
                prefs.end();
            }
        }
        WiFi.scanDelete();

        modoAP = (WiFi.status() != WL_CONNECTED);
        if (!modoAP) {
            configurarRotas();
            server.begin();
        }
        return !modoAP;
    }

    int obterFusoHorario() {
        prefs.begin("wifi", true);
        int fuso = prefs.getInt("fuso", Config::FUSO_PADRAO);
        prefs.end();
        return fuso;
    }

    void salvarFusoHorario(int novoFuso) {
        prefs.begin("wifi", false);
        prefs.putInt("fuso", novoFuso);
        prefs.end();
    }

    bool obterDstAtivo() {
        prefs.begin("wifi", true);
        bool dst = prefs.getBool("dst", false);
        prefs.end();
        return dst;
    }

    void salvarDstAtivo(bool ativo) {
        prefs.begin("wifi", false);
        prefs.putBool("dst", ativo);
        prefs.end();
    }

    void iniciarPortal() {
        modoAP = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ESP32_CONFIG");
        
        configurarRotas();
        server.begin();
    }

    void iniciarUDP() {
        udp.begin(Config::UDP_PORT);
    }

    void pararPortal() {
        WiFi.softAPdisconnect(true);
    }

    void responderUDP(const String& resposta) {
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.print(resposta);
        udp.endPacket();
    }

    void responderUDPPrintf(const char* formato, ...) {
        char buffer[256];
        va_list args;
        va_start(args, formato);
        vsnprintf(buffer, sizeof(buffer), formato, args);
        va_end(args);

        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.print(buffer);
        udp.endPacket();
    }

    bool checarMensagensUDP(String& msgOut) {
        if (modoAP) return false;

        int packetSize = udp.parsePacket();
        if (packetSize) {
            char buffer[256];
            int len = udp.read(buffer, sizeof(buffer) - 1);
            if (len > 0) {
                buffer[len] = '\0';
                msgOut = String(buffer);
                return true;
            }
        }
        return false;
    }

    void processarWebServer() {
        server.handleClient();
    }

    bool estaConectado() {
        return WiFi.status() == WL_CONNECTED;
    }

    void forcarReinicializacaoComLimpeza() {
        prefs.begin("wifi", false);
        prefs.clear();
        prefs.end();
        hardware.piscarSincrono(10, 100);
        ESP.restart();
    }
};

extern DeviceNetwork network;

#endif
