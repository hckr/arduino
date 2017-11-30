#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define TRIG 6
#define ECHO 7
#define BUZZER 8

unsigned long alarm_end = 0;

void setup() {
  sensors.begin();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);
}

float distance() {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH) / 58.0;
}

void loop() {
  lcd.setCursor(0, 0);
  char distStr[4];
  float dist = distance();
  if (dist <= 200) {
    dtostrf(dist, 3, 0, distStr);
    lcd.print(distStr) + 3;
    lcd.print(" cm");

    if (dist < 100) {
      digitalWrite(BUZZER, HIGH);
      alarm_end = millis() + 300;
    }
  } else {
    lcd.print("--- cm");
  }

  if (millis() > alarm_end) {
    digitalWrite(BUZZER, LOW);
  }

  lcd.setCursor(0, 1);
  sensors.requestTemperatures();
  lcd.print(sensors.getTempCByIndex(0));
  lcd.print(" *C ");
}
