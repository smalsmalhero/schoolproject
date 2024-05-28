//  連接網路與連接網址所需的Library
#include <WiFi.h>
#include <HTTPClient.h>

//  解碼取得的資料所需的Library
#include <ArduinoJson.h>

//  取得的現在時間所需的Libraray
#include <WiFiUdp.h>
#include <NTPClient.h>

//  使用WS2812所需的Library
#include <FastLED.h>

#define NUM_LEDS 7  //定義全彩LED數量
#define DATA_PIN 2  //定義全彩LED訊號腳位
#define BRIGHTNESS 64 //定義亮度
CRGB faya_colorSticker[NUM_LEDS];  //定義FastLED類別

int rainbow16[16][3]={{ 85,   0, 255},    //紫  星期日
                      {255,   0,   0},    //紅  星期一
                      {255,  85,   0},    //橙  星期二
                      {255, 255,   0},    //黃  星期三
                      {  0, 255,   0},    //綠  星期四
                      {  0, 255, 255},    //青  星期五
                      {  0,   0, 255}};   //藍  星期六

//  WiFi帳號與密碼
char* ssid = "Xiaomi 13";
char* password = "45184518";

//  定義偵測按鈕狀態
int pin[7] = {12, 14, 27, 26, 25, 33, 32};
int sw = 0;
//  定義millis()需要用的時間
long t1, t2, t3;

//  設定所在的時區與現在時間
const long utcOffsetInSeconds = 28800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
//  取得現在時間
int days, hours, minutes, seconds;

//  定義需使用的網址
String Url = "https://1k27tssj-5000.asse.devtunnels.ms";
String url;
//  取得的資料
String payload;
//  定義動態json文件
DynamicJsonDocument doc(1024);
DynamicJsonDocument doc1(1024);
//  定義取得的指定時間與資料
String IDs;
int morning_time_hours, morning_time_minutes;
int noon_time_hours, noon_time_minutes;
int night_time_hours, night_time_minutes; 
int beforebed_time_hours, beforebed_time_minutes;

void setup() {
  //  設定串列全彩LED參數
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(faya_colorSticker, NUM_LEDS);  
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  Serial.begin(115200);
  //  設定按鈕腳位
  for(int i = 0; i < 7; i++){
    pinMode(pin[i],INPUT);
  }
  //  連接網路
  Serial.print("連線網路中");
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    for(int i = 0;i <= 6; i++){
      faya_colorSticker[i] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      FastLED.show();

      delay(50);

      FastLED.clear();
      FastLED.show();
    }
    for(int i = 6;i >= 0; i--){
      faya_colorSticker[i] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      FastLED.show();

      delay(50);

      FastLED.clear();
      FastLED.show();
    }
  }
  Serial.println("");
  Serial.println("網路已連接");
  // 初始化 time client
  timeClient.begin();
}

void loop() {
  //  偵測是否有段連
  while(WiFi.status() != WL_CONNECTED){
    for(int i = 0;i <= 6; i++){
      faya_colorSticker[i] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      FastLED.show();
          
      delay(50);

      FastLED.clear();
      FastLED.show();
    }
    for(int i = 6;i >= 0; i--){
      faya_colorSticker[i] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      FastLED.show();
          
      delay(50);

      FastLED.clear();
      FastLED.show();
    }
  }
  //  更新指定資料
  if(millis() - t1 >= 10000){
    t1 = millis();

    upgradedata();

    Serial.print(morning_time_hours);
    Serial.println(morning_time_minutes);
    Serial.print(noon_time_hours);
    Serial.println(noon_time_minutes);
    Serial.print(night_time_hours);
    Serial.println(night_time_minutes);
    Serial.print(beforebed_time_hours);
    Serial.println(beforebed_time_minutes);
  }
  //  取得現在時間
  if(millis() - t2 >= 1000){
    t2 = millis();

    Gettime();
  }
  //  指示吃藥
  timeUP();
}

//  寄出http請求後，取得資料
void sendHttpRequest(String Urls) {
  //  創建一個HTTP物件
  HTTPClient http;

  // Send HTTP GET request to URL
  http.begin(Urls);
  int httpCode = http.GET();
  
  // Check the HTTP response
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      Serial.println(payload);
      // Close the HTTP connection
    } else {
      Serial.println("Request failed with HTTP error code: " + String(httpCode));
    }
  } else {
    Serial.println("Unable to connect to URL.");
  }

  // Close the HTTP connection
  http.end();
}
//  更新指定資料
void upgradedata(){
  url = Url + "/data";
  sendHttpRequest(url);

  deserializeJson(doc,payload);
  JsonObject obj = doc.as<JsonObject>();
  String user = obj["user"];
  deserializeJson(doc1, user);
  JsonObject obj1 = doc1.as<JsonObject>();

  String ID = obj1["ID"];
  String morning_time[2] = {obj1["morning_time"][0], obj1["morning_time"][1]};
  String noon_time[2] = {obj1["noon_time"][0], obj1["noon_time"][1]};
  String night_time[2] = {obj1["night_time"][0], obj1["night_time"][0]};
  String beforebed_time[2] = {obj1["beforebed_time"][0], obj1["beforebed_time"][1]};

  IDs = ID;
  morning_time_hours = morning_time[0].toInt();
  morning_time_minutes = morning_time[1].toInt();
  noon_time_hours = noon_time[0].toInt();
  noon_time_minutes = noon_time[1].toInt();
  night_time_hours = night_time[0].toInt();
  night_time_minutes  = night_time[1].toInt();
  beforebed_time_hours = beforebed_time[0].toInt();
  beforebed_time_minutes = beforebed_time[1].toInt();
}
//  取得現在時間
void Gettime(){
  timeClient.update();

  days = timeClient.getDay();
  hours = timeClient.getHours();
  minutes = timeClient.getMinutes();
  seconds = timeClient.getSeconds();
  Serial.println(minutes);
}
//  指示吃藥
void timeUP(){
  url = Url + "/" +IDs;
  //  偵測現在時間是否到了指定時間
  if(hours == morning_time_hours && minutes == morning_time_minutes && seconds <= 1){
    Serial.println("Success");
 
    while(true){
      //  設定 WS2812 的顏色
      faya_colorSticker[days] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      //  更新 WS2818 的顏色
      FastLED.show();
      if(millis() - t3 >= 20){
        if (sw != digitalRead(pin[days])){
          sw = digitalRead(pin[days]);
          if(sw == 1){
            FastLED.clear();
            FastLED.show();
            sendHttpRequest(url);
            break;
          }
          else{
            sw = 0;
          }
        }
      }
    }
  }
  else if(hours == noon_time_hours && minutes == noon_time_minutes && seconds <= 1){
    Serial.println("Success");
    while(true){
      //  設定 WS2812 的顏色
      faya_colorSticker[days] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      //  更新 WS2818 的顏色
      FastLED.show();
      if(millis() - t3 >= 20){
        if (sw != digitalRead(pin[days])){
          sw = digitalRead(pin[days]);
          if(sw == 1){
            FastLED.clear();
            FastLED.show();
            sendHttpRequest(url);
            break;
          }
          else{
            sw = 0;
          }
        }
      }
    }
  }
  else if(hours == night_time_hours && minutes == night_time_minutes && seconds <= 1){
    Serial.println("Success");
    while(true){
      //  設定 WS2812 的顏色
      faya_colorSticker[days] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      //  更新 WS2818 的顏色
      FastLED.show();
      if(millis() - t3 >= 20){
        if (sw != digitalRead(pin[days])){
          sw = digitalRead(pin[days]);
          if(sw == 1){
            FastLED.clear();
            FastLED.show();
            sendHttpRequest(url);
            break;
          }
          else{
            sw = 0;
          }
        }
      }
    }
  }
  else if(hours == beforebed_time_hours && minutes == beforebed_time_minutes && seconds <= 1){
    Serial.println("Success");
    while(true){
      //  設定 WS2812 的顏色
      faya_colorSticker[days] = CRGB{rainbow16[days][0],rainbow16[days][1],rainbow16[days][2]};
      //  更新 WS2818 的顏色
      FastLED.show();
      if(millis() - t3 >= 20){
        if (sw != digitalRead(pin[days])){
          sw = digitalRead(pin[days]);
          if(sw == 1){
            FastLED.clear();
            FastLED.show();
            sendHttpRequest(url);
            break;
          }
          else{
            sw = 0;
          }
        }
      }
    }
  }
}