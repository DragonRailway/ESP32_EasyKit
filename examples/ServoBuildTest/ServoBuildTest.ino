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

// Valid ESP32-S3 GPIOs (avoid JTAG/strapping pins 1, 2, 3, 8, 9, 19, 20, 46)
#define SERVO1 35
#define SERVO2 36

Servo myServo;

void setup() {
  Serial.begin(115200);
  delay(500);

  // Multi-pin attach: attaching SERVO2 must NOT detach SERVO1
  int ch1 = myServo.attach(SERVO1);
  int ch2 = myServo.attach(SERVO2);

  Serial.printf("attach(35) -> %d, attach(36) -> %d\n", ch1, ch2);
  if (ch1 == 0 || ch2 == 0) {
    Serial.println("ERROR: attach failed");
    while (1) { delay(1000); }
  }

  Serial.printf("Attached pins mask: %llu (expect bit 35 and 36)\n",
                (unsigned long long)myServo.attachedPins());
  Serial.printf("Active pins across instances: %d\n", (int)Servo::numAttached());

  myServo.setSpeed(60.0f, 0.8f);  // 60 deg/s, steep easing
  myServo.write(90);              // Start a shared move on all pins
}

void loop() {
  myServo.update();

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 500) {
    lastPrint = millis();
    Serial.printf("Current Angle: %.2f  |  Pulse: %d us\n",
                  myServo.readAngle(), myServo.readMicroseconds());
  }
}
