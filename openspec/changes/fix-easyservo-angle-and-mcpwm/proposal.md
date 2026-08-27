## Why

The `EasyServo` module in `ESP32_EasyKit` fails to drive physical RC servos properly due to four root causes:
1. `maxAngle` is configured to `360.0f` by default instead of the standard Arduino `180.0f`, compressing the standard $0^\circ \dots 180^\circ$ angular range into $500\,\mu\text{s} \dots 1500\,\mu\text{s}$ (mapping $90^\circ$ center to $1000\,\mu\text{s}$ extreme left and sweeping entirely into the lower mechanical end-stop).
2. `attach(pin, config)` does not update the instance's `_config`, causing conversion mismatches between angle and microsecond domains during `writeMicroseconds()` and `_applyDuty()`.
3. Initial comparator value is written before `mcpwm_timer_enable` and `mcpwm_timer_start_stop`, leaving hardware shadow registers unlatched on initialization.
4. `attach(pin, minUs, maxUs)` does not recompute `centerUs`.

Fixing these issues restores full standard RC servo compatibility across 0°–180° sweeps and precise microsecond positioning.

## What Changes

- **Default Angle Range Correction**: Update default `minAngle` to `0.0f` and `maxAngle` to `180.0f` in `ServoConfig` (`pwm_types.h`) and `EasyServo` constructors (`EasyServo.cpp`).
- **Configuration Synchronization**: Update `_config = config;` inside `attach(pin, config)` so that angle limits and pulse boundaries match across `PinEntry` and `_config`.
- **Center Pulse Calculation**: Ensure `attach(pin, minUs, maxUs)` computes `centerUs = (minUs + maxUs) / 2`.
- **Timer Start Sequencing**: Ensure `mcpwm_timer_enable` and `mcpwm_timer_start_stop` run before writing initial compare values and starting generator output.
- **Example Validation**: Update `EasyServo_Basic.ino` to include power enable latching guidance and standard 0°–180° sweep parameters.

## Capabilities

### New Capabilities
<!-- None -->

### Modified Capabilities
- `easykit-servo`: Correct default angular scale to 0°–180° with 90° center mapping to 1500 µs, synchronize per-instance config on attach, and ensure reliable hardware latching.

## Impact

- `ESP32_EasyKit/src/common/pwm_types.h`: `ServoConfig.maxAngle` default changed from `360.0f` to `180.0f`.
- `ESP32_EasyKit/src/EasyServo.h` & `EasyServo.cpp`: Constructor defaults, `attach()`, `setEndpoints()`, and timer sequencing.
- `ESP32_EasyKit/examples/EasyServo_Basic/EasyServo_Basic.ino`: Cleaned up to reflect standard 0°–180° / 1000–2000 µs sweep.
- Downstream projects (`RC_brain`): Full standard compatibility restored without requiring custom angle workarounds.
