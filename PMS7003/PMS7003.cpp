/*
PMS7003.cpp

Code to read the serial stream from a PMS sensor.
Last validated data read can be accessed individually like so:-

pm1_0=PMS7003::AT_PM1_0

*/



#include <PMS7003.h>

//#define DEBUG	// uncomment to enable Serial.print messages

/*
* PMS7003(SensorSerial)
*
* SensorSerial can be a hardware or software serial instance
*
*/
PMS7003::PMS7003(Stream & SensorSerial)
{
_ss=&SensorSerial;
}

/*
* sensorSleep()
*
* does what it says on the tin. 
*
*/ 
void PMS7003::sensorSleep()
{
  uint8_t command[] = { 0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73 };
  _ss->write(command, sizeof(command));
  dataValid=false;
  sensorStatus=sleeping;
  dataAvailable=false;
  #ifdef DEBUG
  Serial.println("Sensor sleeping.");
  #endif
}

/*
*
* sensorSleeping()
*
*/

bool PMS7003::sensorSleeping()
{
return  sensorStatus==sleeping;
}


/*
* sensorWakeup()
*
*
* Does what it says on the tin. It may take a few seconds before valid data is received
*/

void PMS7003::sensorWakeup()
{
  uint8_t command[] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74 };
  _ss->write(command, sizeof(command));
  dataValid=false;
  sensorStatus=waiting42;
  #ifdef DEBUG
  Serial.println("Sensor wakeup");
  #endif
}


/*

loop() is a non-blocking method to gather up the data stream and validate it 

This should be called from loop().

The rxData is 32 bytes long, frame length is 28 (32-4 header and framelength bytes)

*/


void PMS7003::loop()
  {
    uint32_t Now=now();

    if (!_ss->available()) 	
		{
		dataAvailable=false;
		return;
		}

	// get the next serial character
    uint8_t ch=_ss->read();
	
	dataAvailable=true;

	#ifdef DEBUG
	Serial.print("PMS: "); Serial.print(ch,HEX); Serial.print(", Sensor status="); Serial.println(sensorStatus);
	#endif
	
	// accumulated checksum is over the first 30 characters
    if (count<30) chkSum=chkSum+ch; // reset when 0x42 is first seen
  
    switch(sensorStatus){
	
	case sleeping:
		// sensor may be asleep but serial buffer might have
		// residual characters to flush
		return;
		
    case waiting42:
		count=0;
		chkSum=ch;
		frameLen=0;
		if (ch!=0x42) return;	// not a start character
		
		#ifdef DEBUG
		Serial.println("Got start ch 0x42");
		#endif
		
		data[count++]=ch;
		startTime=Now;
		sensorStatus=waiting4D;
		break;
  
    case waiting4D:
		if (ch!=0x4d) {          
			// 2nd char MUST be 0x4D but it isn't
			// so try again
			
			#ifdef DEBUG
			Serial.println("2nd char not 0x4D"); 
			#endif
			
			sensorStatus=waiting42;
			return;
        }
		
		#ifdef DEBUG
		Serial.println("Got 2nd char 0x4D"); 
		#endif

        data[count++]=ch;
        sensorStatus=frameLenH;        
		break;


    case frameLenH:
      data[count++]=ch;
      frameLen=ch<<8;
      
      sensorStatus=frameLenL;
      break;
      
    case frameLenL:
      data[count++]=ch;
      frameLen|=ch;
      #ifdef DEBUG
	  Serial.print("frameLen="); Serial.println(frameLen); 
	  #endif
	  sensorStatus=readingData;
      break;
      
    case readingData:
      data[count++]=ch;
	  
	  #ifdef DEBUG
	  Serial.print("Char count= "); Serial.println(count);
	  #endif

	  // 32 bytes in a valid frame
      if (count<32) 
		{
		return;
		}

	  // validate the checksum

      uint32_t actualChk=data[30]<<8 | data[31];
          
      if (actualChk==chkSum) {
		invalidChecksumCount=0;
		dataValid=true;
		#ifdef DEBUG
	    Serial.println("chksum ok");
		#endif
		CF_PM1_0=data[4]<<8 | data[5];
		CF_PM2_5=data[6]<<8 | data[7];
		CF_PM10_0=data[8]<<8 | data[9];
		AT_PM1_0=data[10]<<8 | data[11];
		AT_PM2_5=data[12]<<8 | data[13];
		AT_PM10_0=data[14]<<8 | data[15];
		PC_0_3=data[16]<<8 | data[17];
		PC_0_5=data[18]<<8 | data[19];
		PC_1_0=data[20]<<8 | data[21];
		PC_2_5=data[22]<<8 | data[23];
		PC_5_0=data[24]<<8 | data[25];
		PC_10_0=data[26]<<8 | data[27];
		
        }
		else {
		#ifdef DEBUG
		Serial.println("chksum failed");
		#endif
		dataValid=false;
		invalidChecksumCount++;
		}

		sensorStatus=waiting42; // wait for next frame    
      break;
      
    }
  }
