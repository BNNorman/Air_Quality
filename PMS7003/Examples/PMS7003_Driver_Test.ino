#include <PMS7003.h>
#include <SoftwareSerial.h>


SoftwareSerial SensorSerial(12,11);
PMS7003 pms7003(SensorSerial);

uint32_t startTime=0;
uint32_t dataValid=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SensorSerial.begin(9600);
  
  Serial.println("PMS7003 sensor driver test");

  pms7003.sensorSleep();

  Serial.println("Sensor sleeping");
  startTime=millis();
}



void loop() {
  // put your main code here, to run repeatedly:
  pms7003.loop();
  
  if (pms7003.sensorSleeping()&& (millis()-startTime)>=5000) 
    {
      Serial.println("waking up sensor");
      pms7003.sensorWakeup();
      startTime=millis();
      dataValid=0;
    }
  
  if (pms7003.dataValid)
    {
      if (dataValid==0) dataValid=millis()-startTime;
      
      Serial.print("Data became valid after ");
      Serial.println(dataValid);

     // always print values
     Serial.print("CF_PM1_0 : "),Serial.println(pms7003.CF_PM1_0);
     Serial.print("CF_PM2_5 : "),Serial.println(pms7003.CF_PM2_5);
     Serial.print("CF_PM10_0 : "),Serial.println(pms7003.CF_PM10_0);
     
     Serial.print("AT_PM1_0 : "),Serial.println(pms7003.AT_PM1_0);
     Serial.print("AT_PM2_5 : "),Serial.println(pms7003.AT_PM2_5);
     Serial.print("AT_PM10_0 : "),Serial.println(pms7003.AT_PM10_0);


     Serial.print("PC_0_3 : "),Serial.println(pms7003.PC_0_3);
     Serial.print("PC_0_5 : "),Serial.println(pms7003.PC_0_5);
     Serial.print("PC_1_0 : "),Serial.println(pms7003.PC_1_0);
     Serial.print("PC_2_5 : "),Serial.println(pms7003.PC_2_5);
     Serial.print("PC_5_0 : "),Serial.println(pms7003.PC_5_0);
     Serial.print("PC_10_0 : "),Serial.println(pms7003.PC_10_0);
    }

   if (! pms7003.sensorSleeping() && (millis()-startTime)>35000)
    {
      Serial.println("Sleeping for 5 seconds");
      pms7003.sensorSleep();
      startTime=millis();
    }

}
