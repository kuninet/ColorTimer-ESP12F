#include <Arduino.h>
#include <Arduino.h>
// #include <WiFi.h>  //ESP32
// #include <HTTPClient.h>  //ESP32
#include <ESP8266HTTPClient.h>  
#include <ESP8266WiFi.h>
#include "Ambient.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLEDの幅
#define SCREEN_HEIGHT 64 // OLEDの高さ
#define OLED_RESET    -1 // リセットピン（不要）

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//ESP32-S3-pico
// #define RED_LED_PIN 42
// #define BLUE_LED_PIN 41åå
// #define BUTTON_PIN 9

//ESP-12F
#define RED_LED_PIN 13
#define BLUE_LED_PIN 12
#define BUTTON_PIN 16

bool buttonPressed = false;
unsigned long timer = 0;
bool redBlinking = false;

// Wi-Fi設定
const char* ssid = "Wifi-SSID"; // Wi-FiのSSID
const char* password = "Wifi-Pass";
// const char* serverUrl = "http://example.com";
unsigned int channelId = xxxxx; // AmbientのチャンネルID
const char* writeKey = "yyyyy"; // Ambientの書き込みキー

// 待ち時間（秒で指定可能）
const int delaySeconds = 15; //秒（変更可能）
const unsigned long delayMillis = delaySeconds * 1000;

// void sendHttpRequest(const char* message) {
//     if (WiFi.status() == WL_CONNECTED) {
//         HTTPClient http;
//         http.begin(serverUrl);
//         http.addHeader("Content-Type", "application/json");
//         String jsonPayload = String("{\"message\":\"") + message + "\"}";
//         int httpResponseCode = http.POST(jsonPayload);
//         Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
//         http.end();
//     } else {
//         Serial.println("Wi-Fi not connected, HTTP request failed");
//     }
// }

WiFiClient client;
Ambient ambient;

// void sendHttpRequest(const char* message) {
//     if (WiFi.status() == WL_CONNECTED) {
//         HTTPClient http;

//         // 新しいAPI形式: begin(WiFiClient, url)
//         http.begin(client, serverUrl);
//         http.addHeader("Content-Type", "application/json");

//         String jsonPayload = String("{\"message\":\"") + message + "\"}";
//         int httpResponseCode = http.POST(jsonPayload);

//         if (httpResponseCode > 0) {
//             Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
//         } else {
//             Serial.printf("Error Code: %d\n", httpResponseCode);
//         }
//         http.end();
//     } else {
//         Serial.println("Wi-Fi not connected, HTTP request failed");
//     }
// }

#define BUZZER_PIN 14  // 圧電サウンダの接続ピン

void playTone(int frequency, int duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

void pinpon() {  // 「ピンポン」音
  playTone(1000, 300);
  playTone(1500, 300);
}

void bubu() {  // 「ブブー」音
  playTone(500, 400);
  playTone(400, 400);
}

void piropiro() {  // 「ピロピロ」音
  for (int freq = 1000; freq < 2000; freq += 50) {
    playTone(freq, 100);
  }
  for (int freq = 2000; freq > 1000; freq -= 50) {
    playTone(freq, 100);
  }
}

void byuunpin() {  // 「びゅーん、ピン」音
  for (int freq = 500; freq < 3000; freq += 100) {
    playTone(freq, 50);
  }
  playTone(2000, 400);
}

void clearLine(int row) {
  display.fillRect(0, row * 8, 128, 8, BLACK); // 指定した行を黒で塗りつぶす
  display.display(); // 画面更新
}

void displayTextAt(int row, int col, String text) {
  clearLine(row);
  display.setCursor(col * 6, row * 8);
  display.println(text);
  display.display();
}




void setup() {
    Serial.begin(115200); // シリアル通信初期化

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // OLEDのI2Cアドレスは0x3C
        Serial.println(F("SSD1306の初期化に失敗しました"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.display();

    displayTextAt(0, 0, "ColorTimer");  // 1行目 1桁目
    display.setTextSize(1);

    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    IPAddress ip = WiFi.localIP();  // Wi-FiのIP取得

    Serial.println("\nWi-Fi connected");
    Serial.println("\nIP:"+ip.toString());

    displayTextAt(3, 0, "IP:"+ip.toString()); 
    delay(1000);
    displayTextAt(3, 0, "Wifi Connected!! "); 

    ambient.begin(channelId, writeKey, &client);
    ambient.set(1, 0);
    ambient.setcmnt("マイコン起動");
    ambient.send();

    // 起動時に赤LED点灯
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
    Serial.println("System started: Red LED ON");
    displayTextAt(4, 0, "System started!!"); 
    displayTextAt(6, 0, "START Red LED ON!!!!!     "); 
                piropiro();
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(200); // ボタン押しのデバウンス
        if (!buttonPressed) {
            buttonPressed = true;
            timer = millis();
            if (redBlinking) {
                // 赤点滅をやめて赤点灯
                redBlinking = false;
                digitalWrite(RED_LED_PIN, HIGH);
                // sendHttpRequest("赤点灯");
                ambient.set(1, 2);
                ambient.setcmnt("仕事終り");
                ambient.send();
                Serial.println("Button pressed: Stopped red blinking, Red LED ON");
                displayTextAt(6, 0, "Red LED ON!!!!!     ");
                byuunpin();
            } else if (digitalRead(BLUE_LED_PIN) == HIGH) {
                // 青点灯中にボタンが押された場合、赤点灯に切り替え
                digitalWrite(BLUE_LED_PIN, LOW);
                digitalWrite(RED_LED_PIN, HIGH);
                // sendHttpRequest("青LEDから赤LEDへ切り替え");
                Serial.println("Button pressed: Blue LED OFF, Red LED ON");
                displayTextAt(6, 0, "BLUE -> Red LED ON  "); 
                bubu(); 
            } else {
                // 青点灯に切り替え、HTTP通信
                digitalWrite(RED_LED_PIN, LOW);
                digitalWrite(BLUE_LED_PIN, HIGH);
                // sendHttpRequest("青点灯");
                ambient.set(1, 1);
                ambient.setcmnt("仕事開始");
                ambient.send();
                Serial.println("Button pressed: Red LED OFF, Blue LED ON");
                displayTextAt(6, 0, "Red OFF, Blue ON     "); 
                pinpon();
            }
        }
    } else {
        buttonPressed = false;
    }

    if (!redBlinking && digitalRead(BLUE_LED_PIN) == HIGH && millis() - timer >= delayMillis) {
        // 指定時間後、青消灯して赤点滅開始
        digitalWrite(BLUE_LED_PIN, LOW);
        redBlinking = true;
        timer = millis();
        Serial.println("Time elapsed: Blue LED OFF, Red LED blinking");
        displayTextAt(6, 0, "Time elapsed: blink "); 
    }

    if (redBlinking) {
        // 赤点滅処理
        if ((millis() - timer) % 1000 < 500) {
            digitalWrite(RED_LED_PIN, HIGH);
            bubu();
        } else {
            digitalWrite(RED_LED_PIN, LOW);
        }
    }
}