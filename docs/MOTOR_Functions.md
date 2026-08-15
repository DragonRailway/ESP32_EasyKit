# EasyMotor Functions Reference

> This is a high-precision, direct-drive motor control engine powered by ESP32 MCPWM.

The `EasyMotor` class is optimized for various DC motor architectures, providing low-latency control of speed, direction, and braking.

```cpp
// Explicit detachment (Default)
EasyMotor motor;

// Convenience instantiation (Auto-attach pins)
EasyMotor motor(pin1, pin2);
EasyMotor motor(pin1, pin2, enablePin);

// Convenience instantiation (Explicit Type)
EasyMotor motor(EasyMotor::DriverType::DRIVER_1PWM_1DIR, pin1, pin2);
EasyMotor motor(EasyMotor::DriverType::DRIVER_2PWM, pin1, pin2, enablePin);
```

---

## Setup

### begin()

Initializes hardware pins and allocates MCPWM resources. This function supports various driver architectures via a unified interface and returns an `EasyKit::Result` you can check with `EasyKit::ok()`.

**Syntax**

```cpp
// Implicit DRIVER_2PWM Mode (Default)
motor.begin(pin1, pin2);              // returns EasyKit::Result
motor.begin(pin1, pin2, enablePin);
motor.begin(pin1, pin2, invert);
motor.begin(pin1, pin2, enablePin, invert);

// Explicit Mode Selection
motor.begin(TYPE, pin1, pin2);
motor.begin(TYPE, pin1, pin2, enablePin);
motor.begin(TYPE, pin1, pin2, invert);
motor.begin(TYPE, pin1, pin2, enablePin, invert);

// Full Config Struct
motor.begin(EasyKit::EasyMotorConfig{ ... });
```

**Parameters**

- **TYPE (DriverType)**: The hardware architecture. Defaults to `DRIVER_2PWM`.

  | Type | Description |
  | --- | --- |
  | `DRIVER_2PWM` | **Default.** H-Bridge style (e.g. TB6612). Uses two PWM pins. |
  | `DRIVER_1PWM_1DIR` | Speed (PWM) + Direction (Digital) style (e.g. Cytron). |
  | `DRIVER_1PWM` | Unidirectional style. Single PWM pin. |

- **pin1 / pin2 (int)**: The GPIO numbers for the motor driver inputs.
- **enablePin (int)**: Optional. Hardware enable pin (Active HIGH).
- **invert (bool)**: Optional. If `true`, logically inverts the motor direction. Defaults to `false`.

**Returns**: `EasyKit::Result` — `OK`, or an error (`ERR_NO_FREE_OPERATOR`, `ERR_HW_FAULT`, `ERR_INVALID_PIN`, …).

### end()

Releases all MCPWM resources, pins, and the allocated operator slot. Safe to call repeatedly.

```cpp
motor.end();
```

---

## Configuration

### setDirection()

Inverts the logical direction of the motor. Useful for correcting wiring polarity in software.

**Syntax**

```cpp
motor.setDirection(forward);  // bool: true = forward, false = reversed
```

### setFrequency() / setResolution()

```cpp
motor.setFrequency(20000);   // Hz — 20 kHz recommended to avoid audible whine
motor.setResolution(10);     // bit depth (default 10 MHz base timer)
```

### Duty Limits

Clamps the output magnitude so speed commands stay inside a safe window (e.g. 20%–90%).

```cpp
motor.setMinDuty(20.0f);   // percent (0–100)
motor.setMaxDuty(90.0f);   // percent (0–100)
float min = motor.getMinDuty();
float max = motor.getMaxDuty();
```

### setDeadtime()

Configures dead-time (in microseconds) to prevent "shoot-through" in drivers during direction changes.

**Syntax**

```cpp
motor.setDeadtime(0.5f);   // microseconds
float dt = motor.getDeadTime();
```

---

## Control

### write()

The primary function for controlling motor speed and direction. Commands are processed **instantly** on the hardware comparators.

**Syntax**

```cpp
motor.write(speed);
```

**Parameters**

- **speed (float)**: The target speed from **-100.0f (Full Reverse)** to **100.0f (Full Forward)**.
- *Note: Always use the `f` suffix for float literals to utilize the hardware FPU.*

### stop() / brake() / coast()

Controls how the motor stops.

```cpp
motor.stop();   // Alias for write(0)
motor.brake();  // Active brake (Short-circuits the motor for immediate stop)
motor.coast();  // Passive stop (Allows the motor to free-wheel)
```

### Legacy / Discovery API

The classic Arduino-style API is also available:

```cpp
motor.run(EasyKit::MotorDir::FORWARD, 128);   // dir enum + 0–255 speed
motor.forward(128);                            // 0–255
motor.backward(128);                           // 0–255
```

### Read-back

```cpp
float speed = motor.getSpeed();   // last commanded speed (-100.0f .. 100.0f)
```

---

## Hardware Notes

- **PWM Frequency**: Higher frequencies (e.g., 20000Hz) are recommended to eliminate audible motor whine.
- **Dead-time**: Configured in microseconds to prevent shoot-through during direction changes.
- On chips without MCPWM (ESP32-S2, ESP32-C3, ESP32-C2), `begin()` returns `ERR_HW_FAULT`.
