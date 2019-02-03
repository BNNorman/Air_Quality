/*
 * LPO_Pulse_Simulator.ino
 * 
 * Emulates a low pulse occupancy signal for a dust sensor
 * 
 * The emulator drives a pin low for a % of 1 second
 * 
 * The sketch could be modified to:
 * 
 * 1  drive two pins
 * 2  cycle the LPO % instead of just choosing a random value
 * 
 * Brian N Norman Feb 2019
 * 
 */
#include <Arduino.h>

#define LPO_PIN     12

uint8_t   LPO=random(1,90);   		// millisec LPO pulse DOWN time 1..90%
uint32_t  LPO_LOW=LPO*10;     		// 1% is 10 millisec out of 1000
uint32_t  LPO_HIGH=1000-LPO_LOW;



void setup() {
   Serial.begin(9600);
   pinMode(LPO_PIN,OUTPUT);
   digitalWrite(LPO_PIN,HIGH);

  Serial.print("LPO=");
  Serial.print(LPO);
  Serial.print("ms = ");
  Serial.print(LPO*10);   // millisec 1% = 10 millisec
  Serial.println("%");
   
}

void loop() {
  digitalWrite(LPO_PIN,LOW);
  delay(LPO_LOW);
  digitalWrite(LPO_PIN,HIGH);
  delay(LPO_HIGH);
}
