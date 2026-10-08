#include "Config.h"
#include "DisplayManager.h"
#include "HardwareController.h"
#include "NetworkManager.h"
#include "CommandHandler.h"
#include "NTPUtil.h"
DisplayManager oled;
HardwareController hardware;
DeviceNetwork network;
NTPUtil ntp;
void setup(){
 Serial.begin(115200);hardware.begin();oled.begin();network.begin();
 ntp.configurarRelogio(network.obterFusoHorario(),network.obterDstAtivo());
}
void loop(){
 hardware.atualizarBlink();network.processarWebServer();
 String command;if(network.checarMensagensUDP(command))CommandHandler::executar(command);
 static int prior=HIGH,stable=HIGH;static uint32_t changed=0,pressed=0;static bool armed=false;
 int reading=digitalRead(Config::PIN_BOTAO_RESET);uint32_t now=millis();
 if(reading!=prior)changed=now;
 if(now-changed>=50 && reading!=stable){stable=reading;
  if(stable==LOW){pressed=now;armed=false;}
  else if(armed || now-pressed>=5000)network.forcarReinicializacaoComLimpeza();
  else oled.avancarPagina();
 }
 prior=reading;
 if(stable==LOW && !armed && now-pressed>=5000){armed=true;oled.adicionarLinha("Solte para limpar");}
 oled.atualizarTela(ntp);
}
