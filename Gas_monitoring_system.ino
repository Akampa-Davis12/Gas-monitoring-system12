#include <WiFi.h>
#include <WebServer.h>

// ===== Pin Definitions =====
const int mq2Pin   = 32;   // MQ-2 AO connected to GPIO34 (Analog input)
const int redLed   = 18;   // Red LED -> GPIO18
const int greenLed = 19;   // Green LED -> GPIO19
const int yellowLed= 21;   // Yellow LED -> GPIO21
const int buzzer   = 5;    // Buzzer -> GPIO5

// ===== PPM Thresholds (adjust based on calibration) =====
const int safePPM    = 300;   // Below this = Safe
const int warningPPM = 800;   // Between safePPM and this = Warning
// Above warningPPM = Danger

// ===== WiFi Settings =====
const char* ssid = "TECNO POP 8";
const char* password = "Akampa.com";  // No password for Wokwi-Guest

// ===== System Variables =====
int gasPPM = 0;
bool alarmSilenced = false;
unsigned long lastBuzzerToggle = 0;
int buzzerState = LOW;
bool wifiConnected = false;
unsigned long lastSensorRead = 0;

WebServer server(80);

void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Gas Leak Detector</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; }";
  html += ".container { max-width: 600px; margin: 0 auto; }";
  html += ".status { font-size: 24px; margin: 20px 0; padding: 15px; border-radius: 10px; }";
  html += ".safe { background-color: #4CAF50; color: white; }";
  html += ".warning { background-color: #FFC107; color: black; }";
  html += ".danger { background-color: #F44336; color: white; }";
  html += "button { background-color: #2196F3; color: white; border: none; padding: 10px 20px;";
  html += "text-align: center; text-decoration: none; display: inline-block; font-size: 16px;";
  html += "margin: 10px 2px; cursor: pointer; border-radius: 5px; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>Gas Leak Detector</h1>";
  
  // Display current status
  if (gasPPM < safePPM) {
    html += "<div class='status safe'>✅ Air is Safe</div>";
  } else if (gasPPM < warningPPM) {
    html += "<div class='status warning'>⚠️ Warning: Gas level rising</div>";
  } else {
    html += "<div class='status danger'>🚨 DANGER! Gas Leak Detected 🚨</div>";
  }
  
  html += "<p>Current Gas Level: <strong>" + String(gasPPM) + " ppm</strong></p>";
  
  // Add silence button if in warning or danger
  if (gasPPM >= safePPM) {
    if (alarmSilenced) {
      html += "<p>Alarm is currently silenced</p>";
    } else {
      html += "<a href='/silence'><button>Silence Alarm</button></a>";
    }
  }
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
  Serial.println("[WEB] Dashboard served to client");
}

void handleSilence() {
  alarmSilenced = true;
  digitalWrite(buzzer, LOW);
  Serial.println("[WEB] User silenced the alarm");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleData() {
  String json = "{";
  json += "\"gasLevel\":" + String(gasPPM) + ",";
  json += "\"status\":\"";
  
  if (gasPPM < safePPM) {
    json += "safe\"";
  } else if (gasPPM < warningPPM) {
    json += "warning\"";
  } else {
    json += "danger\"";
  }
  
  json += ",\"alarmSilenced\":" + String(alarmSilenced ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void readGasSensor() {
  // Read analog value from MQ-2 sensor (0 - 4095)
  int sensorValue = analogRead(mq2Pin);
  Serial.println(sensorValue);
  // Convert ADC value to PPM scale (0 - 2000 ppm)
  gasPPM = map(sensorValue, 0, 4095, 0, 100000);

  Serial.print("Sensor ADC: ");
  Serial.print(sensorValue);
  Serial.print(" -> Gas Level: ");
  Serial.print(gasPPM);
  Serial.println(" ppm");

  // Control LEDs and buzzer based on gas levels
  if (gasPPM < safePPM) {
    // Safe air
    digitalWrite(greenLed, HIGH);
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, LOW);
    digitalWrite(buzzer, LOW);
    alarmSilenced = false; // Reset silence when gas levels return to safe
    Serial.println("✅ Air is Safe");
  }
  else if (gasPPM >= safePPM && gasPPM < warningPPM) {
    // Warning level
    digitalWrite(greenLed, LOW);
    digitalWrite(yellowLed, HIGH);
    digitalWrite(redLed, LOW);
    
    // Beeping buzzer in warning mode
    if (!alarmSilenced && millis() - lastBuzzerToggle > 500) {
      buzzerState = !buzzerState;
      digitalWrite(buzzer, buzzerState);
      lastBuzzerToggle = millis();
    }
    Serial.println("⚠️ Warning: Gas level rising");
  }
  else {
    // Danger level
    digitalWrite(greenLed, LOW);
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, HIGH);
    
    // Continuous buzzer in danger mode
    if (!alarmSilenced) {
      digitalWrite(buzzer, HIGH);
    } else {
      digitalWrite(buzzer, LOW);
    }
    Serial.println("🚨 DANGER! Gas Leak Detected 🚨");
  }
}

void setup() {
  Serial.begin(115200);

  // Setup LED pins as outputs
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(mq2Pin, INPUT);

  // Start with all off
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(buzzer, LOW);

  // Connect to WiFi (non-blocking)
  WiFi.begin(ssid, password);
  Serial.println("Attempting to connect to WiFi...");
  
  // Setup web server routes (will work if WiFi connects)
  server.on("/", handleRoot);
  server.on("/silence", handleSilence);
  server.on("/data", handleData);
  server.begin();

  Serial.println("🚨 Gas Leak Detection System Started 🚨");
  Serial.println("System will continue monitoring gas levels regardless of WiFi status");
}

void loop() {
  // Handle WiFi connection status
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 5000) { // Check every 5 seconds
    if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
      wifiConnected = true;
      Serial.println("WiFi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    } 
    else if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      wifiConnected = false;
      Serial.println("WiFi connection lost!");
    }
    lastWiFiCheck = millis();
  }

  // Handle web server if connected
  if (wifiConnected) {S
    server.handleClient();
  }

  // Read sensor and update indicators every second
  if (millis() - lastSensorRead >= 10000) {
    readGasSensor();
    lastSensorRead = millis();
  }
}