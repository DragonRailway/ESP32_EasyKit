## ADDED Requirements

### Requirement: Multi-pin attach on a single EasyServo instance
`EasyServo` SHALL support attaching multiple GPIO pins to one instance. Attaching a second pin SHALL NOT detach the first. The instance SHALL share one MCPWM timer and operator across its pins and allocate a comparator+generator pair per pin, up to the hardware generator limit (3 per operator). `attachedPins()` SHALL return a bitmask covering all attached pins.

#### Scenario: Two pins attached
- **WHEN** `attach(35)` is called, then `attach(36)` on the same `EasyServo` instance, both succeeding
- **THEN** `attachedPins()` returns a bitmask with bits 35 and 36 set, and `numAttached()` counts both pins

#### Scenario: Attach past hardware limit
- **WHEN** attaching a 4th pin to an instance already holding 3 pins
- **THEN** the attach fails (returns `0` from the int overload / non-OK `EasyKit::Result` from the config overload) and the previously attached pins remain attached

#### Scenario: Detach one pin
- **WHEN** `detach(35)` is called on an instance with pins 35 and 36
- **THEN** pin 35's output is released, pin 36 remains attached, and `attachedPins()` no longer has bit 35 set

### Requirement: Instance-wide command semantics
All position commands and animation controls on a multi-pin instance SHALL drive every attached pin in lockstep from a single shared animation state: `write()`, `writeMicroseconds()`, `update()`, `stop()`, `sleep()`, and `wake()` affect all attached pins. Read-back methods (`read()`, `readAngle()`, `readMicroseconds()`) SHALL return the state of the most recently attached pin.

#### Scenario: All pins move together
- **WHEN** an instance with pins 35 and 36 receives `write(90.0f, 60.0f, 0.2f, 0.8f)` followed by loop `update()` calls
- **THEN** both pins emit the same eased pulse trajectory toward the 90° target

#### Scenario: Sleep silences all pins
- **WHEN** `sleep()` is called on a multi-pin instance
- **THEN** no attached pin emits a PWM signal, and `wake()` restores signal on all pins

### Requirement: Per-pin pulse configuration
Each attached pin SHALL retain its own pulse boundaries captured at attach time (minUs/maxUs/centerUs). Instance-level `setEndpoints()` SHALL update the pulse mapping for all attached pins; `setFrequency()` SHALL apply to the shared timer.

#### Scenario: Pins with different ranges
- **WHEN** pin 35 is attached with endpoints (500, 2500) and pin 36 with (1000, 2000)
- **THEN** a single `write()` moves each pin within its own range, and `readMicroseconds()` reflects the last-attached pin's range

### Requirement: Single-pin behavior unchanged
For instances with exactly one attached pin, behavior SHALL remain compatible with the current single-pin API: `attach(pin)` returns the pin on success or 0 on failure, `attached()` reflects attachment, and existing consumer usage (one pin per servo instance) behaves identically.

#### Scenario: Existing consumer builds unchanged
- **WHEN** a consuming project that attaches exactly one pin per `EasyServo` instance compiles against the updated library
- **THEN** the build succeeds and existing steering/ESC servo behavior is unchanged
