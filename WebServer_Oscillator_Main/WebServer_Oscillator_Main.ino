#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"
#include "arduino_secrets.h"

// Motor control parameters
const int directionPinA = 12;
const int pwmPinA = 3;
const int brakePinA = 9;
const int directionPinB = 13;
const int pwmPinB = 11;
const int brakePinB = 8;

// WiFi and server parameters
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char server[] = "dual-power-obhp.onrender.com";
WiFiSSLClient client;

int status = WL_IDLE_STATUS;
int leftBulbValue = 0;
int rightBulbValue = 0;

float t = 0.0;           // time in the sine function for oscillation
float frequency = 0.01;  // speed of oscillation
float rapidFrequency = 0.1;  // Increased speed of oscillation for rapid effect
int minBrightness = 0;
int maxBrightness = 255;
unsigned long startMillis;
const unsigned long rapidDuration = 5000;  // Rapid oscillation for 5 seconds

unsigned long lastOscillationTime = 0;
const unsigned long oscillationInterval = 30000; // 30 seconds in milliseconds

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  // Motor setup
  pinMode(directionPinA, OUTPUT);
  pinMode(pwmPinA, OUTPUT);
  pinMode(brakePinA, OUTPUT);
  pinMode(directionPinB, OUTPUT);
  pinMode(pwmPinB, OUTPUT);
  pinMode(brakePinB, OUTPUT);

  // Setup WiFi
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true) { ; }
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  startMillis = millis();
  while (status != WL_CONNECTED && (millis() - startMillis) < rapidDuration) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);

    // Rapid oscillation effect
    rapidOscillation();

    delay(10);  // Short delay for very rapid oscillation
  }

  if (status != WL_CONNECTED) {
    Serial.println("Failed to connect within the time limit.");
  } else {
    printWifiStatus();
  }
}
void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    // If WiFi is not connected, perform rapid oscillation
    Serial.println("WiFi disconnected. Starting rapid oscillation.");
    unsigned long startOscillation = millis();
    while ((millis() - startOscillation) < rapidDuration) {
      rapidOscillation();
      delay(10); // Short delay for very rapid oscillation
    }
    // Attempt to reconnect to WiFi
    reconnectToWiFi();
  } else {
    
    // Check if it's time for rapid oscillation
    if (millis() - lastOscillationTime >= oscillationInterval) {
      // Perform rapid oscillation for a short duration
      unsigned long startOscillation = millis();
      while ((millis() - startOscillation) < rapidDuration) {
        rapidOscillation();
        delay(10); // Short delay for rapid oscillation
      }
      lastOscillationTime = millis(); // Reset the timer
    }
    
    // If WiFi is connected, check if the client is connected to the server
    if (!client.connected()) {
      reconnectToServer();
    }
    // Read server response
    String response = getServerResponse();
    parseResponse(response);

    // Update motor speed based on server response
    int brightnessA = map(leftBulbValue, 0, 255, 0, 100) + 10 * sin(t);
    int brightnessB = map(rightBulbValue, 0, 255, 0, 100) + 10 * sin(t + PI / 2);

    // Oscillate brightness
    t += frequency;
    if (t > 2 * PI) {
      t -= 2 * PI;
    }

    // Control the motors
    updateMotor(directionPinA, pwmPinA, brakePinA, brightnessA);
    updateMotor(directionPinB, pwmPinB, brakePinB, brightnessB);

    // Print values
    Serial.print("Server Left Bulb Value: ");
    Serial.println(leftBulbValue);
    Serial.print("Server Right Bulb Value: ");
    Serial.println(rightBulbValue);
    Serial.print("Calculated Brightness A: ");
    Serial.println(brightnessA);
    Serial.print("Calculated Brightness B: ");
    Serial.println(brightnessB);
  }
}

void reconnectToWiFi() {
  Serial.println("Attempting to reconnect to WiFi...");
  status = WiFi.begin(ssid, pass);
  if (status != WL_CONNECTED) {
    Serial.println("Failed to reconnect to WiFi.");
  } else {
    Serial.println("Reconnected to WiFi.");
    printWifiStatus();
    lastOscillationTime = millis(); // Reset the timer upon successful reconnection
  }
}
void reconnectToServer() {
  if (client.connect(server, 443)) {
    Serial.println("Reconnected to server");
    client.println("GET /get-latest-data HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Failed to reconnect to server");
    delay(1000);
  }
}

String getServerResponse() {
  String response = "";
  while (client.available()) {
    char c = client.read();
    response += c;
  }
  return response;
}

void parseResponse(const String& response) {
  int leftIndex = response.indexOf("\"left\",\"totalVotes\":");
  int rightIndex = response.indexOf("\"right\",\"totalVotes\":");
  if (leftIndex != -1 && rightIndex != -1) {
    int leftStart = leftIndex + String("\"left\",\"totalVotes\":").length();
    int rightStart = rightIndex + String("\"right\",\"totalVotes\":").length();
    int leftEnd = response.indexOf(',', leftStart);
    int rightEnd = response.indexOf('}', rightStart);
    leftBulbValue = response.substring(leftStart, leftEnd).toInt();
    rightBulbValue = response.substring(rightStart, rightEnd).toInt();
  }
}

void rapidOscillation() {
  // Calculate brightness for motor A and B with rapid oscillation
  int brightnessA = (sin(t) + 1) / 2 * (maxBrightness - minBrightness) + minBrightness;
  int brightnessB = (sin(t + PI / 2) + 1) / 2 * (maxBrightness - minBrightness) + minBrightness;

  // Update motor A and B
  updateMotor(directionPinA, pwmPinA, brakePinA, brightnessA);
  updateMotor(directionPinB, pwmPinB, brakePinB, brightnessB);

  // Increment time
  t += rapidFrequency;
  if (t > 2 * PI) {
    t -= 2 * PI;
  }
}

void updateMotor(int directionPin, int pwmPin, int brakePin, int value) {
  digitalWrite(brakePin, LOW);
  digitalWrite(directionPin, value > 127 ? HIGH : LOW);
  analogWrite(pwmPin, value > 127 ? 255 - value : value);
}


void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

