/*

  PMS7003_TEST2.ino

  testing the PMS7003 sensor with a Heltec

*/
#include <PMS7003.h>

PMS7003 pms(Serial2);

uint32_t  nxtPrint=0; // controls the speed of printing sensor values;

void setup(void) {
  Serial.begin(115200);                   // serial monitor
  Serial2.begin(9600,SERIAL_8N1,12,13);   // sensor settings (Heltec)
    
  Serial.println("Starting");     
  nxtPrint=now();
}



void loop(void) {

  pms.readSensor();

  // we don't want to be doing this printing on every iteration of the loop
  if ((pms.dataValid) and (now()>=nxtPrint))
    {
    Serial.print("pm1.0="); Serial.println(pms.AT_PM1_0);
    Serial.print("pm2.5="); Serial.println(pms.AT_PM2_5);   
    Serial.print("pm10.0="); Serial.println(pms.AT_PM10_0);    
    nxtPrint=now()+5;  // print values once every 5 seconds
    }
    
}
