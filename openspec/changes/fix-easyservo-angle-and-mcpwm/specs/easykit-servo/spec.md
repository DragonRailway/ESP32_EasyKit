## MODIFIED Requirements

### Requirement: EasyServo Pulse Timing Correctness

`EasyServo` SHALL generate servo pulse widths in microseconds accurately: a `writeMicroseconds(1500)` call SHALL produce a 1500 µs high pulse at the configured refresh rate. A `write(90)` call on a default instance (0°–180° range) SHALL produce the center pulse (1500 µs).

#### Scenario: Microsecond pulse accuracy
- **WHEN** a servo is attached at 50 Hz and commanded to 1500 µs
- **THEN** the MCPWM output produces a 1500 µs pulse every 20 ms

#### Scenario: Angle write center mapping
- **WHEN** a servo is attached with default range (0°–180°, 500–2500 µs) and commanded to `write(90)`
- **THEN** the output produces a 1500 µs pulse

#### Scenario: Frequency reconfiguration
- **WHEN** `setFrequency()` is called on an attached `EasyServo`
- **THEN** the underlying MCPWM timer period is reconfigured to match the new frequency

### Requirement: Per-pin pulse configuration

Each attached pin SHALL retain its own pulse boundaries captured at attach time (minUs/maxUs/centerUs). Instance-level `_config` SHALL synchronize with attached configuration parameters so angle-to-microsecond conversions match. Instance-level `setEndpoints()` SHALL update the pulse mapping for all attached pins; `setFrequency()` SHALL apply to the shared timer.

#### Scenario: Pins with different ranges
- **WHEN** pin 35 is attached with endpoints (500, 2500) and pin 36 with (1000, 2000)
- **THEN** a single `write()` moves each pin within its own range, and `readMicroseconds()` reflects the last-attached pin's range

#### Scenario: Custom attach endpoints
- **WHEN** `attach(5, 1000, 2000)` is called
- **THEN** `centerUs` is automatically computed as 1500 µs and `write(90)` outputs 1500 µs
