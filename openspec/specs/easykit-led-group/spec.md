# easykit-led-group Specification

## Purpose

Defines the `EasyLEDGroup` class API of the ESP32_EasyKit library: coordinated multi-LED step-sequencing on a shared `millis()` timeline, built-in pattern factories, duty ownership while a pattern runs, and clean stop semantics. Firmware-consumer requirements (how a vehicle controller wires the group) live in the consuming project's own OpenSpec repository.

## Requirements

### Requirement: Coordinated multi-LED patterns via a step-sequencer group

The EasyKit library SHALL provide an `EasyLEDGroup` class that owns multiple
`EasyLED` members and plays a timed pattern — a table of steps, each a
per-member duty vector (0–100%) plus a duration in ms — on a shared
`millis()` timeline, repeating until stopped. The group SHALL be polled via
an `update()` method (called from the main loop alongside the per-LED
engines) and SHALL provide built-in pattern factories: `alternate`,
`syncFlash`, `chase`, and `doubleStrobe`.

#### Scenario: Alternating pair (ditch lights)

- **WHEN** a group of two LEDs starts the `alternate(8)` pattern
- **THEN** LED A is at full duty for 8 ms while LED B is off, then LED B is
  full for 8 ms while LED A is off, repeating (~60 alternations per second)

#### Scenario: Sync flash

- **WHEN** a group starts `syncFlash(500, 500)`
- **THEN** all members light together for 500 ms, then all are off for
  500 ms, repeating

#### Scenario: Chase

- **WHEN** a three-member group starts `chase(60)`
- **THEN** exactly one member is lit at a time, moving from first to last
  every 60 ms, repeating

### Requirement: Pattern duty ownership and clean stop

While a pattern is running, the group SHALL own its members' duty (no other
light path may write those pins), and `stop()` SHALL land every member at 0
and mark the group idle, so consumers (battery cutoff, hot-reload teardown)
can halt patterns with a single call.

#### Scenario: Stop lands members off

- **WHEN** `stop()` is called on a running group
- **THEN** all member LEDs are at 0% duty and the group reports not running

### Requirement: Existing single-LED blink behavior unchanged

The existing per-LED blink modes (SIMPLE, BURST, HEARTBEAT, CANDLE, MORSE) and the fade/breathing engines SHALL behave exactly as before, and the group SHALL be a separate, coexisting abstraction.

#### Scenario: Turn-signal blinking unaffected

- **WHEN** a per-LED blink runs on one member while a group pattern runs on
  other LEDs
- **THEN** both operate independently and correctly
