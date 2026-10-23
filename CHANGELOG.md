# Changelog

## 2026-10-21 — Esperas HTTP e detalhamento do rastro

- Servidores do portal e das métricas usam adaptação local do WebServer: espera
  inicial de 1,5 s e inatividade de leitura de 250 ms. Parser e middlewares do
  core preservados, incluindo leitura de cabeçalhos/formulários fragmentados.
- Respostas normais dos handlers String enviadas por TCP não bloqueante,
  em blocos de até 1024 bytes e com pausa entre tentativas. Prazo compartilhado
  de um segundo para enviar cabeçalho e corpo; conexão encerrada em falha.
- Contadores HTTP de rejeição e respostas interrompidas por servidor, em RAM.
- Rastro RTC distingue portal, servidor de métricas, montagem, envio e manutenção
  de Wi-Fi/NVS, mantendo o significado dos códigos de etapa anteriores.
- Alterações motivadas por watchdogs repetidos com última etapa HTTP, sem comando
  anterior registrado. O rastro é indicativo, não comprovação da causa.
- Limites e testes na placa documentados. Sem compilação ou gravação, conforme
  preferência do usuário; comportamento real ainda precisa ser validado.

## 2026-10-21 — Formato parseável do help

- Padronizada a saída de `help` com o cabeçalho `COMANDOS` e um comando por
  linha, com a sintaxe dos argumentos após `:`.
- Atualizado o guia de comandos.

## 2026-10-20 — UDP independente da renderização OLED

- Substituído o mutex compartilhado do OLED por uma fila estática de 24 eventos.
  Produtores usam espera zero; apenas o loop de periféricos altera textos,
  páginas e framebuffer. Isso remove a espera da tarefa de rede pelo desenho.
- Textos de eventos limitados a 32 caracteres, com o limite visual de 21 por linha
  preservado. Consumo da fila limitado por ciclo; eventos novos podem ser descartados
  na saturação, contabilizados em `esp_oled_events_dropped_total` (RAM).
- Timeout I2C de 25 ms por transação. A proteção e a navegação continuam disponíveis.
- Corrigida a seção do rastro para `RTC_NOINIT_ATTR`; a seção anterior só garantia
  retenção no ciclo de deep sleep. A marca de etapa é volátil e diferencia agora
  monitor/ping, métricas/CPU e HTTP, para auxiliar a investigação de novos resets.
- Alterações motivadas pelo relato de UDP voltando após tocar no botão. A causa
  de todos os watchdogs ainda depende de validação em hardware.
- Documentação atualizada. Sem compilação ou gravação, por preferência do usuário.

## 2026-10-19 — Recuperação e diagnóstico do UDP

- Abertura da porta UDP 4210 verificada, com repetição após cinco segundos em falha.
- Reabertura solicitada por eventos de Wi-Fi/AP e erros detectados de socket/envio.
  Eventos apenas sinalizam a ação; a tarefa de rede executa a recuperação.
- Socket protegido por mutex de tarefa; retornos de `beginPacket`, escrita e
  `endPacket` verificados. Datagramas rejeitados são descartados sem loop de leitura.
- Atendimento de um comando UDP antes do HTTP/OTA a cada ciclo, mantendo os demais serviços.
- Métricas de abertura, recebimento, envio, erros, reaberturas e uptime do último pacote.
  Contadores UDP ficam na RAM; os de boot/watchdog na NVS não são alterados.
- Documentado o diagnóstico de HTTP ativo com UDP sem resposta. Causa do problema
  relatado ainda não confirmada em hardware. Sem compilação ou gravação.

## 2026-10-19 — Clima de Porto Alegre no Prometheus

- Adicionada consulta HTTPS ao Open-Meteo em tarefa separada, com cache sincronizado,
  intervalo de dez minutos e retry de um minuto. O scrape não faz requisição externa.
- Exportadas `sensor_temperature_celsius` e `sensor_humidity_percent` com
  `source="open_meteo",location="porto_alegre"` e o MAC no label `device`.
- Adicionadas idade, timestamp e disponibilidade dos dados climáticos. Dados
  inválidos não substituem o cache; valores com mais de uma hora são omitidos.
- Resposta HTTP limitada a 2047 bytes, validação de unidades, valores e horário;
  TLS usa certificados do core e depende do relógio sincronizado.
- Documentadas as consultas com `and on(instance, job) esp_info{model="$device"}`
  e a consulta do painel de idade. Guias e portal sincronizados.
- Sem compilação ou gravação, conforme preferência do usuário. Validação na placa
  e conectividade real com a API ainda pendentes.

## 2026-10-31 — Reset dos contadores NVS

- Adicionado `reset_counters` para zerar boot/watchdog em `telemetry` sem
  remover as configurações Prometheus nem os perfis Wi-Fi.
- Atualizados o menu `help`, o guia de comandos e o catálogo consolidado.

## 2026-10-19 — Concorrência e diagnóstico de watchdog

- Protegidos os textos e o estado do OLED por mutex de tarefa recursivo,
  incluindo os comandos que chegam pela tarefa de rede. A renderização tenta
  adquirir o mutex sem esperar e libera o acesso em todos os retornos.
- Monitor de rede disponibiliza uma cópia protegida para o OLED, evitando
  ler Strings do histórico enquanto a tarefa de rede as altera.
- `crash_info` reserva espaço para a resposta e deixa de consultar o maior
  bloco livre do heap. O total de watchdogs é identificado como acumulado na NVS.
- Rastro RTC da última etapa da tarefa de rede e nome de comando, sem argumentos,
  para auxiliar o diagnóstico após novos panics/watchdogs sem depender da Serial.
  Não recupera crashes anteriores à instalação e não comprova a causa da falha.
- Garantido ao menos um tick de pausa na tarefa de rede. O watchdog permanece ativo.
- Causa exata dos resets relatados ainda não confirmada sem log/backtrace.
  Revisão estática; compilação e testes na placa ficam com o usuário.

## Revisão documental — comando de identificação

- Documentado `name`, que retorna `ESP32`, no guia local e no catálogo geral.
- Corrigido o espaçamento da linha `help` para separar `name` e `cmdcount`.

## 2026-10-18 — Comando crash_info

- Adicionado `crash_info` ao handler e ao `help`, com código/motivo do último
  reset e classificação de panic, watchdog ou brownout.
- Reutilizados os contadores persistentes do serviço de métricas, com indicação
  explícita quando a persistência está indisponível, sem novas gravações na consulta.
- Incluídos firmware/SDK, uptime e memória do boot atual, sem apresentar esses
  valores como memória do instante da falha. Backtrace e registradores não são salvos.
- Guias atualizados. Validação de compilação e teste físico ainda pendentes.

## Revisão documental — 2026-10-18

- Sincronizados o catálogo geral e os guias de comando, mantendo o registro
  da carinha OLED incluída no commit `5c56bcf`.
- Esclarecido que a proteção automática não adiciona comandos `protetor:*`.
  Naquela revisão, `crash_info` ainda não estava implementado; foi adicionado acima.
- Referência: [histórico ESP32](../../ESP32_Documents/changelog.md).
  Não houve nova compilação ou gravação nesta revisão de documentos.

## 2026-10-18 — Proteção de tela animada

- Removidas a moldura e as laterais do robô; olhos e boca ampliados para o OLED
  128×64, exibindo uma carinha por vez.
- Dez expressões inspiradas na referência, com troca a cada quatro segundos,
  piscadas, movimento dos olhos e deslocamento leve do rosto dentro da tela.
- Animação atualizada a cada 100 ms após três minutos sem interação, mantendo
  o display ligado e sem depender da sincronização NTP.
- Botão e comandos encerram a proteção e renovam a contagem de inatividade.
  Toque curto avança a página ao soltar; pressão longa preserva a limpeza.
- Guias e lista de comandos atualizados. Compilação e validação física no OLED
  ainda pendentes; não houve gravação do firmware nesta alteração.

## 2026-10-16 — Prometheus

- Coleta direta (`pull`) em `/metrics`, no formato Prometheus 0.0.4.
- Página `/prometheus` com endereço IPv4/DNS do servidor, porta de coleta e
  intervalo persistidos juntos em Preferences/NVS, namespace `telemetry`.
- Interface web, terminal e configuração mantidos na porta 80. Métricas na porta
  salva (padrão 9120), aplicada sem reboot e restaurada no boot, inclusive no AP.
  A porta 80 pode ser compartilhada sem abrir dois listeners na mesma porta.
- Download de YAML por `/prometheus.yml` e link para a porta de métricas salva.
  O servidor Prometheus precisa receber/recarregar o YAML; não há envio ativo.
- Métricas reais de identificação, disponibilidade, Wi-Fi, CPU, tarefas, memória,
  flash, botão GPIO0, LED GPIO2 e requisições HTTP de ambos os servidores.
- Contadores persistentes de boot, reinicializações e watchdog. Desconexões e
  contadores HTTP ficam em RAM. Falhas de persistência são informadas na Serial.
- Uso de CPU por runtime das tarefas idle dos núcleos, quando suportado pelo SDK,
  sem inventar valores durante aquecimento ou amostras inválidas.
- Energia, bateria, sensores ambientais, relés, ADC e MQTT documentados como
  indisponíveis, pois o projeto não tem hardware/integração para essas séries.
- Documentação de configuração, portas, persistência, métricas, diagnóstico e
  validação em [PROMETHEUS.md](PROMETHEUS.md), com referências nos guias e comandos.

## 2.0.0

- Inventário geral de comandos sincronizado com os handlers de rede, diagnóstico,
  OLED e LED do firmware.
- Portal responsivo de informações e edição de cinco redes, com recursos locais.
- Gravação verificada, validação e tentativa de restauração em falha.
- Busca assíncrona e reconexão automática mantendo AP/UDP disponíveis.
- Cinco telas OLED e monitor ICMP com histórico em RAM.
- Botão com toque para navegação e pressão longa para limpeza, inclusive offline.
- Comandos de redes, monitor, telas e diagnóstico; help e lista completa atualizados.
- LED contínuo e pulsos sem bloqueios.
- NTP sem espera no boot e ajuste de DST manual explícito.
- Senhas ocultadas dos logs e pacotes UDP grandes rejeitados.
- Pinos e comandos anteriores preservados.
