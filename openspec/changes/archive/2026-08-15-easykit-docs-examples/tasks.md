## 1. Multi-pin EasyServo source

- [x] 1.1 Add a per-pin entry struct in `src/EasyServo.h` (pin, MCPWM comparator handle, generator handle, `minUs`/`maxUs`/`centerUs`) and a bounded container (max 3 entries) replacing the single `_pin`/`_cmpr`/`_gen`/`_config` fields
- [x] 1.2 Refactor `attach(pin)` / `attach(pin, config)` to add a new pin entry without detaching existing pins, reconfigure an already-attached pin, and return `0`/non-OK with partial cleanup when the 3-pin limit or MCPWM allocation fails
- [x] 1.3 Refactor `detach(pin)` to release only that pin's generator/comparator + pin claim; `detach()` releases all pins and the shared operator slot
- [x] 1.4 Update `attached()`, `attachedPins()`, and `numAttached()` to reflect multi-pin state (bitmask over all pins; total pins across instances)
- [x] 1.5 Drive every attached pin in `write()`, `writeMicroseconds()`, `update()`, `stop()`, `sleep()`, `wake()` via a per-pin `_applyDuty()` that maps the shared current angle to each pin's own µs range
- [x] 1.6 Make `setEndpoints()` update all pins' pulse mapping, `setFrequency()` update the shared timer, and keep `setSpeed()`/easing instance-wide
- [x] 1.7 Update `onPinStolen()` to detach only the stolen pin
- [x] 1.8 Verify single-pin behavior is unchanged (attach returns pin/0, `attached()`, one-pin read-back) so existing consumers compile against the same API

## 2. Documentation

- [x] 2.1 Rewrite `README.md`: ESP32_EasyKit branding, fix `docs/` links (`GENERAL.md`, `MOTOR_Functions.md`), add `EasyLEDGroup.md` link, keep accurate quick-start snippets
- [x] 2.2 Fix `docs/GENERAL.md`: retitle, fix `DCMOTOR_Functions.md` cross-link, keep FPU guide
- [x] 2.3 Update `docs/LED_Functions.md` to the full `EasyLED` API (fix `EasyLED led;` trap, `EasyKit::` scoping, add missing methods)
- [x] 2.4 Update `docs/MOTOR_Functions.md` to the full `EasyMotor` API (Result returns, `stop`/`end`, duty limits, legacy run/forward/backward)
- [x] 2.5 Update `docs/SERVO_Functions.md` to the full `EasyServo` API (angle/µs semantics, multi-pin, missing methods, `Servo` alias)
- [x] 2.6 Add `docs/EasyLEDGroup.md` covering members, `startPattern`, `alternate`, `syncFlash`, `chase`, `doubleStrobe`, `stop`, `update`, `isRunning`

## 3. Examples

- [x] 3.1 Fix `examples/EasyLED_Basic`: replace `write(ON)`/`write(OFF)` with `write(true/false)` or `on()/off()`, update stale `@file` header
- [x] 3.2 Fix `examples/ServoBuildTest`: valid S3 GPIOs (e.g. 35/36), true multi-pin attach, correct prints
- [x] 3.3 Audit remaining examples (`EasyLED_Blink`, `EasyLED_Fade`, `EasyLED_MultiPin`, `EasyMotor_Basic`, `EasyServo_Basic`) for stale branding, invalid pins, and API drift
- [x] 3.4 Add `examples/EasyLEDGroup_Basic` demonstrating `alternate` then `chase`

## 4. Hygiene

- [x] 4.1 Delete all `examples/*/build/` directories (stale upstream arduino-cli artifacts)
- [x] 4.2 Add `examples/**/build/` to `.gitignore`

## 5. Verification

- [x] 5.1 Best-effort arduino-cli compile of the example sketches against the in-repo library (manual if cli unavailable)
- [x] 5.2 Grep-check: no `ESP32_PWM_Fusion` in `README.md` + `docs/`, no broken relative `docs/` links, no `write(ON)`/`write(OFF)` in examples
- [x] 5.3 Optional: build a consuming project (e.g. RC_brain at `../RC_brain`) to confirm the single-pin API stays source-compatible
