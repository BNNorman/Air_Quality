/*
 * ZPH01_Simulator.ino
 * 
 * Emulates a ZPH01 Dust sensor in serial mode by sending out serial data with random
 * low pulse occupancy percentage values.
 * 
 * Written primarily to run on a NANO - you may need to change TX_PIN/RX_PIN
 * defines
 * 
 * Use this to ensure your driver can interpret the serial data from a sensor whilst
 * waiting for Aliexpress to send you a real one
 * 
 * The ZPH01 only sends data, it does not respond to commands from the MCU.
 * 
 * command format
 * 
 * Byte   value   Comment
 * 0      0xFF    always 0xFF  
 * 1      0x18    detection type code, always 0x18
 * 2      0x00    low pulse rate - unexplained in manual
 * 3      0->0x63 decimal 0-99 integer part of Low Pulse Occupancy % (LPO)
 * 4      0->0x63 decimal 0-99 LPO/100
 * 5      0x00    reserved
 * 6      0x01    mode (1 Hz)
 * 7      0x00    reserved
 * 8      0>0xFF  Checksum
 * 
 * checksum=sum(bytes 1 to 7 inclusive) bit inverted plus 1
 * 
 * The sketch echos what is being sent to the serial port
 * 
 * Brian N Norman Feb 2019
 * 
 */

#define RATE      1   // number of transmissions per second - standard ZPH01 does 1
#define RX_PIN    12
#define TX_PIN    11
#define MAX_DATA  9   // 9 bytes sent by the device


#include <Arduino.h>
#include <SoftwareSerial.h>
SoftwareSerial sensor(RX_PIN,TX_PIN); //RX,TX

uint8_t data[MAX_DATA]; // data packet

void sendData()
{
  float value=99.99;
  data[0]=0xFF;
  data[1]=0x18;
  data[2]=0x0;
  data[3]=random(0,99);
  data[4]=random(0,99);
  data[5]=0x0;
  data[6]=0x01;
  data[7]=0x0;

  // compute the checksum
  uint8_t temp=0;
  for (uint8_t i=1;i<=7;i++) temp=temp+data[i];
  data[8]=~temp+1;


  // send the data

  for (uint8_t c=0;c<MAX_DATA;c++)
    {
      sensor.write(data[c]);  // send to the device

      // echo for serial monitor if needed
      Serial.print(" ");  
      if (data[c]<16) Serial.print(0);  // leading zero
      Serial.print(data[c],HEX);
    }
  Serial.println();
}


void setup() {
   Serial.begin(9600);
   sensor.begin(9600);     // config the sensor
   
  // simple randomiser
  randomSeed(42);
}

void loop() {
  // put your main code here, to run repeatedly:
  sendData();
  delay(1000/RATE);
}
