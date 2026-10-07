# 📝 Changelog

Todas as mudanças notáveis neste projeto serão documentadas neste arquivo.

O formato segue [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/), e este projeto segue [Semantic Versioning](https://semver.org/lang/pt-BR/).

---

## [Unreleased]

### 🎨 Added
- **Documentação README atualizada** com a estrutura real dos arquivos do repositório
- **Descrição detalhada de cada módulo** com funções e responsabilidades
- **Guia visual com diagramas Mermaid** para fluxo de boot e arquitetura
- **Tabela completa de arquivos** com as funções de cada componente
- **Seção de troubleshooting expandida** com soluções práticas
- **Referências a comandos UDP** com exemplos de uso
- **Instruções de quickstart** melhoradas para facilitar onboarding

### 🔄 Changed
- Reorganização da documentação para refletir a estrutura real do projeto (sem `src/`, `include/`, `docs/`)
- Melhoria visual do README com emojis e tabelas markdown estruturadas
- Atualização do protocolo de comunicação UDP com exemplos práticos
- Revisão completa dos badges no header do README
- Aprimoramento das seções de diagnóstico e monitoramento

### ✅ Fixed
- Correção da referência aos arquivos que realmente existem no repositório
- Alinhamento da documentação com o código-fonte implementado
- Remoção de referências a estruturas de pastas inexistentes
- Atualização dos links de suporte e contribuição

---

## [1.0.0] - 2026-10-04

### 🚀 Initial Release

#### ✨ Funcionalidades principais implementadas

**Provisionamento e conectividade:**
- Sistema automático de Wi‑Fi com fallback para modo AP (Access Point)
- Portal web de configuração para primeira configuração
- Armazenamento persistente de credenciais usando `Preferences`
- Reconexão automática à rede salva
- Reset de credenciais via comando dedicado

**Comunicação e controle remoto:**
- Servidor UDP na porta 4210 para recebimento de comandos
- Protocolo simples baseado em texto
- Processamento de comandos de diagnóstico, configuração e hardware
- Resposta direta para cliente remoto via UDP

**Monitoramento do sistema:**
- Diagnóstico de CPU, RAM, flash e PSRAM
- Leitura de temperatura interna
- Monitoramento de uptime
- Informações de rede (IP, gateway, máscara, RSSI)
- Endereço MAC
- Motivo do último reset

**Sincronização de tempo:**
- NTP para sincronização automática de data e hora
- Suporte a timezone customizável
- Suporte a DST (Daylight Saving Time)
- Atualização contínua de tempo do sistema

**Interface visual:**
- Display OLED para apresentação de status e logs
- Modo relógio grande para visualização de hora/data
- Histórico de linhas de log
- LED de indicação de status

**Hardware:**
- Controle de LED físico
- Leitura de botão de reset
- Gerenciamento de estado do hardware
- Suporte a blink sincronizado

#### 📦 Arquitetura modular

O projeto foi estruturado em 8 componentes principais:

| Componente | Arquivo | Descrição |
| :--- | :--- | :--- |
| Firmware principal | `wifi.ino` | Ponto de entrada e loop de execução |
| Configuração global | `Config.h` | Constantes, portas, fuso, HTML do portal |
| Gerenciador de rede | `NetworkManager.h` | Wi‑Fi, AP, UDP, Preferences |
| Processador de comandos | `CommandHandler.h` | Parser, diagnósticos, ações do sistema |
| Gerenciador de display | `DisplayManager.h` | Renderização OLED, logs, relógio |
| Controlador de hardware | `HardwareController.h` | LED, botão, temperatura |
| Utilitário NTP | `NTPUtil.h` | Sincronização de tempo, timezone, DST |

#### 🧠 Comandos implementados (17+)

**Diagnóstico:**
- `help` - Lista de comandos disponíveis
- `info` - Status completo do dispositivo
- `status` - Resumo rápido de rede, heap e NTP
- `reason` - Motivo do último reset
- `version` - Versão do firmware
- `build` - Data e hora da compilação
- `cpu` - Informações do processador
- `ram` - Memória heap (total, livre, menor e maior bloco)
- `flash` - Informações de flash
- `temp` - Temperatura da CPU
- `mac` - Endereço MAC
- `net_info` - Informações de rede
- `time` - Hora atual
- `date` - Data atual
- `uptime` - Tempo de atividade
- `alive` - Verificação de presença
- `psram` - Status da PSRAM

**Configuração:**
- `reset_wifi` - Limpa credenciais
- `set_fuso` - Ajusta timezone
- `dst_on` - Ativa horário de verão
- `dst_off` - Desativa horário de verão

**Hardware:**
- `led_on` - Liga LED
- `led_off` - Desliga LED
- `led_pisca` - Pisca N vezes
- `led_blink` - Blink sincronizado

#### 🌐 Protocolo de comunicação

- **Porta:** 4210 (UDP)
- **Formato:** Texto simples (ASCII)
- **Resposta:** Estruturada em linhas de texto
- **Cliente:** Pode ser qualquer aplicativo UDP na rede local

#### 🎯 Recursos de hardware compatíveis

- ESP32 (qualquer variante com WiFi)
- Display OLED SSD1306 (128x64)
- LED RGB e LED simples
- Botão de reset
- NTP para sincronização

#### 🔐 Segurança e limitações

- Comunicação UDP **sem encriptação** (recomendado para redes locais)
- Portal web **sem autenticação** por padrão
- Recomendado uso em redes privadas ou com firewall/VPN
- Para produção, considere adicionar:
  - Autenticação básica HTTP
  - HTTPS
  - Validação de comandos
  - Rate limiting

#### ⚡ Performance esperada

- **Boot time:** < 3 segundos (com credenciais salvas)
- **Latência UDP:** < 50ms
- **Memória RAM:** ~60KB heap livre
- **Uso de flash:** ~350KB (~1% do total)
- **Reconexão Wi‑Fi:** < 8 segundos
- **Consumo idle:** ~50mA @ 3.3V

#### 📊 Estrutura de código

```text
esp32_wifi/
├── wifi.ino                 # 73 linhas - Ponto de entrada
├── Config.h                 # 49 linhas - Configuração global
├── NetworkManager.h         # 129 linhas - Gerenciamento de rede
├── CommandHandler.h         # ~300 linhas - Processamento de comandos
├── DisplayManager.h         # ~134 linhas - Interface OLED
├── HardwareController.h     # 44 linhas - Controle de hardware
├── NTPUtil.h                # 62 linhas - Sincronização de tempo
├── CHANGELOG.md             # Este arquivo
└── README.md                # Documentação principal
```

#### 🔗 Recursos relacionados

- [Documentação M5Stack M5NanoC6](https://docs.m5stack.com/en/core/nanoC6)
- [Referência técnica ESP32](https://www.espressif.com/en/products/socs/esp32)
- [Guia NeoPixel Adafruit](https://learn.adafruit.com/adafruit-neopixel-uberguide)

#### 🎓 Requisitos de desenvolvimento

- Arduino IDE 1.8.x+ ou PlatformIO
- ESP32 Board Support Package
- Bibliotecas padrão: WiFi, WebServer, Preferences
- Bibliotecas opcionais: Adafruit NeoPixel (para RGB)

#### 📝 Notas iniciais

- Projeto criado em ambiente de desenvolvimento com ESP32-C6
- Testado com redes Wi‑Fi 2.4 GHz
- Compatível com Arduino Framework
- Estrutura modular permite fácil integração em projetos maiores
- Sem dependências externas obrigatórias

---

## 🗺️ Roadmap futuro

- [ ] Suporte a mais tipos de display
- [ ] Integração com MQTT
- [ ] API REST adicional
- [ ] Webserver melhorado com interface moderna
- [ ] OTA updates automáticos
- [ ] Logging em memória persistente
- [ ] Dashboard web de monitoramento
- [ ] Suporte a múltiplas redes Wi‑Fi

---

## 📞 Suporte e contribuição

- **Issues:** https://github.com/paulocfmarques-collab/esp32_wifi/issues
- **Pull Requests:** https://github.com/paulocfmarques-collab/esp32_wifi/pulls
- **Discussões:** Abertas para sugestões e perguntas

---

<div align="center">

**Última atualização:** Outubro 6, 2026

Desenvolvido com ❤️ para a comunidade ESP32

</div>
