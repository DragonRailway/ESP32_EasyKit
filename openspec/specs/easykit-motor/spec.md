# easykit-motor Specification

## Purpose

Defines the `EasyMotor` class behavior of the ESP32_EasyKit library: MCPWM-based brushed DC motor control, forward/reverse/brake/coast drive modes, and ultrasonic PWM frequency maintenance across CPU clock frequencies.

## Requirements

### Requirement: Ultrasonic Motor PWM Carrier Frequency
`EasyMotor` SHALL maintain its configured PWM carrier frequency (defaulting to 20 kHz ultrasonic operation) across all supported CPU clock frequencies (80 MHz, 160 MHz, and 240 MHz) so that motor drive remains silent and smooth regardless of CPU clock frequency scaling.

#### Scenario: 20 kHz ultrasonic operation at 80 MHz CPU
- **WHEN** an `EasyMotor` is initialized at default 20 kHz carrier frequency while the CPU is clocked at 80 MHz
- **THEN** the MCPWM timer resolution is compensated such that the output PWM frequency is 20 kHz ± 100 Hz without human-audible coil whine

#### Scenario: Silent operation across 160 MHz and 240 MHz
- **WHEN** the CPU is clocked at 160 MHz or 240 MHz
- **THEN** the output PWM carrier frequency matches the configured frequency without distortion
