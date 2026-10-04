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

    static String obterMotivoReset() {
        esp_reset_reason_t reason = esp_reset_reason();
        switch (reason) {
            case ESP_RST_UNKNOWN:   return "DESCONHECIDO";
            case ESP_RST_POWERON:   return "POWER-ON (Tomada/VCC)";
            case ESP_RST_EXT:       return "PINO RESET (Botao EN)";
            case ESP_RST_SW:        return "SOFTWARE / OUTROS (Restart)";
            case ESP_RST_PANIC:     return "CRASH / PANIC (Exception)";
            case ESP_RST_INT_WDT:   return "WATCHDOG INTERNO (Core Travado)";
            case ESP_RST_TASK_WDT:  return "TASK WATCHDOG (Fila/Loop Travado)";
            case ESP_RST_WDT:       return "OUTROS WATCHDOGS";
            case ESP_RST_DEEPSLEEP: return "ACORDOU DO DEEP SLEEP";
            case ESP_RST_BROWNOUT:  return "BROWNOUT (Queda de Tensao)";
            case ESP_RST_SDIO:      return "RESET VIA SDIO";
            default:                return "CODIGO NAO MAPEADO";
        }
    }
    
    static void executar(const String& cmd) {
        Serial.print(F("Comando recebido: "));
        Serial.println(cmd);
        oled.adicionarLinha(cmd);
        String resp = "";

        if (cmd == "help") {
            resp = "===== COMANDOS ACEITOS =====\n"
                   "--- DIAGNOSTICO LOCAL ---\n"
                   "help      : Lista os comandos do sistema\n"
                   "info      : Exibe status completo do mestre\n"
                   "status    : Resumo rapido de conexao e heap\n"
                   "reason    : Exibe o motivo do ultimo reset\n"
                   "version   : Versao atual do firmware mestre\n"
                   "build     : Data e hora da compilacao\n"
                   "cpu       : Modelo, cores e frequencia\n"
                   "ram       : Heap total e heap livre atual\n"
                   "flash     : Tamanho e velocidade do chip\n"
                   "temp      : Temperatura interna da CPU C\n"
                   "mac       : Endereco MAC fisico do Wi-Fi\n"
                   "net_info  : Exibe IP, RSSI e SSID atual\n"
                   "uptime    : Tempo de atividade em segundos\n"
                   "time      : Hora calculada via NTP\n"
                   "date      : Data calculada via NTP\n"
                   "reset     : Reinicia o ESP32\n"
                   "--- CONFIGURACOES ---\n"
                   "set_fuso: : Altera GMT do NTP (Ex: set_fuso:-3)\n"
                   "reset_wifi: Limpa a Flash e abre o Portal AP\n"
                   "========================";
        }
        else if (cmd == "info") {
            String dataHoraCompleta; ntp.getDateTime(dataHoraCompleta, 100);
            String dataStr = "Sem Sinc.", horaStr = "--:--:--";
            if (dataHoraCompleta.length() >= 19) {
                dataStr = dataHoraCompleta.substring(0, 10);
                horaStr = dataHoraCompleta.substring(11, 19);
            }
            uint32_t heapLivre = ESP.getFreeHeap() / 1024;
            float flashLivre = (float)ESP.getFreeSketchSpace() / (1024.0 * 1024.0);

            resp = "===== DEVICE INFO =====\n"
                "Hostname: ESP32\n"
                "Firmware: 1.0.0\n"
                "Build: " + String(__DATE__) + " " + String(__TIME__) + "\n" +
                "SSID: " + WiFi.SSID() + "\n" +
                "IP: " + WiFi.localIP().toString() + "\n" +
                "MAC: " + WiFi.macAddress() + "\n" +
                "RSSI: " + String(WiFi.RSSI()) + " dBm\n" +
                "Heap Livre: " + String(heapLivre) + " KB\n" +
                "Flash Livre: " + String(flashLivre, 1) + " MB\n" +
                "SD Card: N/A\n" + 
                "Data: " + dataStr + "\n" +
                "Hora: " + horaStr + "\n" +
                "Uptime: " + String(millis()) + " ms\n" +
                "Reset: " + obterMotivoReset() + "\n" + 
                "=======================";
            network.responderUDP(resp + "\n");
        }
        else if (cmd == "reason") {
            resp = "===== ULTIMO RESET =====\n"
                   "Motivo: " + obterMotivoReset() + "\n"
                   "Uptime Atual: " + String(millis() / 1000) + " s\n"
                   "========================";
            network.responderUDP(resp + "\n");
        }
        else if (cmd == "version") {
            network.responderUDP("Versao Firmware: v1.0.0\n");
        }
        else if (cmd == "build") {
            network.responderUDP("Data/Hora Build: " + String(__DATE__) + " " + String(__TIME__) + "\n");
        }
        else if (cmd == "status") {
            String status = "===== STATUS =====\n";
            status += "Wi-Fi: " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado") + "\n";
            status += "IP: " + WiFi.localIP().toString() + "\n";
            status += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
            status += "Heap Livre: " + String(ESP.getFreeHeap() / Config::KB, 1) + " KB\n";
            status += "==================\n";
            network.responderUDP(status);
        }
        else if (cmd == "reset") {
            network.responderUDP("Reiniciando o ESP32...\n");
            delay(1000);
            ESP.restart();
        }
        else
        if (cmd == "reset_wifi") {
            network.forcarReinicializacaoComLimpeza();
        }
        else if (cmd.startsWith("set_fuso")) {
            int p = cmd.indexOf(':');
            if (p > 0) {
                int novoFuso = cmd.substring(p + 1).toInt();
                if (novoFuso >= -12 && novoFuso <= 14) {
                    network.salvarFusoHorario(novoFuso);
                    bool dstAtual = network.obterDstAtivo();
                    
                    // Reconfigura o relógio interno imediatamente com o novo fuso
                    ntp.configurarRelogio(novoFuso, dstAtual);
                    
                    String confirma = "Fuso alterado: GMT" + String(novoFuso >= 0 ? "+" : "") + String(novoFuso);
                    oled.adicionarLinha(confirma);
                    network.responderUDP(confirma + "\n");
                } else {
                    network.responderUDP("Erro: Fuso invalido (-12 a 14).\n");
                }
            } else {
                network.responderUDP("Use o padrao: SET_FUSO:X (Ex: SET_FUSO:-3)\n");
            }
        }
        else if (cmd == "dst_on") {
            network.salvarDstAtivo(true);
            int fusoAtual = network.obterFusoHorario();
            
            // Ativa o Horário de Verão recalculando a string POSIX
            ntp.configurarRelogio(fusoAtual, true);
            
            oled.adicionarLinha("Horario de Verao ON");
            network.responderUDP("Horario de Verao ativado com sucesso!\n");
        }
        else if (cmd == "dst_off") {
            network.salvarDstAtivo(false);
            int fusoAtual = network.obterFusoHorario();
            
            // Retorna ao horário padrão
            ntp.configurarRelogio(fusoAtual, false);
            
            oled.adicionarLinha("Horario de Verao OFF");
            network.responderUDP("Horario de Verao desativado com sucesso!\n");
        }
        else if (cmd == "time") {
            String dataHoraCompleta;
            ntp.getDateTime(dataHoraCompleta, 100);
            String hora = "--:--:--";
            if (dataHoraCompleta.length() >= 19) {
                hora = dataHoraCompleta.substring(11);
            }
            int fusoAtual = network.obterFusoHorario();
            bool dstAtivo = network.obterDstAtivo();
            String strFuso = " (GMT" + String(fusoAtual >= 0 ? "+" : "") + String(fusoAtual) + (dstAtivo ? " DST" : "") + ")";
            
            oled.adicionarLinha("Hora: " + hora);
            network.responderUDP("Hora atual: " + hora + strFuso + "\n");
        }
        else if (cmd == "date") {
            String dataHoraCompleta;
            ntp.getDateTime(dataHoraCompleta, 100);
            String data = "Erro NTP";
            if (dataHoraCompleta.length() >= 19) {
                data = dataHoraCompleta.substring(0, 10);
            }
            oled.adicionarLinha("Data: " + data);
            network.responderUDP("Data atual: " + data + "\n");
        }
        else if (cmd == "led_on") {
            hardware.setLed(true);
            network.responderUDP("LED ligado\n");
        }
        else if (cmd == "led_off") {
            hardware.setLed(false);
            network.responderUDP("LED desligado\n");
        }
        else if (cmd.startsWith("led_pisca")) {
            int piscadas = 10, tempo = 250;
            int p1 = cmd.indexOf(':'), p2 = cmd.indexOf(':', p1 + 1);
            if (p1 > 0 && p2 > 0) {
                piscadas = cmd.substring(p1 + 1, p2).toInt();
                tempo = cmd.substring(p2 + 1).toInt();
            }
            hardware.piscarSincrono(piscadas, tempo);
            network.responderUDPPrintf("LED piscou %d vezes com %d ms\n", piscadas, tempo);
        }
        else if (cmd.startsWith("led_blink")) {
            int intervalo = 500;
            int p = cmd.indexOf(':');
            if (p > 0) intervalo = cmd.substring(p + 1).toInt();
            hardware.iniciarBlinkAsync(intervalo);
            network.responderUDPPrintf("Blink iniciado (%d ms)\n", intervalo);
        }
        else if (cmd == "temp") {
            float t = hardware.lerTemperatura();
            oled.adicionarLinha("Temp: " + String(t) + "C");
            network.responderUDPPrintf("CPU Temp: %.2f\n", t);
        }
        else if (cmd == "cpu") {
            network.responderUDPPrintf("Modelo: %s\nRevisao: %d\nNucleos: %d\nCPU: %d MHz\nRAM livre: %0.2f KB\n", 
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz(), ESP.getFreeHeap() / Config::KB);
        }
        else if (cmd == "ram") {
            network.responderUDPPrintf("Heap livre: %0.2f KB\nMenor heap livre: %0.2f KB\nMaior bloco livre: %0.2f KB\n", 
                ESP.getFreeHeap() / Config::KB, ESP.getMinFreeHeap() / Config::KB, ESP.getMaxAllocHeap() / Config::KB);
        }
        else if (cmd == "flash") {
            network.responderUDPPrintf("Flash total: %0.2f MB\nVelocidade Flash: %u\nTamanho Sketch: %0.2f MB\nEspaco livre: %0.2f MB\n", 
                ESP.getFlashChipSize() / Config::MB, ESP.getFlashChipSpeed(), ESP.getSketchSize() / Config::MB, ESP.getFreeSketchSpace() / Config::MB);
        }
        else if (cmd == "uptime") {
            network.responderUDPPrintf("Uptime: %lu ms\n", millis());    
        }
        else if (cmd == "mac") {
            network.responderUDP("MAC: " + WiFi.macAddress() + "\n");
        }
        else if (cmd == "net_info") {
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
