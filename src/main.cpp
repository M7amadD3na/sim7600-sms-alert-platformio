#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#define MODEM_PWRKEY      4
#define MODEM_TX          27
#define MODEM_RX          26
#define WIRE_INPUT_PIN    GPIO_NUM_13 
#define BOARD_LED         12 

HardwareSerial SerialAT(1);
AsyncWebServer server(80);
Preferences prefs;

String apn, phone, sms_text;

// HTML with a "Go to Sleep" confirmation
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>SIM7600 Setup</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; background: #ececec; }
    .card { background: white; padding: 20px; display: inline-block; border-radius: 10px; margin-top: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
    input { width: 80%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; }
    button { background: #007bff; color: white; border: none; padding: 12px 20px; border-radius: 5px; cursor: pointer; }
  </style>
</head><body>
  <div class="card">
    <h2>Device Configuration</h2>
    <form action="/get">
      APN: <input type="text" name="apn" placeholder="zain">
      Phone: <input type="text" name="phone" placeholder="079xxxx">
      Message: <input type="text" name="msg" placeholder="Alert!">
      <button type="submit">Save & Start Monitoring</button>
    </form>
    <p><small>Note: Device will sleep after saving.</small></p>
  </div>
</body></html>)rawliteral";

void enterDeepSleep() {
    Serial.println("Entering Sleep Mode... Monitoring Pin 13.");
    esp_sleep_enable_ext0_wakeup(WIRE_INPUT_PIN, 0); // Wake when Pin 13 goes LOW
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    pinMode(BOARD_LED, OUTPUT);

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        // --- TRIGGERED WAKEUP ---
        digitalWrite(BOARD_LED, HIGH);
        prefs.begin("config", true);
        phone = prefs.getString("phone", "");
        sms_text = prefs.getString("msg", "Triggered!");
        prefs.end();

        // Power on modem and send SMS
        pinMode(MODEM_PWRKEY, OUTPUT);
        digitalWrite(MODEM_PWRKEY, LOW); delay(100);
        digitalWrite(MODEM_PWRKEY, HIGH); delay(1000); 
        digitalWrite(MODEM_PWRKEY, LOW);
        delay(4000);

        SerialAT.println("AT+CMGF=1"); delay(500);
        SerialAT.print("AT+CMGS=\""); SerialAT.print(phone); SerialAT.println("\"");
        delay(500);
        SerialAT.print(sms_text); delay(100);
        SerialAT.write(26); delay(5000);

        enterDeepSleep(); 
    } else {
        // --- CONFIG MODE (Cold Boot) ---
        WiFi.softAP("SIM7600_CONFIG", "CHANGE_ME_STRONG_AP_PASS");
        Serial.print("Web Server at: "); Serial.println(WiFi.softAPIP());

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html);
        });

        server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
            prefs.begin("config", false);
            if (request->hasParam("apn")) prefs.putString("apn", request->getParam("apn")->value());
            if (request->hasParam("phone")) prefs.putString("phone", request->getParam("phone")->value());
            if (request->hasParam("msg")) prefs.putString("msg", request->getParam("msg")->value());
            prefs.end();
            
            request->send(200, "text/plain", "Settings Saved. Device is now entering sleep/monitor mode.");
            delay(3000);
            enterDeepSleep();
        });

        server.begin();
        // In Config mode, LED stays on so you know it's awake
        digitalWrite(BOARD_LED, HIGH); 
    }
}

void loop() {
    // Stays awake here during Config Mode
}