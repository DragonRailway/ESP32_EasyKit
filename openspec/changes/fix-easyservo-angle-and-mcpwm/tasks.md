## 1. Type and Header Definitions

- [x] 1.1 Update `ServoConfig` default `maxAngle` to `180.0f` in `src/common/pwm_types.h`
- [x] 1.2 Update `EasyServo` default `_config.maxAngle` to `180.0f` in `src/EasyServo.h` and `src/EasyServo.cpp`
- [x] 1.3 Ensure default `usToAngle` and `angleToUs` static helper overloads use `0.0f` and `180.0f`

## 2. EasyServo Implementation Fixes

- [x] 2.1 Update `attach(int pin, int minUs, int maxUs)` to compute `centerUs = (minUs + maxUs) / 2`
- [x] 2.2 Update `attach(int pin, const ServoConfig& config)` to synchronize instance `_config = config;`
- [x] 2.3 Ensure `mcpwm_timer_enable` and `mcpwm_timer_start_stop` run cleanly before setting initial compare values
- [x] 2.4 Verify `setEndpoints()` updates both `_pins` array and `_config`

## 3. Example & Hardware Verification

- [x] 3.1 Update `examples/EasyServo_Basic/EasyServo_Basic.ino` with clear 0°–180° sweep parameters and power latching notes
- [x] 3.2 Flash `EasyServo_Basic` and `ServoBuildTest` to hardware on `MIKRO_V2` and verify physical servo sweep across full range

## 4. MCPWM Hardware Output Fix

- [x] 4.1 Reproduce and measure raw-IDF MCPWM signal on GPIO 5 directly via loopback/input sampling
- [x] 4.2 Determine root cause — confirmed: `GlobalPinManager::claimPin` executed `perimanSetPinBus(pin, ESP32_BUS_TYPE_GPIO, ...)`, triggering Arduino Core PeriMan to disconnect the MCPWM Generator matrix routing back to `SIG_GPIO_OUT_IDX`
- [x] 4.3 Apply the fix to `src/common/pin_manager.cpp` (exempt `MCPWM_SERVO` / `MCPWM_MOTOR` from generic GPIO PeriMan reassignment) and clean up generator force level in `src/EasyServo.cpp`
- [x] 4.4 Re-verify multi-pin sweep with `ServoBuildTest` on GPIO 5 and GPIO 6
