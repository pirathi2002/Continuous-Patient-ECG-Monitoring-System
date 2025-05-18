# 📈Continuous Patient ECG Monitoring System

*A modular IoT-based system for real-time ECG monitoring and abnormal heart rate detection using ESP32 and MQTT.*

---

## 📌 Overview

This project implements a **non-invasive ECG monitoring system** using the **ESP32 microcontroller**, which simulates ECG signals, tracks **heart rate**, and sends real-time alerts via **Wi-Fi using the MQTT protocol** to **HiveMQ**. The system detects abnormal conditions when the heart rate falls below **60 BPM** or rises above **100 BPM**, issuing warnings accordingly.

---

## ⚙️ System Architecture

### 1️⃣ Signal Simulation & Processing
- Simulated ECG signal is processed by the ESP32.
- Heart rate is calculated based on the interval between R-R peaks.

### 2️⃣ Abnormality Detection
- Thresholds set:  
  - Below **60 BPM** = Bradycardia (Low HR)  
  - Above **100 BPM** = Tachycardia (High HR)  
- When these conditions are met, a **warning message** is triggered.

### 3️⃣ Wi-Fi and MQTT Alert Transmission
- The **ESP32** connects to a Wi-Fi network.
- Sends warning messages to the **HiveMQ broker** using **MQTT protocol**.
- The messages are logged and viewed via a cloud dashboard or subscriber client.

---

## 🖥️ Block Diagram

```mermaid
graph TD;
    A[Simulated ECG Signal] --> B[ESP32 Microcontroller]
    B -->|Heart Rate Calculation| C[Check BPM Range]
    C -->|If Abnormal| D[Trigger Warning]
    D -->|MQTT via Wi-Fi| E[HiveMQ Broker]
    E -->|Subscribed Client| F[Warning Display]
