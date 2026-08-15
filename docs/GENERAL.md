# ESP32_EasyKit

The All-in-one library to control Servos, DC Motors, and LEDs on ESP32.


## Getting Started

Explore the specialized documentation for each component:

- 📑 **[Servo Functions](SERVO_Functions.md)**.
- 📑 **[Motor Functions](MOTOR_Functions.md)**.
- 📑 **[LED & PWM Functions](LED_Functions.md)**.
- 📑 **[EasyLEDGroup](EasyLEDGroup.md)**: Coordinated multi-LED patterns.

---


## High-Performance FPU Optimization

The ESP32 (and S3) feature a single-precision hardware Floating Point Unit (FPU). Any code that uses double-precision math (`double`) forces the CPU to switch to software emulation.

To keep the library running at peak efficiency, we adhere to a strict "Float" standard. You should do the same in your application code:

- **Always use `float**`: Avoid the `double` keyword.
- **Suffix Literals**: Use the `f` suffix on all decimal numbers (e.g., `0.5f` instead of `0.5`).

If you are porting legacy code or using third-party libraries that frequently forget the `f` suffix, you can force the compiler to protect you. This GCC flag treats all unsuffixed floating-point constants as `float` instead of `double`.

**How to enable it:**

#### PlatformIO

Add this to your `platformio.ini`:

```ini
build_flags = 
    -fsingle-precision-constant
    -Wdouble-promotion
```

*(The `-Wdouble-promotion` flag is recommended as it will trigger a warning if a value is accidentally promoted to double).*

#### Arduino IDE

Add it to your `compiler.cpp.extra_flags` in `Arduino/platform.local.txt`:

```text
compiler.cpp.extra_flags=-fsingle-precision-constant -Wdouble-promotion
```

With this flag enabled, a value like `2.0` is treated as `2.0f` automatically, ensuring your motion engine stays locked to the hardware FPU.

---