#include "WiFiS3.h"
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password
int keyIndex = 0;           // your network key index number (not used for WPA/WPA2)

int status = WL_IDLE_STATUS;

// Replace with your computer's local IP address and the port number
char serverAddress[] = "10.0.0.217";  // Replace with your computer's local IP
int serverPort = 3000;

WiFiClient client;

int leftBulbValue = 0;
int rightBulbValue = 0;

void setup() {
  Serial.begin(9600);
  // Check for the presence of the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  // Attempt to connect to the WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);  // Wait 10 seconds for connection to establish
  }

  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // Connect to the server:
  if (client.connect(serverAddress, serverPort)) {
    Serial.println("Connected to server");
    // Make an HTTP GET request:
    client.println("GET /get-latest-data HTTP/1.1");
    client.println("Host: " + String(serverAddress));
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Connection failed");
  }
}

void loop() {
  // Check if the client is connected
  if (!client.connected()) {
    Serial.println("Reconnecting to server...");
    if (client.connect(serverAddress, serverPort)) {
      Serial.println("Reconnected to server");
      client.println("GET /get-latest-data HTTP/1.1");
      client.println("Host: " + String(serverAddress));
      client.println("Connection: close");
      client.println();
    } else {
      Serial.println("Failed to reconnect to server");
      delay(1000);  // Wait for 1 second before retrying
      return;       // Skip the rest of the loop on failure
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  String response = "";
  while (client.available()) {
    char c = client.read();
    response += c;  // Append each char to the response string
  }

 // Debug: Print the raw response
  Serial.println("Raw response:");
  Serial.println(response);

  // Simple Text Parsing
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

  // Print the parsed values
  Serial.print("Left Bulb Value: ");
  Serial.println(leftBulbValue);
  Serial.print("Right Bulb Value: ");
  Serial.println(rightBulbValue);

  // Wait for 1 second before making the next request
  delay(1000);
}


void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
