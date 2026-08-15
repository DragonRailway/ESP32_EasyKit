/**
 * @file EasyLED_Fade.ino
 * @brief FPU-Optimized LED Fade Demo for ESP32.
 *
 * Demonstrates the modernized EasyLED animation engine with strict 
 * hardware FPU optimization (float-only math).
 *
 * NOTE: update() MUST be called in loop() for software fading to work.
 */

#include <ESP32_EasyKit.h>

const uint8_t LED1_PIN = 6;
const uint8_t LED2_PIN = 7;

EasyLED led1(LED1_PIN);
EasyLED led2(LED2_PIN);

enum DemoPhase {
    PHASE_SW_LINEAR,
    PHASE_SW_EASE_IN,
    PHASE_SW_EASE_OUT,
    PHASE_SW_SINE,
    PHASE_BREATHING,
    PHASE_COUNT,
};

DemoPhase phase = PHASE_SW_LINEAR;
bool phaseStarted = false;
uint32_t phaseTimer = 0;

void onSWFadeComplete(void* arg) {
    const char* name = (const char*)arg;
    Serial.print("  [");
    Serial.print(name);
    Serial.println("] SW Fade complete!");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("=== ESP32_EasyKit FPU-Optimized Fade Demo ===");
    Serial.print("Max duty: ");
    Serial.println(led1.getMaxDuty());
}

void loop() {
    // REQUIRED: Drive SW animation engines
    led1.update();
    led2.update();

    if (!phaseStarted) {
        phaseStarted = true;
        phaseTimer = millis();

        switch (phase) {
            case PHASE_SW_LINEAR:
                Serial.println("--- Phase 1: Linear software fading ---");
                led1.fadeIn(2000, EasyLED::Curve::LINEAR, onSWFadeComplete, (void*)"LED1-linear");
                led2.fadeIn(2000);
                break;

            case PHASE_SW_EASE_IN:
                Serial.println("--- Phase 2: Quadratic Ease-In ---");
                led1.fade(0, led1.getMaxDuty(), 2000, EasyLED::Curve::EASE_IN,
                            onSWFadeComplete, (void*)"LED1-easeIn");
                break;

            case PHASE_SW_EASE_OUT:
                Serial.println("--- Phase 3: Quadratic Ease-Out ---");
                led1.fade(led1.getMaxDuty(), 0, 2000, EasyLED::Curve::EASE_OUT,
                            onSWFadeComplete, (void*)"LED1-easeOut");
                break;

            case PHASE_SW_SINE:
                Serial.println("--- Phase 4: Sine crossfade (LED1 in, LED2 out) ---");
                led1.fadeIn(3000, EasyLED::Curve::SINE, onSWFadeComplete, (void*)"LED1-sine");
                led2.fadeOut(3000, EasyLED::Curve::SINE, onSWFadeComplete, (void*)"LED2-sine");
                break;

            case PHASE_BREATHING:
                Serial.println("--- Phase 5: Breathing (runs for 10 seconds) ---");
                // Breathing uses software (SINE/EASE) curves natively
                led1.startBreathing(1500, EasyLED::Curve::SINE);
                led2.startBreathing(2000, EasyLED::Curve::EASE_IN_OUT);
                break;

            default:
                break;
        }
    }

    // Advance to next phase after timeout
    uint32_t phaseDuration = (phase == PHASE_BREATHING) ? 10000 : 3500;
    if (millis() - phaseTimer >= phaseDuration) {
        // Clean up breathing
        if (phase == PHASE_BREATHING) {
            led1.stopBreathing();
            led2.stopBreathing();
        }

        led1.off();
        led2.off();
        delay(500);

        phase = (DemoPhase)((int)phase + 1);
        if (phase >= PHASE_COUNT) {
            phase = PHASE_SW_LINEAR;
            Serial.println("\n=== Restarting demo ===\n");
        }
        phaseStarted = false;
    }
}
