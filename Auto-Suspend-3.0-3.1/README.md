<b><h1>AutoSuspend</h1></b>

AutoSuspend is a lightweight C-based Windows utility that automatically monitors active processes and suspends idle ones to conserve system resources. Ideal for users seeking performance and thermal efficiency, especially on constrained hardware.



<h2>🔧 Features</h2>h2>

- ✅ Automatically suspends/resumes processes based on CPU activity
- ✅ Detects processes with visible windows
- ✅ Excludes critical system and user-defined processes
- ✅ Logs all actions to `SuspendLog.txt`
- ✅ Displays a startup notification when launched
- ✅ Mirrors log output to terminal for real-time feedback

<h2>🚀 Build Instructions</h2>

Using Code::Blocks

1. Install Code::Blocks with MinGW
2. Create a new Console Application (C language)
3. Add all `.c` and `.h` files to the project
4. In Project → Build Options → Linker Settings, add:
   ```
   psapi
   ```
5. Build and run the project

Using GCC (Command Line)

bash
gcc main.c process_utils.c window_utils.c cpu_utils.c log_utils.c notify_utils.c -o AutoSuspend.exe -lpsapi -Wall

<h2>🖥️ Usage</h2>

1. Launch `AutoSuspend.exe`
2. A startup notification will confirm the program is running
3. The program monitors processes every 10 seconds
4. Idle processes are suspended; active ones are resumed
5. All actions are logged to `SuspendLog.txt` and printed to the terminal


<h2>⚙️ Customization</h2>

- Modify the exclusion list in `process_utils.c`
- Adjust monitoring interval via `Sleep(10000)` in `main.c`
- Replace `MessageBox` in `notify_utils.c` with tray or toast notifications if desired


<h2>🧠 Technical Notes</h2>

- Uses Windows APIs: `CreateToolhelp32Snapshot`, `SuspendThread`, `GetProcessTimes`, etc.
- Written entirely in C (not C++)
- Suitable for integration with thermal-aware systems or Snapdragon blueprints


<h2>📜 License</h2>

This project is free to use and modify for personal, educational, or research purposes. For commercial distribution, please include attribution.
