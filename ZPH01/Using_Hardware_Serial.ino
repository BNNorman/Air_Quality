/***
 * Using_Hardware_Serial.ino
 * 
 * Assumes that Serial2 exists
 * 
 * 
 */
#include <ZPH01.h>


ZPH01 sensor(Serial2);

void setup() {
  Serial.begin(9600);
  sensor.begin(9600);

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
