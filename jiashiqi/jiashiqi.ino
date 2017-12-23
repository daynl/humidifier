#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "./ESP8266.h"
#include "I2Cdev.h"
#include <IRremote.h>
#include <U8glib.h>
#include "./SHT2x.h"
#include <SoftwareSerial.h>
#define INTERVAL_Time 10
#define SSID           "mi"
#define PASSWORD       "58586768"
#define IDLE_TIMEOUT_MS  3000  
#define HOST_NAME   "api.heclouds.com"
#define DEVICEID   "20489645"
#define PROJECTID "107043"
#define HOST_PORT   (80)
String apiKey = "9CbEwVYVmqEdEd5qFhyHIFi7A2o=";
char buf[10];
#define INTERVAL_SENSOR   17000             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      17000   
#define INTERVAL_sensor 2000
bool humanHotState = false;
unsigned long sensorlastTime = millis();
unsigned long lcd_time = millis();
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
#define setFont_L u8g.setFont(u8g_font_7x13)
float tempOLED, humiOLED, lightnessOLED;
#define INTERVAL_OLED 1000
String mCottenData;
String jsonToSend;
float sensor_tem, sensor_hum, sensor_lux;
char  sensor_tem_c[7], sensor_hum_c[7], sensor_lux_c[7];
SoftwareSerial mySerial(2, 3);
ESP8266 wifi(mySerial);
unsigned long net_time1 = millis();
unsigned long sensor_time = millis();
String postString;
void getSensorData() {
  sensor_tem = SHT2x.GetTemperature();
  sensor_hum = SHT2x.GetHumidity();
  sensor_lux = analogRead(A0);
  delay(1000);
  dtostrf(sensor_tem, 2, 1, sensor_tem_c);
  dtostrf(sensor_hum, 2, 1, sensor_hum_c);
  dtostrf(sensor_lux, 3, 1, sensor_lux_c);
}
void setup(void)   
{       
    Wire.begin();
    Serial.begin(9600);   
    while(!Serial);
    pinMode(A0, INPUT);   
    Serial.print("setup begin\r\n");
    Serial.print("FW Version: ");
    Serial.println(wifi.getVersion().c_str());
    if (wifi.setOprToStation()) {
      Serial.print("to station ok\r\n");
 } else {
Serial.print("to station err\r\n");
 }



    if (wifi.joinAP(SSID, PASSWORD)) {

        Serial.print("Join AP success\r\n");

        Serial.print("IP: ");       

        Serial.println(wifi.getLocalIP().c_str());

    } else {

        Serial.print("Join AP failure\r\n");

    }

    

    Serial.print("setup end\r\n");
  pinMode(4, INPUT);
}
void loop(void)
{   
  
  humanHotState = digitalRead(4);
    if (humanHotState) {
    u8g.firstPage();
    do {
        setFont_L;
        u8g.setPrintPos(4, 8);
        u8g.print("current humidity");
        u8g.setPrintPos(100, 20);
        u8g.print(sensor_hum,DEC);
          }while( u8g.nextPage() );
    }
    else
     u8g.firstPage();
    
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR) 
  {  
    getSensorData(); 
    sensor_time = millis();
  }  
  if (net_time1 > millis())  
  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET) 
    updateSensorData();  
    net_time1 = millis();
  }
  
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
    Serial.print("create tcp ok\r\n");
    jsonToSend = "{\"Temperature\":";
    dtostrf(sensor_tem, 1, 2, buf);
    jsonToSend += "\"" + String(buf) + "\"";
    jsonToSend += ",\"Humidity\":";
    dtostrf(sensor_hum, 1, 2, buf);
    jsonToSend += "\"" + String(buf) + "\"";
    jsonToSend += ",\"Light\":";
    dtostrf(sensor_lux, 1, 2, buf);
    jsonToSend += "\"" + String(buf) + "\"";
    jsonToSend += "}";
    postString = "POST /devices/";
    postString += DEVICEID;
    postString += "/datapoints?type=3 HTTP/1.1";
    postString += "\r\n";
    postString += "api-key:";
    postString += apiKey;
    postString += "\r\n";
    postString += "Host:api.heclouds.com\r\n";
    postString += "Connection:close\r\n";
    postString += "Content-Length:";
    postString += jsonToSend.length();
    postString += "\r\n";
    postString += "\r\n";
    postString += jsonToSend;
    postString += "\r\n";
    postString += "\r\n";
    postString += "\r\n";
    const char *postArray = postString.c_str();
    Serial.println(postArray);
    wifi.send((const uint8_t*)postArray, strlen(postArray));
    Serial.println("send success");
    if (wifi.releaseTCP()) {
      Serial.print("release tcp ok\r\n");
    }
    else {
      Serial.print("release tcp err\r\n");
    }
    postArray = NULL;
  }
  else {
    Serial.print("create tcp err\r\n");
  }
}
