// =============================
// ESP32 Multi-Sensor System + Web Dashboard + Buzzer Alarm
// MQ-135 Smoke + MQ Gas + Flame Sensor + DHT11
// =============================

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// --- WiFi Credentials ---
const char* ssid = "Rande 219 2.4G";
const char* password = "haidang2014";

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

// =====================
// Sensor Calibration Variables
// =====================
int smokeBaseline = 0;
int gasBaseline = 0;
const int smokeMargin = 200; // adjust after testing
const int gasMargin   = 260; // adjust after testing

// =====================
// HTML Dashboard
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
// Functions
// =====================
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int smokeValue = analogRead(MQ135_PIN);
  int gasValue   = analogRead(GAS_SENSOR_PIN);
  int fireValue  = analogRead(FIRE_SENSOR_PIN);

  String smoke_status = (smokeValue > smokeBaseline + smokeMargin) ? "⚠️ Detected" : "✅ Clean";
  String gas_status   = (gasValue   > gasBaseline   + gasMargin)   ? "⚠️ Detected" : "✅ Clean";
  String fire_status  = (fireValue < 2000) ? "🔥 Fire Detected!" : "✅ No Fire";

  // Control Buzzer
  if (smoke_status.indexOf("⚠️") >= 0 || gas_status.indexOf("⚠️") >= 0 || fire_status.indexOf("🔥") >= 0) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
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
// Setup
// =====================
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.println("ESP32 IP Address: " + WiFi.localIP().toString());

  // Sensor calibration
  long smokeSum = 0;
  long gasSum   = 0;
  const int samples = 100;
  for(int i=0;i<samples;i++){
    smokeSum += analogRead(MQ135_PIN);
    gasSum   += analogRead(GAS_SENSOR_PIN);
    delay(20);
  }
  smokeBaseline = smokeSum / samples;
  gasBaseline   = gasSum / samples;
  Serial.print("Smoke baseline: "); Serial.println(smokeBaseline);
  Serial.print("Gas baseline: "); Serial.println(gasBaseline);

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");
}

// =====================
// Loop
// =====================
void loop() {
  server.handleClient();
}
