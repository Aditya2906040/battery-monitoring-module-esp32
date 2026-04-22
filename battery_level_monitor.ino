#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ADC_PIN 34

const unsigned long interval = 1000;
unsigned long previousMillis = 0;

const float ADC_REF_VOLTAGE = 3.43;
const int ADC_RESOLUTION = 4095;
const float DIVIDER_RATIO = 2.0;

const int NUM_SAMPLES = 300;

int lastPercent = -1;  // for change detection

// 🔋 Battery percentage mapping
int getBatteryPercentage(float voltage) {
  if (voltage >= 4.2) return 100;
  if (voltage >= 4.1) return 90 + (voltage - 4.1) * 100;
  if (voltage >= 4.0) return 80 + (voltage - 4.0) * 100;
  if (voltage >= 3.9) return 70 + (voltage - 3.9) * 100;
  if (voltage >= 3.8) return 60 + (voltage - 3.8) * 100;
  if (voltage >= 3.7) return 50 + (voltage - 3.7) * 100;
  if (voltage >= 3.6) return 40 + (voltage - 3.6) * 100;
  if (voltage >= 3.5) return 30 + (voltage - 3.5) * 100;
  if (voltage >= 3.4) return 20 + (voltage - 3.4) * 100;
  if (voltage >= 3.3) return 10 + (voltage - 3.3) * 100;
  if (voltage >= 3.2) return 5 + (voltage - 3.2) * 50;
  return 0;
}

// 🔋 Custom characters (battery bar)
byte barEmpty[8] = {0,0,0,0,0,0,0,0};
byte barFull[8]  = {31,31,31,31,31,31,31,31};

// 🔋 Draw battery bar
void drawBatteryBar(int percent) {
  int totalBlocks = 10;
  int filled = (percent * totalBlocks) / 100;

  lcd.setCursor(0, 0);
  lcd.print("[");
  
  for (int i = 0; i < totalBlocks; i++) {
    if (i < filled) 
      lcd.write(byte(1)); // full
    else
       lcd.write(byte(0));            // empty
  }

  lcd.print("]");
}

// 🔋 Read battery voltage
float readBatteryVoltage() {
  uint32_t sum = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(ADC_PIN);
    delay(1); // small delay OK
  }

  float adc_avg = sum / (float)NUM_SAMPLES;

  float v_pin = (adc_avg / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
  float v_battery = v_pin * DIVIDER_RATIO;

  return v_battery;
}

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  // Create custom chars
  lcd.createChar(0, barEmpty);
  lcd.createChar(1, barFull);

  lcd.setCursor(0, 0);
  lcd.print("Battery Monitor");
  delay(1000);
  lcd.clear();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis += interval;

    float v_battery = readBatteryVoltage();
    int batteryPercent = getBatteryPercentage(v_battery);

    // Serial output
    Serial.print("Voltage: ");
    Serial.print(v_battery, 3);
    Serial.print(" V | Battery: ");
    Serial.print(batteryPercent);
    Serial.println(" %");

    // Update LCD ONLY if changed (important)
    if (batteryPercent != lastPercent) {
      lastPercent = batteryPercent;

      drawBatteryBar(batteryPercent);

      // Print %
      lcd.setCursor(13, 0);
      lcd.print(batteryPercent);
      lcd.print("% ");

      // Voltage line
      lcd.setCursor(0, 1);
      lcd.print("V: ");
      lcd.print(v_battery, 2);
      lcd.print("V   ");
    }

    // 🔔 Low battery warning
    if (v_battery < 3.3) {
      lcd.setCursor(0, 1);
      lcd.print("LOW BATTERY!  ");
      Serial.println("⚠ LOW BATTERY!");
    }
  }
}