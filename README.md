# ESP32 Wi-Fi / OLED

Firmware para ESP32 com portal local de configuração Wi-Fi, display OLED SSD1306 128x64, monitor de rede e comandos via serial/UDP.

## Destaques

- Portal web local para configurar até cinco redes Wi-Fi
- Reconexão automática e AP de fallback quando necessário
- OLED com múltiplas telas de status e diagnóstico
- Monitor ICMP do gateway com histórico em memória
- Comandos para rede, LED, telas, logs e manutenção
- Sem dependência de serviços externos no portal

## Requisitos de hardware

| Recurso | Configuração |
|---|---|
| Placa | ESP32 compatível com Arduino-ESP32 3.x |
| LED | GPIO 2 |
| Botão / BOOT | GPIO 0, ligado ao GND, com `INPUT_PULLUP` |
| OLED SDA / SCL | GPIO 21 / GPIO 22 |
| Display | SSD1306 128x64, I2C 0x3C, monocromático |
| UDP | Porta 4210 |
| Fuso padrão | UTC -3 |

## Dependências

- Arduino-ESP32 3.x
- Adafruit SSD1306
- Adafruit GFX
- suporte nativo de `ping/ping_sock.h` do ESP-IDF

## Instalação

1. Baixe ou clone o repositório.
2. Abra `wifi/wifi.ino` no Arduino IDE.
3. Mantenha a pasta `wifi` e os arquivos auxiliares juntos.
4. Selecione a placa, a porta e as opções usadas no projeto original.
5. Compile, grave e abra o Monitor Serial em 115200 baud.

## Primeiro arranque

Ao iniciar sem ligação ativa, o ESP32 cria o AP **ESP32_CONFIG** em **192.168.4.1** enquanto tenta ligar às redes guardadas.

- Aceda a `/wifi` para configurar até cinco perfis.
- Após ligação bem-sucedida, o AP é desligado.
- O OLED mostra o IP atribuído e esse IP passa a servir o portal.
- Se a ligação cair, o AP é reativado durante novas tentativas.

## Funcionalidades do portal

- `/info`: mostra rede, IP, RSSI, memória, tempo ligado e estado do monitor de conexão.
- `/wifi`: edição de cinco perfis, indicação da rede ativa, mostrar/ocultar senha e suporte a rede aberta.

Regras importantes:

- O portal funciona sem Internet.
- Não usa fontes, estilos ou scripts externos.
- Senhas salvas nunca são enviadas ao navegador.
- Se o SSID não mudar, uma senha vazia mantém a anterior.
- Marque **Rede aberta** para gravar sem senha.
- Apagar o SSID remove o perfil.

## Procura e recuperação de rede

- A varredura é assíncrona.
- Após a varredura, os perfis são testados sequencialmente.
- A última rede utilizada é tentada primeiro.
- Cada perfil pode ficar até dez segundos em tentativa.
- SSIDs ocultos também são suportados.
- Se nenhuma rede ligar, o AP permanece disponível e um novo ciclo começa após trinta segundos.

## OLED e botão

O firmware inclui cinco páginas principais:

1. relógio
2. rede Wi-Fi
3. sistema
4. ping ao gateway
5. histórico

- Atualização a cada segundo
- Rotação automática a cada oito segundos
- Renderização em buffer no OLED
- Toque curto no GPIO 0 avança a tela
- Pressão longa de cinco segundos limpa redes, cursor, fuso e DST e reinicia o equipamento

## Monitor de rede e horário

- Ping ICMP ao gateway aproximadamente a cada cinco segundos
- Timeout de um segundo em tarefa separada
- Gráfico com as últimas 24 respostas/timeouts
- RTT mínimo, médio e máximo
- Estado do NTP configurado no arranque sem bloqueio
- Fuso configurável entre -12 e 14
- DST manual com adição fixa de uma hora

## Comandos

A lista completa está em `COMANDOS.md` e também pode ser vista com o comando `help`.

## Notas

- Credenciais são ocultadas dos logs.
- Pacotes UDP acima de 255 bytes são rejeitados.
- O botão continua a ser lido mesmo durante proteção de ecrã.
- Compilação e testes em hardware real ainda devem ser validados na tua placa.

## Referências

- [Wi-Fi Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
- [ICMP Echo Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/icmp_echo.html)
