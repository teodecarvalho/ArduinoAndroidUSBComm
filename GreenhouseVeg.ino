/*
 * Code by Teotonio Soares de Carvalho
 * Brazil
 * November 7, 2020
 * Lincensed under MIT license
 * 
 * This code is intended to control irrigation and lighting in a greenhouse using an arduino.
 * 
 * Instructions for setting parameters from serial:
 * <s1XXX> Sets drySoil to XXX
 * <s2XXX> Sets wetSoil to XXX
 * <s3XXX> Sets minIrrigInterval to XXX
 * <s4XXX> Sets startLight to XXX
 * <s5XXX> Sets stopLight to XXX
 * <s6XXX> Sets lowTemp to XXX
 * <s7XXX> Sets highTemp to XXX
 * <s8XXX> Sets minSoilMoist to XXX
 * <s9XXX> Sets irrigTime to XXX
 * <sa> Makes all changes permanent in the EEPROM
 * 
 * 
 * minSoilMoist (int): low soil moisture (0-100) that should trigger irrigation
 * minIrrigInterval (float): time in hours to wait between soil moisture measurements
 * lastIrrig (float): time (hours) when last irrigation occurred
 * irrigTime (float): time in hours the solenoid should be turned on for irrigation
 * drySoil (int): output of soil moisture sensor when dry
 * wetSoil (int): output of soil moisture sensor when immersed in water
 * startLight (float): hour of the day when lights should be turned on
 * stopLight (float): hour of the day when lights should be turned off
 * lowTemp (int): temperature at which cooler should be turned off
 * highTemp (int): temperature at which half of the lamps should be turned off
*/

#include <Wire.h>
#include "RTClib.h" // By Adafruit version 1.12.1
#include <DHT.h> // By Adafruit version 1.3.88
#include <EEPROM.h> 

#define pinLight1 3
#define pinLight2 4
#define pinSolenoid 5
#define pinCooler 6
#define pinMoistSens A0
#define pinTempSens A1
#define DHTTYPE DHT11

///// Parameters
int drySoil; // Address 0
int wetSoil; // Address 2
float minIrrigInterval; // Address 4
float startLight; // Address 8
float stopLight; // Address 12
int lowTemp; // Address 16
int highTemp; // Address 18
int minSoilMoist; // Address 20
float irrigTime; // Address 22

// Variables for handling incoming messages
const int msgSize = 20;
char msg[msgSize];

bool solenoidStatus = HIGH; // Unfortunately, the relay module has this reversed. So HIGH means disabled.

DHT dht(pinTempSens, DHTTYPE);
RTC_DS3231 rtc;
float lastIrrig = 0;
int currentSoilMoist = 100;
float currentTemp;
float currentAirHumid;

DateTime now;

void setup() {
  Serial.begin(9600);
  dht.begin();
  setupRTC();
  
  getEEPROM();
  
  pinMode(pinLight1, OUTPUT);
  pinMode(pinLight2, OUTPUT);
  pinMode(pinSolenoid, OUTPUT);
  pinMode(pinCooler, OUTPUT);
  digitalWrite(pinLight1, HIGH);
  digitalWrite(pinLight2, HIGH);
  digitalWrite(pinSolenoid, HIGH);
  digitalWrite(pinCooler, HIGH);
  now = rtc.now();
  Serial.println("Ready");
}

void loop() {
  updateNow();
  updateTemp();
  updateAirHumid();
  checkIrrig();
  checkLight();
  checkTemp();

  // Manage incoming commands
  if(Serial.available()) receiveMsg();
  if(msg[0] == 's' && msg[1] == '1') s1(); // Update drySoil
  if(msg[0] == 's' && msg[1] == '2') s2(); // Update wetSoil
  if(msg[0] == 's' && msg[1] == '3') s3(); // Update minIrrigInterval
  if(msg[0] == 's' && msg[1] == '4') s4(); // Update startLight
  if(msg[0] == 's' && msg[1] == '5') s5(); // Update stopLight
  if(msg[0] == 's' && msg[1] == '6') s6(); // Update lowTemp
  if(msg[0] == 's' && msg[1] == '7') s7(); // Update highTemp
  if(msg[0] == 's' && msg[1] == '8') s8(); // Update minSoilMoist
  if(msg[0] == 's' && msg[1] == '9') s9(); // Update irrigTime
  if(msg[0] == 's' && msg[1] == 'a') updateEEPROM(); // Update EEPROM
  if(msg[0] == 's' && msg[1] == 'p') showParameters(); // Show all parameters
  if(msg[0] == 's' && msg[1] == 'r') showReadings(); // Show all sensor readings
  if(msg[0] == 'p' && msg[1] == 'p') showParametersPython(); // Show all sensor readings in a format readable by the python script
}

void checkLight(){
  // currentTime is expressed in hours
  float currentTime = float(now.hour()) + float(now.minute()) / 60 + float(now.second()) / 3600;

  ///// Checking if lights should be on
  // Are we above the time when lights should start?
  if(currentTime >= startLight){ // If so:
    // Are we before the time when lights should be off?
    if(currentTime <= stopLight){ // If so, enable light set 1:
      digitalWrite(pinLight1, LOW); // LOW means enabled for this relay module
      // Then check if temperature is above the maximum
      if(currentTemp >= highTemp){ //If so, disable light set 2:
        digitalWrite(pinLight2, HIGH); // HIGH is disabled for this relay module
      } else if(currentTemp < highTemp){ // Otherwise, if temperature is below the maximum, enable light2:
        digitalWrite(pinLight2, LOW); // LOW is enabled for this relay module
      }
      
    // If we are after the time to stop lights, disable lights. 
    } else if(currentTime > stopLight) {
      digitalWrite(pinLight1, HIGH); // HIGH is disabled for this relay module
      digitalWrite(pinLight2, HIGH);
    }
  // If we are before the time to start the lights, disable lights
  } else if (currentTime < startLight) {
    digitalWrite(pinLight1, HIGH); // HIGH is disabled for this relay module
    digitalWrite(pinLight2, HIGH);
  }
}

void checkTemp(){
  // Is temperature above lowTemp?
  if(currentTemp > lowTemp){ // If so, enable the cooler.
    digitalWrite(pinCooler, LOW); // LOW is enabled for this relay module
    // Is temperature above highTemp?
    if(currentTemp > highTemp){ // If so, disable light set 2:
      digitalWrite(pinLight2, HIGH); // HIGH is disabled for this relay module
    }
  // Is temperature below lowTemp?  
  } else if(currentTemp <= lowTemp){ // If so, turn cooler off.
    digitalWrite(pinCooler, HIGH); // HIGH is disabled for this relay module
  }
}

void checkIrrig(){
  // currentTime is expressed in hours
  float currentTime = float(now.hour()) + float(now.minute()) / 60 + float(now.second()) / 3600;

  ///// Measuring soil moisture
  int sensorRead = analogRead(pinMoistSens);
  currentSoilMoist = map(sensorRead, wetSoil, drySoil, 100, 0);  
  
  ///// Checking if irrigation is needed:
  // Is soil moisture low? If not, do nothing. Otherwise:
  if(currentSoilMoist < minSoilMoist){
    // Are we not irrigating now? If yes, do nothing. Otherwise:
    if(solenoidStatus){ // HIGH means we are not irrigating
      // Is the minimum time between irrigations gone? If not, do nothing. Otherwise:
      if((currentTime - lastIrrig) > minIrrigInterval){
        // Update last irrigation tracking.
        lastIrrig = currentTime;
        // Start irrigation
        solenoidStatus = LOW;
        digitalWrite(pinSolenoid, LOW); // This starts the solenoid
      }
    }
  }
  
  ///// Checking if we should stop irrigation
  // Is irrigation happening now? If not, do nothing. Otherwise:
  if(!solenoidStatus){ // LOW means irrigations is taking place.
    // Did we reach the irrigation time? If not, do nothing. Otherwise:
    if((currentTime - lastIrrig) > irrigTime){
      // Stop Irrigation
      solenoidStatus = HIGH;
      digitalWrite(pinSolenoid, solenoidStatus);
      // Update last irrigation tracking.
      lastIrrig = currentTime;
    }
  }
}


void updateNow(){
  now = rtc.now();
}

void updateTemp(){
  currentTemp = dht.readTemperature();
}

void updateAirHumid(){
  currentAirHumid = dht.readHumidity();
}

void s1(){
  drySoil = atoi(&msg[2]);
}

void s2(){
  wetSoil = atoi(&msg[2]);
}

void s3(){
  minIrrigInterval = atof(&msg[2]);
}

void s4(){
  startLight = atof(&msg[2]);
}

void s5(){
  stopLight = atof(&msg[2]);
}

void s6(){
  lowTemp = atoi(&msg[2]);
}

void s7(){
  highTemp = atoi(&msg[2]);
}

void s8(){
  minSoilMoist = atoi(&msg[2]);
}

void s9(){
  irrigTime = atof(&msg[2]);
}

void setupRTC(){
  delay(3000); // wait for console opening
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void showReadings(){
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
    Serial.print("Current soil moisture:");
    Serial.println(currentSoilMoist);
    Serial.print("Raw soil moisture reading: ");
    Serial.println(analogRead(pinMoistSens));
    Serial.print("Current temperature: ");
    Serial.println(currentTemp);
    Serial.print("Current Air Humidity: ");
    Serial.println(currentAirHumid);
    clearMsg();
}

void receiveMsg(){
  int index = 0;
  bool invalid = LOW;
  // Clear msg variable
  for(int i = 0; i < (msgSize - 1); i++) msg[i] = '\0';
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
  if(index >= msgSize) for(int i = 0; i < (msgSize - 1); i++) msg[i] = '\0';
  Serial.println(msg);
}

void clearMsg(){
  for(int i = 0; i < (msgSize - 1); i++) msg[i] = '\0';
}

void updateEEPROM(){
  EEPROM.put(0, drySoil);
  EEPROM.put(2, wetSoil);
  EEPROM.put(4, minIrrigInterval);
  EEPROM.put(8, startLight);
  EEPROM.put(12, stopLight);
  EEPROM.put(16, lowTemp);
  EEPROM.put(18, highTemp);
  EEPROM.put(20, minSoilMoist);
  EEPROM.put(22, irrigTime);
  clearMsg();
}

void getEEPROM(){
  EEPROM.get(0, drySoil);
  EEPROM.get(2, wetSoil);
  EEPROM.get(4, minIrrigInterval);
  EEPROM.get(8, startLight);
  EEPROM.get(12, stopLight);
  EEPROM.get(16, lowTemp);
  EEPROM.get(18, highTemp);
  EEPROM.get(20, minSoilMoist);
  EEPROM.get(22, irrigTime); 
}

void showParameters(){
  Serial.print("Dry soil readings on the sensor: ");
  Serial.println(drySoil);
  Serial.print("Wet soil readings on the sensor: ");
  Serial.println(wetSoil);
  Serial.print("Minimum time between irrigations: ");
  Serial.println(minIrrigInterval, 4);
  Serial.print("Time to start lights: ");
  Serial.println(startLight, 4);
  Serial.print("Time to stop lights: ");
  Serial.println(stopLight, 4);
  Serial.print("Low temperature: ");
  Serial.println(lowTemp);
  Serial.print("High temperature: ");
  Serial.println(highTemp);
  Serial.print("Minimal soil moisture (%): ");
  Serial.println(minSoilMoist);
  Serial.print("Irrigation time: ");
  Serial.println(irrigTime, 4);
  clearMsg();
}

void showParametersPython(){
  Serial.print("drysoil:");
  Serial.println(drySoil);
  Serial.print("wetsoil:");
  Serial.println(wetSoil);
  Serial.print("minirriginterval:");
  Serial.println(minIrrigInterval, 4);
  Serial.print("startlight:");
  Serial.println(startLight, 4);
  Serial.print("stoplight:");
  Serial.println(stopLight, 4);
  Serial.print("lowtemp:");
  Serial.println(lowTemp);
  Serial.print("hightemp:");
  Serial.println(highTemp);
  Serial.print("minsoilmoist:");
  Serial.println(minSoilMoist);
  Serial.print("irrigtime:");
  Serial.println(irrigTime, 4);
  clearMsg();
}