#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "esp_system.h"
#include "DisplayManager.h"
#include "HardwareController.h"
#include "NetworkManager.h"
#include "NTPUtil.h"

class CommandHandler {
    inline static String lastCommand = "Nenhum";
    inline static uint32_t commandCount = 0;
    static bool integer(const String& text,int low,int high,int& out){
        if(text.isEmpty() || text.length()>6)return false;
        for(size_t i=0;i<text.length();i++)if(!isDigit(text[i]) && !(i==0 && text.length()>1 && (text[i]=='-' || text[i]=='+')))return false;
        long n=text.toInt();if(n<low || n>high)return false;out=int(n);return true;
    }
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
    
    static void executar(const String& incoming) {
        String cmd=incoming;
        while(cmd.endsWith("\r") || cmd.endsWith("\n"))cmd.remove(cmd.length()-1);
        int separator=cmd.indexOf(':');String nome=separator<0?cmd:cmd.substring(0,separator);
        String arg=separator<0?String(""):cmd.substring(separator+1);nome.trim();nome.toLowerCase();
        if(nome=="restart")nome="reboot";
        cmd=nome+(separator<0?String(""):String(":")+arg);
        String visible=nome=="wifi_add"?String("wifi_add:[credenciais ocultas]"):cmd;
        if(nome!="lastcmd" && nome!="cmdcount"){lastCommand=visible;commandCount++;}
        Serial.print(F("Comando recebido: "));
        Serial.println(visible);
        oled.adicionarLinha(visible);
        String resp = "";

        if (cmd == "help") {
            network.responderUDP(
                "Sistema: help info status reason version build reboot alive uptime lastcmd cmdcount health\n"
                "Hardware: cpu chip_info ram heap heap_min psram flash temp\n"
                "Rede: net_info net_monitor wifi_status wifi_list wifi_add:SSID|SENHA ssid channel mac rssi ip reset_wifi\n"
                "Hora: time date ntp_status set_fuso:-3 dst_on dst_off\n"
                "LED: led_on led_off led_blink:ms led_pisca:pulsos:ms\n"
                "OLED: tela:0..4 tela:next tela:auto clear_log\n"
                "wifi_add: cinco perfis circulares, senha vazia = rede aberta; reboot aplica.\n"
                "reset_wifi limpa redes, fuso e DST; firmware permanece. restart = reboot.\n");
        }
        else if(nome=="tela") {
            if(arg=="auto"){oled.paginasAutomaticas();network.responderUDP("Telas automaticas\n");}
            else if(arg=="next"){oled.avancarPagina();network.responderUDP("Proxima tela\n");}
            else {int page;if(integer(arg,0,4,page)){oled.selecionarPagina(page);network.responderUDP("Tela selecionada\n");}
              else network.responderUDP("Use tela:0..4, tela:next ou tela:auto\n");}
        }
        else if(cmd=="clear_log"){oled.clearLog();network.responderUDP("Log da tela limpo\n");}
        else if(cmd=="lastcmd"){network.responderUDP(lastCommand+"\n");}
        else if(cmd=="cmdcount"){network.responderUDP(String(commandCount)+"\n");}
        else if(cmd=="net_monitor"){network.responderUDP(network.monitorReport());}
        else if(cmd=="wifi_list"){network.responderUDP(network.listarSSIDs());}
        else if(nome=="wifi_add"){
            int divider=arg.indexOf('|');String error;
            if(divider<=0)network.responderUDP("Use wifi_add:SSID|SENHA\n");
            else if(network.adicionarRede(arg.substring(0,divider),arg.substring(divider+1),error))network.responderUDP("Rede salva e verificada. Use reboot para aplicar.\n");
            else network.responderUDP("Erro: "+error+"\n");
        }
        else if(cmd=="ssid"){network.responderUDP(WiFi.SSID()+"\n");}
        else if(cmd=="channel"){network.responderUDP("Canal: "+String(WiFi.channel())+"\n");}
        else if(cmd=="wifi_status"){network.responderUDP("WiFi: "+String(network.estaConectado()?"ONLINE":"OFFLINE")+"\nModo: "+String((int)WiFi.getMode())+"\nSSID: "+WiFi.SSID()+"\nIP STA: "+WiFi.localIP().toString()+"\nIP AP: "+WiFi.softAPIP().toString()+"\n");}
        else if(cmd=="ntp_status"){network.responderUDP(String("Relogio: ")+(ntp.isSincronizado()?"VALIDO":"SEM SINCRONIZACAO")+"\nFuso: "+String(network.obterFusoHorario())+"\nDST: "+String(network.obterDstAtivo()?"ON":"OFF")+"\nRelogio valido nao confirma alcance atual dos servidores NTP.\n");}
        else if(cmd=="heap_min"){network.responderUDP("Menor heap: "+String(ESP.getMinFreeHeap())+" B\n");}
        else if(cmd=="psram"){network.responderUDP("PSRAM total: "+String(ESP.getPsramSize())+" B\nLivre: "+String(ESP.getFreePsram())+" B\n");}
        else if(cmd=="chip_info"){network.responderUDPPrintf("Chip: %s\nRevisao: %u\nNucleos: %u\nSDK: %s\n",ESP.getChipModel(),(unsigned)ESP.getChipRevision(),(unsigned)ESP.getChipCores(),ESP.getSdkVersion());}
        else if(cmd=="health"){network.responderUDP(String("Diagnostico (somente leitura)\nWiFi: ")+(network.estaConectado()?"ONLINE":"OFFLINE")+"\nOLED: "+String(oled.isPronto()?"OK":"FALHA")+"\nRelogio: "+String(ntp.isSincronizado()?"VALIDO":"SEM SINC")+"\nHeap: "+String(ESP.getFreeHeap())+" B\n");}
        else if (cmd == "info") {
            String dataHoraCompleta; ntp.getDateTime(dataHoraCompleta, 100);
            String dataStr = "Sem Sinc.", horaStr = "--:--:--";
            if (dataHoraCompleta.length() == 19) {
                dataStr = dataHoraCompleta.substring(0, 10);
                horaStr = dataHoraCompleta.substring(11, 19);
            }
            uint32_t heapLivre = ESP.getFreeHeap() / 1024;
            float flashLivre = (float)ESP.getFreeSketchSpace() / (1024.0 * 1024.0);

            resp = "===== DEVICE INFO =====\n"
                "Hostname: ESP32\n"
                "Firmware: 2.0.0 / build " + obterVersaoAutomatica() + "\n"
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
            network.responderUDP("Versao Firmware: 2.0.0 / build " + obterVersaoAutomatica() + "\n");
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
        else if (nome == "set_fuso") {
            int p = cmd.indexOf(':');
            if (p > 0) {
                int novoFuso;
                if (integer(arg,-12,14,novoFuso)) {
                    if(!network.salvarFusoHorario(novoFuso)){network.responderUDP("Erro ao gravar fuso\n");return;}
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
            if(!network.salvarDstAtivo(true)){network.responderUDP("Erro ao gravar DST\n");return;}
            int fusoAtual = network.obterFusoHorario();
            
            // Ativa o Horário de Verão recalculando a string POSIX
            ntp.configurarRelogio(fusoAtual, true);
            
            oled.adicionarLinha("Horario de Verao ON");
            network.responderUDP("Horario de Verao ativado com sucesso!\n");
        }
        else if (cmd == "dst_off") {
            if(!network.salvarDstAtivo(false)){network.responderUDP("Erro ao gravar DST\n");return;}
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
            if (dataHoraCompleta.length() == 19) {
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
            if (dataHoraCompleta.length() == 19) {
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
        else if (nome == "led_pisca") {
            int count=10,interval=250;
            if(separator>=0){int split=arg.indexOf(':');
                if(split<0 || !integer(arg.substring(0,split),1,100,count) || !integer(arg.substring(split+1),50,60000,interval)){network.responderUDP("Use led_pisca:1..100:50..60000\n");return;}}
            hardware.piscarSincrono(count,interval);network.responderUDPPrintf("Sequencia iniciada: %d pulsos, %d ms\n",count,interval);
        }
        else if (nome == "led_blink") {
            int interval=500;if(separator>=0 && !integer(arg,50,60000,interval)){network.responderUDP("Intervalo: 50..60000 ms\n");return;}
            hardware.iniciarBlinkAsync(interval);network.responderUDPPrintf("Blink continuo: %d ms\n",interval);
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
            network.responderUDPPrintf("Flash: %.2f MB\nVelocidade: %.2f MHz\nSketch: %.2f MB\nEspaco disponivel para sketch: %.2f MB\n",ESP.getFlashChipSize()/Config::MB,ESP.getFlashChipSpeed()/1000000.0,ESP.getSketchSize()/Config::MB,ESP.getFreeSketchSpace()/Config::MB);
        }
        else if (cmd == "uptime") {
            network.responderUDPPrintf("Uptime: %lu s\n", (unsigned long)(millis() / 1000));    
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
