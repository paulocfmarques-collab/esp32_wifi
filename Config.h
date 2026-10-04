#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config {
    constexpr uint8_t PIN_LED = 2;
    constexpr uint8_t PIN_BOTAO_RESET = 0;
    constexpr uint8_t PIN_SDA = 21;
    constexpr uint8_t PIN_SCL = 22;

    constexpr uint8_t SCREEN_WIDTH = 128;
    constexpr uint8_t SCREEN_HEIGHT = 64;
    constexpr uint8_t MAX_LINHAS = 7; // Limite de linhas de log na tela OLED
    
    constexpr uint16_t UDP_PORT = 4210;
    constexpr int FUSO_PADRAO = -3; // Fuso de Brasília

    constexpr const float MB = 1024.0 * 1024.0; // Constante para conversão de bytes para megabytes
    constexpr const float KB = 1024.0; // Constante para conversão de bytes para kilobytes
}

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
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
    <label>SSID:</label><input type="text" name="ssid" placeholder="Nome da rede" required>
    <label>Senha:</label><input type="password" name="senha" placeholder="Senha da rede">
    <input type="submit" value="Salvar">
  </form>
</div>
</body>
</html>
)rawliteral";

#endif
