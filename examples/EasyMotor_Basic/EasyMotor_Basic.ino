/**
 * @file EasyMotor_Basic.ino
 * @brief Simplified Direct-Drive DC Motor Example for ESP32-S3.
 * 
 * Demonstrates the modernized, low-latency EasyMotor API optimized for 
 * hardware FPU performance.
 */

#include <ESP32_EasyKit.h>

EasyMotor motor;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32_EasyKit - Simplified DC Motor Example");

  // Initialize motor on pins 7 and 8 (Avoids S3 JTAG conflict)
  if (motor.begin(7, 8) != EasyKit::Result::OK) {
    Serial.println("Failed to initialize MCPWM for motor!");
    while (1)
      ;
  }

  Serial.println("Commands are now instantaneous (Direct Drive).");
}

void loop() {
  // Direct speed control
  Serial.println("Running Forward at 50%...");
  motor.write(50.0f);
  delay(1000);

  Serial.println("Running Forward at 100%...");
  motor.write(100.0f);
  delay(1000);

  Serial.println("Active Braking...");
  motor.brake();
  delay(1000);

  Serial.println("Running Reverse at 75%...");
  motor.write(-75.0f);
  delay(1000);

  Serial.println("Running Reverse at 50%...");
  motor.write(-50.0f);
  delay(1000);

  Serial.println("Running Reverse at 25%...");
  motor.write(-25.0f);
  delay(1000);

  Serial.println("Coasting to stop...");
  motor.coast();
  delay(1000);
}
