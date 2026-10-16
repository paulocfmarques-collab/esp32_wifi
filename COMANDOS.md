# Comandos — ESP32 Wi-Fi / OLED 2.0.0

Envie por **UDP 4210** ao IP STA ou ao IP AP **192.168.4.1**. Limite de entrada:
**255 bytes**. Os nomes são normalizados para minúsculas; os argumentos preservam
caixa e espaços. Comandos que alteram dados executam diretamente no dispositivo.

## Prometheus e interface web

A configuração Prometheus é feita exclusivamente pela página
`http://IP_DO_ESP32/prometheus`, na **porta 80**; não foram adicionados comandos
UDP de configuração.

| Rota | Porta | Função |
|---|---|---|
| `GET /prometheus` | 80 | Formulário com endereço do servidor, porta e intervalo salvos. |
| `POST /prometheus` | 80 | Salva `host`, `port`, `interval` em Preferences/NVS. |
| `GET /prometheus.yml` | 80 | Download da configuração de coleta. |
| `GET /metrics` | Configurada, padrão 9120 | Métricas reais em formato Prometheus 0.0.4. |

O Prometheus coleta os dados no ESP32 (**pull**). O endereço do servidor é
referência persistente; não habilita envio ativo. A porta salva é a de métricas
do ESP32, não a de acesso ao servidor Prometheus. Painel e terminal permanecem
na porta 80, sem redirecionamento, inclusive no AP. Se a coleta usar 80, ela
compartilha o servidor web.

Porta: 1..65535; intervalo: 5..3600 segundos. A alteração de porta é aplicada em
cerca de um segundo, sem reiniciar. O intervalo só é aplicado à coleta quando o
YAML é instalado/recarregado no servidor. Entrada inválida retorna HTTP 400;
falha de gravação retorna 503, preservando a configuração ativa.

`reset_wifi` e a limpeza pelo botão não apagam as configurações Prometheus nem
os contadores de boot/watchdog, armazenados no namespace NVS `telemetry`.
Inventário de métricas e limitações em [PROMETHEUS.md](PROMETHEUS.md).

## Sistema

| Comando | Função |
|---|---|
| `help` | Lista todos os comandos. |
| `info` | Informações completas do dispositivo. |
| `status` | Resumo de Wi-Fi, relógio e heap. |
| `reason` | Motivo do último reset. |
| `version` | Versão 2.0.0 e identificação da compilação. |
| `build` | Data e hora da compilação. |
| `reboot` | Responde e reinicia. |
| `alive` | Teste de resposta do dispositivo. |
| `uptime` | Tempo ligado em segundos. |
| `lastcmd` | Último comando, com credenciais ocultas. |
| `cmdcount` | Quantidade de comandos; exclui lastcmd/cmdcount. |
| `health` | Diagnóstico somente leitura: Wi-Fi, OLED, relógio e heap. |

## Hardware

| Comando | Função |
|---|---|
| `cpu` | Modelo, revisão, núcleos e frequência. |
| `chip_info` | Modelo, revisão, núcleos e SDK. |
| `ram` | Heap total/livre/mínimo, maior bloco e uso. |
| `heap` | Heap livre em KB. |
| `heap_min` | Menor heap livre desde o boot, em bytes. |
| `psram` | PSRAM total e livre; zero se indisponível. |
| `flash` | Flash total, velocidade e espaço do sketch. |
| `temp` | Temperatura interna do chip; não é temperatura ambiente. |

## Rede

| Comando | Função |
|---|---|
| `net_info` | SSID, IP, gateway, máscara, DNS e RSSI. |
| `net_monitor` | Ping atual/mínimo/médio/máximo, idade, quedas, tempo offline e eventos. |
| `wifi_status` | Estado Wi-Fi, modo, SSID e IP STA/AP. |
| `wifi_list` | Somente SSIDs salvos, um por linha; vazio responde uma linha vazia. |
| `wifi_add:SSID\|SENHA` | Adiciona/atualiza perfil circular. Use reboot para aplicar. |
| `ssid` | SSID conectado. |
| `channel` | Canal Wi-Fi. |
| `mac` | MAC Wi-Fi. |
| `rssi` | Sinal Wi-Fi em dBm. |
| `ip` | IP da interface STA. |
| `reset_wifi` | Limpa redes, cursor, fuso e DST, e reinicia. Não apaga firmware. |

## Horário

| Comando | Função |
|---|---|
| `time` | Hora local, fuso e DST. |
| `date` | Data local. |
| `ntp_status` | Relógio válido, fuso e DST; não testa alcance atual dos servidores. |
| `set_fuso:N` | Inteiro UTC de -12 a +14. Exemplo: set_fuso:-3. |
| `dst_on` | Ativa e salva avanço manual de uma hora. |
| `dst_off` | Desativa e salva DST. |

## LED

| Comando | Função |
|---|---|
| `led_on` | Acende e cancela pisca. |
| `led_off` | Apaga e cancela pisca. |
| `led_blink[:ms]` | Pisca continuamente; intervalo 50..60000 ms, padrão 500. Não bloqueia. |
| `led_pisca[:pulsos:ms]` | Sequência de 1..100 pulsos; intervalo 50..60000 ms. Padrão 10 pulsos/250 ms. Não bloqueia. |

## OLED

| Comando | Função |
|---|---|
| `tela:0` | Relógio e data. |
| `tela:1` | Wi-Fi, rede em tentativa, IP, RSSI e canal. |
| `tela:2` | CPU, heap, flash e tempo ligado. |
| `tela:3` | Ping ao gateway e histórico de 24 medições. |
| `tela:4` | Três últimos eventos de conexão. |
| `tela:next` | Avança tela e desativa rotação automática. |
| `tela:auto` | Retoma rotação a cada oito segundos. |
| `clear_log` | Limpa somente o histórico mostrado no OLED. |

## Exemplos

```text
wifi_add:MinhaRede|MinhaSenha123
wifi_add:RedeAberta|
wifi_list
reboot
net_monitor
tela:3
tela:auto
led_pisca:5:250
led_off
set_fuso:-3
```

`restart` é alias de `reboot`.

## Redes circulares

Até cinco perfis. Novos SSIDs usam o próximo slot (inicia no perfil 1) e substituem
circularmente após o quinto. Repetir um SSID atualiza a senha sem avançar o cursor.
O cursor é independente da última rede conectada. O formulário HTTP edita os cinco
perfis diretamente e mantém o cursor. Alterações por `wifi_add` são aplicadas à
busca automática após `reboot`; o formulário HTTP reinicia após salvar.

SSID: 1..32 bytes; não pode conter `|`. Senha: 8..63 bytes, 64 caracteres hexadecimais
ou vazia para rede aberta. A senha pode conter `|`; só o primeiro separador é usado.
Espaços da senha são preservados, mas CR/LF finais do pacote são removidos.
Credenciais não aparecem nos logs nem em `lastcmd`.

## Telas e botão

Telas atualizam uma vez por segundo e alternam automaticamente a cada oito segundos.
Comandos/eventos podem exibir o console por cinco segundos. Selecionar uma tela
cancela esse intervalo para exibi-la na próxima atualização.

GPIO 0 / BOOT: toque curto e solte para avançar; segure cinco segundos e solte
para limpar redes, fuso e DST e reiniciar. Funciona também sem Wi-Fi.
Segurar BOOT durante reset/alimentação pode entrar no modo de gravação da placa.
O EN continua sendo reset físico.

## Diagnósticos

Ping ICMP ao gateway: uma medição aproximadamente a cada cinco segundos, timeout
um segundo, em tarefa separada. O gráfico usa barras para RTT e X para timeout;
a escala é adaptativa. Um timeout pode significar bloqueio de ICMP e não comprova
queda de Wi-Fi ou ausência de Internet. O histórico e as estatísticas ficam na RAM,
são observados pelo loop e reiniciam no boot.
`health` consulta estados disponíveis; não executa teste físico do hardware.
`ntp_status` consulta a validade do relógio, sem disparar um teste NTP.
