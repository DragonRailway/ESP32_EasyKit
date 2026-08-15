/**
 * @file EasyLED_Basic.ino
 * @brief Basic LED brightness control using ESP32_EasyKit EasyLED.
 *
 * Demonstrates:
 *   - Unified write() — digital ON/OFF (bool) and PWM in one call
 *   - analogWrite() compatibility
 *   - Auto-allocation (no begin() needed)
 *   - Explicit begin() with custom config
 */

#include <ESP32_EasyKit.h>

const uint8_t LED1_PIN = 6;
const uint8_t LED2_PIN = 7;

// pin attachment happens at runtime
EasyLED led1;
EasyLED led2;


void setup() {
  Serial.begin(115200);
  delay(500);

  // led1: begin() with defaults — simplest explicit attach
  led1.begin(LED1_PIN);
  Serial.printf("LED1 attached: pin=%d\n", led1.getPin());

  // led2: explicit begin() with custom resolution
  EasyKit::LEDConfig cfg;
  cfg.freq = 4000;
  cfg.resolution = EasyKit::LEDCResolution::Bits10;  // 10-bit for finer control

  EasyKit::Result res = led2.begin(LED2_PIN, cfg);
  if (!EasyKit::ok(res)) {
    Serial.printf("ERROR: LED2 begin failed (code %d)\n", (int)res);
  } else {
    Serial.printf("LED2 attached: pin=%d, channel=%d, maxDuty=%lu\n",
                  led2.getPin(), led2.getChannel(), led2.getMaxDuty());
  }
}

void loop() {
  // ── Digital write demo ───────────────────────────────────────────────────
  Serial.println("Digital HIGH");
  led1.write(true);
  delay(500);

  Serial.println("Digital LOW");
  led1.write(false);
  delay(500);

  // ── PWM sweep on led1 using write(float) ──────────────────────────────────
  Serial.println("PWM sweep (write float percentage)...");
  for (float v = 0.0f; v <= 100.0f; v += 2.0f) {
    led1.write(v);
    delay(20);
  }
  for (float v = 100.0f; v >= 0.0f; v -= 2.0f) {
    led1.write(v);
    delay(20);
  }
  led1.write(false);
  delay(300);

  // ── analogWrite sweep on led2 ────────────────────────────────────────────
  Serial.println("analogWrite sweep...");
  for (uint8_t b = 0; b <= 250; b += 5) {
    led2.analogWrite(b);
    delay(20);
  }
    for (uint8_t b = 255; b >= 5; b -= 5) {
    led2.analogWrite(b);
    delay(20);
  }
  led2.on();
  delay(500);
  led2.off();
  delay(500);

  // ── Normalized duty on led2 ────────────────────────────────────────────────
  Serial.println("Percent duty: 25.0% → 50.0% → 75.0% → 100.0%");
  float duties[] = { 25.0f, 50.0f, 75.0f, 100.0f };
  for (float d : duties) {
    led2.write(d);
    Serial.printf("  duty=%.1f%%  raw=%lu\n", d, led2.getDuty());
    delay(500);
  }
  led2.off();
  delay(1000);
}
