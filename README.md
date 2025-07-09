# 🔐 PAAK-using-BLE – Fusion Processor Firmware

This repository contains the firmware for the **Fusion Processor** used in our graduation project: *Phone As A Key (PAAK)* using Bluetooth Low Energy (BLE). The Fusion Processor is responsible for processing localization data received from BLE Anchors over CAN, determining the user's position around the car, and triggering appropriate actions (e.g., unlocking the door, opening the trunk, etc.).

It runs on a **Tiva C Series TM4C123GXL LaunchPad**, using **TI's Code Composer Studio (CCS)** IDE and **TivaWare** peripheral libraries.

---

## 📚 Table of Contents

- [Overview](#-overview)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [How to Build and Run](#-how-to-build-and-run)
- [Features](#-features)
- [Development Notes](#-development-notes)


---

## 📖 Overview

- **Target MCU:** TM4C123GH6PM (Tiva C Series)
- **IDE:** Code Composer Studio (TI CCS)
- **Architecture:** Bare-metal C (no third-party RTOS)
- **Purpose:** Fuse RSSI/ToF localization data from BLE Anchors to determine user zone and trigger zone-specific vehicle actions
- **Communication Protocol:** CAN
- **Debugging Interface:** UART (for status and logs)

---

## 🛠️ Hardware Requirements

- [x] Tiva C Series TM4C123GXL LaunchPad
- [x] CAN transceiver (e.g., MCP2551)
- [x] BLE Anchors (e.g., NXP KW45 boards)
- [x] Micro-USB cable (for power & programming)
- [x] Power source (for other boards)
- [x] Optional: logic analyzer or UART-to-USB for debugging

---

## 💻 Software Requirements

- [x] [Code Composer Studio (CCS)](https://www.ti.com/tool/CCSTUDIO)
- [x] [TivaWare™ Peripheral Driver Library](https://www.ti.com/tool/SW-TM4C)
- [x] Git (for cloning this repository)
- [x] CAN analysis tool (optional – e.g., USB-CAN adapter)

---

## 🚀 How to Build and Run

### 1. Clone the Repository

```bash
git clone https://github.com/Phone-As-A-Key-using-BLE/PAAK-using-BLE-Fusion-Processor.git
````

---

### 2. Install Code Composer Studio

Download and install CCS from the TI website:
👉 [https://www.ti.com/tool/CCSTUDIO](https://www.ti.com/tool/CCSTUDIO)

During installation, make sure to include **Tiva C / ARM Cortex-M support**.

---

### 3. Install and Configure TivaWare

1. Download TivaWare from TI:
   👉 [https://www.ti.com/tool/SW-TM4C](https://www.ti.com/tool/SW-TM4C)

2. Extract it to a known location (e.g., `C:\ti\TivaWare`)

3. In CCS:

   * Open **Project → Properties → Build → Include Options**
   * Add the following include paths:

     ```
     C:/ti/TivaWare/driverlib
     C:/ti/TivaWare/inc
     ```

4. Link or copy `driverlib.lib` if not already included in the linker settings:

   * **Project → Properties → Build → Linker → File Search Path**
   * Add path to `driverlib.lib`

---

### 4. Import the Project into CCS

1. Open Code Composer Studio
2. Go to **File → Import → CCS Projects**
3. Browse to the cloned project folder
4. Select the project and click **Finish**

---

### 5. Build and Flash

1. Connect the Tiva C board via USB
2. In CCS, click the **Build** button (hammer icon)
3. Click **Debug** (bug icon), then **Run** to flash and execute the firmware
4. Use a serial terminal (e.g., PuTTY) or logic analyzer to view UART/CAN output

---


## ✨ Features

- Receives RSSI/ToF data from BLE anchors via CAN
- Performs **trilateration** and **particle filter** localization algorithms
- Tracks and manages **BLE Anchor statuses** (online/offline/invalid data)
- Tracks and manages **user device statuses** (authenticated/unauthenticated/inactive)
- All status management and zone decision logic are handled through a **central Finite State Machine (FSM)**
- Acts as a **centralized decision-making node** for the entire BLE-based vehicle access system
- Determines user zone (e.g., front door, trunk, inside) based on fused data
- Triggers zone-specific vehicle actions(e.g., unlock door, pop trunk)
- Outputs detailed logs and states over UART for debugging and testing
- Modular and easy-to-extend codebase 

---

## 🧠 Development Notes

* Make sure the board is correctly recognized by CCS (check device manager)
* All CAN messages must follow the format defined in the system protocol
* Watchdog timer is disabled by default for easier debugging
* Use `SysTick` for internal timing tasks
* Project is built for **TM4C123GH6PM** – do not change device target

---





