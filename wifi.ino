#include "Config.h"
#include "DisplayManager.h"
#include "HardwareController.h"
#include "NetworkManager.h"
#include "CommandHandler.h"
#include "NTPUtil.h"

// Instanciação corrigida usando o novo tipo estruturado
DisplayManager oled;
HardwareController hardware;
DeviceNetwork network;
NTPUtil ntp;

DeviceNetwork* DeviceNetwork::instancia = nullptr;
bool udpInicializado = false;

void setup() {
    Serial.begin(115200);
    delay(500); 
    
    hardware.begin();
    oled.begin();

    if (network.conectar()) {
        Serial.print(F("\nConectado com sucesso! IP obtido: "));
        Serial.println(WiFi.localIP());
        oled.adicionarLinha(WiFi.localIP().toString());
        
        network.pararPortal();
        network.iniciarUDP();
        udpInicializado = true;

        int fusoSalvo = network.obterFusoHorario();
        ntp.initNTP(fusoSalvo);
    } else {
        network.iniciarPortal();
    }
}

void loop() {
    hardware.atualizarBlink();

    if (!network.estaConectado()) {
        network.processarWebServer();
        udpInicializado = false;
    } 
    else if (hardware.verificarBotaoReset()) {
        network.forcarReinicializacaoComLimpeza();
    } 
    else {
        if (!udpInicializado) {
            network.iniciarUDP();
            udpInicializado = true;
        }

        String comandoRecebido;
        if (network.checarMensagensUDP(comandoRecebido)) {
            CommandHandler::executar(comandoRecebido);
        }
    }

    hardware.atualizarBlink();
    oled.atualizarTela(ntp);
}
