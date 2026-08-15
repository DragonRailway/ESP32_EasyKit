# EasyLEDGroup Reference

`EasyLEDGroup` coordinates multiple `EasyLED` instances on a single `millis()` timeline using timed **step sequences** — a table of steps, each a per-member duty vector plus a duration. It powers professional multi-LED effects (ditch lights, chases, strobes) without blocking.

> [!IMPORTANT]
> **Polling Dependency**: Like `EasyLED`, the group requires `group.update()` to be called regularly in your `loop()` for patterns to advance.

---

## Construction

```cpp
EasyLEDGroup group;                                   // empty — add members later
EasyLEDGroup group({&ledA, &ledB});                   // initializer list
EasyLEDGroup group(std::vector<EasyLED*>{&a, &b, &c}); // vector overload
```

**Membership limits**: up to `EASYLED_GROUP_MAX_MEMBERS` (8) members per group.

---

## Members

```cpp
group.addMember(&ledA);          // add an EasyLED to the group
group.clearMembers();            // remove all members
size_t n = group.getMemberCount(); // number of members
```

Members should be `begin()`/`attach()`ed before the pattern starts. Each member may have its own pin and config.

---

## Patterns

All built-in factories return `bool` (`true` if the pattern started).

### alternate()

Alternates a two-or-more member group: one lit, then the next, round-robin.

```cpp
group.alternate(intervalMs);   // switch every intervalMs
```

**Example** — ditch lights flashing alternately every 8 ms:

```cpp
EasyLED ditchL(4), ditchR(5);
EasyLEDGroup ditchGroup({&ditchL, &ditchR});

void setup() {
  ditchGroup.alternate(8);
}
void loop() {
  ditchGroup.update();
}
```

### syncFlash()

All members flash together.

```cpp
group.syncFlash(onMs, offMs);   // all on for onMs, all off for offMs
```

### chase()

One member lit at a time, sweeping from first to last.

```cpp
group.chase(intervalMs);        // advance one member every intervalMs
```

### doubleStrobe()

Double-burst strobe effect.

```cpp
group.doubleStrobe(intervalMs, gapMs);
```

### startPattern()

Custom step table. Each step is a `EasyLEDStep`:

```cpp
struct EasyLEDStep {
    uint8_t  duty[EASYLED_GROUP_MAX_MEMBERS];  // duty % (0–100) per member
    uint16_t durationMs;                        // step duration in ms
};
```

```cpp
// Array form
EasyLEDStep steps[] = {
    { {100, 0},  200 },   // member 0 full, member 1 off, 200 ms
    { {0, 100},  200 },   // member 0 off, member 1 full, 200 ms
};
group.startPattern(steps, 2, /*repeat=*/true);

// Vector form
std::vector<EasyLEDStep> v = { { {100, 0}, 200 }, { {0, 100}, 200 } };
group.startPattern(v, true);
```

---

## Control

```cpp
group.stop();           // stop the pattern, land every member at 0% duty
bool running = group.isRunning();  // true while a pattern is active
```

While a pattern runs, the group owns its members' duty — don't write those pins from another path until the pattern is stopped.

### update()

Advances the pattern timeline and applies the current step to every member. **Must be called every `loop()` iteration.**

```cpp
void loop() {
    group.update();
}
```

---

## Typical Flow

```cpp
EasyLED lights[3] = { EasyLED(6), EasyLED(7), EasyLED(8) };
EasyLEDGroup group({&lights[0], &lights[1], &lights[2]});

void setup() {
    group.chase(60);          // sweep
}

void loop() {
    group.update();           // drive the pattern
}
```
