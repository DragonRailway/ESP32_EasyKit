# easykit-docs-examples Specification

## Purpose

Defines the documentation and examples requirements for the ESP32_EasyKit library: an accurate, self-consistent reference for the actual `src/` API, correct branding and cross-links, honest compiling examples, and no stale build artifacts in the repo.

## Requirements

### Requirement: Docs accurately reference the in-repo API

The library documentation SHALL describe only APIs that exist in the in-repo `src/`, using the ESP32_EasyKit branding. Every relative link between README and docs SHALL resolve to an existing file, with `docs/GENERAL.md` (not `INTRODUCTION.md`) and `docs/MOTOR_Functions.md` (not `DCMOTOR_Functions.md`) as the canonical names.

#### Scenario: No broken links in README

- **WHEN** walking every markdown link in `README.md` to a `docs/` file
- **THEN** each link resolves to an existing file

#### Scenario: No stale branding

- **WHEN** searching `README.md` and `docs/` for the string "ESP32_PWM_Fusion"
- **THEN** no occurrences remain (all branding reads ESP32_EasyKit)

### Requirement: EasyLED / EasyMotor / EasyServo reference completeness

Each per-class reference doc (`docs/LED_Functions.md`, `docs/MOTOR_Functions.md`, `docs/SERVO_Functions.md`) SHALL document the full public API of its class as declared in `src/`, including the `EasyKit::` namespace qualification on enums and config types, correct overload signatures, and correct value semantics (e.g. EasyServo `write()` treating values `< 500` as angles and `>= 500` as microseconds).

#### Scenario: LED doc matches header

- **WHEN** comparing every public method of `EasyLED` in `src/EasyLED.h` against `docs/LED_Functions.md`
- **THEN** every method is documented with a matching signature, and no documented method is absent from the header

#### Scenario: Motor doc matches header

- **WHEN** comparing every public method of `EasyMotor` in `src/EasyMotor.h` against `docs/MOTOR_Functions.md`
- **THEN** every method is documented with a matching signature, including `EasyKit::Result` returns and duty-limit getters/setters

#### Scenario: Servo doc matches header

- **WHEN** comparing every public method of `EasyServo` in `src/EasyServo.h` against `docs/SERVO_Functions.md`
- **THEN** every method is documented with a matching signature, and the angle/microsecond write semantics are stated correctly

#### Scenario: No vexing-parse traps

- **WHEN** reading the LED constructor examples in `docs/LED_Functions.md`
- **THEN** default construction is shown as `EasyLED led;` (never `EasyLED led();`)

### Requirement: EasyLEDGroup reference doc and example

The library SHALL include `docs/EasyLEDGroup.md` documenting the group API (`addMember`, `clearMembers`, `startPattern`, `alternate`, `syncFlash`, `chase`, `doubleStrobe`, `stop`, `update`, `isRunning`) and an example sketch `examples/EasyLEDGroup_Basic` demonstrating at least one pattern.

#### Scenario: Group doc exists and is linked

- **WHEN** checking for `docs/EasyLEDGroup.md` and a link to it from the README
- **THEN** both exist

#### Scenario: Group example compiles

- **WHEN** compiling `examples/EasyLEDGroup_Basic/EasyLEDGroup_Basic.ino` against the in-repo library
- **THEN** it compiles successfully and exercises `alternate` or `chase`

### Requirement: Examples compile and are honest

Every example sketch in `examples/` SHALL compile against the in-repo `src/` without undefined symbols (e.g. no bare `ON`/`OFF`), SHALL use GPIOs valid for the target boards, and SHALL not demonstrate features the implementation does not provide.

#### Scenario: EasyLED_Basic compiles

- **WHEN** compiling `examples/EasyLED_Basic/EasyLED_Basic.ino`
- **THEN** it compiles successfully (digital writes use `write(true/false)` or `on()/off()`, never undefined `ON`/`OFF`)

#### Scenario: Multi-pin claim is honest

- **WHEN** inspecting `examples/ServoBuildTest/ServoBuildTest.ino` and the `EasyServo` implementation
- **THEN** any multi-pin attach the example performs is actually supported by the implementation (see `easykit-servo` capability)

### Requirement: No stale build artifacts

The `examples/` tree SHALL contain no `build/` directories or compiled artifacts, and `.gitignore` SHALL prevent `examples/**/build/` from being committed.

#### Scenario: Examples tree is clean

- **WHEN** listing `examples/`
- **THEN** no `build/` directories exist under it

#### Scenario: Artifacts stay ignored

- **WHEN** running `git status` after adding an example build output under `examples/`
- **THEN** the artifact is ignored by `.gitignore`
