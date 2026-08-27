## Context

The `EasyServo` module was written using ESP-IDF v5 MCPWM APIs to support multi-pin synchronized servo output. However, discrepancies in default angular ranges (0°–360° instead of 0°–180°), unsynchronized instance configuration upon attachment, and premature comparator writing prior to timer enablement cause standard RC servos to fail or jam.

## Goals / Non-Goals

**Goals:**
- Correct default angle range to 0°–180° across all structs, constructors, and helper methods.
- Ensure `write(90)` maps accurately to 1500 µs center pulse.
- Synchronize `_config` with attached pin configurations in `attach(pin, config)` and `attach(pin, minUs, maxUs)`.
- Ensure MCPWM timer starts cleanly before initial duty application.
- Maintain backward compatibility with multi-pin lockstep servo driving.

**Non-Goals:**
- Changing LEDC or Motor drivers.
- Changing easing algorithm math.

## Decisions

### 1. Set default angle range to 0.0f .. 180.0f
- *Rationale*: Standard Arduino `Servo.h` operates on 0°–180° angles with 90° as center. Initializing `maxAngle` to 360.0f distorted standard angle calls.
- *Alternatives*: Keeping 360.0f would require every caller to know to pass custom angles, breaking drop-in Arduino compatibility.

### 2. Synchronize `_config` on `attach(pin, config)`
- *Rationale*: `writeMicroseconds(us)` uses `_config.minAngle / maxAngle` with `PinEntry.minUs / maxUs`. If `_config` is not updated when custom endpoints are supplied, round-trip mapping produces invalid compare values.

### 3. Compute `centerUs` in `attach(pin, minUs, maxUs)`
- *Rationale*: `centerUs = (minUs + maxUs) / 2` ensures symmetric neutral pulse when non-standard ranges (e.g. 1000–2000 µs) are passed.

### 4. Sequence MCPWM timer start before initial duty apply
- *Rationale*: Starting the timer with `mcpwm_timer_enable` and `mcpwm_timer_start_stop` ensures compare registers update synchronously on the first TEZ event.

### 5. Bypass Arduino Core PeriMan GPIO reassignment for MCPWM pins
- *Rationale*: In `arduino-esp32 3.x`, calling `perimanSetPinBus(pin, ESP32_BUS_TYPE_GPIO, ...)` forces the GPIO Matrix output index back to `SIG_GPIO_OUT_IDX`, silently detaching raw ESP-IDF MCPWM generator signals from physical pads. `GlobalPinManager` must exempt MCPWM owners (`MCPWM_SERVO`, `MCPWM_MOTOR`) from generic GPIO PeriMan registration so the hardware matrix routing remains intact.

## Risks / Trade-offs

- [Risk] Existing code expecting 0–360 degree scale might rotate further than before.
  - *Mitigation*: 0–180 is the universal standard for Arduino Servo and RC hobby servos. Code requiring 0–360 can explicitly configure `setEndpoints(minUs, maxUs, 0, 360)`.

