/**
 * @file pin_manager.cpp
 * @brief Implementation of GlobalPinManager with Periman integration.
 */

#include "pin_manager.h"
#include <Arduino.h>

namespace EasyKit {

IPinOwner* GlobalPinManager::_owners[64] = {nullptr};

void GlobalPinManager::claimPin(uint8_t pin, IPinOwner* newOwner) {
    if (pin >= 64 || newOwner == nullptr) return;

    // 1. Check for internal library conflicts
    if (_owners[pin] != nullptr && _owners[pin] != newOwner) {
        // The pin is currently owned by another library object. Notify them.
        _owners[pin]->onPinStolen(pin);
    }
    
    // 2. Arduino Core Periman Integration
#ifdef HAS_PERIMAN
    // If the owner manages its own peripheral matrix (e.g. MCPWM), do NOT force ESP32_BUS_TYPE_GPIO
    // because perimanSetPinBus(..., ESP32_BUS_TYPE_GPIO) resets the pin's GPIO matrix out to SIG_GPIO_OUT_IDX.
    const char* periph = newOwner->getPeripheralName();
    bool isMcpwm = (periph && (strcmp(periph, "MCPWM_SERVO") == 0 || strcmp(periph, "MCPWM_MOTOR") == 0));
    if (!isMcpwm) {
        perimanSetPinBus(pin, ESP32_BUS_TYPE_GPIO, (void*)newOwner, -1, -1);
        perimanSetPinBusExtraType(pin, periph);
    }
#endif

    // 3. Register the new owner internally
    _owners[pin] = newOwner;
}

void GlobalPinManager::releasePin(uint8_t pin, IPinOwner* currentOwner) {
    if (pin >= 64 || currentOwner == nullptr) return;

    if (_owners[pin] == currentOwner) {
        _owners[pin] = nullptr;
#ifdef HAS_PERIMAN
        const char* periph = currentOwner->getPeripheralName();
        bool isMcpwm = (periph && (strcmp(periph, "MCPWM_SERVO") == 0 || strcmp(periph, "MCPWM_MOTOR") == 0));
        if (!isMcpwm) {
            perimanClearPinBus(pin);
        }
#endif
    }
}

IPinOwner* GlobalPinManager::getOwner(uint8_t pin) {
    if (pin >= 64) return nullptr;
    return _owners[pin];
}

} // namespace EasyKit
