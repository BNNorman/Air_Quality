/*
 MQTT_SHARP_ZPH.ino

 Author: Brian Norman
 Date: December 2018

 Read sensors and publish to an MQTT broker.

 Purpose is for comparison of two co-located sensors. Sample rates are high to test responsiveness and sensor tracking.
 If deployed sample reporting would be done maybe once every 30 minutes.

 Handles the Sharp gp2y1010 (analogue+digital) and zph01 (serial) sensors. The sharp can run at 100Hz
 the zph01 runs at one sample per second

 All sensors are running continuously and sampled once SAMPLE_INTERVAL seconds. Samples are added to a rolling average
 of AVERAGE_SIZE samples.

 NTP is used to set the system time.
 
 An MQTT JSON message, once every LOG_INTERVAL seconds, is transmitted to an MQTT broker. The message contains
 epoch time (seconds since 1st Jan 1970.
 
 SHARP  5V device but Heltec manages it ok, 
        pin 3---------- pin 22 LED LOW=ON, HIGH=OFF
        pin 4 --------- pin 34 Analog in (max signal from Sharp is 3.6v) 
 
 ZPH01  5V device but Heltec seems to handle it ok.
        pin 1 --------- GND (Enables 
        pin 2 (rx)-----pin 12 (tx)
        pin 3 (tx)-----pin 13 (rx)
        pin 4 VCC------5v

*/

/////////////// Configuration /////////////////////////////////
#define LOG_INTERVAL 60   // seconds between published messages
#define SAMPLE_INTERVAL 1 // seconds between sensor samples - zph01 must be >=1s
#define AVERAGE_SIZE 30   // number of samples in rolling average
#define MAX_PAYLOAD 100		// maximum payload size

const char* ssid = "ssidssidssid";   // local Wifi
const char* password = "pwdpwdpwd";

#define MQTT_TOPIC "Heltec/SHARP_ZPH"
const char* mqtt_server = "192.168.1.44"; // the broker, this is mine - use your own.
const char* payloadFormat="{ \"Epoch_Time\":%d,\"Sharp (v)\":%s,\"ZPH01 %\":%s}";

//////////////////////////////////////////////////////////////////////////
#include <TimeLib.h>
#include <WiFi.h>
#include <PubSubClient.h>

////////// for NTP /////////
#include <NTPClient.h> // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);



////////// SHARP ////////////////////
#define SHARP_LED   22   // digital out LOW=LED On, HIGH=LED Off, handled by the driver
#define SHARP_PIN   34   // analogue in
#define OFF HIGH
#define ON  LOW
#include "SHARP.h"        // https://github.com/BNNorman/Air_Quality/tree/master/SHARP_gp22y1010
SHARP   Sharp(SHARP_LED,SHARP_PIN);

/////////// ZPH01 ////////////////////
#include <ZPH01.h> // https://github.com/BNNorman/Air_Quality/tree/master/ZPH01
ZPH01   zph01(Serial2); // pins 12 & 13

/////////// Moving average /////////////////
#include <MovingAverage.h>
MovingAverage sharp(AVERAGE_SIZE);
MovingAverage zph(AVERAGE_SIZE);

///////////  WiFi and MQTT clients //////////
WiFiClient espClient;
PubSubClient client(espClient);


////////// OLED //////////////////////////
#include <U8g2lib.h> //https://github.com/olikraus/u8glib
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// setupOLED should be called once from setup()

void setupOLED(void) {
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.setBitmapMode(false);  // solid - text overwrites
}


void updateDisplay(char *ZPH,char *SHARP)
{
  char msg[50]="";
  
  snprintf(msg,sizeof msg,"ZPH01 %:%s",ZPH);
  u8g2.clearBuffer();
  u8g2.drawStr(0,0,msg);
  
  snprintf(msg,sizeof msg,"Sharp v:%s",SHARP);
  u8g2.drawStr(0,16,msg);

  snprintf(msg,sizeof msg,"Time: %0.2d:%0.2d:%0.2d",hour(),minute(),second());
  u8g2.drawStr(0,32,msg);

  u8g2.sendBuffer();
}

void blankLine(uint8_t ypos)
{
  // black out the time line with a black box first otherwise
  u8g2.setDrawColor(0);
  u8g2.drawBox(0,ypos,128,16);
  u8g2.setDrawColor(1);
}

void updateTimeDisplay()
{
  //blankLine(32);  // NOT needed if the time string length is constant and bitmapMode is false
  char msg[50]="";
  snprintf(msg,sizeof msg,"Time: %0.2d:%0.2d:%0.2d",hour(),minute(),second());
  u8g2.drawStr(0,32,msg);
  u8g2.sendBuffer();
}

/////////////// Setup WiFi ////////////////////////
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.persistent(false);   // WiFi.begin() crashes if not set
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

///////////////// MQTT callback ( for subscribe) ////////////////////////////////

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED_BUILTIN if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED_BUILTIN on (Note that LOW is the voltage level
    // but actually the LED_BUILTIN is on; this is because
    // it is active low on the ESP-01)
  } else {
   ;  // Turn the LED_BUILTIN off by making the voltage HIGH
  }

}

/////////////// MQTT reconnect ////////////////////////

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "HELTEC-";
    clientId += String(random(0xffff), HEX);
   
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("client connect failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//////////////// SETUP //////////////////////
void setup() {
  
  // OLED
  setupOLED();

  // Sharp sensor 
  pinMode(SHARP_LED,OUTPUT);
  digitalWrite(SHARP_LED,OFF);
  
  Serial.begin(9600);
  Serial2.begin(9600,SERIAL_8N1,13,12); // ZPH01 sensor
  while(!Serial) ;
  
  setup_wifi();

  // get the right time
  timeClient.begin();

  // connect to the MQTT server

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // set the time
  timeClient.update();
  setTime(timeClient.getEpochTime());
  Serial.print("Time now is:");
  Serial.println(timeClient.getFormattedTime());
  
  Serial.println("Setup finished.");
}
////////////// SENSORS //////////////
// read the sensors

char payload[MAX_PAYLOAD]="";
uint32_t nxtLog=now();  // forces immediate publish, then waits LOG_INTERVAL

void readSensors()
{ 
   // buffers are bigger than required to avoid problems with conversion to float
   // Serial.println("Reading sensors");
    char sharpstr[20]="";
    char zphstr[20]="";
 
    // add to moving averages
    sharp.addValue(3.3*Sharp.getRaw()/4096.0);
    zph.addValue(zph01.GetLowPercent());

    // only log readings every LOG_INTERVAL seconds

    uint32_t Now=now();
    
    if (Now>=nxtLog)
      {
      nxtLog=Now+LOG_INTERVAL;
      dtostrf(sharp.getAvg(),7,4,sharpstr);
      dtostrf(zph.getAvg(),7,4,zphstr);

      //Serial.println("Updating OLED");
      updateDisplay(zphstr,sharpstr);

      // create some simple JSON
      snprintf(payload,MAX_PAYLOAD,payloadFormat,timeClient.getEpochTime(),sharpstr,zphstr);
      client.publish(MQTT_TOPIC,payload);
      Serial.println("Payload sent");
      }
      
    }

///////// Loop ///////////////////////////////////

void loop() {

  timeClient.update();
  updateTimeDisplay();
  
  //Serial.println(timeClient.getFormattedTime());
    
  if (!client.connected()) {
    reconnect();
  }

  //Serial.println("Calling client.loop()");
  client.loop();

  readSensors();

  delay(SAMPLE_INTERVAL*1000);

}
