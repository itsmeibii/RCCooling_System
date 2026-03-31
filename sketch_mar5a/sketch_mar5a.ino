#include <math.h>

const int THERM_PIN = 0;   // ADC1_CH0
const int FAN_PIN   = 18;

const float R_FIXED = 10000.0;
const float R0      = 10000.0;
const float T0      = 298.15;
const float BETA    = 3590.0;

float CAL_OFFSET_C = 10.0;       // <-- your calibration fix

int readAdcAvg() {
  long sum = 0;
  const int N = 32;
  for (int i = 0; i < N; i++) {
    sum += analogRead(THERM_PIN);
    delayMicroseconds(200);
  }
  return (int)(sum / N);
}

float adcToTempC(int adc) {
  adc = constrain(adc, 1, 4094);
  float rTherm = R_FIXED * (4095.0 / adc - 1.0);
  float invT = (1.0 / T0) + (1.0 / BETA) * log(rTherm / R0);
  return (1.0 / invT) - 273.15;
}

int tempToDuty(float tempC) {
  // Tune these however you want
  const float T_OFF  = 26.0;   // fan off below this
  const float T_FULL = 36.0;   // full speed above this
  const int DUTY_MIN_SPIN = 90;

  if (tempC <= T_OFF) return 0;
  if (tempC >= T_FULL) return 255;

  float frac = (tempC - T_OFF) / (T_FULL - T_OFF);
  return DUTY_MIN_SPIN + (int)((255 - DUTY_MIN_SPIN) * frac);
}

void setup() {
  Serial.begin(115200);
  ledcAttach(FAN_PIN, 25000, 8);   // ESP32-C6 core 3.x
}

void loop() {
  int adc = readAdcAvg();
  float tempC = adcToTempC(adc) + CAL_OFFSET_C;
  int duty = tempToDuty(tempC);

  ledcWrite(FAN_PIN, duty);

  Serial.print("TempC=");
  Serial.print(tempC, 1);
  Serial.print("  Duty=");
  Serial.println(duty);

  delay(300);
}