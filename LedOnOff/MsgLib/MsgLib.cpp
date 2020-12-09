#include "MsgLib.h"
MsgLib::MsgLib(int msgSize){
  _msgSize = msgSize;
}

void MsgLib::begin(int baudRate){
  Serial.begin(baudRate);
}

void MsgLib::receiveMsg(){
  int index = 0;
  bool invalid = LOW;
  // Clear msg variable
  clearMsg();
  // Check if first character is '<'
  if(Serial.read() != '<') return;
  delay(2);
  while(Serial.available()){
    msg[index] = Serial.read();
    // Check if message is finished, if so replace '>' by '\0'
    if(msg[index] == '>') msg[index] = '\0';
    index++;
    delay(2);
  }
  // Clear message if longer than msgSize
  if(index >= _msgSize) clearMsg();
  Serial.println(msg);
}

void MsgLib::clearMsg(){
  for(int i = 0; i < (_msgSize - 1); i++) msg[i] = '\0';
}
