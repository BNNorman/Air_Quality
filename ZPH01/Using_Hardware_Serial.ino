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
  
}


void loop() 
{ 
  sensor.loop();  // needed to avoid blocking
  
  // this returns -1 until a valid data stream has been read
  // the data is cached so remains unchanged till the next
  // valid datastream is read
 float LowPercent=sensor.getLowPercent();

 Serial.print("Low pulse rate %=");
 Serial.println(LowPercent);

 delay(5000);  // let's not be too hasty
}
