# LED Functions Reference

This document provides a comprehensive reference for the **EasyLED** class. This unified class uses the ESP32's **LEDC** peripheral to provide precise PWM, fading, and non-blocking blink patterns, optimized for high-resolution hardware FPU performance.

```cpp
EasyLED led;         // no pin yet — call begin()/attach() later
EasyLED led(pin);    // add a pin during instantiation
```

> [!IMPORTANT]
> `EasyLED led();` declares a function, not an object (most vexing parse).
> Use `EasyLED led;` for default construction.

---

## Setup

### begin() / attach()

Attaches the LED control to a GPIO pin and automatically allocates a hardware LEDC channel.

**Syntax**

```cpp
led.begin(pin);                       // defaults: 4 kHz, 12-bit
led.begin(pin, config);               // custom config struct
led.attach(pin);                      // auto-attach (same as begin with defaults)
led.attach(pin, out_invert);          // with output inversion
```

**Parameters**

- **pin (uint8_t)**: The GPIO pin number.
- **config (EasyKit::LEDConfig)**: Optional `{ freq, resolution, channel, invertOutput }`.
- **out_invert (bool)**: If true, the pin output logic will be inverted.

**Returns** (`begin` only): `EasyKit::Result` — `EasyKit::Result::OK` on success, or an error code (`ERR_NO_FREE_CHANNEL`, `ERR_INVALID_PIN`, …). Check with `EasyKit::ok(result)`.

### end() / detach() / isAttached()

```cpp
led.end();               // detach all pins and release all channels
led.detach();            // detach all pins
led.detach(pin);         // detach a specific pin
bool ok = led.isAttached();  // true if at least one pin is attached
```

---

## Write & Brightness

### write()

Sets the output level using a polymorphic interface.

**Syntax**

```cpp
led.write(bool value);     // Digital ON/OFF
led.write(uint16_t ticks); // Raw duty ticks (0–4095 for default 12-bit)
led.write(float percent);  // Percentage (0.0f–100.0f)
```

**Parameters**

- **value (bool)**: Digital control. Use `true` (100% duty) or `false` (0% duty).
- **ticks (uint16_t)**: Raw PWM duty cycle ticks. Supports high-resolution control up to 16-bit.
- **percent (float)**: Brightness as a percentage. Recommended for most applications to ensure portability across different resolutions.

### on() / off() / analogWrite() / setDuty()

```cpp
led.on();                  // Full brightness on all pins
led.off();                 // Duty = 0 on all pins
led.analogWrite(128);      // 8-bit analogWrite scale (0–255)
led.setDuty(2048);         // Raw duty cycle ticks
uint32_t duty = led.getDuty();           // raw ticks
float pct   = led.getDutyPercent();      // normalized 0.0f–100.0f
```

---

## Fading

The library provides synchronized fading logic via a non-blocking software engine.

> [!IMPORTANT]
> **Polling Dependency**: To support advanced non-linear easing without CPU interrupt locks, it requires `led.update()` to be called regularly in your `loop()`.

### fade() / fadeTo() / fadeIn() / fadeOut()

Unified non-blocking fading interface.

**Syntax**

```cpp
bool fade(start, target, durationMs, curve, onComplete, arg);
bool fadeTo(target, durationMs, curve, onComplete, arg);
bool fadeIn(durationMs, curve, onComplete, arg);
bool fadeOut(durationMs, curve, onComplete, arg);
```

**Parameters**

- **start / target (uint32_t)**: Duty in raw ticks (0 to `getMaxDuty()`).
- **durationMs (uint32_t)**: Fade duration in milliseconds.
- **curve (EasyLED::Curve)**: Easing curve (see below). Defaults to `LINEAR`.
- **onComplete (callback) / arg (void\*)**: Optional completion callback (fires once when the fade finishes).

**Curves (EasyLED::Curve)**

- **LINEAR**: Constant rate of change.
- **EASE_IN**: Quadratic acceleration (Slow start, fast finish).
- **EASE_OUT**: Quadratic deceleration (Fast start, slow finish).
- **EASE_IN_OUT**: Tunable symmetric S-curve (Sigmoid style).
- **SINE**: Smooth trigonometric S-curve.

### startBreathing()

Starts a continuous fade-in and fade-out cycle.

**Syntax**

```cpp
led.startBreathing(halfCycleMs, curve);
led.stopBreathing();                 // stop breathing (returns to manual control)
bool breathing = led.isBreathing();  // query state
```

### Fade status

```cpp
bool fading = led.isFading();  // true while a fade is in progress
led.stopFade();                // cancel the current fade immediately
```

---

## Blink Patterns

Non-blocking pattern generation. Requires `update()` polling.

### startBlink()

Creates a simple repetitive ON/OFF pulse.

**Syntax**

```cpp
led.startBlink(onMs);                      // onMs on, onMs off
led.startBlink(onMs, offMs);
led.startBlink(onMs, offMs, dutyTicks);    // uint16_t duty (raw ticks)
led.startBlink(onMs, offMs, dutyPercent);  // float duty (0.0f–100.0f)
```

**Parameters**

- **onMs (uint32_t)**: Duration the LED stays ON (in milliseconds).
- **offMs (uint32_t)**: Duration the LED stays OFF.
- **duty**: Optional. Brightness during the ON phase. Defaults to 100%.

### blinkN()

Blinks a fixed number of times, then stops (optionally notifying a callback).

**Syntax**

```cpp
led.blinkN(onMs, offMs, count, onComplete, arg);
```

### startBurst()

Repeats a series of quick pulses (a "burst") followed by a longer pause.

**Syntax**

```cpp
led.startBurst(onMs, offMs, pauseMs, pulses, dutyTicks);
led.startBurst(onMs, offMs, pauseMs, pulses, dutyPercent);
```

**Parameters**

- **onMs (uint32_t)**: Duration of each individual pulse.
- **offMs (uint32_t)**: Time between pulses within the same burst.
- **pauseMs (uint32_t)**: Time between bursts.
- **pulses (uint16_t)**: Number of pulses to perform per burst.
- **duty**: Optional. Brightness during the ON phase.

### startHeartbeat()

Simulates a life-like heartbeat pattern (double-beat).

**Syntax**

```cpp
led.startHeartbeat(beat1Ms, beat2Ms, beatGapMs, pauseMs, dutyTicks);
led.startHeartbeat(beat1Ms, beat2Ms, beatGapMs, pauseMs, dutyPercent);
```

**Parameters**

- **beat1Ms / beat2Ms (uint32_t)**: Duration of the first / second beat.
- **beatGapMs (uint32_t)**: Time between the first and second beat.
- **pauseMs (uint32_t)**: Time between pairs of beats.
- **duty**: Optional. Brightness during the ON phase (0 = full by default).

### startCandle()

Simulates a random flickering candle effect.

**Syntax**

```cpp
led.startCandle(speed, jitter);
```

**Parameters**

- **speed (uint8_t)**: Determines the frequency of shifts (default 6).
- **jitter (uint8_t)**: Determines the amplitude of shifts (default 15).

### startMorse()

Encodes a string into International Morse Code and transmits it via the LED.

**Syntax**

```cpp
led.startMorse(text, ditMs, repeat, onComplete, arg);
```

**Parameters**

- **text (const char\*)**: The ASCII text to transmit (e.g., "SOS").
- **ditMs (uint32_t)**: Base time unit (duration of a "dot"). Default 120ms.
- **repeat (bool)**: If true, the message repeats indefinitely.
- **onComplete / arg**: Optional callback when a non-repeating message finishes.

### Blink status & control

```cpp
led.stopBlink();                 // stop any active blink pattern (led stays where it is)
bool blinking = led.isBlinking(); // true while a pattern is active
```

---

## Control & Polling

### update()

Must be called once per `loop()` iteration to drive the software fade engine and the blink pattern state machine.

```cpp
void loop() {
    led.update();
}
```

### stop()

Stops all active animations (fades, blinks, breathing) in one call.

```cpp
led.stop();
```

---

## Hardware Configuration (Defaults)

The library is pre-configured for smooth, professional LED control:

- **Default Frequency**: 4000Hz (4kHz).
- **Default Resolution**: 12-bit (0–4095).

### setFrequency() / getFrequency() / setResolution() / configure()

```cpp
bool ok = led.setFrequency(4000);
uint32_t hz = led.getFrequency();
led.setResolution(EasyKit::LEDCResolution::Bits12);  // Bits8/10/12/14
led.configure(4000, 10);                             // frequency + bit resolution
```

**Parameters**

- **hz (uint32_t)**: PWM frequency.
- **res (EasyKit::LEDCResolution)**: `Bits8`, `Bits10`, `Bits12` (default), or `Bits14`.
- **res (uint8_t, configure only)**: bit depth, e.g. `10`.

### Info getters

```cpp
int8_t  ch      = led.getChannel();   // LEDC channel (-1 if unattached)
uint32_t maxDuty = led.getMaxDuty();  // (1 << resolution) - 1
uint8_t pin     = led.getPin();       // lowest attached pin, or 255
uint64_t mask   = led.getPinMask();   // bitmask of attached pins
```

---

## Fade & Animation Engine

The library features a high-precision, non-blocking fade engine that allows for fluid brightness transitions without stalling the CPU.

### Easing Curves

The engine supports multiple easing presets to make visual transitions feel more natural.

- **LINEAR**: Consistent rate of change.
- **EASE_IN / EASE_OUT**: Quadratic curves for soft starts or gentle finishes.
- **SINE / EASE_IN_OUT**: Advanced S-curves for organic, "breathing" transitions.

### Flicker-Free Transitions

If a new `fade()` or `write()` command is issued while the LED is already transitioning, the engine captures the **current instantaneous brightness** and calculates the new trajectory from that exact point. This prevents sudden brightness jumps or "popping".
