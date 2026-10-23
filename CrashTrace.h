#pragma once
#include <Arduino.h>
#include <esp_system.h>

struct CrashBreadcrumb
{
    uint32_t magic;
    volatile uint32_t stage;
    uint32_t uptimeMs;
    char command[32];
};
extern CrashBreadcrumb crashBreadcrumb;

// Apenas TaskRede escreve depois do setup. Sem NVS, ponteiros ou alocacoes.
class CrashTrace
{
    inline static CrashBreadcrumb previous{};
    inline static bool previousValid = false;
public:
    enum Stage : uint32_t { Idle = 1, WebMetrics, OTA, UDP, Command, CrashMemory, CrashReply,
                           MonitorUpdate, CpuMetrics, Http, PortalHttp, MetricsHttp,
                           MetricsBuild, MetricsSend, NetworkMaintenance };
    static void begin()
    {
        const auto reason = esp_reset_reason();
        previousValid = crashBreadcrumb.magic == 0x43525331 &&
                        crashBreadcrumb.stage >= Idle && crashBreadcrumb.stage <= NetworkMaintenance &&
                        (reason == ESP_RST_PANIC || reason == ESP_RST_INT_WDT ||
                         reason == ESP_RST_TASK_WDT || reason == ESP_RST_WDT);
        if (previousValid) {
            previous = crashBreadcrumb;
            previous.command[sizeof(previous.command) - 1] = '\0';
        }
        crashBreadcrumb = {};
        crashBreadcrumb.magic = 0x43525331;
        mark(Idle);
    }
    static void mark(Stage stage)
    {
        crashBreadcrumb.uptimeMs = millis();
        crashBreadcrumb.stage = stage;
    }
    static void command(const String &name)
    {
        // Somente o nome; argumentos podem conter senhas.
        name.toCharArray(crashBreadcrumb.command, sizeof(crashBreadcrumb.command));
        mark(Command);
    }
    static String report()
    {
        if (!previousValid)
            return "Rastro RTC anterior: indisponivel neste boot\n";
        const char *stages[] = {"", "Espera da rede", "Web / metricas / monitor",
                               "OTA", "Leitura UDP", "Executando comando",
                               "crash_info: memoria", "crash_info: resposta UDP",
                               "Monitor / ping", "Metricas / CPU", "Atendimento HTTP",
                               "Portal HTTP", "Servidor HTTP de metricas", "Montagem das metricas",
                               "Envio HTTP das metricas", "Manutencao WiFi / NVS"};
        return "Etapa antes do reset (RTC): " + String(stages[previous.stage]) +
               "\nUptime na etapa: " + String(previous.uptimeMs / 1000) + " s\n" +
               "Ultimo nome de comando: " + String(previous.command) +
               "\nRastro indica a etapa observada, nao comprova a causa.\n";
    }
};
