#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Preferences.h>
#include "esp_system.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define LED 2
#define BOTAO_RESET 0 // Pino do botão de Reset (G4 conectado ao GND)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MAX_LINHAS 8

#define PIN_SDA 21
#define PIN_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool blinkAtivo = false;
bool estadoLed = false;

unsigned long ultimoToggle = 0;
unsigned long intervaloBlink = 500; // ms
String historicoLinhas[MAX_LINHAS];
int totalLinhas = 0;
bool oledInicializado = false;

WebServer server(80);
Preferences prefs;

// HTML da página de configuração
const char* htmlPage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Configuração WiFi</title>
<style>
  body { font-family: Arial, sans-serif; margin: 40px; background-color: #f4f4f9; text-align: center; }
  .container { background: white; max-width: 300px; margin: auto; padding: 20px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
  input { width: 100%; padding: 8px; margin: 10px 0; box-sizing: border-box; }
  input[type="submit"] { background: #007bff; color: white; border: none; cursor: pointer; }
</style>
</head>
<body>
<div class="container">
  <h2>Configuração WiFi</h2>
  <form action="/salvar" method="POST">
    <label>SSID:</label>
    <input type="text" name="ssid" placeholder="Nome da rede" required>

    <label>Senha:</label>
    <input type="password" name="senha" placeholder="Senha da rede">

    <input type="submit" value="Salvar">
  </form>
</div>
</body>
</html>
)rawliteral";

WiFiUDP udp;
const int udpPort = 4210;

// Função para escrever linha por linha dinamicamente com rolagem automática
void adicionarLinha(String novoTexto) {
  Serial.println("[OLED] " + novoTexto); // Também espelha no Monitor Serial
  
  if (!oledInicializado) return; // Proteção contra ponteiro nulo se o display falhar

  // Move o histórico para cima se a tela estiver cheia
  if (totalLinhas >= MAX_LINHAS) {
    for (int i = 0; i < MAX_LINHAS - 1; i++) {
      historicoLinhas[i] = historicoLinhas[i + 1];
    }
    historicoLinhas[MAX_LINHAS - 1] = novoTexto;
  } else {
    historicoLinhas[totalLinhas] = novoTexto;
    totalLinhas++;
  }

  // Renderiza o histórico atualizado na tela
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  for (int i = 0; i < totalLinhas; i++) {
    display.setCursor(0, i * 8); 
    display.println(historicoLinhas[i]);
  }
  display.display();
}

void salvarWifi() 
{
  Serial.println("===== SALVAR WIFI =====");
  String novoSSID = server.arg("ssid");
  String novaSenha = server.arg("senha");

  prefs.begin("wifi", false);
  prefs.putString("ssid", novoSSID);
  prefs.putString("senha", novaSenha);
  prefs.end();

  Serial.println("Dados gravados");
  server.send(200, "text/html", "<h2>Configuracao salva! O ESP32 esta reiniciando...</h2>");
  delay(2000);
  ESP.restart();
}

void iniciarPortal() 
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_CONFIG");
  Serial.println("\nPortal WiFi iniciado");
  Serial.print("Conecte-se e acesse o IP: ");
  Serial.println(WiFi.softAPIP());

  adicionarLinha("\nPortal WiFi iniciado");
  adicionarLinha("Conecte-se e acesse o IP: ");
  adicionarLinha("IP: 192.168.4.1");

  server.on("/", HTTP_GET, []() {
      server.send(200, "text/html", htmlPage);
  });
  server.on("/salvar", HTTP_POST, salvarWifi);
  server.begin();
}

bool conectarWifi() 
{
  prefs.begin("wifi", true);
  String ssid = prefs.getString("ssid", "");
  String password = prefs.getString("senha", "");
  prefs.end();

  if (ssid == "") return false;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Conectando a rede: "); Serial.println(ssid);
  adicionarLinha("Conectando a:"); adicionarLinha(ssid);


  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) 
  {
    delay(500);
    Serial.print(".");
    digitalWrite(LED, !digitalRead(LED)); 
    tentativas++;
  }
  digitalWrite(LED, LOW);
  return WiFi.status() == WL_CONNECTED;
}

void zerarConfiguracoes() 
{
  Serial.println("\n===== APAGANDO CONFIGURAÇÕES DE WI-FI =====");
  adicionarLinha("Limpando Memoria...");


  prefs.begin("wifi", false);
  prefs.clear(); // Apaga o SSID e a Senha salvos
  prefs.end();
  
  Serial.println("Memoria limpa com sucesso!");
  adicionarLinha("Memoria limpa!");
  
  // Resposta final via UDP antes de reiniciar
  udp.beginPacket(udp.remoteIP(), udp.remotePort());
  udp.print("WiFi zerado. Reiniciando em modo Portal...\n");
  udp.endPacket();
  
  // Pisca o LED rápido como feedback visual
  for(int i=0; i<10; i++) {
    digitalWrite(LED, HIGH); delay(100);
    digitalWrite(LED, LOW); delay(100);
  }
  
  ESP.restart(); // Reinicia o ESP32
}

void executa_comando(String cmd)
{
  Serial.print("Comando recebido: ");
  Serial.println(cmd);
  adicionarLinha(cmd);


  if (cmd == "RESET_WIFI") // Comando de RESET_WIFI
  {
    zerarConfiguracoes(); //Chama a função que limpa a memória e reinicia
  }
  else if (cmd == "LED_ON") // Comando para ligar o LED
  {
    blinkAtivo = false;
    estadoLed = true;
    digitalWrite(LED, HIGH);

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print("LED ligado\n");
    udp.endPacket();
  }
  else if (cmd == "LED_OFF") // Comando para desligar o LED
  {
    blinkAtivo = false;
    estadoLed = false;
    digitalWrite(LED, LOW);

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print("LED desligado\n");
    udp.endPacket();
  }
  else if (cmd.startsWith("LED_PISCA")) // Comando para piscar o LED uma quantidade de vezes
  {
    int piscadas = 10;
    int tempo = 250;

    int p1 = cmd.indexOf(':');
    int p2 = cmd.indexOf(':', p1 + 1);

    if (p1 > 0 && p2 > 0) 
    {
      piscadas = cmd.substring(p1 + 1, p2).toInt();
      tempo = cmd.substring(p2 + 1).toInt();
    }

    for (int i = 0; i < piscadas; i++) 
    {
      digitalWrite(LED, HIGH);  delay(tempo);
      digitalWrite(LED, LOW);   delay(tempo);
    }

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("LED piscou %d vezes com %d ms\n", piscadas, tempo);
    udp.endPacket();
  }
  else if (cmd.startsWith("LED_BLINK")) // Comando para piscar led com tempo
  {
    int p = cmd.indexOf(':');

    if (p > 0) 
    {
      intervaloBlink = cmd.substring(p + 1).toInt();
    }

    blinkAtivo = true;

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("Blink iniciado (%lu ms)\n", intervaloBlink);
    udp.endPacket();
  }
  else if (cmd == "TEMP") // Comando ler a temperatura
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("CPU Temp: %.2f\n", temperatureRead());
    udp.endPacket();
  }
  else if (cmd == "CPU") // Informações sobre a CPU
  {
    //Serial.println(udp.remoteIP());
    //Serial.println(udp.remotePort());
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("Modelo: %s\n", ESP.getChipModel());
    udp.printf("Revisao: %d\n", ESP.getChipRevision());
    udp.printf("Nucleos: %d\n", ESP.getChipCores());
    udp.printf("CPU: %d MHz\n", ESP.getCpuFreqMHz());
    udp.printf("RAM livre: %u bytes\n", ESP.getFreeHeap());
    udp.endPacket();
  }
  else if (cmd == "RAM") // Informações sobre a RAM
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("Heap livre: %u\n", ESP.getFreeHeap());
    udp.printf("Menor heap livre: %u\n", ESP.getMinFreeHeap());
    udp.printf("Maior bloco livre: %u\n", ESP.getMaxAllocHeap());
    udp.endPacket();
  }
  else if (cmd == "FLASH") // Informações sobre a flash
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("Flash total: %u\n", ESP.getFlashChipSize());
    udp.printf("Velocidade Flash: %u\n", ESP.getFlashChipSpeed());
    udp.printf("Tamanho Sketch: %u\n", ESP.getSketchSize());
    udp.printf("Espaco livre: %u\n", ESP.getFreeSketchSpace());
    udp.endPacket();
  }
  else if (cmd == "INIT") // Motivo do reset
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("Motivo reset: %d\n", esp_reset_reason());    
    udp.endPacket();
  }
  else if (cmd == "UPTIME") // Tempo ligado
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("Uptime: %lu ms\n", millis());    
    udp.endPacket();
  }
  else if (cmd == "MAC") // MAC address
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("MAC: ");
    udp.println(WiFi.macAddress());
    udp.endPacket();
  }
  else if (cmd == "NET_INFO") // MAC address
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("IP: ");
    udp.println(WiFi.localIP());
    udp.printf("Gateway: ");
    udp.println(WiFi.gatewayIP());
    udp.printf("Mascara de rede: ");
    udp.println(WiFi.subnetMask());
    udp.printf("RSSI: %d dbm\n", WiFi.RSSI());
    udp.printf("Nome da Rede: %s\n", WiFi.SSID());
    udp.endPacket();
  }
  else // Comando invalido
  {
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print("Comando desconhecido\n");
    udp.endPacket();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(BOTAO_RESET, INPUT_PULLUP);

  // Inicializa o barramento I2C explicitando os pinos do P4
  Wire.begin(PIN_SDA, PIN_SCL); 
  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledInicializado = true;
    display.clearDisplay();
    display.display();
    adicionarLinha("OLED Pronto!");
  } else {
    Serial.println("Falha ao encontrar o Display OLED nos pinos mapeados!");
  }  

  if (conectarWifi()) 
  {
    Serial.print("\nConectado com sucesso! IP obtido: ");
    Serial.println(WiFi.localIP());
    adicionarLinha(WiFi.localIP().toString());
    server.stop();
    WiFi.softAPdisconnect(true);
    udp.begin(udpPort);
  } 
  else 
  {
    iniciarPortal();
  }
}

void loop() {
  // O servidor web só roda se o Wi-Fi NÃO estiver conectado (Modo AP)
  if (WiFi.status() != WL_CONNECTED) 
  {
    server.handleClient();
  }
  else if (digitalRead(BOTAO_RESET) == LOW)
  {
    zerarConfiguracoes();
  }
  else 
  {
    // Código UDP (só roda se estiver conectado no Wi-Fi)
    if (udp.parsePacket()) 
    {
      char packetBuffer[255];
      int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1);

      if (len > 0) 
      {
        packetBuffer[len] = '\0';
      }
      executa_comando(String(packetBuffer));
    }
  }

  // Controle do Blink assíncrono
  if (blinkAtivo) 
  {
    unsigned long agora = millis();
    if (agora - ultimoToggle >= intervaloBlink) 
    {
      ultimoToggle = agora;
      estadoLed = !estadoLed;
      digitalWrite(LED, estadoLed);
    }
  }  
}
