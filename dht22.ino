#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> // Include the Manager library
#include "time.h"
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// NTP Config
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600; 
const int   daylightOffset_sec = 0;

#define DHT22_PIN  23
DHT dht22(DHT22_PIN, DHT22);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Timing & Data
unsigned long lastClockUpdate = 0;
unsigned long lastSensorUpdate = 0;
float lastHumi = -1.0, lastTempC = -1.0;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  dht22.begin();

  // --- WiFi Manager Implementation ---
  WiFiManager wm;
  
  lcd.setCursor(0, 0);
  lcd.print("WiFi Config Mode");
  lcd.setCursor(0, 1);
  lcd.print("Connect to: ESP32_AP");

  // This will block until connected or timed out
  // If it can't connect to old WiFi, it starts an AP named "ESP32_Config_AP"
  bool res = wm.autoConnect("ESP32_Config_AP"); 

  if(!res) {
      Serial.println("Failed to connect");
      lcd.clear();
      lcd.print("Connect Failed");
      // ESP.restart(); // Optional: restart to try again
  } else {
      Serial.println("Connected...yeey :)");
      lcd.clear();
      lcd.print("WiFi Connected!");
      delay(2000);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastClockUpdate >= 1000) {
    lastClockUpdate = currentMillis;
    displayClock();
  }

  if (currentMillis - lastSensorUpdate >= 2000) {
    lastSensorUpdate = currentMillis;
    readAndLogSensors();
  }
}

void displayClock() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) return;
  lcd.setCursor(0, 0);
  lcd.printf("Date: %02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  lcd.setCursor(0, 1);
  lcd.printf("Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void readAndLogSensors() {
  float h = dht22.readHumidity();
  float t = dht22.readTemperature();

  if (isnan(h) || isnan(t)) return;

  lcd.setCursor(0, 2);
  lcd.printf("Temp: %.1f C   ", t);
  lcd.setCursor(0, 3);
  lcd.printf("Humi: %.1f %%   ", h);

  if (t != lastTempC || h != lastHumi) {
    Serial.printf("Update: %.1fC | %.1f%%\n", t, h);
    lastTempC = t;
    lastHumi = h;
  }
}