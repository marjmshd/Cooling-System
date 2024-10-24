#include <DHT.h>    // Include the DHT sensor library

#define DHTPIN 21       // Define the pin where the DHT11 sensor is connected
#define DHTTYPE DHT11   // Define the type of DHT sensor (DHT11)
#define FAN_PIN 9       // Use GPIO pin 9 for fan control
#define TURN_ON_THRESHOLD 30    // Temperature threshold to turn the fan ON
#define TURN_OFF_THRESHOLD 29   // Temperature threshold to turn the fan OFF

DHT dht(DHTPIN, DHTTYPE);  // Create the DHT object

bool fanState = false;      // Track whether the fan is on or off
bool manualOverride = false;   // Track whether manual override is active
unsigned long manualOverrideEndTime = 0;  // Time when manual override should end (for temporary manual control)

void setup() {
    Serial.begin(9600);   // Start the Serial communication (for USB)
    Serial1.begin(9600);  // Bluetooth serial communication with HC-05 (for communication with Pi)
    dht.begin();          // Initialize the DHT sensor
    pinMode(FAN_PIN, OUTPUT);  // Set the fan pin as an output

    // Initialize the fan in the OFF state
    digitalWrite(FAN_PIN, LOW);
    Serial.println("Teensy ready to control the fan...");
}

void loop() {
    // Read temperature from the DHT11 sensor
    float temperature = dht.readTemperature();
    Serial.print("Current Temperature: ");
    Serial.println(temperature);
    Serial1.print("Current Temperature: ");
    Serial1.println(temperature);

    // Check for manual override commands from Bluetooth (Serial1)
    if (Serial1.available() > 0) {
        String command = Serial1.readString().trim();  // Read the incoming Bluetooth command

        // Debugging: Print the exact command received to check for invisible characters
        Serial.print("Received command from Raspberry Pi: [");
        Serial.print(command);  // Print the raw command for debugging
        Serial.println("]");

        // Manual control commands
        if (command == "MANUAL_ON") {
            manualOverride = true;   // Enable manual override
            manualOverrideEndTime = millis() + 10000;  // Set 10 seconds of manual override
            fanState = true;         // Set fan state to ON
            digitalWrite(FAN_PIN, HIGH);  // Turn fan ON
            Serial.println("Fan ON (Manual Command)");
        } else if (command == "MANUAL_OFF") {
            manualOverride = true;   // Enable manual override
            manualOverrideEndTime = millis() + 10000;  // Set 10 seconds of manual override
            fanState = false;        // Set fan state to OFF
            digitalWrite(FAN_PIN, LOW);   // Turn fan OFF
            Serial.println("Fan OFF (Manual Command)");
        }
    }

    // If manual override was set, check if the timeout has been reached
    if (manualOverride && millis() > manualOverrideEndTime) {
        manualOverride = false;  // Disable manual override after timeout
        Serial.println("Manual control timeout, resuming automatic control...");
    }

    // Automatic control only if manual override is not active
    if (!manualOverride) {
        if (temperature > TURN_ON_THRESHOLD && !fanState) {
            digitalWrite(FAN_PIN, HIGH);  // Turn fan ON based on temperature
            fanState = true;
            Serial.println("Fan ON (Temperature Control)");
        } else if (temperature < TURN_OFF_THRESHOLD && fanState) {
            digitalWrite(FAN_PIN, LOW);   // Turn fan OFF based on temperature
            fanState = false;
            Serial.println("Fan OFF (Temperature Control)");
        }
    }

    delay(2000);  // Delay for 2 seconds before the next reading
}
