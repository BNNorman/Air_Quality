/***
 * Using_Sofware_Serial.ino
 * 
 * 
 * 
 * 
 */
#include <ZPH01.h>
#include <SoftwareSerial.h>



#define txPin 10
#define rxPin 9

SoftwareSerial ss(rxPin,txPin);

ZPH01 sensor(ss);

void setup() {
  Serial.begin(9600);
  ss.begin(9600);

  Serial.println("Checking for serial data from Sensor");

  uint32_t Now=millis();
  
  if (!ss.available())
    {
      // sensor may not become ready for 10 secconds
      if (millis()-Now>10000)
        {          
        Serial.println("No serial data from sensor");
        while(1);    
        }
    }
  
}


void loop() 
{ 
 float LowPercent=sensor.GetLowPercent();

 Serial.print("Low pulse rate %=");
 Serial.println(LowPercent);

delay(5000);  // let's not be too hasty
}
