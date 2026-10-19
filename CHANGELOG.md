# Changelog

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
