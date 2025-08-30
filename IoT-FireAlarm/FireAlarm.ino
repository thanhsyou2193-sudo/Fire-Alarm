// =============================
// ESP32 Multi-Sensor System + Web Dashboard + Buzzer Alarm + FreeRTOS (+ ThingsBoard optional)
// MQ-135 Smoke + MQ Gas + Flame Sensor + DHT11
// =============================

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <PubSubClient.h>      // <-- cần nếu bật ThingsBoard

// Function prototypes
void TaskSensor(void *pv);
void TaskTelemetry(void *pv);

// ====== Toggle tính năng Cloud (ThingsBoard) ======
static const bool ENABLE_THINGSBOARD = true;   // đặt false nếu không muốn gửi cloud

// --- WiFi Credentials ---
const char* ssid     = "Your SSID";
const char* password = "Your password";

// --- ThingsBoard (chỉ dùng nếu ENABLE_THINGSBOARD = true) ---
const char* mqtt_server  = "mqtt.thingsboard.cloud";
const int   mqtt_port    = 1883;
const char* access_token = "Bke7Faw0U7GsG6A37VJv";

// ===== Pins =====
#define MQ135_PIN       35   // Smoke analog
#define GAS_SENSOR_PIN  34   // Gas analog
#define FIRE_SENSOR_PIN 32   // Flame analog
#define DHT_PIN         27
#define DHT_TYPE        DHT11
#define BUZZER_PIN      26   // Buzzer pin

// =====================
// Objects
// =====================
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// =====================
// Sensor Calibration Variables
// =====================
int smokeBaseline = 0;
int gasBaseline   = 0;
const int smokeMargin = 220; // kept for reference, not used
const int gasMargin   = 260; // kept for reference, not used

// =====================
// Shared telemetry (được cập nhật bởi TaskSensor, đọc bởi web / cloud)
// =====================
float g_temperature = NAN;
float g_humidity    = NAN;
int   g_smokeValue  = 0;
int   g_gasValue    = 0;
int   g_fireValue   = 0;

String g_smoke_status = "N/A";
String g_gas_status   = "N/A";
String g_fire_status  = "N/A";

// Mutex bảo vệ dữ liệu dùng chung
SemaphoreHandle_t gDataMutex;

// =====================
// HTML Dashboard (giữ nguyên)
// =====================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Fire Alarm Dashboard</title>
<style>
body { 
  font-family: Arial, sans-serif; 
  text-align: center; 
  background: linear-gradient(135deg, #ff9a9e, #fad0c4); 
  margin: 0; padding: 0;
}
h1 { color: #333; margin-top: 20px; }

.main-box {
  background: white;
  border-radius: 16px;
  padding: 30px;
  margin: 40px auto;
  width: 90%;
  max-width: 900px;
  box-shadow: 0 8px 20px rgba(0,0,0,0.2);
}

.container { display: flex; flex-wrap: wrap; justify-content: center; }
.card { 
  background: #fff; 
  border-radius: 12px; 
  padding: 20px; 
  margin: 15px; 
  width: 180px; 
  box-shadow: 0 4px 10px rgba(0,0,0,0.1); 
}
.card h3 { margin: 0; font-size: 18px; }
.card p { font-size: 16px; margin-top: 10px; }
.ok { color: green; font-weight: bold; }
.alert { color: red; font-weight: bold; }
</style>
</head>
<body>
  <div class="main-box">
    <h1>🔥 Fire Alarm Dashboard</h1>
    <div class="container">
      <div class="card"><h3>🌡 Temp</h3><p id="temp">-- °C</p></div>
      <div class="card"><h3>💧 Humidity</h3><p id="hum">-- %</p></div>
      <div class="card"><h3>💨 Smoke</h3><p id="smoke">--</p></div>
      <div class="card"><h3>⚗ Gas</h3><p id="gas">--</p></div>
      <div class="card"><h3>🔥 Fire</h3><p id="fire">--</p></div>
    </div>
  </div>

<script>
async function fetchData() {
  const res = await fetch("/data");
  const data = await res.json();
  document.getElementById("temp").innerText = data.temperature + " °C";
  document.getElementById("hum").innerText  = data.humidity + " %";
  document.getElementById("smoke").innerText = data.smoke_value + " [" + data.smoke_status + "]";
  document.getElementById("gas").innerText   = data.gas_value + " [" + data.gas_status + "]";
  document.getElementById("fire").innerText  = data.fire_status;
}
setInterval(fetchData, 2000);
fetchData();
</script>
</body>
</html>
)rawliteral";

// =====================
// Web Handlers
// =====================
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  // Lấy snapshot dữ liệu an toàn với mutex
  float temperature; float humidity;
  int smokeValue; int gasValue; int fireValue;
  String smoke_status, gas_status, fire_status;

  if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    temperature   = g_temperature;
    humidity      = g_humidity;
    smokeValue    = g_smokeValue;
    gasValue      = g_gasValue;
    fireValue     = g_fireValue;
    smoke_status  = g_smoke_status;
    gas_status    = g_gas_status;
    fire_status   = g_fire_status;
    xSemaphoreGive(gDataMutex);
  } else {
    // nếu không lấy được mutex, gửi giá trị gần nhất cũng được
    temperature   = g_temperature;
    humidity      = g_humidity;
    smokeValue    = g_smokeValue;
    gasValue      = g_gasValue;
    fireValue     = g_fireValue;
    smoke_status  = g_smoke_status;
    gas_status    = g_gas_status;
    fire_status   = g_fire_status;
  }

  String json = "{";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"smoke_value\":" + String(smokeValue) + ",";
  json += "\"smoke_status\":\"" + smoke_status + "\",";
  json += "\"gas_value\":" + String(gasValue) + ",";
  json += "\"gas_status\":\"" + gas_status + "\",";
  json += "\"fire_status\":\"" + fire_status + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// =====================
// MQTT helpers (ThingsBoard)
// =====================
void ensureMqttConnected() {
  if (!ENABLE_THINGSBOARD) return;

  if (!mqttClient.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (mqttClient.connect("ESP32_FireAlarm", access_token, NULL)) {
      Serial.println("OK");
    } else {
      Serial.print("failed, state=");
      Serial.println(mqttClient.state());
    }
  }
}

void publishToThingsBoard() {
  if (!ENABLE_THINGSBOARD) return;

  // Chuẩn bị payload từ snapshot dữ liệu
  float temperature; float humidity;
  int smokeValue; int gasValue; int fireValue;
  String smoke_status, gas_status, fire_status;

  if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    temperature   = g_temperature;
    humidity      = g_humidity;
    smokeValue    = g_smokeValue;
    gasValue      = g_gasValue;
    fireValue     = g_fireValue;
    smoke_status  = g_smoke_status;
    gas_status    = g_gas_status;
    fire_status   = g_fire_status;
    xSemaphoreGive(gDataMutex);
  } else {
    temperature   = g_temperature;
    humidity      = g_humidity;
    smokeValue    = g_smokeValue;
    gasValue      = g_gasValue;
    fireValue     = g_fireValue;
    smoke_status  = g_smoke_status;
    gas_status    = g_gas_status;
    fire_status   = g_fire_status;
  }

  String json = "{";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"smoke_value\":" + String(smokeValue) + ",";
  json += "\"smoke_status\":\"" + smoke_status + "\",";
  json += "\"gas_value\":" + String(gasValue) + ",";
  json += "\"gas_status\":\"" + gas_status + "\",";
  json += "\"fire_value\":" + String(fireValue) + ",";
  json += "\"fire_status\":\"" + fire_status + "\"";
  json += "}";

  mqttClient.publish("v1/devices/me/telemetry", json.c_str());
}

// =====================
// FreeRTOS Tasks
// =====================
void TaskSensor(void *pv) {
  const TickType_t PERIOD = pdMS_TO_TICKS(1000); // đọc mỗi 1s
  for (;;) {
    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();
    int smokeValue    = analogRead(MQ135_PIN);
    int gasValue      = analogRead(GAS_SENSOR_PIN);
    int fireValue     = analogRead(FIRE_SENSOR_PIN);

    // Sử dụng ngưỡng cố định thay vì baseline + margin
    String smoke_status = (smokeValue > 2100) ? "⚠️ Detected" : "✅ Clean";
    String gas_status   = (gasValue   > 1400) ? "⚠️ Detected" : "✅ Clean";
    String fire_status  = (fireValue < 1800) ? "🔥 Fire Detected!" : "✅ No Fire";

    // Điều khiển buzzer với ngưỡng cố định
    if (smokeValue > 2100 || gasValue > 1400 || fireValue < 1800) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }

    // Cập nhật dữ liệu dùng chung 
    if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      g_temperature  = temperature;
      g_humidity     = humidity;
      g_smokeValue   = smokeValue;
      g_gasValue     = gasValue;
      g_fireValue    = fireValue;
      g_smoke_status = smoke_status;
      g_gas_status   = gas_status;
      g_fire_status  = fire_status;
      xSemaphoreGive(gDataMutex);
    }

    vTaskDelay(PERIOD);
  }
}

void TaskTelemetry(void *pv) {
  const TickType_t PERIOD = pdMS_TO_TICKS(5000); // gửi mỗi 5s
  for (;;) {
    if (ENABLE_THINGSBOARD) {
      ensureMqttConnected();
      if (mqttClient.connected()) {
        publishToThingsBoard();
        mqttClient.loop(); // xử lý MQTT background
      }
    }
    vTaskDelay(PERIOD);
  }
}

// =====================
// Setup
// =====================
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Mutex
  gDataMutex = xSemaphoreCreateMutex();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.println("ESP32 IP Address: " + WiFi.localIP().toString());

  // MQTT
  if (ENABLE_THINGSBOARD) {
    mqttClient.setServer(mqtt_server, mqtt_port);
    ensureMqttConnected();
  }

  // Sensor calibration (giữ lại để tham khảo)
  long smokeSum = 0;
  long gasSum   = 0;
  const int samples = 100;
  for (int i = 0; i < samples; i++) {
    smokeSum += analogRead(MQ135_PIN);
    gasSum   += analogRead(GAS_SENSOR_PIN);
    delay(20);
  }
  smokeBaseline = smokeSum / samples;
  gasBaseline   = gasSum   / samples;
  Serial.print("Smoke baseline: "); Serial.println(smokeBaseline);
  Serial.print("Gas baseline: ");   Serial.println(gasBaseline);

  // Web routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");

  // ===== Tạo FreeRTOS tasks =====
  xTaskCreatePinnedToCore(TaskSensor,    "TaskSensor",    4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskTelemetry, "TaskTelemetry", 4096, NULL, 1, NULL, 1);
}

// =====================
// Loop
// =====================
void loop() {
  server.handleClient();
  if (ENABLE_THINGSBOARD) {
    mqttClient.loop();
  }
  delay(2);
}
