<!--
  Sync Impact Report
  ==================
  Version change: N/A (initial) -> 1.0.0
  Modified principles: N/A (initial creation)
  Added sections:
    - Core Principles (5): Power Efficiency, Memory Management,
      Configuration Separation, Resilient Error Handling, Modularity
    - Hardware & Platform Constraints
    - Development Workflow
    - Governance
  Removed sections: N/A
  Templates requiring updates:
    - .specify/templates/plan-template.md        -> ✅ no update needed (generic gates)
    - .specify/templates/spec-template.md         -> ✅ no update needed (generic structure)
    - .specify/templates/tasks-template.md        -> ✅ no update needed (generic phases)
  Follow-up TODOs: None
-->

# esp32-weather-epd Constitution

## Core Principles

### I. Extreme Power Efficiency (NON-NEGOTIABLE)

The primary constraint of this project is battery life, targeting
6-12 months on a single charge. Every design decision MUST prioritize
minimizing energy consumption.

- The device operates on a strict **wake-fetch-refresh-sleep** cycle.
  All work MUST complete within a single wake window.
- Code MUST NOT introduce continuous polling, `delay()` loops, or
  blocking network calls that keep the CPU or radio active beyond
  the minimum required time.
- WiFi MUST be enabled only for the duration of data fetching and
  MUST be explicitly disabled before entering deep sleep.
- ESP32 Deep Sleep MUST be used for all idle periods. Light sleep
  or idle loops are not acceptable substitutes.
- New features MUST justify their impact on active processing time.
  If a feature cannot operate within the wake-fetch-refresh-sleep
  model, it MUST NOT be added.

**Rationale**: A battery-powered, wall-mounted display has no access
to mains power. Every millisecond of CPU or radio time directly
reduces deployment lifetime.

### II. Memory Management

The ESP32 has approximately 320 KB of SRAM. Heap fragmentation or
excessive dynamic allocation will cause crashes and watchdog resets.

- Code MUST avoid the Arduino `String` class. Use `const char*`,
  pre-allocated `char` arrays, and static buffers instead.
- JSON parsing (OpenWeatherMap API responses) MUST use fixed-size
  `StaticJsonDocument` or stack-allocated `JsonDocument` buffers
  sized to the expected payload. Never use unbounded dynamic
  allocation for JSON deserialization.
- Large data structures MUST be statically allocated or placed in
  function-scoped stack variables with known maximum sizes.
- Heap allocations, when unavoidable, MUST be performed early in
  the wake cycle and freed before sleep to prevent fragmentation
  across wake cycles.

**Rationale**: Unlike a server or desktop application, the ESP32
cannot recover from memory exhaustion gracefully. A crash means a
missed display update and wasted battery on a reboot cycle.

### III. Configuration Separation

User-configurable parameters MUST NOT be scattered across source
files. All configuration MUST be isolated to `config.h` and
`config.cpp`.

- API keys, WiFi credentials, geographic coordinates, update
  intervals, and display preferences MUST reside exclusively in
  `config.h` / `config.cpp`.
- Hardware pin assignments and I2C addresses MUST be defined in
  `config.h`, not in driver or rendering code.
- UI layout coordinates, font selections, and screen region
  definitions MUST be configurable from `config.h` or dedicated
  display configuration sections within it.
- Core logic files MUST reference configuration through includes,
  never through hardcoded literals.

**Rationale**: End users configure this project by editing a single
file. Scattering settings across the codebase makes the project
inaccessible and error-prone for non-developers.

### IV. Resilient Error Handling

The device operates autonomously without user intervention. Errors
MUST NOT cause the device to hang, drain the battery, or enter an
unrecoverable state.

- Network operations MUST use finite timeouts. If a connection or
  API call fails, the device MUST log the error, display a failure
  indicator on the E-Paper screen, and enter deep sleep.
- API rate limiting or HTTP error responses MUST be detected and
  handled identically to network failures: log, display status,
  sleep.
- Missing or degraded I2C sensor data (BME280/BME680) MUST be
  handled gracefully. The display MUST render with available data
  and indicate which readings are unavailable.
- Error recovery MUST NOT involve retry loops. A single failed
  attempt per wake cycle is acceptable; retries waste battery. The
  next scheduled wake cycle serves as the retry mechanism.
- Serial logging MUST be used for diagnostics but MUST NOT block
  execution or delay sleep entry.

**Rationale**: A hung device is worse than a device showing stale
data. The user can tolerate a missed update; they cannot tolerate
a dead battery.

### V. Modularity

Network fetching, hardware interfacing, and UI rendering MUST remain
strictly decoupled.

- **Network layer** (`client_utils`, `api_response`): Responsible
  for WiFi management, HTTP requests, and API response parsing.
  MUST NOT reference display or sensor code.
- **Hardware layer** (`display_utils`, sensor drivers): Responsible
  for E-Paper and I2C sensor communication. MUST NOT contain
  network or rendering logic.
- **Rendering layer** (`renderer`): Responsible for composing the
  display layout from parsed data. MUST NOT perform network calls
  or direct hardware I/O beyond the GxEPD2 display interface.
- **Configuration layer** (`config`): Provides constants and
  settings to all other layers. MUST NOT contain business logic.
- `main.cpp` serves as the orchestrator, calling each layer in
  sequence. Business logic in `main.cpp` MUST be limited to
  sequencing the wake-fetch-refresh-sleep cycle.

**Rationale**: Clean separation enables independent reasoning about
power, memory, and correctness within each layer, and prevents
changes in one area from creating regressions in another.

## Hardware & Platform Constraints

- **MCU**: ESP32 (Espressif32 platform) with Arduino framework
- **Build system**: PlatformIO (`platformio.ini` is authoritative
  for dependencies and build configuration)
- **Language**: C++17 (`-std=gnu++17`)
- **Display**: E-Paper via GxEPD2 library (partial refresh where
  supported; full refresh otherwise)
- **Sensors**: BME280 / BME680 via Adafruit libraries over I2C
- **API**: OpenWeatherMap (JSON responses parsed with ArduinoJson)
- **Test generation**: Not used. Validation is manual (compile,
  flash, observe).
- New library dependencies MUST be added to `platformio.ini` with
  pinned versions. Do not use unpinned or floating version ranges.

## Development Workflow

- All code changes MUST compile cleanly with `-Wall` and produce
  no new warnings.
- Changes affecting power behavior (WiFi timing, sleep entry, wake
  sources) MUST be explicitly called out in commit messages and
  PR descriptions.
- Display layout changes SHOULD include a photograph or screenshot
  of the rendered output on actual hardware when possible.
- Configuration changes MUST NOT break the default `config.h`
  template. New configuration options MUST include sensible
  defaults.

## Governance

This constitution is the authoritative guide for all development
decisions on esp32-weather-epd. When a proposed change conflicts
with a principle above, the principle takes precedence unless the
constitution is formally amended.

- **Amendment process**: Propose a change via pull request modifying
  this file. The PR description MUST state which principle is
  affected and why the change is necessary. Version MUST be bumped
  per the versioning policy below.
- **Versioning policy**: MAJOR for principle removal or incompatible
  redefinition; MINOR for new principles, sections, or material
  expansion; PATCH for clarifications and wording fixes.
- **Compliance review**: All code contributions SHOULD be reviewed
  against the five core principles. Violations MUST be justified
  in the PR description with a clear rationale for the exception.

**Version**: 1.0.0 | **Ratified**: 2026-04-03 | **Last Amended**: 2026-04-03
