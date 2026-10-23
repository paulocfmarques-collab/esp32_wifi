#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <esp_timer.h>
#include <esp_ota_ops.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <atomic>
#include "Config.h"
#include "WeatherService.h"
#include "DisplayManager.h"

class MetricsService
{
public:
  struct Settings
  {
    uint32_t version;
    char host[254];
    uint16_t port;
    uint16_t interval;
  };

private:
  Settings settings{};
  WeatherService weather;
  struct BootRecord
  {
    uint32_t version, boots, watchdogs;
  } boots{};
  bool countersOk = false;
  String device;
  uint32_t imageBytes = 0, partitionBytes = 0;
  std::atomic<uint32_t> disconnects{0};
  std::atomic<bool> udpListening{false};
  std::atomic<uint32_t> udpReceived{0}, udpSent{0}, udpReadErrors{0}, udpSendErrors{0};
  std::atomic<uint32_t> udpBindErrors{0}, udpRebinds{0}, udpLastReceiveSeconds{0};
  uint64_t requests = 0, errors = 0;
  uint32_t httpRejected[2]{}, httpAborted[2]{};
  double responseMs = 0;
  float cpuUsage = NAN;
#if configUSE_TRACE_FACILITY && configGENERATE_RUN_TIME_STATS
  TaskStatus_t tasks[64];
  configRUN_TIME_COUNTER_TYPE previousIdle[portNUM_PROCESSORS]{};
  int64_t previousTime = 0;
  uint32_t sampledAt = 0;
  bool cpuBaseline = false;
#endif

  static String escaped(String value)
  {
    value.replace("\\", "\\\\");
    value.replace("\"", "\\\"");
    value.replace("\n", "\\n");
    value.replace("\r", "\\n");
    return value;
  }
  static String label(const char *key, const String &value)
  {
    return String(key) + "=\"" + escaped(value) + "\"";
  }
  void sample(String &out, const char *name, const char *type, const String &value,
              const String &labels = "") const
  {
    out += "# HELP " + String(name) + " ESP32: " + name + ".\n";
    out += "# TYPE " + String(name) + " " + type + "\n";
    out += String(name) + "{device=\"" + escaped(device) + "\"";
    if (labels.length())
      out += "," + labels;
    out += "} " + value + "\n";
  }

public:
  static bool validHost(const String &host)
  {
    if (host.isEmpty() || host.length() > 253)
      return false;
    unsigned size = 0;
    char previous = 0;
    for (size_t i = 0; i < host.length(); ++i)
    {
      char c = host[i];
      bool alnum = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
      if (c == '.')
      {
        if (!size || previous == '-')
          return false;
        size = 0;
      }
      else
      {
        if ((!alnum && c != '-') || (!size && c == '-') || ++size > 63)
          return false;
      }
      previous = c;
    }
    return size && previous != '-';
  }
  static bool parseNumber(const String &text, uint32_t minimum, uint32_t maximum, uint16_t &value)
  {
    if (text.isEmpty() || text.length() > 5)
      return false;
    uint32_t parsed = 0;
    for (size_t i = 0; i < text.length(); ++i)
    {
      if (text[i] < '0' || text[i] > '9')
        return false;
      parsed = parsed * 10 + text[i] - '0';
    }
    if (parsed < minimum || parsed > maximum)
      return false;
    value = static_cast<uint16_t>(parsed);
    return true;
  }
  const Settings &configuration() const { return settings; }
  bool persistenceOk() const { return countersOk; }
  void recordHttpLimits(uint32_t webRejected, uint32_t webAborted,
                        uint32_t metricsRejected, uint32_t metricsAborted) {
    httpRejected[0] = webRejected; httpAborted[0] = webAborted;
    httpRejected[1] = metricsRejected; httpAborted[1] = metricsAborted;
  }
  void setUdpListening(bool ready) { udpListening.store(ready); }
  void recordUdpBind(bool ok, bool rebind) {
    udpListening.store(ok);
    if (!ok) ++udpBindErrors;
    else if (rebind) ++udpRebinds;
  }
  void recordUdpReceive() {
    ++udpReceived;
    udpLastReceiveSeconds.store(uint32_t(esp_timer_get_time() / 1000000));
  }
  void recordUdpReadError() { ++udpReadErrors; }
  void recordUdpSend(bool ok) { if (ok) ++udpSent; else ++udpSendErrors; }
  bool resetCounters()
  {
    Preferences prefs;
    if (!prefs.begin("telemetry", false))
    {
      countersOk = false;
      return false;
    }
    BootRecord zero{1, 0, 0};
    bool ok = prefs.putBytes("boots", &zero, sizeof(zero)) == sizeof(zero);
    prefs.end();
    countersOk = ok;
    if (ok)
    {
      boots = zero;
    }
    return ok;
  }
  String bootDiagnostics() const
  {
    if (!countersOk)
      return "Contadores NVS: indisponiveis (falha de leitura/gravacao)\n";
    return "Boots registrados: " + String(boots.boots) +
           "\nReinicializacoes registradas: " + String(boots.boots ? boots.boots - 1 : 0) +
           "\nResets por watchdog (acumulados NVS): " + String(boots.watchdogs) + "\n";
  }
  bool save(const String &host, uint16_t port, uint16_t interval)
  {
    if (!validHost(host) || !port || interval < 5 || interval > 3600)
      return false;
    Settings next{};
    next.version = 1;
    host.toCharArray(next.host, sizeof(next.host));
    next.port = port;
    next.interval = interval;
    Preferences prefs;
    if (!prefs.begin("telemetry", false))
      return false;
    bool ok = prefs.putBytes("settings", &next, sizeof(next)) == sizeof(next);
    prefs.end();
    if (ok)
      settings = next;
    return ok;
  }
  void begin()
  {
    settings.version = 1;
    settings.port = 9120;
    settings.interval = 15;
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char id[18];
    snprintf(id, sizeof(id), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    device = id;
    imageBytes = ESP.getSketchSize();
    const esp_partition_t *partition = esp_ota_get_running_partition();
    partitionBytes = partition ? partition->size : 0;
    Preferences prefs;
    if (prefs.begin("telemetry", false))
    {
      Settings saved{};
      size_t settingsLength = prefs.getBytesLength("settings");
      if (settingsLength == sizeof(saved) &&
          prefs.getBytes("settings", &saved, sizeof(saved)) == sizeof(saved) &&
          saved.version == 1 && memchr(saved.host, 0, sizeof(saved.host)) &&
          validHost(String(saved.host)) && saved.port && saved.interval >= 5 && saved.interval <= 3600)
        settings = saved;
      else if (settingsLength)
        Serial.println("[METRICS] Preferencias invalidas; usando porta 9120 e intervalo 15s.");
      size_t length = prefs.getBytesLength("boots");
      bool valid = length == 0;
      if (length == sizeof(boots))
        valid = prefs.getBytes("boots", &boots, sizeof(boots)) == sizeof(boots) && boots.version == 1;
      if (valid)
      {
        boots.version = 1;
        if (boots.boots < UINT32_MAX)
          ++boots.boots;
        esp_reset_reason_t reason = esp_reset_reason();
        if ((reason == ESP_RST_WDT || reason == ESP_RST_INT_WDT || reason == ESP_RST_TASK_WDT) &&
            boots.watchdogs < UINT32_MAX)
          ++boots.watchdogs;
        countersOk = prefs.putBytes("boots", &boots, sizeof(boots)) == sizeof(boots);
      }
      prefs.end();
    }
    else
      Serial.println("[METRICS] NVS indisponivel; usando configuracao padrao.");
    if (!countersOk)
      Serial.println("[METRICS] Contadores persistentes indisponiveis.");
    weather.begin();
    WiFi.onEvent([this](WiFiEvent_t, WiFiEventInfo_t) { disconnects.fetch_add(1); },
                 ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  }
  void recordHttp(uint64_t elapsedUs, int status)
  {
    ++requests;
    if (status >= 400 || status <= 0)
      ++errors;
    responseMs = elapsedUs / 1000.0;
  }
  void update()
  {
#if configUSE_TRACE_FACILITY && configGENERATE_RUN_TIME_STATS
    if (millis() - sampledAt < 5000UL)
      return;
    sampledAt = millis();
    if (uxTaskGetNumberOfTasks() > 64)
    {
      cpuUsage = NAN;
      cpuBaseline = false;
      return;
    }
    configRUN_TIME_COUNTER_TYPE total = 0;
    UBaseType_t count = uxTaskGetSystemState(tasks, 64, &total);
    int64_t now = esp_timer_get_time();
    uint64_t idleDelta = 0;
    bool foundAll = count != 0;
    for (BaseType_t core = 0; core < portNUM_PROCESSORS; ++core)
    {
      TaskHandle_t idleTask = xTaskGetIdleTaskHandleForCore(core);
      bool found = false;
      for (UBaseType_t i = 0; i < count; ++i)
        if (tasks[i].xHandle == idleTask)
        {
          configRUN_TIME_COUNTER_TYPE idle = tasks[i].ulRunTimeCounter;
          idleDelta += static_cast<configRUN_TIME_COUNTER_TYPE>(idle - previousIdle[core]);
          previousIdle[core] = idle;
          found = true;
          break;
        }
      foundAll = foundAll && found;
    }
    // ESP-IDF runtime counters use microseconds; sum idle time across all cores.
    uint64_t capacity = (now - previousTime) * portNUM_PROCESSORS;
    cpuUsage = cpuBaseline && foundAll && capacity && idleDelta <= capacity
                   ? 100.0 * (1.0 - static_cast<double>(idleDelta) / capacity)
                   : NAN;
    previousTime = now;
    cpuBaseline = foundAll;
#endif
  }
  String exposition() const
  {
    String out;
    out.reserve(7000);
    sample(out, "esp_info", "gauge", "1", label("chip", ESP.getChipModel()) + ",model=\"ESP32-WiFi-OLED\"");
    sample(out, "esp_firmware_version", "gauge", "1",
           "version=\"2.0.0\",build=\"" + String(__DATE__) + " " + __TIME__ + "\"");
    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected)
      sample(out, "esp_ip_address", "gauge", "1", label("ip", WiFi.localIP().toString()) + ",interface=\"sta\"");
    if ((WiFi.getMode() & WIFI_AP) && WiFi.softAPIP())
      sample(out, "esp_ip_address", "gauge", "1", label("ip", WiFi.softAPIP().toString()) + ",interface=\"ap\"");
    sample(out, "esp_mac_address", "gauge", "1", label("mac", device));
    sample(out, "esp_status_online", "gauge", connected ? "1" : "0");
    sample(out, "esp_uptime_seconds", "gauge", String(esp_timer_get_time() / 1000000.0, 3));
    sample(out, "esp_last_restart_reason", "gauge", "1", label("reason", String(static_cast<int>(esp_reset_reason()))));
    if (countersOk)
    {
      sample(out, "esp_boot_count", "gauge", String(boots.boots));
      sample(out, "esp_reboot_count", "gauge", String(boots.boots ? boots.boots - 1 : 0));
      sample(out, "esp_watchdog_resets", "gauge", String(boots.watchdogs));
    }
    if (connected)
    {
      sample(out, "esp_wifi_rssi_dbm", "gauge", String(WiFi.RSSI()));
      sample(out, "esp_wifi_channel", "gauge", String(WiFi.channel()));
      int8_t power;
      if (esp_wifi_get_max_tx_power(&power) == ESP_OK)
        sample(out, "esp_wifi_tx_power", "gauge", String(power / 4.0f, 2));
    }
    sample(out, "esp_wifi_disconnect_count", "gauge", String(disconnects.load()));
    sample(out, "esp_oled_events_dropped_total", "counter", String(oled.eventosDescartados()));
    const String udpLabels = label("port", String(Config::UDP_PORT));
    sample(out, "esp_udp_listener_up", "gauge", udpListening.load() ? "1" : "0", udpLabels);
    sample(out, "esp_udp_packets_received_total", "counter", String(udpReceived.load()), udpLabels);
    sample(out, "esp_udp_replies_sent_total", "counter", String(udpSent.load()), udpLabels);
    sample(out, "esp_udp_receive_errors_total", "counter", String(udpReadErrors.load()), udpLabels);
    sample(out, "esp_udp_send_errors_total", "counter", String(udpSendErrors.load()), udpLabels);
    sample(out, "esp_udp_bind_errors_total", "counter", String(udpBindErrors.load()), udpLabels);
    sample(out, "esp_udp_rebinds_total", "counter", String(udpRebinds.load()), udpLabels);
    if (udpReceived.load())
      sample(out, "esp_udp_last_receive_uptime_seconds", "gauge", String(udpLastReceiveSeconds.load()), udpLabels);
    sample(out, "esp_cpu_frequency_mhz", "gauge", String(ESP.getCpuFreqMHz()));
    if (isfinite(cpuUsage))
      sample(out, "esp_cpu_usage_percent", "gauge", String(cpuUsage, 3));
    sample(out, "esp_task_count", "gauge", String(uxTaskGetNumberOfTasks()));
    sample(out, "esp_free_rtos_tasks", "gauge", String(uxTaskGetNumberOfTasks()));
    sample(out, "esp_heap_free_bytes", "gauge", String(ESP.getFreeHeap()));
    sample(out, "esp_heap_total_bytes", "gauge", String(ESP.getHeapSize()));
    sample(out, "esp_heap_minimum_free_bytes", "gauge", String(ESP.getMinFreeHeap()));
    sample(out, "esp_psram_free_bytes", "gauge", String(ESP.getFreePsram()));
    sample(out, "esp_psram_total_bytes", "gauge", String(ESP.getPsramSize()));
    if (partitionBytes)
      sample(out, "esp_flash_usage_percent", "gauge", String(100.0 * imageBytes / partitionBytes, 3));
    float temperature = temperatureRead();
    if (isfinite(temperature))
      sample(out, "esp_cpu_temperature_celsius", "gauge", String(temperature, 2));
    const auto climate = weather.snapshot();
    const String climateLabels = "source=\"open_meteo\",location=\"porto_alegre\"";
    const double climateAge = climate.valid ? WeatherService::ageSeconds(climate) : 0;
    const bool fresh = climate.valid && climateAge <= Config::WEATHER_MAX_AGE_S;
    sample(out, "esp_weather_up", "gauge",
           climate.lastAttemptOk && connected && fresh ? "1" : "0", climateLabels);
    if (climate.valid) {
      sample(out, "esp_weather_data_age_seconds", "gauge", String(climateAge, 1), climateLabels);
      sample(out, "esp_weather_data_timestamp_seconds", "gauge", String(climate.sourceTime, 0), climateLabels);
    }
    if (fresh) {
      sample(out, "sensor_temperature_celsius", "gauge", String(climate.temperature, 2), climateLabels);
      sample(out, "sensor_humidity_percent", "gauge", String(climate.humidity, 2), climateLabels);
    }
    sample(out, "gpio_digital_input", "gauge", String(digitalRead(Config::PIN_BOTAO_RESET) == HIGH ? 1 : 0),
           label("pin", String(Config::PIN_BOTAO_RESET)));
    sample(out, "gpio_digital_output", "gauge", String(digitalRead(Config::PIN_LED) == HIGH ? 1 : 0),
           label("pin", String(Config::PIN_LED)));
    sample(out, "application_requests_total", "counter", String(requests));
    sample(out, "application_errors_total", "counter", String(errors));
    sample(out, "application_response_time_ms", "gauge", String(responseMs, 3));
    // Uma declaracao HELP/TYPE por familia, com amostras para os dois servidores.
    out += "# HELP esp_http_rejected_requests_total Requests rejected on initial timeout or parser failure.\n"
           "# TYPE esp_http_rejected_requests_total counter\n";
    for (int i = 0; i < 2; ++i)
      out += "esp_http_rejected_requests_total{device=\"" + escaped(device) +
             "\",server=\"" + String(i == 0 ? "portal" : "metrics") + "\"} " + String(httpRejected[i]) + "\n";
    out += "# HELP esp_http_aborted_responses_total Responses aborted on timeout or send failure.\n"
           "# TYPE esp_http_aborted_responses_total counter\n";
    for (int i = 0; i < 2; ++i)
      out += "esp_http_aborted_responses_total{device=\"" + escaped(device) +
             "\",server=\"" + String(i == 0 ? "portal" : "metrics") + "\"} " + String(httpAborted[i]) + "\n";
    return out;
  }
  String scrapeConfiguration() const
  {
    String ip = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    return "scrape_configs:\n  - job_name: 'esp32_wifi'\n    scrape_interval: " +
           String(settings.interval) + "s\n    metrics_path: /metrics\n    static_configs:\n      - targets: ['" +
           ip + ":" + String(settings.port) + "']\n";
  }
};
