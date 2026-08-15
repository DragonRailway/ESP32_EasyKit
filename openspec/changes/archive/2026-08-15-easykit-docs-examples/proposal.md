## Why

The `docs/` and `examples/` folders in this repo were carried over from the upstream library and describe a different API than the current `src/`. The README links to `docs/INTRODUCTION.md` and `docs/DCMOTOR_Functions.md` (which don't exist — the actual files are `GENERAL.md` and `MOTOR_Functions.md`), the docs still use the old "ESP32_PWM_Fusion" branding, `EasyLEDGroup` (the ditch-light pattern feature) has no reference doc and no example, and two examples would not compile or would mislead against the current source (`EasyLED_Basic` uses an undefined `ON`/`OFF`; `ServoBuildTest` implies multi-pin attach that the implementation doesn't support). Each example also carries ~40–75 MB of stale arduino-cli `build/` artifacts from the upstream path.

## What Changes

- **Rewrite `README.md`**: correct title to ESP32_EasyKit, fix all broken doc links (`INTRODUCTION.md` → `GENERAL.md`, `DCMOTOR_Functions.md` → `MOTOR_Functions.md`), keep the accurate quick-start snippets.
- **Fix `docs/GENERAL.md`**: retitle, fix the `DCMOTOR_Functions.md` cross-link.
- **Update `docs/LED_Functions.md`** to match `EasyLED.h`: fix the `EasyLED led();` most-vexing-parse, add `EasyKit::` scope to `LEDCResolution`, and document the missing API surface (`begin(pin, config)`, `on()/off()`, `analogWrite()`, duty getters/setters, `blinkN()`, blink/fade status & stop controls, `end()/detach()/isAttached()`).
- **Update `docs/MOTOR_Functions.md`** to match `EasyMotor.h`: document `EasyKit::Result` returns, `stop()/end()`, `setResolution()`, duty limits & getters, legacy `run()/forward()/backward()`.
- **Update `docs/SERVO_Functions.md`** to match `EasyServo.h`: correct the angle/µs write semantics (value `< 500` is angle, `≥ 500` is µs; not "-360..360"), document `writeMicroseconds()`, `read()`, `attached()`, `detach()`, `numAttached()`, the `Servo` alias + `Servo.h` compat header, and the config-struct `attach()`.
- **Add `docs/EasyLEDGroup.md`**: full reference for `EasyLEDGroup` (members, `startPattern`, `alternate`, `syncFlash`, `chase`, `doubleStrobe`, `stop`, `update`).
- **Fix `examples/EasyLED_Basic`**: replace undefined `ON`/`OFF` with valid `write(true/false)` (or `on()/off()`), update the stale `@file` header.
- **Rework `examples/ServoBuildTest`** as a real multi-pin test against the new `easykit-multipin-servo` capability.
- **Clean examples**: remove the ~396 MB of `examples/*/build/` arduino-cli artifacts; ensure they stay out of git (gitignore/library.json export already excludes `build`).
- **Add an `EasyLEDGroup` example** demonstrating `alternate`/`chase`/`doubleStrobe`.
- **Add `docs/` index or cross-links** so the README is the single navigation hub.

## Capabilities

### New Capabilities
- `easykit-docs-examples`: Accurate, self-consistent documentation and examples for the ESP32_EasyKit library — correct API reference for EasyLED/EasyMotor/EasyServo/EasyLEDGroup, fixed cross-links and branding, compiling examples with no stale build artifacts.
- `easykit-multipin-servo`: EasyServo supports attaching multiple servo pins to one instance (shared timer/operator, per-pin comparator+generator, `attachedPins()` honest, `detach(pin)` selective) so the API matches its documented intent.

### Modified Capabilities
<!-- No behavior changes to existing capabilities — docs/examples only -->

## Impact

- `README.md` — rewritten.
- `docs/GENERAL.md`, `LED_Functions.md`, `MOTOR_Functions.md`, `SERVO_Functions.md` — updated; new `EasyLEDGroup.md`.
- `examples/*/*.ino` — fixed/updated; new EasyLEDGroup example; `build/` dirs deleted.
- `.gitignore` — `examples/**/build/` entries so artifacts never get committed.
- `src/EasyServo.h` / `src/EasyServo.cpp` — multi-pin attach support (per the `easykit-multipin-servo` capability).
- No changes to EasyLED / EasyMotor / EasyLEDGroup runtime behavior; the single-pin `EasyServo` path stays compatible so existing consumers build unchanged.
