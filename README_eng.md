# Log System — Logging Library + Multithreaded Application + Statistics Server

## Overview

This project implements a thread-safe a logging library with support for **log levels**, the ability to write logs **to a file or a socket**, and two console applications:

1. `app` — a multithreaded application that uses the logger.
2. `log_stats` — a log statistics collection server that receives data via a TCP socket.

**Supports both dynamic (`.so`) and static (`.a`) builds.**
**A Makefile is provided for convenient build and execution.**

---

## Project Structure

```
├── include/logger/        # Logger header files
├── src/                   # Logger implementation
├── app/                   # Main application (multithreaded)
├── stats/                 # Statistics server (log_stats)
├── tests/                 # Unit tests (Google Test)
├── CMakeLists.txt         # Top-level CMake file
├── Makefile               # Simplified build and run commands
└── README_eng.md              # This file
```

---

## Build and Run

### Directories and Variables

* Two build directories are used:

  * `build` — dynamic (shared) build (default)
  * `build_static` — static build when using `STATIC=ON`

* Default variables:

  * `LOG_FILE` — log output file (`./build/logs.txt` or `./build_static/logs.txt`)
  * `LOG_LEVEL` — logging level (`info` by default)

---

### Building

Dynamic build (default):

```bash
make build
```

Static build:

```bash
make build STATIC=ON
```

---

### Running

## Main Application (`app`)

**Run with logging to a file:**

```bash
make run_app
```

```bash
make run_app STATIC=ON
```

You can override `LOG_FILE` and `LOG_LEVEL` at runtime, for example:

```bash
make run_app LOG_FILE=./my_logs.txt LOG_LEVEL=warning
```

```bash
make run_app STATIC=ON LOG_FILE=./my_logs.txt LOG_LEVEL=warning
```

---

### Interactive Commands in `app`

In interactive mode, the application supports the following commands:

```
change_level <level>   — change logging level (info, warning, error)
exit                  — terminate the application
<level> <message>     — send a message with the specified level (error, warning, info)
<message>             — send a message using the current logging level
```

Log levels in descending priority: `error` > `warning` > `info`.

---

## Logging to the Statistics Server

To send logs from the multithreaded `app` application to the `log_stats` statistics server:

### 1. Start the statistics server (in a separate terminal)

```bash
make run_stats
```

```bash
make run_stats STATIC=ON
```

By default, the server is started with the following parameters:

```bash
./build/bin/log_stats 5000 3 10
```

```bash
./build_static/bin/log_stats 5000 3 10
```

Where:

* `5000` — port for receiving logs via socket
* `3` — `N`: print statistics every N messages
* `10` — `T`: print statistics if changes occur after T seconds

To specify custom parameters:

```bash
make run_stats PORT=6000 N=5 T=20
```

```bash
make run_stats STATIC=ON PORT=6000 N=5 T=20
```

When building the project, two directories are created — `build` (shared) and `build_static` (static). Each contains its own copy of `log_stats`. The statistics server works independently of the library build type and supports receiving logs from applications built with both dynamic and static versions of the library.

---

### 2. Run the `app` application with socket-based logging

```bash
make run_app_stats
```

```bash
make run_app_stats STATIC=ON
```

In this mode, logs are sent to the statistics server instead of being written to a file.

---

## Tests

Dynamic build:

```bash
make run_tests
```

Static build:

```bash
make run_tests STATIC=ON
```

---

## Additional Commands

```bash
make clean   — remove all build directories (build and build_static)
make help    — show available targets and variables
```

---

## Technical Details

* C++17 (exceptions disabled)
* STL only
* Multithreading (`std::thread`, `std::mutex`, `std::condition_variable`)
* CMake + Makefile
* Google Test for unit testing (no external package managers)
* Ubuntu/Debian compatible

---

**Author:**
Anastasiia Glushakova

This project was implemented as part of a technical test assignment.
All components were developed manually, including the socket-based logging protocol and the statistics aggregation server.
