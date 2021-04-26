/*
   換成線上轉換base64 https://www.convertstring.com/zh_TW/EncodeDecode/Base64Encode

*/


#include <WiFi.h>
#include <WiFiClient.h>
#include <base64.h>
#include "time.h"

// Wifi Seting
char ssid[] = "live3"; // edit this
char pass[] = "0938925650"; // edit this
WiFiClient client;
int status = WL_IDLE_STATUS;

// Email server setting
char server[] = "mail.bojiang.com.tw";// edit this 郵件主機
int port = 25;
char myIP[] = "myIP"; // edit this 發送郵件電腦名稱
char username[] = "d20xOQ=="; // edit this要轉換成base64 live520→bGl2ZTUyMA==
char password[] = "b2ZmaWNlc2Nhbg=="; // edit this要轉換成base64 live520→bGl2ZTUyMA==
char sender[] = "<wm19@bojiang.com.tw>"; // edit this 發件者
char reciver[] = "<d2586808@bojiang.com.tw>"; // edit this 收件者
char cc[] = "<d2586808@yahoo.com.tw>";// edit this 副本
char Subject[] = "電腦室溫度測試異常回報";// edit this 主旨
char timeStringBuff[50]; //日期時間設定50字符

//===========================================

const char* ntpServer = "time.nist.gov";
const long  gmtOffset_sec = 28800; //edit this 時區（以秒為單位） 8*60*60
const int   daylightOffset_sec = 0; //edit this 定義夏令時的偏移量（以秒為單位）

void setup() {
  Serial.begin(115200);

  // Wifi Connect
  Serial.print("連線至");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();  //斷開當前網絡
    WiFi.begin(ssid, pass);//連接網絡
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi 已連接");
  Serial.println("IP 位址: ");
  Serial.println(WiFi.localIP());

  // 初始化並獲取時間
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();


  // 發送電子郵件
  Serial.println("\n正在開始發送電子郵件...");
  if (sendEmail()) {
    Serial.println(F("郵件已發送"));
  } else {
    Serial.println(F("電郵發送失敗"));
  }
  
}

void loop() {
  delay(1000);
}

//從網路ntpServer獲取時間
void printLocalTime() {
  time_t rawtime;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("無法獲取時間");
    return;
  }
  strftime(timeStringBuff, sizeof(timeStringBuff), "%a, %d %b %Y %H:%M:%S +0800", &timeinfo);
  Serial.println(timeStringBuff);//顯示日期時間
}

byte sendEmail()
{
  byte thisByte = 0;
  byte respCode;
  //  char tBuf[64];

  if (client.connect(server, port) == 1) {
    Serial.println(F("已連接"));
  } else {
    Serial.println(F("連接失敗"));
    return 0;
  }

  if (!eRcv()) return 0;

  Serial.println(F("發送 hello"));
  client.println((String)"EHLO " + myIP);
  if (!eRcv()) return 0;

  Serial.println(F("發送身份驗證登錄"));
  client.println("auth login");
  if (!eRcv()) return 0;

  Serial.println(F("發送用戶"));
  client.println(username);
  if (!eRcv()) return 0;

  Serial.println(F("發送密碼"));
  client.println(password);
  if (!eRcv()) return 0;

  // 更改為發送人地址
  Serial.println(F("發送 From"));
  client.println((String)"MAIL From: " + sender);
  if (!eRcv()) return 0;

  // 更改為收件人地址
  Serial.println(F("發送 To"));
  client.println((String)"RCPT To: " + reciver);
  if (!eRcv()) return 0;
  
  // 更改為副本收件人地址
  Serial.println(F("發送 Cc"));
  client.println((String)"RCPT To: " + cc);
  if (!eRcv()) return 0;
  
  Serial.println(F("發送數據"));
  client.println(F("DATA"));
  if (!eRcv()) return 0;

  // 郵件內容在這裡!!!!!!!!!
  Serial.println(F("發送 email"));
  client.println((String)"From: " + sender);
  client.println((String)"To: " + reciver);
  client.println((String)"Cc: " + cc);
  client.println((String)"Subject: =?UTF-8?B?" + base64::encode(Subject) + "?=");
  client.println((String)"Date: " + timeStringBuff);
  client.println("MIME-Version: 1.0");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Content-Transfer-Encoding: base64\r\n");
  client.println( base64::encode("<html><body>電腦室溫度測試異常回報<br>溫度: <font color='blue'>33</font><br>濕度:<br></body></html>") );
  client.println("");
  client.println(F("."));
  if (!eRcv()) return 0;

  Serial.println(F("發送 QUIT"));
  client.println(F("QUIT"));
  if (!eRcv()) return 0;

  client.stop();
  Serial.println(F("斷開連接"));
  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\n超時"));
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}


void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println(F("QUIT"));

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\n超時"));
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  client.stop();

  Serial.println(F("斷開連接"));
}
