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

    static void handleRoot() {
        if (instancia) instancia->server.send(200, "text/html", htmlPage);
    }

    static void handleSalvar() {
        if (instancia) {
            Serial.println(F("===== SALVAR WIFI ====="));
            String novoSSID = instancia->server.arg("ssid");
            String novaSenha = instancia->server.arg("senha");

            instancia->prefs.begin("wifi", false);
            instancia->prefs.putString("ssid", novoSSID);
            instancia->prefs.putString("senha", novaSenha);
            instancia->prefs.end();

            Serial.println(F("Dados gravados"));
            instancia->server.send(200, "text/html", "<h2>Configuracao salva! O ESP32 esta reiniciando...</h2>");
            delay(2000);
            ESP.restart();
        }
    }

public:
    DeviceNetwork() : server(80), modoAP(false) {
        instancia = this;
    }

    bool conectar() {
        prefs.begin("wifi", true);
        String ssid = prefs.getString("ssid", "");
        String senha = prefs.getString("senha", "");
        prefs.end();

        if (ssid == "") return false;

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), senha.c_str());
        Serial.print(F("Conectando a rede: ")); Serial.println(ssid);
        oled.adicionarLinha("Conectando a:"); 
        oled.adicionarLinha(ssid);

        int tentativas = 0;
        while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
            delay(500);
            Serial.print(".");
            hardware.alternarLed();
            tentativas++;
        }
        hardware.setLed(false);

        modoAP = (WiFi.status() != WL_CONNECTED);
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

    void iniciarPortal() {
        modoAP = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ESP32_CONFIG");
        
        Serial.println(F("\nPortal WiFi iniciado"));
        Serial.print(F("Conecte-se e acesse o IP: "));
        Serial.println(WiFi.softAPIP());

        oled.adicionarLinha("Portal WiFi iniciado");
        oled.adicionarLinha("IP: 192.168.4.1");

        server.on("/", HTTP_GET, handleRoot);
        server.on("/salvar", HTTP_POST, handleSalvar);
        server.begin();
    }

    void iniciarUDP() {
        udp.begin(Config::UDP_PORT);
    }

    void pararPortal() {
        server.stop();
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
        if (modoAP) {
            server.handleClient();
        }
    }

    bool estaConectado() {
        return WiFi.status() == WL_CONNECTED;
    }

    void forcarReinicializacaoComLimpeza() {
        Serial.println(F("\n===== APAGANDO CONFIGURAÇÕES DE WI-FI ====="));
        oled.adicionarLinha("Limpando Memoria...");

        prefs.begin("wifi", false);
        prefs.clear();
        prefs.end();
        
        Serial.println(F("Memoria limpa com sucesso!"));
        oled.adicionarLinha("Memoria limpa!");
        
        if (!modoAP) {
            responderUDP("WiFi zerado. Reiniciando...\n");
        }
        
        hardware.piscarSincrono(10, 100);
        ESP.restart();
    }
};

extern DeviceNetwork network;

#endif
