/**
 * @file EasyServo_Basic.ino
 * @brief Basic servo sweep using ESP32_EasyKit.
 *
 * Demonstrates:
 *   - EasyServo.h drop-in replacement
 *   - attach(), write(degrees), writeMicroseconds()
 *   - read() / readMicroseconds()
 */

#include <ESP32_EasyKit.h>

EasyServo myServo;

const int SERVO_PIN = 35;

void setup() {
  Serial.begin(115200);
  delay(500);

  int ch = myServo.attach(SERVO_PIN);
  if (ch == 0) {
    Serial.println("ERROR: Failed to attach servo!");
    while (true) { delay(1000); }
  }

  Serial.printf("EasyServo attached on pin %d (channel %d)\n", SERVO_PIN, ch);
  Serial.printf("Active servos: %d\n", Servo::numAttached());

  // Move to center
  myServo.write(90);
  delay(1000);
}

void loop() {
  // Sweep from 0° to 180°
  for (int angle = 0; angle <= 180; angle += 5) {
    myServo.write(angle);
    Serial.printf("Angle: %3d° | Pulse: %d µs\n",
                  myServo.read(), myServo.readMicroseconds());
    delay(30);
  }

  delay(500);

  // Sweep back from 180° to 0°
  for (int angle = 180; angle >= 0; angle -= 5) {
    myServo.write(angle);
    delay(30);
  }

  delay(500);

  // Demonstrate microsecond control
  Serial.println("\n--- Microsecond control ---");
  myServo.writeMicroseconds(1000);  // near 0°
  delay(1000);
  myServo.writeMicroseconds(1500);  // center
  delay(1000);
  myServo.writeMicroseconds(2000);  // near 180°
  delay(1000);
}
