# 🚀 ESP32 Wi-Fi Provisioning & Remote Control

Sistema completo para ESP32 com configuração Wi-Fi via portal web, armazenamento persistente de credenciais, controle remoto UDP, monitoramento de hardware e gerenciamento remoto do dispositivo. Desenvolvido para facilitar a implantação de dispositivos IoT sem necessidade de recompilar o firmware para alterar configurações de rede. 【1-12ffc9】

---

## ✨ Recursos

✅ Configuração Wi-Fi através de Portal Web

✅ Armazenamento permanente de SSID e senha utilizando Preferences (NVS)

✅ Reconexão automática após reinicialização

✅ Servidor UDP para controle remoto

✅ Reset de configurações através de botão físico

✅ Reset remoto por comando UDP

✅ Controle do LED onboard

✅ Monitoramento da CPU do ESP32

✅ Informações de memória RAM

✅ Informações da memória Flash

✅ Leitura de temperatura interna

✅ Consulta do MAC Address

✅ Informações completas de rede

✅ Consulta do motivo do último reset

✅ Monitoramento de uptime do dispositivo

---

# 🏗️ Arquitetura

```mermaid
flowchart TB

subgraph Usuario
A[Navegador]
B[Cliente UDP]
end

subgraph ESP32
C[Portal Web]
D[Preferences]
E[WiFi]
F[Servidor UDP]
G[Controle LED]
H[Monitoramento Sistema]
end

A --> C
C --> D

D --> E

B --> F

F --> G
F --> H

H --> B
G --> B
```

---

# 📦 Hardware Utilizado

| Item | Descrição |
|--------|--------|
| ESP32 | Controlador principal |
| LED GPIO 2 | Sinalização visual |
| Botão GPIO 0 | Reset das configurações |
| Wi-Fi 2.4 GHz | Comunicação de rede |

---

# 📚 Bibliotecas Utilizadas

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <WiFiUdp.h>
#include "esp_system.h"
```

【1-12ffc9】

---

# 🔄 Fluxo Geral do Sistema

```mermaid
flowchart TD

A[Inicialização ESP32] --> B{Existem credenciais Wi-Fi?}

B -->|Não| C[Inicia AP ESP32_CONFIG]
C --> D[Usuário conecta ao AP]
D --> E[Acessa Portal Web]
E --> F[Configura SSID e Senha]
F --> G[Salva em Preferences]
G --> H[Reinicia ESP32]

B -->|Sim| I[Tenta conectar ao Wi-Fi]
I --> J{Conectou?}

J -->|Sim| K[Inicia Servidor UDP]
J -->|Não| C

K --> L[Aguardando Comandos]
```

---

# 🌐 Processo de Configuração Wi-Fi

Quando nenhuma credencial estiver armazenada:

1. O ESP32 cria o Access Point:

```text
ESP32_CONFIG
```

2. O usuário conecta-se à rede.

3. Acessa a página de configuração.

4. Informa:

```text
SSID
Senha
```

5. As informações são armazenadas na memória NVS utilizando Preferences.

6. O dispositivo reinicia automaticamente. 【1-12ffc9】

---

## Diagrama de Provisionamento

```mermaid
sequenceDiagram

participant U as Usuário
participant AP as ESP32 AP
participant WEB as Portal Web
participant NVS as Preferences

U->>AP: Conecta ao AP
U->>WEB: Acessa página web
WEB-->>U: Exibe formulário
U->>WEB: Envia SSID/Senha
WEB->>NVS: Salva credenciais
NVS-->>WEB: Confirmação
WEB->>AP: Reinicia dispositivo
```

---

# 📡 Comunicação UDP

Após conectar ao Wi-Fi, o ESP32 inicia um servidor UDP.

Porta padrão:

```text
4210
```

【1-12ffc9】

---

# 🎮 Comandos Disponíveis

## LED_ON

Liga o LED.

```text
LED_ON
```

---

## LED_OFF

Desliga o LED.

```text
LED_OFF
```

---

## LED_PISCA

Pisca o LED um número determinado de vezes.

```text
LED_PISCA:10:250
```

Onde:

```text
10 = quantidade de piscadas
250 = tempo em ms
```

---

## LED_BLINK

Pisca continuamente.

```text
LED_BLINK:500
```

Onde:

```text
500 = intervalo em ms
```

---

## TEMP

Leitura da temperatura interna.

```text
TEMP
```

---

## CPU

Retorna informações da CPU.

```text
CPU
```

Informações:

- Modelo
- Revisão
- Núcleos
- Frequência
- Heap livre

---

## RAM

Informações da memória RAM.

```text
RAM
```

Retorna:

- Heap livre
- Menor heap disponível
- Maior bloco alocável

---

## FLASH

Informações da memória Flash.

```text
FLASH
```

Retorna:

- Tamanho total
- Velocidade da Flash
- Tamanho do firmware
- Espaço livre

---

## INIT

Motivo do último reset.

```text
INIT
```

---

## UPTIME

Tempo de funcionamento.

```text
UPTIME
```

---

## MAC

MAC Address da interface Wi-Fi.

```text
MAC
```

---

## NET_INFO

Informações completas da rede.

```text
NET_INFO
```

Retorna:

- IP
- Gateway
- Máscara
- RSSI
- SSID

---

## RESET_WIFI

Remove todas as configurações Wi-Fi.

```text
RESET_WIFI
```

【1-12ffc9】

---

# 📊 Fluxo dos Comandos UDP

```mermaid
flowchart LR

A[Cliente UDP] --> B[ESP32:4210]

B --> C{Comando}

C --> D[LED_ON]
C --> E[LED_OFF]
C --> F[LED_PISCA]
C --> G[LED_BLINK]
C --> H[TEMP]
C --> I[CPU]
C --> J[RAM]
C --> K[FLASH]
C --> L[INIT]
C --> M[UPTIME]
C --> N[MAC]
C --> O[NET_INFO]
C --> P[RESET_WIFI]

D --> R[Resposta UDP]
E --> R
F --> R
G --> R
H --> R
I --> R
J --> R
K --> R
L --> R
M --> R
N --> R
O --> R
P --> R
```

---

# 🔄 Reset das Configurações

O firmware permite apagar as credenciais armazenadas de duas formas.

## Método 1 - Botão Físico

Pressione o botão conectado ao:

```text
GPIO 0
```

## Método 2 - Comando UDP

```text
RESET_WIFI
```

Após executar:

1. Limpa a memória Preferences.
2. Remove SSID e senha.
3. Pisca o LED como confirmação.
4. Reinicia o dispositivo.
5. Retorna ao modo Portal de Configuração. 【1-12ffc9】

---

## Diagrama de Reset

```mermaid
flowchart TD

A[Botão GPIO0 ou RESET_WIFI]
--> B[Limpa Preferences]

B --> C[Remove SSID]
C --> D[Remove Senha]

D --> E[Pisca LED]
E --> F[Reinicia ESP32]

F --> G[Inicia AP ESP32_CONFIG]
```

---

# 🧠 Máquina de Estados do Firmware

```mermaid
stateDiagram-v2

[*] --> Boot

Boot --> Portal : Sem WiFi Salvo
Boot --> Conectando : WiFi Salvo

Portal --> Reiniciar : Salvar Configuração
Reiniciar --> Boot

Conectando --> Operacional : Conectado
Conectando --> Portal : Falha na Conexão

Operacional --> ResetWiFi : GPIO0
Operacional --> ResetWiFi : RESET_WIFI

ResetWiFi --> Reiniciar
```


