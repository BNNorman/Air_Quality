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
  sensor.begin(9600);

  Serial.println("Checking for serial data from Sensor");
  
}


void loop() 
{ 
  sensor.loop();  // needed for non-blocking working
  
  // this returns -1 until valid data has been read
  // valid data is cached till new data has been received.
 float LowPercent=sensor.GetLowPercent();

 Serial.print("Low pulse rate %=");
 Serial.println(LowPercent);

delay(5000);  // let's not be too hasty
}
