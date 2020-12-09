#include "MsgLib.h"

#define pinLed 13

MsgLib handler;

bool ledState = LOW;

void setup() {
  handler.begin();
  Serial.begin(9600);
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, HIGH);
}

void loop() {
  handler.receiveMsg();
  if(handler.msg[0] == 'o' && handler.msg[1] == 'n') turnLightOn();
  if(handler.msg[0] == 'o' && handler.msg[1] == 'f') turnLightOff();
  if(handler.msg[0] == 'l' && handler.msg[1] == 's') sendLedState();
}

void turnLightOn(){
  ledState = HIGH;
  digitalWrite(13, ledState);
  handler.clearMsg();
}

void turnLightOff(){
  ledState = LOW;
  digitalWrite(13, ledState);
  handler.clearMsg();
}

void sendLedState(){
  Serial.println(ledState);
}
