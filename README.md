# 🔥 IoT Fire Alarm System with ESP32

---

# 🌍 Why Choose an IoT Fire Alarm?

Traditional fire alarms only trigger local alerts.  
**This IoT system enables:**
- 📊 **Real-time, remote monitoring** of multiple sensors
- 🌐 **Dashboard access** from any device on your network
- ⏰ **Early warnings** via buzzer & web dashboard
- ⚙️ **Customizable detection thresholds** for different environments (home, lab, factory)

---

# 🚀 Key Features

- **📡 IoT-enabled dashboard** accessible from any browser
- **🌡 Temperature** & **💧 Humidity** monitoring (DHT11 sensor)
- **💨 Smoke** & **⚗ Gas** detection (MQ-series sensors)
- **🔥 Flame** detection (IR sensor)
- **🎨 Modern UI:** Colorful gradient background & clean sensor display
- **🔔 Smart buzzer:** Immediate alarm on danger detection
- **⚙️ Adjustable thresholds** for tailored safety

---

# 🛠 Hardware Requirements

| Component               | Description                             |
|-------------------------|-----------------------------------------|
| **ESP32**               | Main IoT controller with WiFi           |
| **MQ-135 Sensor**       | Detects smoke & harmful gases           |
| **MQ Gas Sensor**       | Detects flammable gases (LPG, CO, etc.) |
| **Flame Sensor**        | Detects infrared flame light            |
| **DHT11 Sensor**        | Measures temperature & humidity         |
| **Active Buzzer**       | Alarm sound when fire detected          |
| **Breadboard & Jumper Wires** | For connections                  |

---

# ⚡ Pin Configuration

| Component         | ESP32 Pin |
|-------------------|-----------|
| **MQ-135 (Smoke)**| GPIO 35   |
| **MQ Gas Sensor** | GPIO 34   |
| **Flame Sensor**  | GPIO 32   |
| **DHT11 Data**    | GPIO 27   |
| **Buzzer**        | GPIO 26   |
| **VCC**           | 3.3V/5V   |
| **GND**           | GND       |

---

# 📡 Setup Instructions

1. **Install** [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/).
2. **Install required libraries:**
   - [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
   - ESP32 WiFi (built-in)
   - [WebServer library for ESP32](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)
3. **Set your WiFi credentials** in the code:
   ```cpp
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
4. **Upload the code** to your ESP32.
5. Open **Serial Monitor** → note ESP32’s IP Address.
6. Open your browser and go to: `http://<ESP32_IP>` to access the dashboard.

---

# 🌐 Web Dashboard UI

- **Colorful gradient background** 🌈
- **White sensor box** with all values neatly displayed
- **Live values** for:
  - 🌡 Temperature
  - 💧 Humidity
  - 💨 Smoke
  - ⚗ Gas
  - 🔥 Flame

**When fire/smoke/gas is detected:**
- ⚠️ Status changes to **ALERT**
- 🔔 Buzzer starts beeping

---

# 📊 Adjustable Thresholds

Customize inside your code:

```cpp
const int smokeMargin = 200; // smoke detection
const int gasMargin   = 260; // gas detection
const int fireMargin  = 300; // fire detection
```

Sample buzzer logic:
```cpp
if (smokeValue > smokeMargin || gasValue > gasMargin || fireValue < fireMargin) {
    digitalWrite(buzzerPin, HIGH); // ON
} else {
    digitalWrite(buzzerPin, LOW);  // OFF
}
```

---

# ✅ Future Improvements

- 📱 Alerts via Telegram / WhatsApp
- ☁️ Push data to cloud (Firebase, MQTT, ThingsBoard)
- 📧 Email/SMS notifications for remote safety
- 🔋 Battery + Solar power for backup

---

# 📷 Dashboard Example

```
-------------------------------------------------
| 🔥 IoT Fire Alarm Dashboard                   |
|                                               |
| 🌡 Temp: 27.5 °C   💧 Humidity: 48 %           |
| 💨 Smoke: 512 [⚠️ Detected]                   |
| ⚗ Gas:   380 [✅ Clean]                       |
| 🔥 Fire: ✅ No Fire                           |
-------------------------------------------------
```

---

## 👤 Author

- **Name:** Rande Vu
- **GitHub:** [@thanhsyou2193-sudo](https://github.com/thanhsyou2193-sudo)

---

# **Stay safe with real-time fire monitoring!**
