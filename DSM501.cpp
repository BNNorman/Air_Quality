#include <Dsm501.h>  
#include <Arduino.h>
#include <stdint.h>

  
 namespace DSM {
 	using namespace std;
	
	namespace PM25
	{
	//using namespace std;
	uint32_t  	sampleTimeWindow=0;	// typically 30 seconds converted to millisec
	uint32_t	sampleStartTime=0;	// time when sampling started (ms)
	uint32_t  	lowTime=0;  		// millisec when sensorPin went low
	uint8_t   	sensorPin=0;
	volatile float		cumLowTime=0;		// millisec	
	float		lastReading=-1;		// flag idicates value to be ignored	
	uint32_t	intCount=0;
	}
	
	namespace PM10
	{
	//using namespace std;
	uint32_t  	sampleTimeWindow=0;	// typically 30 seconds converted to millisec
	uint32_t	sampleStartTime=0;	// time when sampling started (ms)
	uint32_t  	lowTime=0;  		// millisec when sensorPin went low
	uint8_t   	sensorPin=0;
	volatile float		cumLowTime=0;		// millisec
	float		lastReading=-1;		// flag idicates value to be ignored
	uint32_t	intCount=0;
	}

  /*
  * Init - sets up the interrupt pins and sample periods
  * caller must invoke pmXXBeginSample to start sampling
  *
  */
  bool Init(uint8_t PM25_PIN, uint8_t PM25_SAMPLE_TIME, uint8_t PM10_PIN, uint8_t PM10_SAMPLE_TIME)
  {
      DSM::PM25::sensorPin=PM25_PIN;
	  DSM::PM10::sensorPin=PM10_PIN;
	  
      pinMode(DSM::PM25::sensorPin,INPUT_PULLUP);
	  pinMode(DSM::PM10::sensorPin,INPUT_PULLUP);
      
	  DSM::PM25::sampleTimeWindow=PM25_SAMPLE_TIME*1000;  	// millisec
      DSM::PM10::sampleTimeWindow=PM10_SAMPLE_TIME*1000;  	// millisec
	  
	  attachInterrupt(digitalPinToInterrupt(PM25_PIN),PM25_ISR_CHANGE,CHANGE);
	  attachInterrupt(digitalPinToInterrupt(PM10_PIN),PM10_ISR_CHANGE,CHANGE);
  }
  
  void End()
  {
	   detachInterrupt(digitalPinToInterrupt(DSM::PM25::sensorPin));
	   detachInterrupt(digitalPinToInterrupt(DSM::PM10::sensorPin));
  }
  
  uint32_t pm10IntCount()
	{
	// debugging only
	return DSM::PM25::intCount;
	}
	
  uint32_t pm25IntCount()
	{
	// debugging only
	return DSM::PM25::intCount;
	}
  
  void pm10BeginSample()
  {
	// resets the cumulative low time then starts a new sample
    noInterrupts();
	DSM::PM10::cumLowTime=0;
	DSM::PM10::intCount=0;
	DSM::PM10::sampleStartTime=millis();
	// sensor may already be low
	if (digitalRead(DSM::PM10::sensorPin)==0)
		{
		DSM::PM10::lowTime=DSM::PM10::sampleStartTime;
		}
	interrupts();
  }
  
   void pm25BeginSample()
  {
  	// resets the cumulative low time then starts a new sample
	noInterrupts();
	DSM::PM25::cumLowTime=0;
	DSM::PM25::intCount=0;
	DSM::PM25::sampleStartTime=millis();
	// sensor may already be low
	if (digitalRead(DSM::PM25::sensorPin)==0)
		{
		DSM::PM25::lowTime=DSM::PM25::sampleStartTime;
		}
	interrupts();
  }
  
float calcDensity(float min, float max, float ratio,float minDensity)
	{
	// used by getDensity
	// works out the mg/m^3 between given two points on manuf chart
	float slope=(max-min)/0.2;	// chart is in steps of 0.2mg/m^3
	return ((ratio-min)/slope)+minDensity;
	}

float getDensity(float ratio)
	{
	// not elegant but produces a very good fit with manuf charts
	// calcs are not long so numbers left to clarify
	// returns mg/m^3

	// NOTE, there are a number of 'clever' formulas on the internet BUT
	// when they are used in a spreadsheet they didn't give values which matched the manuf charts
	// hence this :-

	// chart bottoms out around here
	if (ratio<=0.62) return 0.0;

	// work through the mid-bands
	if (ratio>0.62  and ratio<=2.05) 	return calcDensity(0.62,2.05,ratio,0);
	if (ratio>2.05  and ratio<=4.2) 	return calcDensity(2.05,4.2,ratio,0.2);
	if (ratio>4.2  and ratio<=6.1) 		return calcDensity(4.2,6.1,ratio,0.4);
	if (ratio>6.1  and ratio<=7.95) 	return calcDensity(6.1,7.95,ratio,0.6);
	if (ratio>7.95  and ratio<=9.2) 	return calcDensity(7.95,9.2,ratio,0.8);
	if (ratio>9.2 and ratio<=10.7) 		return calcDensity(9.2,10.7,ratio,1.0);
	if (ratio>10.7 and ratio<=12.0) 	return calcDensity(10.7,12.0,ratio,1.2);

	// the sensor range is limited to 1.4mg/m^3
	return 1.4;
	}


float pm10Ratio()
  {
	// rteurns the pm10 ratio (%) or -1 if the sample time window has not
	// been reached
	uint32_t now=millis();
	uint32_t elapsed=now-DSM::PM10::sampleStartTime;
	
	// has the sample time been reached?
	if (elapsed>=DSM::PM10::sampleTimeWindow)
		{
		// if the pin is still low add on the extra
		if (digitalRead(DSM::PM10::sensorPin)==LOW)
			{
			DSM::PM10::cumLowTime=DSM::PM10::cumLowTime+now-DSM::PM10::lowTime;
			}
		
		// cumLowTime is an int, so is elapsed
		// division results in zero so coerse the division into a float
		return 100*((float)PM10::cumLowTime/(float)elapsed);	// percentage
		
		}
		
	// sampleTimeWindow not reached
	return -1;
  }
  
  
  float pm10Density()
  {
	// returns the pm2.5 density in mg/m^3
	float ratio=pm10Ratio(); // returns -1 if sample time not reached
	
	if (ratio<0) return DSM::PM10::lastReading;
	
	pm10BeginSample();
	DSM::PM10::lastReading=getDensity(ratio);
	return DSM::PM10::lastReading;
  }
  
  float pm25Ratio()
  {
	// calculate the low occupancy ratio (%)
	// returns -1 if the sample end time has not been reached
	uint32_t now=millis();
	uint32_t elapsed=now-DSM::PM25::sampleStartTime;
	
	// has the sample time been reached?
	if (elapsed>=DSM::PM25::sampleTimeWindow)
		{

		if (digitalRead(DSM::PM25::sensorPin)==LOW)
			{
			DSM::PM25::cumLowTime=DSM::PM25::cumLowTime+now-DSM::PM25::lowTime;
			}
				
		// cumLowTime is an int, so is elapsed
		// division results in zero so coerse the division into a float
		return 100.0*( (float)DSM::PM25::cumLowTime/(float)elapsed);	//percentage
		}
		
	// signal sample as invalid (sampleTimeWindow not reached)
	return -1;
  }
  
  float pm25Density()
  {
	// returns the pm2.5 density in mg/m^3
  
	float ratio=pm25Ratio();	//percentage
	
	if (ratio<0) return DSM::PM25::lastReading;
	
	pm25BeginSample();
	DSM::PM25::lastReading=getDensity(ratio);
	return DSM::PM25::lastReading;
  }
  

/*
* ISRs - simply keep track of low periods
* the accumulated low time is reset when a reading is taken
*
*/
  
  
  static void PM10_ISR_CHANGE()
  {
	
    uint8_t state=digitalRead(DSM::PM10::sensorPin);
	uint32_t now=millis();
	
	DSM::PM10::intCount++;
	
	if (state==0) 
		{
		// sensor has just gone low
		DSM::PM10::lowTime=now;
		return;
		}
	
	// pin has gone high
	DSM::PM10::cumLowTime=DSM::PM10::cumLowTime+now-DSM::PM10::lowTime;
	
  }
  
   
  static void PM25_ISR_CHANGE()
  {
	
    uint8_t state=digitalRead(DSM::PM25::sensorPin);
	uint32_t now=millis();
	
	DSM::PM25::intCount++;
		
	if (state==0) 
		{
		// sensor has just gone low
		DSM::PM25::lowTime=now;
		return;
		}
	
	// pin has gone high
	DSM::PM25::cumLowTime=DSM::PM25::cumLowTime+now-DSM::PM25::lowTime;
	
  }
  
  
  // end of namespace
  }