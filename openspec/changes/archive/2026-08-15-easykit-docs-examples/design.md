## Context

This is the standalone ESP32_EasyKit library repository (extracted from the RC_brain project, which consumes it as a sibling repo at `../ESP32_EasyKit`). The `docs/` and `examples/` folders were carried over from the upstream ESP32_PWM_Fusion library and describe an API that no longer matches the current `src/`.

The library source has diverged from upstream in a few ways:
- `EasyLEDGroup` was added (not present upstream) — used for ditch-light-style alternation patterns.
- `EasyServo` was documented/intended for multi-pin attach (`attachedPins()`, `ServoBuildTest` example), but the implementation only supports **one pin per instance** (`_pin` field, attach→detach of the previous pin, single MCPWM slot).
- Docs still reference the old "ESP32_PWM_Fusion" branding and non-existent files (`INTRODUCTION.md`, `DCMOTOR_Functions.md`).

Constraints:
- The single-pin `EasyServo` behavior must stay compatible so existing consumers (e.g. RC_brain's steering/ESC path) build and behave unchanged.
- Arduino example sketches live under `examples/<Name>/<Name>.ino`; they are built with arduino-cli (present in `~/.arduino15`), not PlatformIO.
- `library.json` already excludes `build` from packaging; `.gitignore` has a root-level `build/` rule but nothing for `examples/*/build/`.

## Goals / Non-Goals

**Goals:**
- Make the in-repo docs an accurate reference for the actual `src/` API: EasyLED, EasyMotor, EasyServo, EasyLEDGroup.
- Fix all broken cross-links and stale branding in README + docs.
- Implement **genuine multi-pin servo attach** in `EasyServo` so the `attachedPins()` API and `ServoBuildTest` example are honest.
- Make every example compile against the current API and remove stale `build/` artifacts.
- Add an EasyLEDGroup example and reference doc.
- Establish `.gitignore` coverage so `examples/*/build/` never gets committed.

**Non-Goals:**
- No changes to EasyLED / EasyMotor / EasyLEDGroup runtime behavior (docs/examples only for those).
- No new features beyond multi-pin servo (no soft-servo, no new easing modes, no API renames).
- No changes to consumers' firmware logic (RC_brain or others).
- Not publishing the library to Arduino registry / keeping upstream sync.

## Decisions

### D1. Multi-pin EasyServo: shared timer/operator, per-pin comparator+generator
Each MCPWM operator supports 3 comparators and 3 generators. A multi-pin `EasyServo` instance keeps **one timer + one operator** (one `MCPWMManager` slot) and allocates a **comparator + generator pair per attached pin**, capped at 3 pins per instance (hardware limit). This mirrors how the current single-pin code already allocates, just generalized.

- `attach(pin, config)` adds a new pin entry or reconfigures an existing one; no longer detaches a previous pin.
- `attachedPins()` returns the bitmask over all pins.
- `detach(pin)` releases one pin; `detach()` releases all + the operator slot.
- `write()/writeMicroseconds()/update()/stop()/sleep()/wake()` drive **all** attached pins in lockstep (one animation state per instance — the instance is a "group that moves together").
- `read()/readAngle()/readMicroseconds()` return the state of the most recently attached pin (documented behavior).
- Per-pin config (minUs/maxUs/centerUs) captured at attach; instance-level `setEndpoints()/setSpeed()/setFrequency()` apply to all pins (frequency is a timer property, shared anyway).
- Return of `attach(pin)` stays `pin` on success / `0` on failure; `attach(pin, config)` stays `EasyKit::Result`.

**Alternatives considered:**
- *One slot per pin (multiple operators)*: wastes scarce MCPWM operators (a typical consumer uses 2 motors + 1-2 servos); rejected.
- *Keep single-pin, downgrade docs*: user explicitly chose to implement multi-pin; rejected.
- *Full per-pin animation state*: doubles state complexity for a use case (same-angle sweep) that doesn't need it; rejected — documented as instance-lockstep.

### D2. `ServoConfig` becomes per-pin
The current code stores one `_config` on the instance and mutates it on every attach. With multi-pin, each pin keeps its own `minUs/maxUs/centerUs` copy (allocated from the attach-time config), so pins can have different pulse ranges. `setEndpoints()` updates all pins' pulse mapping.

### D3. `ON`/`OFF` fix in `EasyLED_Basic`
`write(ON)` / `write(OFF)` reference symbols that exist nowhere (not in the lib, not in ESP32 core — only `ONLOW/ONHIGH`, which are pin-mode constants). Replace with `led1.write(true)` / `led1.write(false)` (or `led1.on()` / `led1.off()`), keeping the "digital HIGH/LOW" teaching point.

### D4. ServoBuildTest stays as a real multi-pin test
After D1 it can genuinely attach 2+ servos on one instance. Keep the sketch (it tests `attachedPins()`, `setSpeed()`, `update()`, `readAngle()`) but fix its pins to valid S3 GPIOs (current `#define SERVO1 1` / `SERVO2 2` are strapping pins on ESP32-S3 — move to e.g. 35/36 which the other examples already use) and verify the print.

### D5. EasyLEDGroup doc + example
New `docs/EasyLEDGroup.md` covering members, `startPattern`, `alternate`, `syncFlash`, `chase`, `doubleStrobe`, `stop`, `update`, `isRunning`. New example `examples/EasyLEDGroup_Basic` demonstrating `alternate` (ditch-light style) then `chase`. README index gains the new links.

### D6. Documentation structure
- `README.md` = navigation hub + quick-start (fixed links, ESP32_EasyKit branding, `docs/GENERAL.md` instead of `INTRODUCTION.md`).
- `docs/GENERAL.md` = overview + FPU optimization guide (retitled, cross-link fixed).
- Keep the per-class reference files (`LED_/MOTOR_/SERVO_Functions.md`), each updated to full actual API surface, with a `EasyLEDGroup.md` added.
- `docs/` gets an index section in README only (no extra `INDEX.md` file — one hub is enough).

### D7. Build-artifact hygiene
Delete all `examples/*/build/` directories (stale upstream artifacts, ~396 MB). Add `examples/**/build/` to `.gitignore`. Keep the library's `export.exclude` `build` rule. Optionally verify examples compile with arduino-cli using the installed 3.3.7 core (best-effort; the fqbn used upstream was `esp32:esp32:esp32s3`).

## Risks / Trade-offs

- [MCPWM resource exhaustion] → Per-pin cap of 3 + single operator slot; `attach()` returns `ERR_NO_FREE_OPERATOR` / `ERR_HW_FAULT` and cleans up partial state on failure.
- [Multi-pin changes consumer-visible behavior] → Existing consumers attach exactly one pin per servo instance, so their behavior is identical; verify single-pin compatibility against the headers and, when available, a consumer build.
- [Examples can't be CI-verified here] → arduino-cli isn't on PATH in this environment; mark example compile-check as best-effort manual step, rely on careful API matching against headers.
- [Docs drift again later] → README becomes the single hub; examples/docs kept adjacent to src so diffs surface together.
