#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <Timezone.h>

// LCD configuratie
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Wi-Fi gegevens
const char* ssid = "JOUW_WIFI_NAAM";
const char* password = "JOUW_WIFI_WACHTWOORD";

// NTP Client configuratie
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org", 0, 60000);

// Tijdzone voor België
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};
TimeChangeRule CET = {"CET", Last, Sun, Oct, 3, 60};
Timezone belgiumTZ(CEST, CET);

// Alarm instellingen
int alarmHour = 6;
int alarmMinute = 45;
bool alarmEnabled = true;

// Pin configuratie
const int buzzerPin = 25;
const int buttonPin = 26;

// Debounce variabelen voor knop
bool lastButtonState = LOW;
bool currentButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Seriële logging interval
unsigned long lastSerialPrint = 0;
const unsigned long serialInterval = 5000;

void setup() {
  Serial.begin(115200);

  // LCD instellen
  lcd.begin();
  lcd.backlight();
  lcd.print("Verbinden...");

  // Wi-Fi verbinden
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Verbonden");

  // Start NTP Client
  ntpClient.begin();

  // Pins instellen
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Tijd bijwerken
  ntpClient.update();
  unsigned long epochTime = ntpClient.getEpochTime();
  TimeChangeRule* tcr;
  time_t localTime = belgiumTZ.toLocal(epochTime, &tcr);

  int currentHour = hour(localTime);
  int currentMinute = minute(localTime);
  int currentSecond = second(localTime);
  int currentDay = day(localTime);
  int currentMonth = month(localTime);
  int currentYear = year(localTime);

  // Knopstatus controleren
  checkButton();

  // LCD-update
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(formatDate(currentDay, currentMonth, currentYear));
  lcd.setCursor(0, 1);
  lcd.print(formatTime(currentHour, currentMinute));
  lcd.print(alarmEnabled ? " Aan" : " Uit");

  // Alarm controleren
  if (alarmEnabled && currentHour == alarmHour && currentMinute == alarmMinute) {
    speelAlarm();
  }

  // Tijd loggen in seriële monitor
  if (millis() - lastSerialPrint >= serialInterval) {
    lastSerialPrint = millis();
    Serial.print("Tijd: ");
    Serial.println(formatTime(currentHour, currentMinute));
  }

  delay(1000);
}

void checkButton() {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) {
        alarmEnabled = !alarmEnabled;
      }
    }
  }
  lastButtonState = reading;
}

void speelAlarm() {
  for (int i = 0; i < 10; i++) {
    tone(buzzerPin, 1000);
    delay(500);
    noTone(buzzerPin);
    delay(500);
  }
}

String formatTime(int hour, int minute) {
  char timeString[6];
  sprintf(timeString, "%02d:%02d", hour, minute);
  return String(timeString);
}

String formatDate(int day, int month, int year) {
  char dateString[11];
  sprintf(dateString, "%02d/%02d/%04d", day, month, year);
  return String(dateString);
}
