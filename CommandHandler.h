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

    static String obterVersaoAutomatica() {
        // Extração matemática da Data (AAMMDD)
        int ano = ((__DATE__[9] - '0') * 10) + (__DATE__[10] - '0');
        
        int mes = (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n') ? 1 :
                  (__DATE__[0] == 'F')                                             ? 2 :
                  (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r') ? 3 :
                  (__DATE__[0] == 'A' && __DATE__[1] == 'p')                       ? 4 :
                  (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y') ? 5 :
                  (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n') ? 6 :
                  (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l') ? 7 :
                  (__DATE__[0] == 'A' && __DATE__[1] == 'u')                       ? 8 :
                  (__DATE__[0] == 'S')                                             ? 9 :
                  (__DATE__[0] == 'O')                                             ? 10 :
                  (__DATE__[0] == 'N')                                             ? 11 :
                  (__DATE__[0] == 'D')                                             ? 12 : 0;
                  
        int dia = (__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0');

        // Extração matemática do Horário (HHMM)
        int hora   = ((__TIME__[0] - '0') * 10) + (__TIME__[1] - '0');
        int minuto = ((__TIME__[3] - '0') * 10) + (__TIME__[4] - '0');

        // Monta a string de forma segura usando buffers de formatação estáveis
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%02d%02d%02d.%02d.%02d", ano, mes, dia, hora, minuto);
        
        return String(buffer);
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
                   "reboot    : Reinicia o ESP32\n"
                   "heap      : Heap livre em KB\n"
                   "rssi / ip : Sinal e IP do Wi-Fi\n"
                   "alive     : Teste de presenca\n"
                   "--- LED ---\n"
                   "led_on / led_off\n"
                   "led_blink:[ms] / led_pisca:[n]:[ms]\n"
                   "--- CONFIGURACOES ---\n"
                   "set_fuso: : Altera GMT do NTP (Ex: set_fuso:-3)\n"
                   "reset_wifi: Limpa a Flash e abre o Portal AP\n"
                   "========================";
            network.responderUDP(resp + "\n");
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
                "Firmware: " + obterVersaoAutomatica() + "\n"
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
            network.responderUDP("Versao Firmware: " + obterVersaoAutomatica() + "\n");
        }
        else if (cmd == "build") {
            network.responderUDP("Build\nData: " + String(__DATE__) + "\nHora Build: " + String(__TIME__) + "\n");
        }
        else if (cmd == "status") {
            String status = "===== STATUS =====\n";
            status += "Wi-Fi: " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado") + "\n";
            status += "SD: N/A\n";
            status += "NTP: " + String(ntp.isSincronizado() ? "Sincronizado" : "Não sincronizado") + "\n";
            status += "Heap Livre: " + String(ESP.getFreeHeap() / Config::KB, 1) + " KB\n";
            status += "==================\n";
            network.responderUDP(status);
        }
        else if (cmd == "reboot") {
            network.responderUDP("Reiniciando o ESP32...\n");
            delay(1000);
            ESP.restart();
        }
        else if (cmd == "reset_wifi") {
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
            network.responderUDPPrintf("Modelo: %s\nRevisao: %d\nNucleos: %d\nCPU: %d MHz\n", 
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz());
        }
        else if (cmd == "ram") {
            network.responderUDPPrintf("Heap Total: %0.2f KB\nHeap livre: %0.2f KB\nMenor bloco livre: %0.2f KB\nMaior bloco livre: %0.2f KB\nRAM utilizada: %0.1f %%\n", 
                ESP.getHeapSize() / Config::KB, ESP.getFreeHeap() / Config::KB, ESP.getMinFreeHeap() / Config::KB, ESP.getMaxAllocHeap() / Config::KB, 
                (1.0 - ((float)ESP.getFreeHeap() / (float)ESP.getHeapSize())) * 100.0);
        }
        else if (cmd == "flash") {
            network.responderUDPPrintf("Flash total: %0.2f MB\nVelocidade Flash: %0.2f MHz\nFlash mode: %u\nSketch Size: %0.2f MB\nEspaço livre Sketch: %0.2f MB\nFlash livre: %0.1f %%\n", ESP.getFlashChipSize() / Config::MB, ESP.getFlashChipSpeed() / 1000000.0, ESP.getFlashChipMode(), ESP.getSketchSize() / Config::MB, ESP.getFreeSketchSpace() / Config::MB, (1.0 - ((float)ESP.getFreeSketchSpace() / (float)ESP.getSketchSize())) * 100.0);
        }
        else if (cmd == "uptime") {
            network.responderUDPPrintf("Uptime: %d s\n", millis() / 1000);    
        }
        else if (cmd == "mac") {
            network.responderUDP("MAC: " + WiFi.macAddress() + "\n");
        }
        else if (cmd == "net_info") {
            network.responderUDPPrintf("SSID: %s\nIP: %s\nGateway: %s\nSubnet Mask: %s\nDNS1: %s\nDNS2: %s\nRSSI: %d dbm\n", 
                WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(), 
                WiFi.subnetMask().toString().c_str(), WiFi.dnsIP(0).toString().c_str(), WiFi.dnsIP(1).toString().c_str(), WiFi.RSSI());
        }
        else if (cmd == "alive") 
        {
            network.responderUDPPrintf("%s - yes\n", WiFi.localIP().toString().c_str());
        }
        else if (cmd == "heap") {
            network.responderUDPPrintf("Heap livre: %0.2f KB\n", ESP.getFreeHeap() / Config::KB);
        }
        else if (cmd == "rssi") {
            network.responderUDPPrintf("RSSI: %d dBm\n", WiFi.RSSI());
        }
        else if (cmd == "ip") {
            network.responderUDP("IP: " + WiFi.localIP().toString() + "\n");
        }
        else {
            network.responderUDP("Comando desconhecido\n");
        }
    }
};

#endif
