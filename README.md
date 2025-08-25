🔥 IoT Fire Alarm System with ESP32

This project is a smart IoT-based fire alarm system powered by ESP32.
It continuously monitors smoke, gas, flame, temperature, and humidity and provides real-time alerts via a web dashboard.
When smoke, gas, or fire is detected, a buzzer alarm is triggered to warn immediately.

🌍 Why IoT Fire Alarm?

Traditional fire alarms only trigger local alerts.
This IoT system allows you to:
✅ Monitor multiple sensors in real time
✅ Access the dashboard from any device on the same network
✅ Get early warnings with buzzer alerts
✅ Customize thresholds for different environments (home, lab, factory)

🚀 Key Features

📡 IoT-enabled dashboard accessible via browser

🌡 Temperature & 💧 Humidity monitoring (DHT11)

💨 Smoke & ⚗ Gas detection (MQ sensors)

🔥 Flame detection (IR sensor)

🎨 Modern UI with colorful background + clean sensor box

🔔 Smart buzzer alert when danger detected

⚙️ Adjustable detection thresholds for different conditions

🛠 Hardware Requirements
Component	Description
ESP32	Main IoT controller with WiFi
MQ-135 Sensor	Detects smoke & harmful gases
MQ Gas Sensor	Detects flammable gases (LPG, CO, etc.)
Flame Sensor	Detects infrared flame light
DHT11 Sensor	Measures temperature & humidity
Active Buzzer	Alarm sound when fire detected
Breadboard & Jumper Wires	For connections
⚡ Pin Configuration
Component	ESP32 Pin
MQ-135 (Smoke)	GPIO 35
MQ Gas Sensor	GPIO 34
Flame Sensor	GPIO 32
DHT11 Data	GPIO 27
Buzzer	GPIO 26
VCC	3.3V / 5V
GND	GND
📡 Setup Instructions

Install Arduino IDE or PlatformIO.

Install required libraries:

DHT sensor library (Adafruit)

ESP32 WiFi (built-in)

WebServer (ESP32 library)

Open the code and set your WiFi credentials:

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";


Upload the code to your ESP32.

Open Serial Monitor → copy ESP32’s IP Address.

Open a browser → enter http://<ESP32_IP> to access the dashboard.

🌐 Web Dashboard

Colorful gradient background 🌈

White sensor box with all values neatly displayed

Real-time values for:

🌡 Temperature

💧 Humidity

💨 Smoke

⚗ Gas

🔥 Flame

When fire/smoke/gas is detected →
⚠️ Status changes to ALERT + buzzer starts beeping.

🔔 Smart Buzzer Logic

✅ Safe → Buzzer OFF

⚠️ Fire/Smoke/Gas Detected → Buzzer ON

if (smokeValue > smokeMargin || gasValue > gasMargin || fireValue < fireMargin) {
    digitalWrite(buzzerPin, HIGH); // Turn buzzer ON
} else {
    digitalWrite(buzzerPin, LOW);  // Turn buzzer OFF
}

📊 Adjustable Thresholds

Customize margins inside the code:

const int smokeMargin = 200; // for smoke detection
const int gasMargin   = 260; // for gas detection
const int fireMargin  = 300; // for fire detection

📷 Dashboard Example
-------------------------------------------------
| 🔥 IoT Fire Alarm Dashboard                   |
|                                               |
| 🌡 Temp: 27.5 °C   💧 Humidity: 48 %           |
| 💨 Smoke: 512 [⚠️ Detected]                   |
| ⚗ Gas:   380 [✅ Clean]                       |
| 🔥 Fire: ✅ No Fire                           |
-------------------------------------------------

✅ Future Improvements

📱 Send alerts via Telegram / WhatsApp

☁️ Push data to cloud (Firebase, MQTT, ThingsBoard)

📧 Email/SMS notifications for remote safety

🔋 Battery + Solar power for backup
