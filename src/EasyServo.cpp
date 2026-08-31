/**
 * @file EasyServo.cpp
 * @brief EasyServo class implementation using ESP-IDF MCPWM v5.x API.
 */

#include "EasyServo.h"
#include "common/pwm_utils.h"
#include "common/pin_manager.h"
#include <Arduino.h>
#include "driver/mcpwm_prelude.h"
#include "common/mcpwm_manager.h"
#include "common/easing_engine.h"

// 1 MHz timer resolution → 1 tick = 1 µs, so comparator values written in
// microseconds map directly to pulse width. (10 MHz was a 10× pulse-width bug.)
#define SERVO_TIMER_RESOLUTION_HZ 1000000

uint8_t EasyServo::_totalPins = 0;

EasyServo::EasyServo() {
    _config.minAngle = 0.0f;
    _config.maxAngle = 180.0f;
}

EasyServo::EasyServo(int pin) : EasyServo() {
    attach(pin);
}

EasyServo::~EasyServo() {
    if (_attached) detach();
}

// ── Arduino-compatible attach overloads ──────────────────────────────────────

int EasyServo::attach(int pin) {
    EasyKit::Result r = attach(pin, _config);
    return (r == EasyKit::Result::OK) ? pin : 0;
}

int EasyServo::attach(int pin, int minUs, int maxUs) {
    EasyKit::ServoConfig cfg = _config;
    cfg.minUs    = minUs;
    cfg.maxUs    = maxUs;
    cfg.centerUs = (minUs + maxUs) / 2;  // symmetric neutral pulse
    EasyKit::Result r = attach(pin, cfg);
    return (r == EasyKit::Result::OK) ? pin : 0;
}

EasyKit::Result EasyServo::attach(int pin, const EasyKit::ServoConfig& config) {
    if (pin < 0 || pin >= 64) return EasyKit::Result::ERR_INVALID_PIN;

    // Reconfigure an already-attached pin (no new allocation).
    if (PinEntry* existing = _findPin(pin)) {
        existing->minUs    = config.minUs;
        existing->maxUs    = config.maxUs;
        existing->centerUs = config.centerUs;
        _lastPin = pin;
        _config = config;  // keep angle limits + pulse bounds in sync
        _applyDuty();
        return EasyKit::Result::OK;
    }

    if (_pinCount >= MAX_PINS) return EasyKit::Result::ERR_HW_FAULT;

#ifdef SOC_MCPWM_SUPPORTED
    mcpwm_timer_config_t timer_cfg = {};
    mcpwm_operator_config_t oper_cfg = {};
    mcpwm_comparator_config_t cmpr_cfg = {};
    mcpwm_generator_config_t gen_cfg = {};

    // First pin: allocate the shared MCPWM timer + operator.
    if (_timer == nullptr) {
        _slot = EasyKit::MCPWMManager::instance().allocate();
        if (_slot.unit < 0) return EasyKit::Result::ERR_NO_FREE_OPERATOR;

        // Select timer resolution to fit within 16-bit period register (max 65535 ticks)
        _timerResHz = SERVO_TIMER_RESOLUTION_HZ;
        if (config.freq > 0 && (_timerResHz / config.freq) > 65535) {
            _timerResHz = 500000; // 500 kHz (2 µs/tick) for < 16 Hz
        }

        timer_cfg.group_id      = _slot.unit;
#if defined(SOC_MCPWM_SUPPORT_XTAL)
        timer_cfg.clk_src       = MCPWM_TIMER_CLK_SRC_XTAL;
#else
        timer_cfg.clk_src       = MCPWM_TIMER_CLK_SRC_DEFAULT;
#endif
        timer_cfg.resolution_hz = _timerResHz;
        timer_cfg.period_ticks  = _timerResHz / config.freq;
        timer_cfg.count_mode    = MCPWM_TIMER_COUNT_MODE_UP;
        if (mcpwm_new_timer(&timer_cfg, &_timer) != ESP_OK) return EasyKit::Result::ERR_HW_FAULT;

        oper_cfg.group_id = _slot.unit;
        if (mcpwm_new_operator(&oper_cfg, &_oper) != ESP_OK) {
            mcpwm_del_timer(_timer);
            _timer = nullptr;
            EasyKit::MCPWMManager::instance().release(_slot);
            _slot = { -1, -1 };
            return EasyKit::Result::ERR_HW_FAULT;
        }
        if (mcpwm_operator_connect_timer(_oper, _timer) != ESP_OK) {
            mcpwm_del_operator(_oper);
            _oper = nullptr;
            mcpwm_del_timer(_timer);
            _timer = nullptr;
            EasyKit::MCPWMManager::instance().release(_slot);
            _slot = { -1, -1 };
            return EasyKit::Result::ERR_HW_FAULT;
        }
    }

    PinEntry* entry = &_pins[_pinCount];
    mcpwm_cmpr_handle_t cmpr = nullptr;
    mcpwm_gen_handle_t  gen  = nullptr;

    cmpr_cfg.flags.update_cmp_on_tez = true;
    if (mcpwm_new_comparator(_oper, &cmpr_cfg, &cmpr) != ESP_OK) return EasyKit::Result::ERR_HW_FAULT;

    gen_cfg.gen_gpio_num = pin;
    if (mcpwm_new_generator(_oper, &gen_cfg, &gen) != ESP_OK) {
        mcpwm_del_comparator(cmpr);
        return EasyKit::Result::ERR_HW_FAULT;
    }

    // Set actions
    mcpwm_generator_set_action_on_timer_event(gen, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
    mcpwm_generator_set_action_on_compare_event(gen, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpr, MCPWM_GEN_ACTION_LOW));

    EasyKit::GlobalPinManager::claimPin(pin, this);

    // Start the shared timer on the first attach only, BEFORE writing the
    // initial compare value so the hardware latches it on the first TEZ event.
    if (_pinCount == 0) {
        mcpwm_timer_enable(_timer);
        mcpwm_timer_start_stop(_timer, MCPWM_TIMER_START_NO_STOP);
    }

    int initUs = angleToUs(_currentAngle, config.minUs, config.maxUs, config.minAngle, config.maxAngle);
    uint32_t initTicks = ((uint32_t)initUs * _timerResHz) / SERVO_TIMER_RESOLUTION_HZ;
    mcpwm_comparator_set_compare_value(cmpr, initTicks);

    entry->pin      = pin;
    entry->minUs    = config.minUs;
    entry->maxUs    = config.maxUs;
    entry->centerUs = config.centerUs;
    entry->cmpr     = cmpr;
    entry->gen      = gen;
    _pinCount++;
    _totalPins++;
    _lastPin = pin;
    _attached = true;
    _config = config;  // keep angle limits + pulse bounds in sync

    return EasyKit::Result::OK;
#else
    return EasyKit::Result::ERR_HW_FAULT;
#endif
}


bool EasyServo::attached() const {
    return _attached;
}

uint64_t EasyServo::attachedPins() const {
    uint64_t mask = 0;
    for (uint8_t i = 0; i < _pinCount; i++) {
        mask |= (1ULL << _pins[i].pin);
    }
    return mask;
}

void EasyServo::detach(int pin) {
    for (uint8_t i = 0; i < _pinCount; i++) {
        if (_pins[i].pin == pin) {
#ifdef SOC_MCPWM_SUPPORTED
            if (_pins[i].gen)  mcpwm_del_generator(_pins[i].gen);
            if (_pins[i].cmpr) mcpwm_del_comparator(_pins[i].cmpr);
#endif
            EasyKit::GlobalPinManager::releasePin(pin, this);
            // Shift remaining entries down to keep the array compact.
            for (uint8_t j = i; j + 1 < _pinCount; j++) {
                _pins[j] = _pins[j + 1];
            }
            _pinCount--;
            if (_totalPins > 0) _totalPins--;
            _pins[_pinCount] = PinEntry();  // clear the vacated slot

            // Update read-back target to the most recently attached remaining pin.
            _lastPin = (_pinCount > 0) ? _pins[_pinCount - 1].pin : -1;

            if (_pinCount == 0) {
                _attached = false;
                _sleeping = false;
                _moving = false;
#ifdef SOC_MCPWM_SUPPORTED
                if (_timer) mcpwm_timer_start_stop(_timer, MCPWM_TIMER_STOP_FULL);
                if (_timer) mcpwm_timer_disable(_timer);
                if (_oper)  mcpwm_del_operator(_oper);
                if (_timer) mcpwm_del_timer(_timer);
                _oper  = nullptr;
                _timer = nullptr;
                EasyKit::MCPWMManager::instance().release(_slot);
                _slot = { -1, -1 };
#endif
            }
            return;
        }
    }
}

void EasyServo::detach() {
    if (!_attached) return;
    while (_pinCount > 0) {
        detach(_pins[_pinCount - 1].pin);
    }
}

void EasyServo::onPinStolen(uint8_t pin) {
    detach(pin);
}

// ── Position control ─────────────────────────────────────────────────────────

float EasyServo::write(float value, float speed, float kIn, float kOut) {
    if (!_attached) return 0.0f;

    const PinEntry* last = _lastEntry();
    float targetAngle;
    if (value < 500.0f) {
        targetAngle = EasyKit::clampValue(value, _config.minAngle, _config.maxAngle);
    } else {
        targetAngle = usToAngle((int)(value + 0.5f), last->minUs, last->maxUs, _config.minAngle, _config.maxAngle);
    }

    // Capture current velocity for momentum hand-off before resetting move state
    if (_moving) {
        update();
    } else {
        _vNow = 0.0f;
    }

    _startAngle = _currentAngle;
    _targetAngle = targetAngle;

    float moveSpeed = (speed >= 0.0f) ? speed : _speed;
    float ki = _kIn;
    float ko = _kOut;

    if (kIn >= 0.0f) {
        ki = kIn;
        ko = (kOut < 0.0f) ? kIn : kOut;
    }

    if (moveSpeed <= 0.0f) {
        _currentAngle = _targetAngle;
        _moving = false;
        _vNow = 0.0f;
        _applyDuty();
        return 1.0f;
    }

    _kIn = ki;
    _kOut = ko;

    float delta = fabsf(_targetAngle - _startAngle);
    if (delta < 0.01f) {
        _moving = false;
        _vNow = 0.0f;
        return 1.0f;
    }

    _durationMS = (uint32_t)((delta / moveSpeed) * 1000.0f);
    _startTimeMS = millis();
    _lastUpdateMS = _startTimeMS;
    _moving = true;
    _vNow = 0.0f;

    _applyDuty();
    wake();

    return 0.0f;
}

void EasyServo::writeMicroseconds(int us) {
    if (!_attached) return;
    _moving = false;
    const PinEntry* last = _lastEntry();
    _currentUs = EasyKit::clampValue(us, (int)last->minUs, (int)last->maxUs);
    _currentAngle = usToAngle(_currentUs, last->minUs, last->maxUs, _config.minAngle, _config.maxAngle);
    _applyDuty();
}

void EasyServo::update() {
    if (!_attached || !_moving) {
        _vNow = 0.0f;
        return;
    }

    uint32_t elapsed = millis() - _startTimeMS;
    float lastAngle = _currentAngle;

    if (elapsed >= _durationMS) {
        _currentAngle = _targetAngle;
        _moving = false;
        _vNow = 0.0f;
        _applyDuty();
        return;
    }

    float t = (float)elapsed / (float)_durationMS;

    // Apply refined asymmetric sigmoid from unified math library
    float ye = EasyKit::Easing::sigmoid(t, _kIn, _kOut);

    _currentAngle = _startAngle + ye * (_targetAngle - _startAngle);

    // Calculate instantaneous velocity (deg/sec) for momentum hand-off
    uint32_t now = millis();
    float dt = (_lastUpdateMS > 0) ? (float)(now - _lastUpdateMS) / 1000.0f : 0.0f;
    if (dt > 0.0001f) {
        _vNow = (_currentAngle - lastAngle) / dt;
    }
    _lastUpdateMS = now;

    _applyDuty();
}

void EasyServo::stop() {
    _moving = false;
}

void EasyServo::sleep() {
    if (!_attached) return;
    _sleeping = true;
#ifdef SOC_MCPWM_SUPPORTED
    for (uint8_t i = 0; i < _pinCount; i++) {
        if (_pins[i].gen) mcpwm_generator_set_force_level(_pins[i].gen, 0, true);
    }
#endif
}

void EasyServo::wake() {
    if (!_attached) return;
    _sleeping = false;
#ifdef SOC_MCPWM_SUPPORTED
    for (uint8_t i = 0; i < _pinCount; i++) {
        if (_pins[i].gen) mcpwm_generator_set_force_level(_pins[i].gen, -1, true);
    }
#endif
    _applyDuty();
}

void EasyServo::_applyDuty() {
    if (!_attached || _sleeping) return;
    const PinEntry* last = _lastEntry();

#ifdef SOC_MCPWM_SUPPORTED
    for (uint8_t i = 0; i < _pinCount; i++) {
        int us = angleToUs(_currentAngle, _pins[i].minUs, _pins[i].maxUs, _config.minAngle, _config.maxAngle);
        uint32_t ticks = ((uint32_t)us * _timerResHz) / SERVO_TIMER_RESOLUTION_HZ;
        if (ticks != _pins[i].lastTicks) {
            _pins[i].lastTicks = ticks;
            mcpwm_comparator_set_compare_value(_pins[i].cmpr, ticks);
        }
    }
#endif

    // Read-back reflects the most recently attached pin's mapped pulse.
    _currentUs = angleToUs(_currentAngle, last->minUs, last->maxUs, _config.minAngle, _config.maxAngle);
}

// ── Read back ────────────────────────────────────────────────────────────────

int EasyServo::read() const {
    if (!_attached) return (int)_currentAngle;
    const PinEntry* last = _lastEntry();
    return (int)(usToAngle(_currentUs, last->minUs, last->maxUs, _config.minAngle, _config.maxAngle) + 0.5f);
}

float EasyServo::readAngle() const {
    if (!_attached) return _currentAngle;
    const PinEntry* last = _lastEntry();
    return usToAngle(_currentUs, last->minUs, last->maxUs, _config.minAngle, _config.maxAngle);
}

int EasyServo::readMicroseconds() const {
    return _currentUs;
}

// ── Configuration ────────────────────────────────────────────────────────────

void EasyServo::setEndpoints(int minUs, int maxUs) {
    _config.minUs = minUs;
    _config.maxUs = maxUs;
    for (uint8_t i = 0; i < _pinCount; i++) {
        _pins[i].minUs    = minUs;
        _pins[i].maxUs    = maxUs;
        _pins[i].centerUs = (minUs + maxUs) / 2;
    }
    _applyDuty();
}

void EasyServo::setEndpoints(int minUs, int maxUs, float minAngle, float maxAngle) {
    _config.minUs = minUs;
    _config.maxUs = maxUs;
    _config.minAngle = minAngle;
    _config.maxAngle = maxAngle;
    for (uint8_t i = 0; i < _pinCount; i++) {
        _pins[i].minUs    = minUs;
        _pins[i].maxUs    = maxUs;
        _pins[i].centerUs = (minUs + maxUs) / 2;
    }
    _applyDuty();
}

void EasyServo::setMinMaxUs(int minUs, int maxUs) {
    setEndpoints(minUs, maxUs);
}

void EasyServo::setSpeed(float speed, float kIn, float kOut) {
    if (speed >= 0.0f) _speed = speed;
    if (kIn >= 0.0f) {
        _kIn = kIn;
        _kOut = (kOut < 0.0f) ? kIn : kOut;
    }
}

void EasyServo::setFrequency(uint16_t hz) {
    if (hz == 0) return;
    _config.freq = hz;
#ifdef SOC_MCPWM_SUPPORTED
    if (_timer) {
        uint32_t ticks = _timerResHz / hz;
        if (ticks <= 65535) {
            mcpwm_timer_set_period(_timer, ticks);
        }
    }
#endif
}

// ── Static utilities ─────────────────────────────────────────────────────────

float EasyServo::usToAngle(int us, int minUs, int maxUs, float minAngle, float maxAngle) {
    float clampedUs = (float)EasyKit::clampValue(us, minUs, maxUs);
    return minAngle + (clampedUs - minUs) * (maxAngle - minAngle) / (float)(maxUs - minUs);
}

int EasyServo::angleToUs(float angle, int minUs, int maxUs, float minAngle, float maxAngle) {
    float clampedAngle = EasyKit::clampValue(angle, minAngle, maxAngle);
    return minUs + (int)((clampedAngle - minAngle) / (maxAngle - minAngle) * (maxUs - minUs) + 0.5f);
}

float EasyServo::usToAngle(int us, int minUs, int maxUs) {
    return usToAngle(us, minUs, maxUs, 0.0f, 180.0f);
}

int EasyServo::angleToUs(float angle, int minUs, int maxUs) {
    return angleToUs(angle, minUs, maxUs, 0.0f, 180.0f);
}

uint8_t EasyServo::numAttached() {
    return _totalPins;
}

// ── Private helpers ──────────────────────────────────────────────────────────

EasyServo::PinEntry* EasyServo::_findPin(int pin) {
    for (uint8_t i = 0; i < _pinCount; i++) {
        if (_pins[i].pin == pin) return &_pins[i];
    }
    return nullptr;
}

const EasyServo::PinEntry* EasyServo::_lastEntry() const {
    return (_pinCount > 0) ? &_pins[_pinCount - 1] : nullptr;
}
