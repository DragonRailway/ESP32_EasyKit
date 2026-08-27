/**
 * @file ServoBuildTest.ino
 * @brief Multi-pin EasyServo attach build test.
 *
 * Verifies that a single EasyServo instance can drive multiple servo pins in
 * lockstep (shared timer/operator, per-pin comparator+generator):
 *   - attach() a second pin without detaching the first
 *   - attachedPins() returns a bitmask covering both pins
 *   - setSpeed()/write()/update() drive all pins together
 *   - readAngle() reflects the last-attached pin's state
 */

#include <Arduino.h>
#include <Servo.h>

// MIKRO_V2 pins (S1 = GPIO 5, S2 = GPIO 6, POWER_ENABLE = GPIO 15)
#define SERVO1 5
#define SERVO2 6
#define POWER_LATCH_PIN 15

Servo myServo;

void setup() {
  Serial.begin(115200);
  delay(500);

  // Enable MIKRO_V2 5V rail
  pinMode(POWER_LATCH_PIN, OUTPUT);
  digitalWrite(POWER_LATCH_PIN, HIGH);
  Serial.printf("Power latch enabled (GPIO %d)\n", POWER_LATCH_PIN);

  // Multi-pin attach: attaching SERVO2 must NOT detach SERVO1
  int ch1 = myServo.attach(SERVO1);
  int ch2 = myServo.attach(SERVO2);

  Serial.printf("attach(5) -> %d, attach(6) -> %d\n", ch1, ch2);
  if (ch1 == 0 || ch2 == 0) {
    Serial.println("ERROR: attach failed");
    while (1) { delay(1000); }
  }

  Serial.printf("Attached pins mask: %llu (expect bit 5 and 6)\n",
                (unsigned long long)myServo.attachedPins());
  Serial.printf("Active pins across instances: %d\n", (int)Servo::numAttached());

  myServo.setSpeed(90.0f, 0.8f);  // 90 deg/s, easing
  myServo.write(20);
}

void loop() {
  myServo.update();

  static uint32_t lastMove = 0;
  static bool flip = false;
  if (millis() - lastMove >= 2000) {
    lastMove = millis();
    flip = !flip;
    float target = flip ? 160.0f : 20.0f;
    myServo.write(target);
    Serial.printf("Commanded -> %.0f deg\n", target);
  }

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 500) {
    lastPrint = millis();
    Serial.printf("Current Angle: %.2f  |  Pulse: %d us\n",
                  myServo.readAngle(), myServo.readMicroseconds());
  }
}
