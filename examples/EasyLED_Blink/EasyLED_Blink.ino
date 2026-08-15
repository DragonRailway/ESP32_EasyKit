/**
 * @file EasyLED_Blink.ino
 * @brief Non-blocking blink pattern demo using ESP32_EasyKit EasyLED.
 *
 * Demonstrates:
 *   - startBlink()     — simple on/off
 *   - blinkN()         — blink a fixed count with callback
 *   - startBurst()     — rapid bursts with pauses
 *   - startHeartbeat() — double-beat pattern
 *   - startMorse()     — Morse code blinking
 *
 * NOTE: update() MUST be called in loop().
 */

#include <ESP32_EasyKit.h>

const uint8_t LED1_PIN = 6;

EasyLED led(LED1_PIN);

enum DemoPhase {
    PHASE_SIMPLE,
    PHASE_BLINK_N,
    PHASE_BURST,
    PHASE_HEARTBEAT,
    PHASE_MORSE,
    PHASE_COUNT,
};

DemoPhase phase = PHASE_SIMPLE;
bool phaseStarted = false;
uint32_t phaseTimer = 0;

void onBlinkDone(void* arg) {
    Serial.println("  >> blinkN complete!");
}

void onMorseDone(void* arg) {
    Serial.println("  >> Morse message complete!");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("=== ESP32_EasyKit Blink Demo ===\n");
}

void loop() {
    led.update();

    if (!phaseStarted) {
        phaseStarted = true;
        phaseTimer = millis();

        switch (phase) {
            case PHASE_SIMPLE:
                Serial.println("--- Simple Blink: 500ms on / 500ms off ---");
                led.startBlink(500, 500);
                break;

            case PHASE_BLINK_N:
                Serial.println("--- Blink 5 times: 200ms on / 200ms off ---");
                led.blinkN(200, 200, 5, onBlinkDone);
                break;

            case PHASE_BURST:
                Serial.println("--- Burst: 3 rapid pulses, 1s pause ---");
                // Parameter order: onMs, offMs, pauseMs, pulses
                led.startBurst(100, 100, 1000, 3);
                break;

            case PHASE_HEARTBEAT:
                Serial.println("--- Heartbeat pattern ---");
                led.startHeartbeat(100, 100, 100, 700);
                break;

            case PHASE_MORSE:
                Serial.println("--- Morse Code: \"SOS\" (repeat) ---");
                led.startMorse("SOS", 120, true, onMorseDone);
                break;

            default:
                break;
        }
    }

    // Hold each phase for a set duration
    uint32_t phaseDuration;
    switch (phase) {
        case PHASE_SIMPLE:    phaseDuration = 5000;  break;
        case PHASE_BLINK_N:   phaseDuration = 4000;  break;
        case PHASE_BURST:     phaseDuration = 6000;  break;
        case PHASE_HEARTBEAT: phaseDuration = 6000;  break;
        case PHASE_MORSE:     phaseDuration = 10000; break;
        default:              phaseDuration = 3000;  break;
    }

    if (millis() - phaseTimer >= phaseDuration) {
        led.stopBlink();
        led.off();
        delay(500);

        phase = (DemoPhase)((int)phase + 1);
        if (phase >= PHASE_COUNT) {
            phase = PHASE_SIMPLE;
            Serial.println("\n=== Restarting demo ===\n");
        }
        phaseStarted = false;
    }
}
