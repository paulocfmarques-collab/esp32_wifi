#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "esp_system.h"
#include "DisplayManager.h"
#include "HardwareController.h"
#include "NetworkManager.h"
#include "NTPUtil.h"

class CommandHandler {
public:
    static void executar(const String& cmd) {
        Serial.print(F("Comando recebido: "));
        Serial.println(cmd);
        oled.adicionarLinha(cmd);

        if (cmd == "RESET_WIFI") {
            network.forcarReinicializacaoComLimpeza();
        }
        else if (cmd.startsWith("TIME")) {
            int p = cmd.indexOf(':');
            if (p > 0) {
                int novoFuso = cmd.substring(p + 1).toInt();
                if (novoFuso >= -12 && novoFuso <= 14) {
                    network.salvarFusoHorario(novoFuso);
                    ntp.atualizarFusoHorario(novoFuso);
                    
                    String confirma = "Fuso: GMT" + String(novoFuso >= 0 ? "+" : "") + String(novoFuso);
                    oled.adicionarLinha(confirma);
                    network.responderUDP(confirma + "\n");
                } else {
                    network.responderUDP("Erro: Fuso invalido (-12 a 14).\n");
                }
            } else {
                String dataHoraCompleta;
                ntp.getDateTime(dataHoraCompleta, 100);
                String hora = "--:--:--";
                if (dataHoraCompleta.length() >= 19) {
                    hora = dataHoraCompleta.substring(11);
                }
                int fusoAtual = network.obterFusoHorario();
                String strFuso = " (GMT" + String(fusoAtual >= 0 ? "+" : "") + String(fusoAtual) + ")";
                
                oled.adicionarLinha("Hora: " + hora);
                network.responderUDP("Hora atual: " + hora + strFuso + "\n");
            }
        }
        else if (cmd == "DATE") {
            String dataHoraCompleta;
            ntp.getDateTime(dataHoraCompleta, 100);
            String data = "Erro NTP";
            if (dataHoraCompleta.length() >= 19) {
                data = dataHoraCompleta.substring(0, 10);
            }
            oled.adicionarLinha("Data: " + data);
            network.responderUDP("Data atual: " + data + "\n");
        }
        else if (cmd == "LED_ON") {
            hardware.setLed(true);
            network.responderUDP("LED ligado\n");
        }
        else if (cmd == "LED_OFF") {
            hardware.setLed(false);
            network.responderUDP("LED desligado\n");
        }
        else if (cmd.startsWith("LED_PISCA")) {
            int piscadas = 10, tempo = 250;
            int p1 = cmd.indexOf(':'), p2 = cmd.indexOf(':', p1 + 1);
            if (p1 > 0 && p2 > 0) {
                piscadas = cmd.substring(p1 + 1, p2).toInt();
                tempo = cmd.substring(p2 + 1).toInt();
            }
            hardware.piscarSincrono(piscadas, tempo);
            network.responderUDPPrintf("LED piscou %d vezes com %d ms\n", piscadas, tempo);
        }
        else if (cmd.startsWith("LED_BLINK")) {
            int intervalo = 500;
            int p = cmd.indexOf(':');
            if (p > 0) intervalo = cmd.substring(p + 1).toInt();
            hardware.iniciarBlinkAsync(intervalo);
            network.responderUDPPrintf("Blink iniciado (%d ms)\n", intervalo);
        }
        else if (cmd == "TEMP") {
            float t = hardware.lerTemperatura();
            oled.adicionarLinha("Temp: " + String(t) + "C");
            network.responderUDPPrintf("CPU Temp: %.2f\n", t);
        }
        else if (cmd == "CPU") {
            network.responderUDPPrintf("Modelo: %s\nRevisao: %d\nNucleos: %d\nCPU: %d MHz\nRAM livre: %u bytes\n", 
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz(), ESP.getFreeHeap());
        }
        else if (cmd == "RAM") {
            network.responderUDPPrintf("Heap livre: %u\nMenor heap livre: %u\nMaior bloco livre: %u\n", 
                ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
        }
        else if (cmd == "FLASH") {
            network.responderUDPPrintf("Flash total: %u\nVelocidade Flash: %u\nTamanho Sketch: %u\nEspaco livre: %u\n", 
                ESP.getFlashChipSize(), ESP.getFlashChipSpeed(), ESP.getSketchSize(), ESP.getFreeSketchSpace());
        }
        else if (cmd == "INIT") {
            network.responderUDPPrintf("Motivo reset: %d\n", esp_reset_reason());    
        }
        else if (cmd == "UPTIME") {
            network.responderUDPPrintf("Uptime: %lu ms\n", millis());    
        }
        else if (cmd == "MAC") {
            network.responderUDP("MAC: " + WiFi.macAddress() + "\n");
        }
        else if (cmd == "NET_INFO") {
            network.responderUDPPrintf("IP: %s\nGateway: %s\nMascara: %s\nRSSI: %d dbm\nSSID: %s\n", 
                WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(), 
                WiFi.subnetMask().toString().c_str(), WiFi.RSSI(), WiFi.SSID().c_str());
        }
        else {
            network.responderUDP("Comando desconhecido\n");
        }
    }
};

#endif
