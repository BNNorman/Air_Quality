/***
* DSM501.h
*
* Driver for DSM501 dust sensor using interrupts to compute low occupancy and
* return dust density in ug/m^3
*
* NOTICE: If you must use interrupt capable input pins - otherwise you are likely to get
* a 100% low occupancy and hence max density (1400ug/m^3) 
*
*
***/
#ifndef Dsm501_h
#define Dsm501_h

#include <stdint.h>

namespace DSM
{

// initialise the interrupt service routines
// SAMPLE_TIME(s) should be at least 30 seconds according to manuf

bool 	Init(uint8_t PM25_PIN,uint8_t PM25_SAMPLE_TIME,uint8_t PM10_PIN,uint8_t PM10_SAMPLE_TIME);	 

// these routines return the current density in mg/m^3
// when called they restart sampling if the sample is ready otherwise they return
// the previous reading

float 	pm25Density();
float 	pm10Density();

// the following are for internal use only

float 	 pm25Ratio();
float 	 pm10Ratio();
void	 pm25BeginSample();
void	 pm10BeginSample();
uint32_t pm10IntCount();
uint32_t pm25IntCount();
float	 calcDensity(float min,float max,float ratio,float offset);	
float 	 getDensity(float ratio);

void 	End();						// detaches ALL interrupts

// added special case for ESP32 - ref RadioHead issue

#if defined(ESP32_PLATFORM)
static void IRAM_ATTR PM25_ISR_CHANGE();		// interrupt service routine
static void IRAM_ATTR PM10_ISR_CHANGE();
#else
static void PM25_ISR_CHANGE();		// interrupt service routine
static void PM10_ISR_CHANGE();
#endif

}

#endif