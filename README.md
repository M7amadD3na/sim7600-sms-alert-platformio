# SIM7600 SMS Alert — PlatformIO Version

PlatformIO ESP32 project recovered from the supplied `SIM7600_Python_Alert.rar` archive. This is **ESP32/C++ firmware**, despite the original archive name containing “Python.”

## What it does

- Hosts a local Wi-Fi configuration page to set APN, destination number and SMS text.
- Saves settings in ESP32 nonvolatile Preferences storage.
- Enters deep sleep and wakes on an external trigger at GPIO13, then sends an SMS using SIM7600 AT commands.
- Includes `test/main.cpp`, an alternate test sketch for detecting a disconnected wire and sending a text alert.

## Getting started

1. Open this directory as a PlatformIO project in VS Code.
2. Review `platformio.ini` for board selection and library compatibility. The archived dependencies point to older GitHub library URLs and may require updating.
3. Review modem wiring, SIM service and wakeup trigger levels.
4. Replace the local Wi-Fi AP password placeholder in `src/main.cpp` and the test phone placeholder in `test/main.cpp` on your **local copy**. Do not publish personal phone numbers or actual credentials.
5. Build and test with the supported board.

The original archive included `.pio/` compiled objects and downloaded libraries; these are deliberately excluded from the portfolio copy.
