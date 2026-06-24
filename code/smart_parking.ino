#define BLYNK_TEMPLATE_ID "" // ur Blynk Template ID
#define BLYNK_TEMPLATE_NAME "" // ur Blynk Template Name
#define BLYNK_AUTH_TOKEN "" // ur Blynk Auth Token

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// WIFI

char ssid[] = ""; // ur WiFi SSID
char pass[] = ""; // ur WiFi Password

// LCD

LiquidCrystal_I2C lcd(0x27, 16, 2);

// SENSOR

#define SENSOR_ENTRY 19
#define SENSOR_EXIT  18

// MICROSWITCH SLOT

#define SLOT1 14
#define SLOT2 27
#define SLOT3 26
#define SLOT4 25

// SERVO

#define SERVO_ENTRY 13
#define SERVO_EXIT  23

Servo entryServo;
Servo exitServo;

// SLOT COUNTING

int totalSlot = 4;
int availableSlot = 4;

// SENSOR STATE

bool sensorEntryTriggered = false;
bool sensorExitTriggered = false;

// LCD TIMER

unsigned long lcdTimer = 0;

// SERVO FUNCTION

void openEntryGate() {
  entryServo.write(90);
}

void closeEntryGate() {
  entryServo.write(0);
}

void openExitGate() {
  exitServo.write(90);
}

void closeExitGate() {
  exitServo.write(0);
}

// WIFI + BLYNK

void connectWiFi() {

  WiFi.begin(ssid, pass);

  Serial.print("Connecting WiFi");

  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED && timeout < 20) {

    delay(500);
    Serial.print(".");

    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();
    Serial.println("WiFi Connected");

    Blynk.config(BLYNK_AUTH_TOKEN);

    Blynk.connect(3000);

    if (Blynk.connected()) {
      Serial.println("Blynk Connected");
    } else {
      Serial.println("Blynk Failed");
    }

  } else {

    Serial.println();
    Serial.println("WiFi Failed");
  }
}

// LCD DEFAULT

void showDefaultLCD() {

  lcd.setCursor(3, 0);
  lcd.print("AVAILABLE");

  lcd.setCursor(6, 1);
  lcd.print(availableSlot);
  lcd.print("/");
  lcd.print(totalSlot);
  lcd.print("   ");
}

// SETUP

void setup() {

  Serial.begin(115200);

  // I2C LCD
  Wire.begin(21, 22);

  // WIFI
  connectWiFi();

  // SENSOR
  pinMode(SENSOR_ENTRY, INPUT);
  pinMode(SENSOR_EXIT, INPUT);

  // MICROSWITCH
  pinMode(SLOT1, INPUT_PULLUP);
  pinMode(SLOT2, INPUT_PULLUP);
  pinMode(SLOT3, INPUT_PULLUP);
  pinMode(SLOT4, INPUT_PULLUP);

  // SERVO
  entryServo.attach(SERVO_ENTRY);
  exitServo.attach(SERVO_EXIT);

  closeEntryGate();
  closeExitGate();

  // LCD
  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.print("SMART PARKING");

  lcd.setCursor(2, 1);
  lcd.print("SYSTEM READY");

  delay(2000);

  lcd.clear();

  showDefaultLCD();
}

// LOOP

void loop() {

  // BLYNK

  if (WiFi.status() == WL_CONNECTED) {

    if (!Blynk.connected()) {
      Blynk.connect(1000);
    }

    Blynk.run();
  }

  // SENSOR READ

  int s1 = digitalRead(SENSOR_ENTRY);
  int s2 = digitalRead(SENSOR_EXIT);

  // MICROSWITCH READ
  // LOW = OCCUPIED
  // HIGH = EMPTY

  int p1 = digitalRead(SLOT1);
  int p2 = digitalRead(SLOT2);
  int p3 = digitalRead(SLOT3);
  int p4 = digitalRead(SLOT4);

  // BLYNK UPDATE

  if (Blynk.connected()) {

    Blynk.virtualWrite(V0, availableSlot);

    Blynk.virtualWrite(V2, p1 == LOW ? 1 : 0);
    Blynk.virtualWrite(V3, p2 == LOW ? 1 : 0);
    Blynk.virtualWrite(V4, p3 == LOW ? 1 : 0);
    Blynk.virtualWrite(V5, p4 == LOW ? 1 : 0);
  }

  // SENSOR ENTER

  if (s1 == LOW && !sensorEntryTriggered) {

    sensorEntryTriggered = true;

    if (availableSlot > 0) {

      availableSlot--;

      lcd.clear();

      lcd.setCursor(2, 0);
      lcd.print("CAR ENTER");

      lcd.setCursor(4, 1);
      lcd.print("LEFT:");
      lcd.print(availableSlot);

      openEntryGate();

      Serial.print("Car Enter | Slot: ");
      Serial.println(availableSlot);

      delay(2000);

      closeEntryGate();

    } else {

      lcd.clear();

      lcd.setCursor(2, 0);
      lcd.print("FULL SLOT");

      lcd.setCursor(1, 1);
      lcd.print("NO ENTRY");

      Serial.println("Slot Full | No Entry");
    }

    lcdTimer = millis();
  }

  if (s1 == HIGH) {
    sensorEntryTriggered = false;
  }

  // SENSOR EXIT

  if (s2 == LOW && !sensorExitTriggered) {

    sensorExitTriggered = true;

    if (availableSlot < totalSlot) {

      availableSlot++;

      lcd.clear();

      lcd.setCursor(2, 0);
      lcd.print("CAR EXIT");

      lcd.setCursor(4, 1);
      lcd.print("LEFT:");
      lcd.print(availableSlot);

      openExitGate();

      Serial.print("Car Exit | Slot: ");
      Serial.println(availableSlot);

      delay(2000);

      closeExitGate();
    }

    lcdTimer = millis();
  }

  if (s2 == HIGH) {
    sensorExitTriggered = false;
  }

  // AUTO RETURN LCD

  if (millis() - lcdTimer > 2500) {
    showDefaultLCD();
  }

  // SERIAL DEBUG

  Serial.print("Slot1:");
  Serial.print(p1);

  Serial.print(" | Slot2:");
  Serial.print(p2);

  Serial.print(" | Slot3:");
  Serial.print(p3);

  Serial.print(" | Slot4:");
  Serial.println(p4);

  delay(100);
}