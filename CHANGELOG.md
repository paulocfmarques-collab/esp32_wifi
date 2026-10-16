# Changelog

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
