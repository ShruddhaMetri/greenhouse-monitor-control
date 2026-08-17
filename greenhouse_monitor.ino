/*
 * Automated Greenhouse Monitor & Control System
 * ------------------------------------------------
 * Platform  : Arduino Uno (ATmega328P)
 * Sensors   : DHT11 (temperature + humidity), YL-69 soil moisture, LDR (light)
 * Actuators : Fan, artificial-light bulb, water pump, CFL/spray (all via relays)
 * Display   : 16x2 LCD in 4-bit mode
 *
 * Pin mapping (as documented in the project report):
 *   LCD  : RS=2, EN=3, D4=4, D5=5, D6=6, D7=7
 *   LDR (light)        -> A0
 *   DHT11 (temp/hum)   -> A1
 *   YL-69 soil moisture-> A2   (see NOTE below)
 *   Fan relay          -> D8
 *   Light/bulb relay   -> D10
 *   CFL / spray relay  -> D11
 *   Water pump relay   -> D12
 *
 * Control logic:
 *   Temperature > 27 C  -> Fan ON
 *   Low light           -> Bulb ON
 *   Soil dry            -> Pump ON
 *   Humidity > 40 %     -> shown on LCD (+ optional spray)
 *
 * NOTE: This sketch was reconstructed from the project documentation.
 * Before relying on it, verify on your own hardware:
 *   1. The soil-moisture pin (report references pin 9; YL-69 analog output
 *      needs an analog pin such as A2 — set SOIL_PIN to the pin you wired).
 *   2. Relay polarity — most cheap relay boards are ACTIVE-LOW (LOW = ON).
 *   3. Calibrate LIGHT_THRESHOLD and SOIL_THRESHOLD for your sensors/soil.
 *
 * Requires libraries: LiquidCrystal (built-in), DHT sensor library (Adafruit).
 */

#include <LiquidCrystal.h>
#include <DHT.h>

// ---------- LCD ----------
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);   // RS, EN, D4, D5, D6, D7

// ---------- DHT11 ----------
#define DHTPIN   A1
#define DHTTYPE  DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- Sensor pins ----------
const int LDR_PIN  = A0;
const int SOIL_PIN = A2;   // set to the analog pin you actually used

// ---------- Actuator (relay) pins ----------
const int FAN_PIN   = 8;
const int LIGHT_PIN = 10;  // artificial-light bulb
const int SPRAY_PIN = 11;  // CFL / spray
const int PUMP_PIN  = 12;  // water pump

// ---------- Thresholds (tune to your setup) ----------
const float TEMP_THRESHOLD  = 27.0;  // deg C
const float HUM_THRESHOLD   = 40.0;  // %
const int   LIGHT_THRESHOLD = 400;   // 0-1023; below this = "dark" (calibrate)
const int   SOIL_THRESHOLD  = 500;   // 0-1023; above this = "dry"  (calibrate)

// Most relay boards are active-LOW
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();

  pinMode(FAN_PIN,   OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(SPRAY_PIN, OUTPUT);
  pinMode(PUMP_PIN,  OUTPUT);

  // start with everything OFF
  digitalWrite(FAN_PIN,   RELAY_OFF);
  digitalWrite(LIGHT_PIN, RELAY_OFF);
  digitalWrite(SPRAY_PIN, RELAY_OFF);
  digitalWrite(PUMP_PIN,  RELAY_OFF);

  lcd.print("Greenhouse Sys");
  delay(1500);
  lcd.clear();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();
  int   lightLevel  = analogRead(LDR_PIN);
  int   soilLevel   = analogRead(SOIL_PIN);

  // guard against failed DHT reads
  if (isnan(temperature) || isnan(humidity)) {
    temperature = 0;
    humidity = 0;
  }

  // ---- Temperature -> Fan ----
  digitalWrite(FAN_PIN, (temperature > TEMP_THRESHOLD) ? RELAY_ON : RELAY_OFF);

  // ---- Light -> Bulb ----
  digitalWrite(LIGHT_PIN, (lightLevel < LIGHT_THRESHOLD) ? RELAY_ON : RELAY_OFF);

  // ---- Soil moisture -> Pump ----
  digitalWrite(PUMP_PIN, (soilLevel > SOIL_THRESHOLD) ? RELAY_ON : RELAY_OFF);

  // ---- Humidity -> spray (optional) ----
  digitalWrite(SPRAY_PIN, (humidity > HUM_THRESHOLD) ? RELAY_ON : RELAY_OFF);

  // ---- LCD ----
  lcd.setCursor(0, 0);
  lcd.print("T:");  lcd.print(temperature, 0);
  lcd.print("C H:"); lcd.print(humidity, 0); lcd.print("%  ");
  lcd.setCursor(0, 1);
  lcd.print("L:"); lcd.print(lightLevel);
  lcd.print(" S:"); lcd.print(soilLevel); lcd.print("  ");

  // ---- Serial log ----
  Serial.print("Temp=");   Serial.print(temperature);
  Serial.print("C Hum=");  Serial.print(humidity);
  Serial.print("% Light="); Serial.print(lightLevel);
  Serial.print(" Soil=");  Serial.println(soilLevel);

  delay(2000);
}
