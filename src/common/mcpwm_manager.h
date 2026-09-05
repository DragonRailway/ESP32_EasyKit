/**
 * @file mcpwm_manager.h
 * @brief Internal MCPWM operator/unit allocator (singleton).
 *
 * Tracks available MCPWM operator slots across all units.
 * Thread-safe via FreeRTOS mutex.
 */
#pragma once

#include <cstdint>

// Guard: only compile on chips that have MCPWM
#if __has_include("soc/soc_caps.h")
#include "soc/soc_caps.h"
#endif

#ifdef SOC_MCPWM_SUPPORTED

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <Arduino.h>

namespace EasyKit {

/**
 * @brief Computes the compensated resolution_hz for mcpwm_new_timer.
 *
 * On SoCs without XTAL support for MCPWM (such as ESP32-S3 and classic ESP32),
 * the MCPWM peripheral is clocked from PLL_F160M. The ESP-IDF driver assumes a
 * fixed 160 MHz source clock when configuring the timer prescaler divider.
 * When CPU frequency is reduced (e.g., 80 MHz for power savings), the actual
 * peripheral clock drops to match the CPU clock.
 *
 * Scaling resolution_hz by (160 / actual_cpu_mhz) compensates the prescaler
 * so the actual hardware tick rate matches the desired rate.
 */
inline uint32_t getMcpwmTimerResolution(uint32_t desired_res_hz) {
#if defined(SOC_MCPWM_SUPPORT_XTAL)
    // Hardware supports XTAL directly; XTAL is independent of CPU clock frequency.
    return desired_res_hz;
#else
    uint32_t cpu_mhz = getCpuFrequencyMhz();
    if (cpu_mhz == 0 || cpu_mhz >= 160) {
        return desired_res_hz;
    }
    return (uint32_t)((uint64_t)desired_res_hz * 160ULL / cpu_mhz);
#endif
}


class MCPWMManager {
public:
    struct Slot {
        int8_t unit;     ///< MCPWM group index (0 or 1)
        int8_t opIndex;  ///< Operator index within the unit (0–2)
    };

    /// Get the singleton instance.
    static MCPWMManager& instance();

    /// Allocate the next free operator slot.
    /// @return Slot with valid unit/opIndex on success, or {-1, -1} on failure.
    Slot allocate();

    /// Release a previously allocated slot.
    void release(Slot s);

    /// Number of free operator slots remaining.
    uint8_t numFree() const;

    /// Total number of operator slots on this SoC.
    static constexpr uint8_t totalSlots() {
        return SOC_MCPWM_GROUPS * SOC_MCPWM_OPERATORS_PER_GROUP;
    }

private:
    MCPWMManager();
    ~MCPWMManager() = default;
    MCPWMManager(const MCPWMManager&) = delete;
    MCPWMManager& operator=(const MCPWMManager&) = delete;

    SemaphoreHandle_t _mutex;
    uint8_t _used;  ///< Bitmask of allocated slots
};

} // namespace EasyKit

#endif // SOC_MCPWM_SUPPORTED
