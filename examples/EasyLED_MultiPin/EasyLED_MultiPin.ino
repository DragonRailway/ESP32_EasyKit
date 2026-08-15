/**
 * @file EasyLED_MultiPin.ino
 * @brief Demonstrates GlobalPinManager pin ownership between EasyLED and EasyServo.
 *
 * The LED attaches pin 4 first; when the servo attaches the same pin, the pin
 * manager notifies the LED (onPinStolen) and the LED detaches. Verifies that
 * pin ownership transfers cleanly between different EasyKit peripherals.
 */

#include <ESP32_EasyKit.h>

EasyLED myLed;
Servo myServo;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Initial Attach -> LED");
    myLed.attach(4);
    Serial.print("LED isattached: "); Serial.println(myLed.isAttached());

    Serial.println("Attach -> Servo (Stealing Pin 4)");
    myServo.attach(4);

    Serial.print("LED isattached: "); Serial.println(myLed.isAttached());
    Serial.print("Servo isattached: "); Serial.println(myServo.attached());
}

void loop() {}
