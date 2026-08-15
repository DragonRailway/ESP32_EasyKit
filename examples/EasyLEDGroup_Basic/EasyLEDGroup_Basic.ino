/**
 * @file EasyLEDGroup_Basic.ino
 * @brief Coordinated multi-LED patterns using EasyLEDGroup.
 *
 * Demonstrates:
 *   - alternate()  — two ditch-light style outputs flashing alternately
 *   - chase()      — one member lit at a time, sweeping first → last
 *
 * NOTE: update() MUST be called in loop() for patterns to advance.
 */

#include <ESP32_EasyKit.h>

// Valid ESP32-S3 GPIOs (avoid JTAG/strapping pins)
const uint8_t LED1_PIN = 6;
const uint8_t LED2_PIN = 7;
const uint8_t LED3_PIN = 8;   // third member for the chase pattern
const uint8_t LED4_PIN = 9;   // (change per your wiring — 8/9 are safe on S3)

EasyLED led1(LED1_PIN);
EasyLED led2(LED2_PIN);
EasyLED led3(LED3_PIN);
EasyLED led4(LED4_PIN);

EasyLEDGroup group({ &led1, &led2, &led3, &led4 });

enum DemoPhase {
    PHASE_ALTERNATE,
    PHASE_CHASE,
    PHASE_COUNT,
};

DemoPhase phase = PHASE_ALTERNATE;
bool phaseStarted = false;
uint32_t phaseTimer = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("=== ESP32_EasyKit EasyLEDGroup Demo ===\n");
}

void loop() {
    // REQUIRED: Drive the group pattern engine
    group.update();

    if (!phaseStarted) {
        phaseStarted = true;
        phaseTimer = millis();

        switch (phase) {
            case PHASE_ALTERNATE:
                Serial.println("--- Alternate: LED1<->LED2 every 100ms ---");
                group.alternate(100);
                break;

            case PHASE_CHASE:
                Serial.println("--- Chase: one member lit, sweeping every 150ms ---");
                group.chase(150);
                break;

            default:
                break;
        }
    }

    // Hold each phase for a set duration
    uint32_t phaseDuration = 8000;
    if (millis() - phaseTimer >= phaseDuration) {
        group.stop();
        delay(500);

        phase = (DemoPhase)((int)phase + 1);
        if (phase >= PHASE_COUNT) {
            phase = PHASE_ALTERNATE;
            Serial.println("\n=== Restarting demo ===\n");
        }
        phaseStarted = false;
    }
}
