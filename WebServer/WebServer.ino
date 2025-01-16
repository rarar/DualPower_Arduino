#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"

#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "dual-power-e8qo.onrender.com";  // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiSSLClient client;

int leftBulbValue = 0;
int rightBulbValue = 0;

/* -------------------------------------------------------------------------- */
void setup() {
  /* -------------------------------------------------------------------------- */
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network.
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:

  if (client.connect(server, 443)) {
    Serial.println("Connected to server");
    // Make an HTTP GET request:
    client.println("GET /get-latest-data HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println();
  }
}

/* -------------------------------------------------------------------------- */
void loop() {
  // Check if the client is connected
  if (!client.connected()) {
    Serial.println("Reconnecting to server...");
    if (client.connect(server, 443)) {
      Serial.println("Reconnected to server");
      client.println("GET /get-latest-data HTTP/1.1");
      client.println("Host: " + String(server));
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

/* -------------------------------------------------------------------------- */
void printWifiStatus() {
  /* -------------------------------------------------------------------------- */
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
