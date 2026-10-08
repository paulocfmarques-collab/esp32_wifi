#pragma once
#include <Arduino.h>
#include "Config.h"
class HardwareController {
 bool blinkAtivo=false,estadoLed=false,finite=false;
 uint32_t ultimoToggle=0,intervaloBlink=500,toggles=0;
public:
 void begin(){pinMode(Config::PIN_LED,OUTPUT);pinMode(Config::PIN_BOTAO_RESET,INPUT_PULLUP);digitalWrite(Config::PIN_LED,LOW);}
 void setLed(bool on){blinkAtivo=false;finite=false;estadoLed=on;digitalWrite(Config::PIN_LED,on);}
 void alternarLed(){estadoLed=!estadoLed;digitalWrite(Config::PIN_LED,estadoLed);}
 void iniciarBlinkAsync(unsigned long interval){intervaloBlink=constrain(interval,50UL,60000UL);ultimoToggle=millis();finite=false;blinkAtivo=true;}
 // Legacy name retained; pulses now run asynchronously.
 void piscarSincrono(int pulses,int interval){
  setLed(true);intervaloBlink=constrain(interval,50,60000);ultimoToggle=millis();
  toggles=uint32_t(constrain(pulses,1,100))*2-1;finite=true;blinkAtivo=true;
 }
 bool verificarBotaoReset(){return digitalRead(Config::PIN_BOTAO_RESET)==LOW;}
 float lerTemperatura(){return temperatureRead();}
 void atualizarBlink(){
  if(!blinkAtivo)return;uint32_t now=millis();
  if(now-ultimoToggle>=intervaloBlink){ultimoToggle=now;alternarLed();
   if(finite && --toggles==0){blinkAtivo=false;finite=false;estadoLed=false;digitalWrite(Config::PIN_LED,LOW);}
  }
 }
};
extern HardwareController hardware;
