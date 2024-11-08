#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <DHT.h>
#include <TFT_eSPI.h>
#include <ThingSpeak.h>


// WiFi Information
#define WIFI_SSID "Meo"
#define WIFI_PASSWORD "zxcvbnml"
const int myChannelNumber = 2700213;
const char* myApiKey = "8CAU4D1SWWJ6EK8A";

// Firebase Information
#define FIREBASE_HOST "iot-webdemo-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "SDJKsByrq1j8viX9umKdwoESBkzh3HSelgHtE8dz"

FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

MAX30105 particleSensor;
TFT_eSPI tft = TFT_eSPI(172, 320);
DHT dht(19, DHT11);

// Button and LED Pin Definitions
#define BUTTON_PIN 17  // GPIO 17 for button
bool ledState = false;  // Tracks LED state

volatile bool buttonPressed = false; // Track button press in interrupt

uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t bufferLength = 100;
int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate;
float spo2Sum = 0, heartRateSum = 0;
int spo2Count = 0, heartRateCount = 0;

// Interrupt Service Routine (ISR) for button press
void IRAM_ATTR handleButtonPress() {
    buttonPressed = true;  // Set flag to indicate button press
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Initialize TFT Display
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    // Initialize GPIO pins
    pinMode(BUTTON_PIN, INPUT);  // Button connected to 3.3V with no pull-up resistor
    attachInterrupt(BUTTON_PIN, handleButtonPress, FALLING);  // Attach interrupt to button pin

    // Connect to WiFi
    tft.drawString("Connecting WiFi...", tft.width() / 2, 20, 2);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("WiFi Connected!", tft.width() / 2, 20, 2);
    tft.drawString("IP: " + WiFi.localIP().toString(), tft.width() / 2, 50, 2);
    ThingSpeak.begin(client);


    // Configure Firebase
    firebaseConfig.host = FIREBASE_HOST;
    firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&firebaseConfig, &firebaseAuth);
    Firebase.reconnectWiFi(true);
    if (Firebase.ready()) {
        tft.drawString("Firebase Ready!", tft.width() / 2, 80, 2);
    } else {
        tft.drawString("Firebase Failed!", tft.width() / 2, 80, 2);
    }

    dht.begin();
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        tft.drawString("MAX30105 not found", tft.width() / 2, 110, 2);
        delay(1000);
        ESP.restart();
    } else {
        tft.drawString("MAX30105 Ready!", tft.width() / 2, 110, 2);
    }

    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        tft.drawString("DHT11 not ready", tft.width() / 2, 140, 2);
        delay(1000);
        ESP.restart();
    } else {
        tft.drawString("DHT11 Ready!", tft.width() / 2, 140, 2);
    }

    particleSensor.setup(60, 4, 3, 100, 411, 4096);
}

void loop() {
    // Handle button press
    if (buttonPressed) {
        buttonPressed = false;  // Clear flag
        ledState = !ledState;  // Toggle LED state

     
        // Update Firebase with LED state
        if (Firebase.ready()) {
            Firebase.setBool(firebaseData, "/led/ledState", ledState);
        }
    }

    // Reconnect if WiFi or Firebase is disconnected
    if (WiFi.status() != WL_CONNECTED) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Reconnecting WiFi...", tft.width() / 2, 20, 2);
        WiFi.reconnect();
        delay(100);
    }
    if (!Firebase.ready()) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Reconnecting Firebase...", tft.width() / 2, 50, 2);
        Firebase.reconnectWiFi(true);
        delay(100);
    }

    // Collect data from MAX30105 sensor
    bool fingerDetected = false;
    for (int i = 0; i < 100; i++) {
        while (!particleSensor.available()) particleSensor.check();
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        particleSensor.nextSample();
        if (irBuffer[i] > 50000) fingerDetected = true;
    }

    // Process data only if finger is detected
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (fingerDetected) {
        maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer,
                                               &spo2, &validSPO2, &heartRate, &validHeartRate);

        if (validHeartRate && validSPO2) {
            spo2Sum += spo2;
            heartRateSum += heartRate;
            spo2Count++;
            heartRateCount++;
        }

        float averageSPO2 = spo2Count > 0 ? spo2Sum / spo2Count : 0;
        float averageHeartRate = heartRateCount > 0 ? heartRateSum / heartRateCount : 0;

        // Display data on TFT
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("WiFi: Connected", tft.width() / 2, 20, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Firebase: Ready", tft.width() / 2, 50, 2);
        tft.drawString("SPO2: " + String(spo2) + "%", tft.width() / 2, 80, 2);
        tft.drawString("Heart Rate: " + String(heartRate) + " BPM", tft.width() / 2, 110, 2);
        tft.drawString("Avg SPO2: " + String(averageSPO2, 1), tft.width() / 2, 140, 2);
        tft.drawString("Avg Heart Rate: " + String(averageHeartRate, 1), tft.width() / 2, 170, 2);
        tft.drawString("Temp: " + String(temperature) + " C", tft.width() / 2, 200, 2);
        tft.drawString("Humidity: " + String(humidity) + "%", tft.width() / 2, 230, 2);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("LED: " + String(ledState ? "1" : "0"), tft.width() / 2, 260, 2);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Finger: YES", tft.width() / 2, 290, 2);

        // Send data to Firebase
        if (Firebase.ready()) {
            Firebase.setFloat(firebaseData, "/spo2", spo2);
            Firebase.setFloat(firebaseData, "/heartRate", heartRate);
            Firebase.setFloat(firebaseData, "/averageSPO2", averageSPO2);
            Firebase.setFloat(firebaseData, "/averageHeartRate", averageHeartRate);
            Firebase.setFloat(firebaseData, "/temperature", temperature);
            Firebase.setFloat(firebaseData, "/humidity", humidity);
            Firebase.setString(firebaseData, "/fingerStatus", "YES");
            Firebase.setBool(firebaseData, "/led/ledState", ledState);

            if (firebaseData.dataAvailable()) {
                Serial.println("Data sent successfully!");
            } else {
                Serial.println("Failed to send data.");
                Serial.println(firebaseData.errorReason());
            }
        }
    } else {
        // Display message when no finger is detected
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Finger: NO", tft.width() / 2, 20, 2);
        tft.drawString("Place finger on sensor", tft.width() / 2, 50, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Temp: " + String(temperature) + " C", tft.width() / 2, 110, 2);
        tft.drawString("Humidity: " + String(humidity) + "%", tft.width() / 2, 140, 2);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("LED: " + String(ledState ? "1" : "0"), tft.width() / 2, 170, 2);

        // Send only temperature and humidity to Firebase
        ThingSpeak.setField(3, temperature);
        ThingSpeak.setField(2, humidity);
  
        int x = ThingSpeak.writeFields(myChannelNumber, myApiKey);
  
        Serial.println("Temp: " + String(temperature, 2) + "°C");
        Serial.println("Humidity: " + String(humidity, 1) + "%");
  
        if (Firebase.ready()) {
            Firebase.setFloat(firebaseData, "/temperature", temperature);
            Firebase.setFloat(firebaseData, "/humidity", humidity);
            Firebase.setString(firebaseData, "/fingerStatus", "NO");
            Firebase.setBool(firebaseData, "/led/ledState", ledState);

            if (firebaseData.dataAvailable()) {
                Serial.println("Data sent successfully!");
            } else {
                Serial.println("Failed to send data.");
                Serial.println(firebaseData.errorReason());
            }
        }
    }
    delay(10); // Minimal delay for smoother operation
}
