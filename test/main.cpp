#include <Arduino.h>

// --- Pin Definitions for LILYGO SIM7600E-H ---
#define MODEM_PWRKEY          4
#define MODEM_TX              27
#define MODEM_RX              26
#define WIRE_INPUT_PIN        13
#define BOARD_LED             12 // Onboard LED for status

// --- Configuration ---
const char destinationNumber[] = "YOUR_PHONE_NUMBER";
bool isAlertSent = false;

// Hardware Serial 1 for the Modem
HardwareSerial SerialAT(1);

void modemPowerOn() {
    pinMode(MODEM_PWRKEY, OUTPUT);
    Serial.println("MCU: Powering on SIM7600...");
    
    // SIM7600 Power-on pulse (approx 500ms - 2s)
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(100);
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(1000); 
    digitalWrite(MODEM_PWRKEY, LOW);
    
    // Wait for the modem to initialize and register
    delay(10000); 
    Serial.println("MCU: Modem should be awake.");
}

void sendSMS(String message) {
    Serial.println("MCU: Sending SMS...");
    
    SerialAT.println("AT+CMGF=1"); // Set SMS to Text Mode
    delay(500);
    
    SerialAT.print("AT+CMGS=\"");
    SerialAT.print(destinationNumber);
    SerialAT.println("\"");
    delay(500);
    
    SerialAT.print(message);
    delay(100);
    
    SerialAT.write(26); // Ctrl+Z character to end SMS
    delay(5000); 
    
    Serial.println("MCU: SMS Processed.");
}

void setup() {
    Serial.begin(115200);
    
    // Initialize SerialAT for SIM7600
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    
    // Initialize Pins
    pinMode(WIRE_INPUT_PIN, INPUT_PULLDOWN); 
    pinMode(BOARD_LED, OUTPUT);
    
    modemPowerOn();
    
    Serial.println("--- SYSTEM ARMED ---");
    Serial.println("Monitoring Pin 13 (Expecting 3.3V)...");
}

void loop() {
    // Read the state of Pin 13
    int wireState = digitalRead(WIRE_INPUT_PIN);

    // If wire is connected to 3.3V, state is HIGH (1)
    // If wire is floating, state is LOW (0) due to PULLDOWN
    if (wireState == LOW) {
        if (!isAlertSent) {
            digitalWrite(BOARD_LED, HIGH);
            Serial.println("🚨 ALERT: Pin 13 is FLOATING!");
            
            sendSMS("SAFETY ALERT: Pin 13 has been disconnected or is floating.");
            
            isAlertSent = true; // Prevent spamming
        }
    } else {
        // Reset the alert status if the wire is reconnected
        if (isAlertSent) {
            Serial.println("✅ Wire reconnected.");
            isAlertSent = false;
            digitalWrite(BOARD_LED, LOW);
        }
    }

    delay(100); // Small stability delay
}