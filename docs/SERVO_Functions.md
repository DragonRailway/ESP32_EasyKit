# EasyServo Functions Reference

> This is a high-precision motion engine powered by ESP32 MCPWM.

A drop-in replacement for the standard Arduino `Servo.h`, but with advanced non-blocking **asymmetric easing curves**, hardware-synchronized updates, and **multi-pin attach** — one instance can drive up to 3 servo pins in lockstep.

```cpp
EasyServo servo;
EasyServo servo(Pin);
```

> [!IMPORTANT]
> **Pure MCPWM Engine:** This library uses the high-resolution MCPWM peripheral (1 MHz base clock → 1 µs/tick) for sub-microsecond pulse width precision. It is available on **ESP32**, **ESP32-S3**, and **ESP32-P4**. On chips without MCPWM (ESP32-S2, ESP32-C3, ESP32-C2), `attach()` will fail.

> [!NOTE]
> **Multi-pin attach:** A single instance may attach up to 3 pins (the MCPWM generator limit per operator). All pins share one timer/operator and one animation state, so `write()`/`update()` drive them together. Read-back reflects the **most recently attached** pin. `Servo` is an alias for `EasyServo`, and `#include <Servo.h>` is a compatibility shim that redirects to `EasyServo.h`.

---

## Setup

### attach()

Associates a GPIO pin with the servo instance.

**Syntax**

```cpp
int  servo.attach(pin);                  // returns pin on success, 0 on failure
int  servo.attach(pin, minUs, maxUs);
EasyKit::Result servo.attach(pin, config);   // config-struct overload
```

**Parameters**

- **pin (int)**: The GPIO number to attach.
- **minUs / maxUs (int)**: Optional. Pulse width limits (default 500-2500).
- **config (EasyKit::ServoConfig)**: `{ minUs, maxUs, centerUs, minAngle, maxAngle, freq }`.

Attaching a pin that is already attached reconfigures it in place. Attaching additional pins adds them (up to 3 total); it never detaches previously attached pins.

### attached() / attachedPins() / detach()

```cpp
bool ok = servo.attached();          // true if at least one pin is attached
uint64_t mask = servo.attachedPins(); // bitmask of all attached pins (bit N = GPIO N)
servo.detach(pin);                   // release one pin
servo.detach();                      // release all pins + hardware
```

### numAttached()

```cpp
uint8_t n = Servo::numAttached();    // total pins attached across all instances
```

---

## Control

### write()

**Description**
Accepts an angle or microsecond value. Supports non-blocking movement with independent easing for start and stop phases.

**Syntax**

```cpp
float servo.write(value);
float servo.write(value, speed, ke);
float servo.write(value, speed, kIn, kOut);
```

**Parameters**

- **value (float)**: Target **angle** (values below `500.0f`) or **microseconds** (`>= 500.0f`). Angles are clamped to the configured angle range (default 0–180°); µs values are converted through the last-attached pin's pulse range.
- **speed (float)**: Movement speed in degrees/second. Use `0.0f` for instantaneous moves.
- **kIn / ke (float)**: Easing strength at start (or for both if kOut omitted).
- **kOut (float)**: Optional. Easing strength for the stop.

**Returns**

- A normalized `float` (0.0f to 1.0f) representing the progress of the current move.

### writeMicroseconds()

Writes an explicit pulse width in microseconds (instantaneous, no easing).

```cpp
servo.writeMicroseconds(1500);   // 1500 µs → center
```

### update()

Processes the internal animation engine. **Must be called repeatedly in your main `loop()`** for non-blocking moves to function.

```cpp
servo.update();
```

### stop()

Abruptly stops the servo at its current physical position. Clears any pending easing movements.

```cpp
servo.stop();
```

### sleep() / wake()

`sleep()` stops the PWM signal (leaving the servo "limp" to save power or prevent jitter). `wake()` resumes the signal.

```cpp
servo.sleep();
servo.wake();
```

---

## Readback

### read() / readAngle() / readMicroseconds()

Returns the current calculated pulse state (of the most recently attached pin on multi-pin instances).

```cpp
int   angle = servo.read();            // integer angle (mapped to custom range)
float angle = servo.readAngle();       // float angle
int   us    = servo.readMicroseconds(); // pulse width in µs
```

---

## Configuration

### setEndpoints()

Configures the relationship between pulses (microseconds) and logical degrees. This allows for fine-tuning physical limits or expanding the range for 270°/360° servos. Applies to **all** attached pins.

```cpp
servo.setEndpoints(minUs, maxUs);
servo.setEndpoints(minUs, maxUs, minAngle, maxAngle);
```

**Parameters**

- **minUs / maxUs (int)**: Pulse width range (e.g., 500, 2500).
- **minAngle / maxAngle (float)**: Logical degree range (e.g., 0.0f, 360.0f).

### setMinMaxUs()

Legacy alias for `setEndpoints(minUs, maxUs)`.

### setSpeed()

Sets the default movement speed and asymmetric easing constants.

```cpp
servo.setSpeed(speed);
servo.setSpeed(speed, ke);
servo.setSpeed(speed, kIn, kOut);
```

**Parameters**

- **speed (float)**: Default degrees/second.
- **kIn / kOut (float)**: Default easing strengths.

### setFrequency()

Adjusts the PWM frequency (default 50Hz). This can be used to optimize response for high-frequency digital servos. Updates the shared MCPWM timer period in place.

```cpp
servo.setFrequency(50);   // Hz
```

---

## Static Utilities

```cpp
// Microseconds → angle
float a = EasyServo::usToAngle(1500, 500, 2500);
float a = EasyServo::usToAngle(1500, 500, 2500, 0.0f, 360.0f);

// Angle → microseconds
int us = EasyServo::angleToUs(90.0f, 500, 2500);
int us = EasyServo::angleToUs(90.0f, 500, 2500, 0.0f, 360.0f);
```

---

## Motion & Easing Details

### Asymmetric Easing
You can tune the "snap" of the start and the "smoothness" of the stop independently.

- **kIn = 0.5f, kOut = 0.5f**: Standard symmetric S-curve (Sigmoid).
- **kIn = 0.0f, kOut = 0.8f**: Instant start with a very soft, long deceleration at the end.
- **kIn = 0.8f, kOut = 0.0f**: "Spooling up" start with a hard finish at the target.

### Momentum Hand-off (Jerk-Free)

If a new `write()` command is issued while the servo is still moving, the engine captures the **current instantaneous velocity**. It then calculates the new trajectory to match that velocity at the start of the next phase. This prevents sudden mechanical "jerks" and ensures fluid transition even during rapid target changes.
