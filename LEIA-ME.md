# ESP32 Wi-Fi / OLED — versão 2.0.0

Atualização do projeto Wi-Fi com OLED SSD1306 128×64. Inclui portal moderno, cinco
redes persistentes, reconexão em segundo plano, monitor de rede, cinco telas e
comandos padronizados. Os pinos do projeto original foram mantidos.

## Hardware e dependências

| Recurso | Configuração |
|---|---|
| LED | GPIO 2 |
| Botão / BOOT | GPIO 0, ligado ao GND, INPUT_PULLUP |
| OLED SDA / SCL | GPIO 21 / GPIO 22 |
| Display | SSD1306 128×64, I2C 0x3C, monocromático |
| UDP | Porta 4210 |
| Fuso padrão | UTC -3 |

Use a mesma seleção de placa ESP32 da aplicação original e Arduino-ESP32 3.x,
Adafruit SSD1306 e Adafruit GFX. O ping usa `ping/ping_sock.h` nativo do ESP-IDF.
Não foram adicionados periféricos Ethernet ou cartão SD.

## Instalação

1. Extraia o ZIP mantendo a pasta `wifi` e seus arquivos juntos.
2. Abra `wifi/wifi.ino` no Arduino IDE.
3. Selecione sua placa, porta e as opções utilizadas no projeto original.
4. Compile e grave.
5. Abra a Serial a 115200 baud para acompanhar a inicialização.

O arquivo `.cpp` do monitor também deve ficar junto do sketch.

## Primeiro acesso e portal

Ao iniciar sem conexão, o ESP32 disponibiliza o AP **ESP32_CONFIG**, com endereço
**192.168.4.1**, enquanto procura as redes salvas. Abra `/wifi` para cadastrar os
cinco perfis. Após conectar, o AP é desligado e o OLED mostra o IP obtido; acesse
esse IP para usar o portal. Uma queda reabre o AP durante as novas tentativas.

- `/info`: rede, IP, RSSI, memória, tempo ligado e monitor de conexão. Atualiza
  a cada dez segundos.
- `/wifi`: cinco perfis, indicação da rede conectada, mostrar/ocultar senha e
  seleção explícita de rede aberta.

O portal funciona sem Internet e não busca fontes, estilos ou scripts externos.
Senhas salvas nunca são enviadas ao navegador. Senha vazia mantém a anterior
apenas quando o SSID não muda; marque Rede aberta para gravar sem senha.
Apagar o SSID remove o perfil.

O servidor valida todos os perfis, verifica a gravação por leitura e só confirma
sucesso antes de agendar o reboot. Em falha, tenta restaurar os dados anteriores
e informa o resultado. A gravação de vários campos não é atômica contra falta de
energia. Credenciais legadas são migradas para o perfil 1 quando possível.

## Busca e recuperação de rede

A varredura é assíncrona. Após a varredura, os perfis são tentados sequencialmente,
começando pela última rede utilizada, até dez segundos por perfil. Todos os perfis
salvos são tentados, incluindo SSIDs ocultos. Se nenhum conecta, o AP permanece
disponível e um novo ciclo começa após trinta segundos. O loop continua atendendo
portal, comandos, LED, botão e OLED durante as tentativas. As tentativas mostram o
SSID no console do OLED e na página de rede.

`wifi_add` salva perfis na lista circular; consulte `COMANDOS.md` para detalhes.
O UDP também funciona no AP. Nomes de comandos não diferenciam maiúsculas e
minúsculas; argumentos preservam caixa. Pacotes acima de 255 bytes são rejeitados.

## Telas e botão

Cinco páginas: relógio, rede Wi-Fi, sistema, ping ao gateway e histórico.
Atualização de um segundo, rotação de oito segundos e desenho em buffer do OLED.
O display é monocromático: barras mudam de quantidade, não de cor.

Toque curto em GPIO 0 avança a tela. Segure cinco segundos e solte para limpar
redes, cursor, fuso e DST e reiniciar, preservando o firmware. O loop continua
funcionando enquanto o botão estiver pressionado. BOOT pressionado durante reset
ou energização pode acionar o modo de gravação da placa; EN continua sendo reset.

`tela:0..4`, `tela:next`, `tela:auto` e `clear_log` controlam a exibição.

## Monitor de rede e horário

Ping ICMP ao gateway a cada aproximadamente cinco segundos, com timeout de um
segundo em tarefa separada. O gráfico mostra as últimas 24 respostas/timeouts.
O relatório inclui RTT mínimo/médio/máximo, idade da medição, quedas, tempo offline
e três eventos recentes em minutos desde o boot. Timeouts não entram na média.
Tudo fica na RAM e reinicia com a placa. Ping não testa Internet; roteadores podem
bloquear ICMP mesmo com Wi-Fi funcionando.

NTP é configurado no boot sem esperar a sincronização, usando os parâmetros salvos.
Fuso inteiro -12..14, com DST manual fixo de uma hora. Sem sincronização, telas
mostram estado de espera. Hora válida não comprova alcance atual dos servidores.

## Comandos e validação

Lista completa em `COMANDOS.md`, também disponível pelo comando `help`.
LED contínuo e sequências finitas executam sem bloquear; `led_on`/`led_off`
cancelam qualquer sequência. Credenciais são ocultadas dos logs.

Foram realizadas verificações estáticas de estrutura, includes locais, integração,
preservação dos comandos, documentação e ZIP. Compilação e testes na placa ainda
precisam ser feitos. Verifique o primeiro boot, salvamento dos perfis, reconexão,
AP/UDP, todas as telas, LED, botão e NTP com sua placa.

Referências: [Wi-Fi Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
e [ICMP Echo Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/icmp_echo.html).
