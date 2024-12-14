#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 if 0x3F not working

#define SS_PIN 5
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

int GREEN_LED = 3;
int RED_LED = 6;
int PIEZO = 8;

bool isEthernetConnected = false;
// Set to true for more status information to be printed to the Serial Monitor
bool IS_DEBUG_ENABLED = true;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0xA8, 0x61, 0x0A, 0xAE, 0xDE, 0x39 };
int    HTTP_PORT   = 3141;
String HTTP_METHOD = "GET"; // or "POST"
String PATH_NAME   = "/users";

IPAddress subnet(255, 255, 255, 0); // Subnet mask
EthernetUDP Udp;  // Create an EthernetUDP instance
unsigned int localPort = 8888;  // Port to listen on
IPAddress broadcastIP;  // Variable to hold the broadcast IP

// Variables to store the server IP and port for communication
IPAddress serverIP;
unsigned int serverPort = 8888; // Port where the server is listening

String EXPECTED_MESSAGE = "DISCOVER_SERVER";

unsigned long previousMillis = 0; // Stores the last time the function was called
const unsigned long interval = 15000; // Interval in milliseconds (15,000ms = 15 seconds)
unsigned long currentMillis = 0;

boolean isServerOnline = false;
boolean isDisplayingReadiness = false;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):

EthernetClient client;

void setup() 
{
  pinMode(RED_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  pinMode(PIEZO,OUTPUT);
  Serial.begin(9600);   // Initiate a serial communication

  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(F("Initializing..."));
  delay(1000);
  
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  // Check if the Ethernet cable is unplugged
  ethernetUnplugged();
  // Start the Ethernet connection if not already started
  if (!isEthernetConnected) {
    establishConnection();
  }
}

void loop() {
  // Check if the Ethernet cable is unplugged
  ethernetUnplugged();
  // Renew the DHCP lease if needed.
  Ethernet.maintain();

  currentMillis = millis(); // Get the current time

  // Check if 15 seconds have passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Update the last execution time
    getServerIP();
  }

  // Check if the server is online
  if (!isServerOnline) {
    return;
  }

  if (!isDisplayingReadiness) {
    Serial.println(F("Ready to scan."));
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Ready to scan."));
    isDisplayingReadiness = true;
  }
  
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Scanning..."));
  // Show UID on serial monitor
  String rfid= scanRFID();
  Serial.print(F("UID tag: "));
  Serial.print(rfid);
  Serial.println();
  // Change here the UID of the card/cards that you want to give access
  if (validateRFID(rfid)) {
    authorized();
  } else {
    denied();
  }
}

// Function to send a broadcast message
void sendBroadcastMessage(String message) {
  Udp.beginPacket(broadcastIP, localPort);  // Send to broadcast IP and local port
  Udp.write(message.c_str());
  Udp.endPacket();
  Serial.println(F("Broadcast message sent!"));
}

// Function to listen for a response and find the server IP
boolean listenForResponse() {
  Serial.println(F("Listening for response..."));
  int packetSize = Udp.parsePacket();  // Check for incoming UDP packets
  if (packetSize) {
    char packetBuffer[packetSize];
    Udp.read(packetBuffer, packetSize);
    String response = String(packetBuffer);
    Serial.print(F("Received response: "));
    Serial.println(response);

    // Check if the response contains the server IP (you can customize this as needed)
    if (response.indexOf("SERVER_IP") != -1) {
      // Parse the server IP from the response
      int ipStart = response.indexOf(":") + 1;  // Assuming the format is SERVER_IP:<ip>
      String ipString = response.substring(ipStart);
      serverIP = parseIPAddress(ipString);

      // Once we have the server IP, we can stop listening and start communication
      Serial.print(F("Server IP found: "));
      Serial.println(serverIP);
      return true;
      // You can now send requests to the server using serverIP
    }
  } else {
    Serial.println(F("Error! No packets received! Server may be offline!"));
    return false;
  }
}

// Function to parse an IP address from a string (e.g., "192.168.1.100")
IPAddress parseIPAddress(String ipString) {
  int idx1 = ipString.indexOf(".");
  int idx2 = ipString.indexOf(".", idx1 + 1);
  int idx3 = ipString.indexOf(".", idx2 + 1);

  int byte1 = ipString.substring(0, idx1).toInt();
  int byte2 = ipString.substring(idx1 + 1, idx2).toInt();
  int byte3 = ipString.substring(idx2 + 1, idx3).toInt();
  int byte4 = ipString.substring(idx3 + 1).toInt();

  return IPAddress(byte1, byte2, byte3, byte4);
}

// Function to calculate broadcast IP
IPAddress calculateBroadcastIP(IPAddress localIP, IPAddress subnetMask) {
  byte broadcast[4];
  for (int i = 0; i < 4; i++) {
    broadcast[i] = localIP[i] | (~subnetMask[i] & 255);  // Bitwise OR with inverse subnet mask
  }
  return IPAddress(broadcast[0], broadcast[1], broadcast[2], broadcast[3]);
}

void ethernetUnplugged() {
  if (Ethernet.linkStatus() == LinkOFF) {
    isEthernetConnected = false;
    Serial.println(F("Error! The Ethernet cable is unplugged!"));
    tone(PIEZO,800,200);
    delay(200);
    tone(PIEZO,400,200);
    delay(200);
    digitalWrite(RED_LED,0);
    digitalWrite(GREEN_LED,0);
    if (Ethernet.linkStatus() == LinkOFF) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("Ethernet cable"));
      lcd.setCursor(0,1);
      lcd.print(F("not connected!"));
      isDisplayingReadiness = false;
    }
    while (Ethernet.linkStatus() == LinkOFF) {
      digitalWrite(RED_LED,1);
      delay(500);
      digitalWrite(RED_LED,0);
      delay(500);
    }
    lcd.clear();
    establishConnection();
  }
}

void establishConnection() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Establishing"));
  lcd.setCursor(0,1);
  lcd.print(F("connection..."));
  ethernet:
  Serial.println(F("Attempting to establish an Ethernet connection..."));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // No point in carrying on, so do nothing forevermore:
    digitalWrite(RED_LED,1);
    tone(PIEZO,800,200);
    delay(200);
    tone(PIEZO,600,200);
    delay(200);
    tone(PIEZO,400,200);
    delay(10200);
    goto ethernet;
  }
  isEthernetConnected = true;
  Serial.print(F("Ethernet connected with ip: "));
  Serial.println(Ethernet.localIP());
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Connected w/ ip:"));
  lcd.setCursor(0,1);
  lcd.print(Ethernet.localIP());
  isDisplayingReadiness = false;
  
  // Calculate the broadcast address dynamically
  broadcastIP = calculateBroadcastIP(Ethernet.localIP(), subnet);

  // Give the Ethernet shield a second to initialize
  delay(1000);
  
  // Start UDP on specified port
  Udp.begin(localPort);
  Serial.print(F("Listening on port: "));
  Serial.println(localPort);
  
  Serial.print(F("Broadcast IP: "));
  Serial.println(broadcastIP);
  
  digitalWrite(RED_LED,0);
  digitalWrite(GREEN_LED,1);
  tone(PIEZO,400,100);
  delay(100);
  digitalWrite(RED_LED,1);
  tone(PIEZO,400,100);
  delay(100);
  digitalWrite(RED_LED,0);
  tone(PIEZO,500,100);
  delay(100);
  digitalWrite(GREEN_LED,0);

  delay(1000);
  
  getServerIP();
}

void getServerIP() {
  // Broadcast a message to discover the server
  sendBroadcastMessage(EXPECTED_MESSAGE);

  delay(1000);
  
  // Wait for a response (the server's IP)
  if(!listenForResponse()) {
    digitalWrite(RED_LED,1);
    digitalWrite(GREEN_LED,1);
    isServerOnline = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Server not found"));
    isDisplayingReadiness = false;
    return;
  }
  digitalWrite(RED_LED,0);
  digitalWrite(GREEN_LED,0);
  isServerOnline = true;
}

String scanRFID() {
  String rfid= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     rfid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
     rfid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  rfid.toUpperCase();
  return rfid;
}

bool validateRFID(String rfid) {
  if (IS_DEBUG_ENABLED) {
    Serial.print(F("Connecting to "));
    Serial.println(serverIP);
  }
  // If you get a connection, report back via serial:
  if (client.connect(serverIP, HTTP_PORT)) {
    if (IS_DEBUG_ENABLED) {
      Serial.println(F("Connected"));
    }
    // Send HTTP request header
    client.print(HTTP_METHOD);
    client.print(F(" "));
    client.print(PATH_NAME);
    client.print(F("?rfid="));
    client.print(rfid);
    client.println(F(" HTTP/1.1"));
    client.print(F("Host: "));
    client.println(serverIP);
    client.println(F("Connection: close"));
    client.println(); // end HTTP request header
  } else {
    // If you didn't get a connection to the server:
    if (IS_DEBUG_ENABLED) {
      Serial.println(F("Connection failed"));
    }
    ethernetUnplugged();
    getServerIP();
  }
  // If there are incoming bytes available
  // from the server, read them and print them:
  String response = "";
  while(client.connected()) {
    if (client.available()) {
      char c = client.read();
      response.concat(c);
    }
  }
  // If the server's disconnected, stop the client:
  if (!client.connected()) {
    if (IS_DEBUG_ENABLED) {
      Serial.println(response);
      Serial.println(F("disconnecting."));
    }
    client.stop();
    if (getResponseBody(response).equalsIgnoreCase(F("true"))) {
      return true;
    } else {
      return false;
    }
  }
}

String getResponseBody(String response) {
  String contentLengthString = response.substring(response.indexOf("Content-Length: ")+16,response.indexOf("\nETag"));
  int contentLength = contentLengthString.toInt();
  response = response.substring(response.length()-contentLength);
  return response;
}


void authorized() {
  isDisplayingReadiness = false;
  Serial.println(F("Access granted!\n"));
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Access granted!"));
  tone(PIEZO,1000,200);
  digitalWrite(GREEN_LED,1);
  delay(200);
  digitalWrite(GREEN_LED,0);
  delay(1000);
}

void denied() {
  isDisplayingReadiness = false;
  Serial.println(F("Access denied!\n"));
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Access denied!"));
  tone(PIEZO,500,500);
  for(int i = 0; i < 3; i++) {
    digitalWrite(RED_LED,1);
    delay(100);
    digitalWrite(RED_LED,0);
    delay(100);
  }
  delay(1000);
}
