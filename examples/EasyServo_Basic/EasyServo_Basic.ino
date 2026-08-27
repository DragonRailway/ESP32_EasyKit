#include <Arduino.h>
#include <ESP32Servo.h>

Servo servo1;
Servo servo2;

const int SERVO_PIN1 = 5;       // S1
const int SERVO_PIN2 = 7;       // S3
const int POWER_LATCH_PIN = 15; // 5V rail enable

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
  delay(500);

  // Enable 5V power rail
  if (POWER_LATCH_PIN >= 0) {
    pinMode(POWER_LATCH_PIN, OUTPUT);
    digitalWrite(POWER_LATCH_PIN, HIGH);
    Serial.printf("Power latch enabled (GPIO %d)\n", POWER_LATCH_PIN);
  }

  Serial.println("\n=== ESP32Servo (LEDC Peripheral) Sweep Test (GPIO 5 & 7) ===");

  // Allow allocation of all timers for ESP32PWM/LEDC
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servo1.setPeriodHertz(50); // Standard 50 Hz
  servo2.setPeriodHertz(50);

  servo1.attach(SERVO_PIN1, 600, 2400); // 600-2400 µs
  servo2.attach(SERVO_PIN2, 600, 2400);

  Serial.printf("[ATTACH] ESP32Servo attached on GPIO %d and GPIO %d (50 Hz | 600-2400 µs)\n",
                SERVO_PIN1, SERVO_PIN2);
}

void loop() {
  for (int pos = 20; pos <= 160; pos += 1) {
    servo1.write(pos);
    servo2.write(pos);
    if (pos % 20 == 0) {
      Serial.printf("LEDC Sweep UP: %3d° | Pulse: %d µs\n", pos, servo1.readMicroseconds());
    }
    delay(20);
  }

  for (int pos = 160; pos >= 20; pos -= 1) {
    servo1.write(pos);
    servo2.write(pos);
    if (pos % 20 == 0) {
      Serial.printf("LEDC Sweep DOWN: %3d° | Pulse: %d µs\n", pos, servo1.readMicroseconds());
    }
    delay(20);
  }
}
