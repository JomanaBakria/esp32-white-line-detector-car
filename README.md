# ESP32 White Stripe Detector Car 🚗⚪

An autonomous **ESP32-based competition robot** that drives a fixed track, detects **white stripes** using reflectance sensors, and adapts its behavior according to the rules.  
The system is optimized to **complete the track as fast as possible** while maintaining reliable detection and accurate stopping.

---

## 🎯 Objectives
- Detect white stripes on the track with **high accuracy**
- Perform actions depending on stripe count:
  - Stop briefly on each of **3 forward stripes**
  - After the 3rd stripe → **switch to reverse**
  - Stop again on the **third backward stripe** only
- Minimize total run time with tuned braking and timing windows

---

## ✨ Features
- **ESP32 DevKit v1** microcontroller
- **2× reflectance sensors (CNY70)** with IR LED illumination
- **4× DC motors** controlled by dual H-Bridge drivers (L293D)
- **Bluetooth control** (start/stop via smartphone)
- **PWM motor control** for speed tuning
- **Active braking pulses** for sharper, faster stops

---

## 🛠️ Hardware Architecture
- **MCU:** ESP32 DevKit v1  
- **Sensors:** 2× CNY70 reflectance sensors + IR LEDs  
- **Motors:** 4× DC motors  
- **Motor Driver:** 2× L293D H-Bridge  
- **Power:** 4× 18650 Li-ion cells + LM7805 regulators  

📂 Detailed schematics are in [`docs/project_book.pdf`](docs/project_book.pdf)

---

## 🧩 Control Algorithm
- Forward mode:
  - Drive straight using base PWM
  - Use timed “sampling windows” to reduce false positives
  - On stripe detection → active brake + short pause
  - After 3 stripes → switch to reverse
- Backward mode:
  - Drive back for a fixed window
  - Enable stripe detection
  - On first stripe → brake + pause → stop

---

## 🚀 Getting Started

### Arduino IDE
1. Install **ESP32 Board Support** via Boards Manager  
2. Open the firmware: [`firmware/src/FinalCarCode.ino`](firmware/src/FinalCarCode.ino)  
3. Select your ESP32 board + COM port  
4. Upload the code  
5. Connect via Bluetooth (device name: **RJ CAR**)  
   - Send `1` → Start forward mode  
   - Send `0` → Stop immediately  

---

## 📊 Performance Tuning
- **LINE_PAUSE** → minimum delay that still shows a visible stop  
- **OFF_MS / ON_MS** → shorter windows = less idle time  
- **PWM balance** → adjust motor speeds to keep straight path  
- **Active brake power** → strong enough to stop fast, not to overshoot  

---

## 📷 Media
- Photos → `media/photos/`  
- Demo video → `media/demos/demo.mp4`  
- GIF preview → `media/demos/demo.gif`  

---

## 📑 Project Report
The full project documentation and competition details are available here:  
[`docs/project_book.pdf`](docs/project_book.pdf)

---

## 📜 License
This project is released under the [MIT License](LICENSE).

