/*
PMS7003.h

Code to read the serial stream from a PMS sensor. Written in a namespace
so can only manage one sensor at a time

You can use any serial stream. e.g.

	for a NANO (or similar with only one serial port)
		#include <SoftwareSerial.h>
		SoftwareSerial SS(11,12);	// rx,tx pins
		
		PMS7003::init(SS);
		
	for a Heltec or similar with more than one HDW serial port
	
		PMS7003::init(Serial2);	
	
In setup():-

	using SoftwareSerial
		SS.begin(9600);
	
	using hardware serial
		Serial2.begin(9600,SERIAL_8N1,12,13);	// for Heltec using Serial2 on pins 12 & 13

In loop():-

	PMS7003::readSensor();	// it is non-blocking

	if (PMS7003::dataValid)
		{
		Serial.print("Atmospheric pm1.0="); Serial.println(PMS7003::AT_PM1_0);
		Serial.print("Atmospheric pm2.5="); Serial.println(PMS7003::AT_PM2_5);
		Serial.print("Atmospheric pm10.0="); Serial.println(PMS7003::AT_PM10_0);
		}

*/

#pragma once

#include <Arduino.h>
#include <TimeLib.h>

#define MAXDATA	32	// number of bytes in the PMS7003 transmission

// used to control readSensor()
enum SENSOR_STATUS {waiting42,waiting4D,frameLenH,frameLenL,readingData};

class PMS7003{

public: 
	// validated data

	// Factory values using a standard particle
	uint32_t	CF_PM1_0=0;		// 1.0um
	uint32_t	CF_PM2_5=0;		// 2.5um
	uint32_t	CF_PM10_0=0;	// 10.0um

	// Atmospheric values (you would normally use these)
	uint32_t	AT_PM1_0=0;		// 1.0um
	uint32_t	AT_PM2_5=0;		// 2.5um
	uint32_t	AT_PM10_0=0;	// 10.0um

	// particle counts in 0.1L of air
	uint32_t	PC_0_3=0;		// > 0.3um
	uint32_t	PC_0_5=0;		// > 0.5um
	uint32_t	PC_1_0=0;		// > 1.0um
	uint32_t	PC_2_5=0;		// > 2.5um
	uint32_t	PC_5_0=0;		// > 5.0um
	uint32_t	PC_10_0=0;		// > 10.0um

	// dataValid is set as soon as any validated data is received
	// The validated data is retained until the next validated reading
	// dataValid is set fals when sensorSleep() or sensorWakeup() are called

	bool dataValid=false;		// set if validated dat is valid

	/*
	* PMS7003(SensorSerial)
	*
	* SensorSerial can be a hardware or software serial instance
	*
	*/
	PMS7003(Stream & SensorSerial);

	/*
	* sensorSleep()
	*
	* does what it says on the tin. sets dataValid to false
	*
	*/ 
	void sensorSleep();
	/*
	* sensorWakeup()
	*
	*
	* Does what it says on the tin. Sets dataValid to false
	* It may take a few seconds before the data becomes valid
	*/

	void sensorWakeup();

	/*

	readSensor() is a non-blocking method to gather up the data stream and validate it 

	This should be called from loop().

	*/

	void readSensor();

private:
	// private variables

	Stream *_ss;
	uint8_t data[MAXDATA];

	uint8_t count=0;		// used as an index into data
	uint32_t startTime=0;	// used to timeout if a data stream starts but doesn't end
	uint8_t frameLen=0;		// calculated frame length (excludes first 4 bytes)
	uint32_t chkSum=0;		// accumulated checksum

	SENSOR_STATUS sensorStatus=waiting42;

}; // class