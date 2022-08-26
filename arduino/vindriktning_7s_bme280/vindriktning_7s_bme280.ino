/*
  Copyright 2022 ERR <e@richiardone.eu>, Marco Martin <notmart@gmail.com>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details
  
  You should have received a copy of the GNU Library General Public
  License along with this program; if not, write to the
  Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  Add following libraries:
    WiFiMQTTManager Library 1.0.1-beta
    WiFiManager 2.0.11-beta
    PubSubClient 2.8.0
    BME280 3.0.0 by Tyler Glenn
    
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <BME280I2C.h>

#include "display7s.h"
#include "Tokenizer.h"
#include "WifiMQTTManager.h"
#include "CommandLine.h"
#include "Settings.h"
#include "pm1006.h"


#define DISPLAYDELAY 10
#define MEASURETIME 10000
#define MAXPMHISTORY 5

#define TEMP_COMPENSATION -5 // temperature adjust because of esp8266 heating
#define MQTT_TOPIC_PM2_5 "/pm2_5"
#define MQTT_TOPIC_TEMP "/temp"
#define MQTT_TOPIC_PRES "/pres"
#define MQTT_TOPIC_HUM "/hum"

#define PIN_PM1006_RX  2 //D4
#define PIN_PM1006_TX  0 // NOT USED, recycling digit
// #define PIN_RESETFLASH 0
// #define PIN_LDR        A0 //A0
// #define PIN_FAN        13 //D7
// #define PIN_PIXELS     14 //D5

#define PIN_BME280_SCL 2
#define PIN_BME280_SDA 12

static SoftwareSerial pmSerial(PIN_PM1006_RX, PIN_PM1006_TX);
static PM1006 pm1006(&pmSerial);

BME280I2C::Settings bmesettings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_16,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76
);

BME280I2C bme(bmesettings);

uint32_t currentMillis;
uint32_t lastMillis;
bool flag;
bool valid;

uint16_t pmhistory[MAXPMHISTORY];
uint8_t pmhistorycnt;
uint16_t value;

bool bme_flag;
float bme_temp(NAN);
float bme_hum(NAN);
float bme_pres(NAN);


WifiMQTTManager wifiMQTT("Vindriktning");

Tokenizer tokenizer;


// calculate average over MAXPMHISTORY readings
uint16_t calculateAVG(float newpm){
  uint8_t i;
  
  for(i = (MAXPMHISTORY - 1); i > 1; i--){
    pmhistory[i] = pmhistory[i - 1];
  }
  pmhistory[0] = (uint16_t) newpm;
  
  if(pmhistorycnt < MAXPMHISTORY){
    pmhistorycnt++;
    valid = false;
    return pmhistory[0];
  }

  pmhistorycnt = 0;
  newpm = pmhistory[0];
  for(i = 1; i < MAXPMHISTORY; i++){
    newpm += (pmhistory[i] / 5.0);
  }
  valid = true;
  
  Serial.println("AVG: " + String(newpm));
  return (uint16_t) newpm;
}


void getBMEData(){

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(bme_pres, bme_temp, bme_hum, tempUnit, presUnit);

  // temperature correction
  bme_temp += TEMP_COMPENSATION;
  
  Serial.println("Info: read " + String(bme_temp) + " °C " + String(bme_hum) + " %RH " + String(bme_pres) + " hPa");
 
}


void setup(){
  Serial.begin(115200);
  flag = false;
  valid = false;
  pmhistorycnt = 0;
  
  enableWiFiAtBootTime();
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);

  bme_flag = true;
  Wire.begin(PIN_BME280_SDA, PIN_BME280_SCL);
  if(!bme.begin()){
    #ifdef VERBOSE
    Serial.println("");
    Serial.println("");
    Serial.println("Warning: BME280 sensor not found");
    #endif
    bme_flag = false;
  }
  getBMEData();

  pmSerial.begin(PM1006::BIT_RATE);

  Settings *s = Settings::self();
  s->load();

  if (s->useWifi()) {
    wifiMQTT.setup();
  }

}


void loop(){
  currentMillis = millis();
  
  Settings *s = Settings::self();

  if((currentMillis - lastMillis) > MEASURETIME){
    lastMillis = millis();

    // first round wait
    if(flag){
      printf("Attempting measurement:\n");
      if(pm1006.read_pm25(&value)){
  
        Serial.print("New sensor value:");
        Serial.println(String(value).c_str());        
        
        value = calculateAVG(value);          

        // publish
        if (s->useWifi()) {
          wifiMQTT.tryPublish(Settings::self()->mqttTopic() + MQTT_TOPIC_PM2_5, String(value));
          if(bme_flag){
            wifiMQTT.tryPublish(Settings::self()->mqttTopic() + MQTT_TOPIC_TEMP, String(bme_temp));
            wifiMQTT.tryPublish(Settings::self()->mqttTopic() + MQTT_TOPIC_PRES, String(bme_pres));
            wifiMQTT.tryPublish(Settings::self()->mqttTopic() + MQTT_TOPIC_HUM, String(bme_hum));            
          }
        }
      

          
      } else {
        Serial.println("Measurement failed");
         
      }
    } else {
      flag = true;
    }
  }
  
  // display value if is valid 
  if(valid == true){
    display7s::write(1, (value % 10));
    delay(DISPLAYDELAY);
    display7s::write(0, (value / 10));
    delay(DISPLAYDELAY);
    
  } else {
    display7s::write(1, -1);
    delay(DISPLAYDELAY);
    display7s::write(0, -1);
    delay(DISPLAYDELAY);
  }

      
  // TODO: parse some commands form the tokenizer
  if (tokenizer.tokenizeFromSerial()) {
    parseCommand(tokenizer, wifiMQTT);
  }

  // Commandline may have modified settings
  if (s->isDirty()) {
    s->save();
  }

 
}
