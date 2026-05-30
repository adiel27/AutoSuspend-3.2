# AuSp (Auto-Suspend) v3.1

**AuSp** (Auto-Suspend) is an ultra-lightweight, high-performance system utility written in pure C. It acts as a dynamic process orchestrator for Windows 11, bridging the gap between user intent and operating system resource management. By continuously monitoring the foreground window state, AuSp enforces a highly efficient **3-Way Dynamic Resource Allocation Strategy** to maximize foreground responsiveness (*snappiness*) while achieving a near-zero noise, thermal-optimized background state reminiscent of macOS.

---

## 🚀 Key Features & Novelty

Unlike standard Windows 11 Power Modes or Microsoft PC Manager—which are passive, reactive, or heavily reliant on manual intervention—AuSp operates proactively in real-time using native Windows APIs with a **Zero-Configuration / Plug-and-Play** approach.

* **Behavior-Driven Resource Orchestration:** Dynamically matches hardware performance to active user behavior.
* **Active Telemetry & Noise Reduction:** Eliminates background micro-stuttering, dramatically lowers CPU temperatures, and keeps laptop fans quiet by suppressing uninvited background spikes.
* **Zero Hardware Interference:** Does not alter Global Windows Registry keys or lock hardware ACPI boundaries (keeps EPP at default natural levels like `50`), ensuring 100% safety and high reproducibility for system research.

---

## 🛠 The 3-Way Dynamic Orchestration Logic

AuSp continuously loops through running processes, evaluating the current user-focus context (`isNotForeground`) and applying strict state machine rules:

```
                  [ USER BEHAVIOR DETECTION ]
                              |
              +---------------+---------------+
              | (Active App)                  | (Idle/Background App)
              v                               v
     [ 1. FOREGROUND BOOST ]         [ 2. BACKGROUND ECO MODE ]
     - Scale to HIGH_PRIORITY        - Demote to IDLE_PRIORITY
     - Unshackled CPU Spikes         - Enforce Native Efficiency Mode
              |                               |
              +---------------+---------------+
                              |
                              v
                   [ 3. MEMORY TRIMMING ]
                   - Purge Background Working Sets
                   - Compress Idle RAM down to ~2MB

```

1. **Foreground Boost:** The currently focused application is granted the `HIGH_PRIORITY_CLASS` registry token. Coupled with a balanced hardware preference, this grants the active application instantaneous access to CPU clock spikes (e.g., up to 3 GHz+), ensuring ultra-smooth scrolling, prompt rendering, and optimal responsiveness.
2. **Background Eco Mode:** Once an app or a silent Windows background service (such as telemetry, updater modules, or browser web-wrappers) loses focus for a specified threshold, AuSp immediately demotes it to `IDLE_PRIORITY_CLASS` and injects the official Windows kernel-level `SetWindowsEfficiencyMode`. This isolates background processes strictly to the energy-efficient cores (E-Cores).
3. **Memory Trimming:** Utilizing native `EmptyWorkingSet` sweeps, idle background allocations (especially notorious RAM-vampires like Chromium/WebView2 web wrappers) are heavily purged, safely compressing inactive processes down to minimal working footprints (~2.0 MB) without causing software crashes.

---

## 📁 Repository Structure

The project directory is meticulously modularized to follow clean Win32 systems programming practices:

```text
/Auto-Suspend-3.0-3.1/
├── AuSp/
│   ├── bin/
│   │   └── Debug/
│   │       ├── AuSp.exe            # Compiled debug executable
│   │       ├── config.txt          # User threshold and process configurations
│   │       └── SuspendLog.txt      # Real-time state change log output
│   ├── obj/Debug/                  # Intermediate compilation binaries (.o files)
│   ├── AuSp.cbp                    # Code::Blocks Project configuration
│   ├── AuSp.depend                 # Project dependency maps
│   ├── AuSp.layout                 # IDE editor layout metadata
│   ├── main.c                      # Central orchestration loop & WinMain entry
│   ├── cpu_utils.c / .h            # Priority classes & Eco Mode API wrappers
│   ├── process_utils.c / .h        # PID lookups & process handling utilities
│   ├── window_utils.c / .h         # Win32 Foreground window hook abstractions
│   ├── log_utils.c / .h            # Safe IO logging interfaces
│   ├── notify_utils.c / .h         # System tray & behavioral notifications
│   └── SuspendLog.txt              # Root copy duplicate runtime telemetry log
├── AuSp.exe                        # Root Production/Deployment Executable
└── README.md                       # Documentation

```

---

## 🛡 Security Alignment with Modern Windows 11 (24H2+)

Modern editions of Windows (including Windows 11 Enterprise LTSC 2024 and version 24H2+) strictly protect root system access (`C:\`) to mitigate malware propagation and DLL hijacking vectors.

To ensure AuSp runs organically within this modern secure infrastructure:

* **Isolate Runtime Assets:** Keep `config.txt` and `SuspendLog.txt` strictly within the user space directory alongside the executing binary. Do not host execution paths in root system paths.
* **Elevated Deployment:** Because AuSp invokes low-level OS Process Scheduler manipulation APIs (`SetPriorityClass`), it must be configured to bypass standard kernel limitations via the **Windows Task Scheduler**.



## ⚙ Deployment & Automation Setup

To transition AuSp from an experimental utility to a transparent, permanent system background service, it should be configured via the Windows Task Scheduler using the following parameters:

1. **General Tab:** Set the trigger execution rule to **"Run with highest privileges"**. This ensures the kernel grants the program authorization to intercept process IDs (PIDs) owned by auxiliary background accounts.
2. **Triggers Tab:** Set to initiate **"At log on"** for the primary user profile. This guarantees a seamless boot phase, preventing delayed initial Windows initialization routines.
3. **Conditions Tab:** Uncheck **"Start the task only if the computer is on AC power"**. This permits AuSp to continuously safeguard battery longevity and manage performance curves organically while on mobile juice.
4. **Settings Tab:** Disable the default constraint **"Stop the task if it runs longer than..."** to allow the utility to maintain uninterrupted system resource monitoring throughout the entire OS uptime lifecycle.

## Build Flow
## 🛠 Compilation & Build Workflow

Since AuSp is modularly structured across separate C source files, the compilation process follows a standard Win32 toolchain pipeline (GCC/MinGW via Code::Blocks) to compile, link, deploy, and automate the application.

Below is the conceptual architecture of how the files are built and deployed:

```text
 [ SOURCE STAGE ]              [ COMPILATION STAGE ]          [ LINKING & RUNTIME STAGE ]
 
  +------------+
  |   main.c   | ------------> [ GCC Compiler ] ------+
  +------------+                                      |
  +------------+                                      |
  | cpu_utils  | ------------> [ GCC Compiler ] ------|      +---------------------+
  +------------+                                      |--->  |      AuSp.exe       |
  +------------+                                      |      | (Linked Executable) |
  |proc_utils  | ------------> [ GCC Compiler ] ------|      +---------------------+
  +------------+                                      |                 |
  +------------+                                      |                 v
  |win_utils   | ------------> [ GCC Compiler ] ------+       [ Reads System Rules ]
  +------------+                                                        |
  +------------+                                                        v
  | log/notify | ------------> [ GCC Compiler ]               +---------------------+
  +------------+                                              |     config.txt      |
                                                              +---------------------+
                                                                        |
                                                                        v
                                                             [ DEPLOYMENT AUTOMATION ]
                                                                        |
                                                                        v
                                                              +---------------------+
                                                              |Windows Task Scheduler|
                                                              | (Highest Privileges)|
                                                              +---------------------+
                                                                        |
                                                                        v
                                                              [ SYSTEM ENFORCEMENT ]
                                                               - High Priority (FG)
                                                               - Eco Mode (BG Core)
                                                               - RAM Purge (~2MB)
