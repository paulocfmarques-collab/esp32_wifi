#include <ArduinoOTA.h>
#include "Config.h"
#include "DisplayManager.h"
#include "HardwareController.h"
#include "NetworkManager.h"
#include "CommandHandler.h"
#include "NTPUtil.h"


void injetarComandoDoWebTerminal(const String& cmd) {
    CommandHandler::executar(cmd);
}

DisplayManager oled;
HardwareController hardware;
DeviceNetwork network;
NTPUtil ntp;

// Handle da Task que rodará no Core 0
TaskHandle_t TaskRedeHandle = NULL;

// Função que será executada exclusivamente no Core 0
void TaskRede(void *pvParameters) {
    Serial.print("Task de Rede inicializada no core: ");
    Serial.println(xPortGetCoreID());

    // Inicializa o OTA
    ArduinoOTA.setHostname(Config::OTA_HOSTNAME);
    if (strlen(Config::OTA_PASSWORD) > 0) {
        ArduinoOTA.setPassword(Config::OTA_PASSWORD);
    }

    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("Iniciando update remoto via OTA: " + type);
        oled.clearLog();
        oled.adicionarLinha("Atualizando...");
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nUpdate Concluido com sucesso!");
        oled.adicionarLinha("Sucesso! Reboot...");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int porcentagem = progress / (total / 100);
        char buf[20];
        snprintf(buf, sizeof(buf), "Progresso: %d%%", porcentagem);
        oled.adicionarLinha(buf);
        Serial.printf("Progresso: %u%%\r", porcentagem);
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Erro [%u]: ", error);
        if (error == OTA_AUTH_ERROR) oled.adicionarLinha("Erro: Autenticacao");
        else if (error == OTA_BEGIN_ERROR) oled.adicionarLinha("Erro: Inicio");
        else if (error == OTA_CONNECT_ERROR) oled.adicionarLinha("Erro: Conexao");
        else if (error == OTA_RECEIVE_ERROR) oled.adicionarLinha("Erro: Recepcao");
        else if (error == OTA_END_ERROR) oled.adicionarLinha("Erro: Fim");
    });

    ArduinoOTA.begin();

    // Loop infinito interno do Core 0
    for (;;) {
        network.processarWebServer();
        ArduinoOTA.handle();
        
        String command;
        if (network.checarMensagensUDP(command)) {
            CommandHandler::executar(command);
        }
        
        // Pequeno delay para impedir que a Task trave o Watchdog do Core 0
        vTaskDelay(pdMS_TO_TICKS(5)); 
    }
}

void setup()
{
    Serial.begin(115200);
    hardware.begin();
    oled.begin();
    network.begin();
    ntp.configurarRelogio(network.obterFusoHorario(), network.obterDstAtivo());

    // Cria a tarefa assíncrona no Core 0
    // Parâmetros: Função, Nome da Task, Tamanho da Stack, Parâmetros, Prioridade, Handle, Core ID (0)
    xTaskCreatePinnedToCore(
        TaskRede,
        "TaskRede",
        8192,
        NULL,
        1,
        &TaskRedeHandle,
        0
    );
}

// O loop padrão roda nativamente no Core 1
void loop()
{
    // Core 1 cuida estritamente de periféricos, botões e renderização visual
    hardware.atualizarBlink();
    
    static int prior = HIGH, stable = HIGH;
    static uint32_t changed = 0, pressed = 0;
    static bool armed = false;
    int reading = digitalRead(Config::PIN_BOTAO_RESET);
    uint32_t now = millis();
    
    if (reading != prior)
        changed = now;
        
    if (now - changed >= 50 && reading != stable)
    {
        stable = reading;
        if (stable == LOW)
        {
            pressed = now;
            armed = false;
        }
        else if (armed || now - pressed >= 5000)
            network.forcarReinicializacaoComLimpeza();
        else
            oled.avancarPagina();
    }
    prior = reading;
    
    if (stable == LOW && !armed && now - pressed >= 5000)
    {
        armed = true;
        oled.adicionarLinha("Solte para limpar");
    }
    
    oled.atualizarTela(ntp);
    
    // Deixa o Core 1 respirar por 1ms para o sistema operacional do ESP32
    delay(1); 
}
