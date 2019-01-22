/*
PMS7003.cpp

Code to read the serial stream from a PMS sensor.
Last validated data read can be accessed individually like so:-

pm1_0=PMS7003::AT_PM1_0

*/



#include <PMS7003.h>


/*
* init(SensorSerial)
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
}

/*
* sensorWakeup()
*
*
* Does what it says on the tin. Iy may take a few seconds before valid data is received
*/

void PMS7003::sensorWakeup()
{
  uint8_t command[] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74 };
  _ss->write(command, sizeof(command));
  dataValid=false;
}


/*

readSensor() is a non-blocking method to gather up the data stream and validate it 

This should be called from loop().

*/



void PMS7003::readSensor()
  {
    uint32_t Now=now();

    if (!_ss->available())   return;

    uint8_t ch=_ss->read();

    if (count<30) chkSum=chkSum+ch;
  
    switch(sensorStatus)
    {
    case waiting42:
      count=0;
      chkSum=ch;
      frameLen=0;
      if (ch==0x42)
        {
          data[count++]=ch;
          startTime=Now;
          sensorStatus=waiting4D;
        }
       break;
  
    case waiting4D:
      if (ch!=0x4d) 
        {
          // 2nd char MUST be 4d
          sensorStatus=waiting42;
        }
      else 
        {
          data[count++]=ch;
          sensorStatus=frameLenH;
        }
      break;


    case frameLenH:
      data[count++]=ch;
      frameLen=ch<<8;
      
      sensorStatus=frameLenL;
      break;
      
    case frameLenL:
      data[count++]=ch;
      frameLen|=ch;
      sensorStatus=readingData;
      break;
      
    case readingData:
      data[count++]=ch;
      if (count!=32) return;
      
	  // validate the checksum

      uint32_t actualChk=data[30]<<8 | data[31];
          
      if (actualChk==chkSum) {
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
        dataValid=true;
        }
		sensorStatus=waiting42; // wait for next frame    
      break;
      
    }
  }
