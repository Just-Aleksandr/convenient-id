#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>

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
char   HOST_NAME[] = "192.168.1.9"; // hostname of web server:
String PATH_NAME   = "/users";

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
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  // Check if the Ethernet cable is unplugged
  ethernetUnplugged();
  // Start the Ethernet connection if not already started
  if (!isEthernetConnected) {
    establishConnection();
  }
  // Give the Ethernet shield a second to initialize
  delay(1000);
  Serial.println("Ready to scan.");
}

void loop() {
  // Check if the Ethernet cable is unplugged
  ethernetUnplugged();
  // Renew the DHCP lease if needed.
  Ethernet.maintain();
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
  // Show UID on serial monitor
  String rfid= scanRFID();
  Serial.print("UID tag: " + rfid);
  Serial.println();
  // Change here the UID of the card/cards that you want to give access
  if (validateRFID(rfid)) {
    authorized();
  } else {
    denied();
  }
}

void ethernetUnplugged() {
  if (Ethernet.linkStatus() == LinkOFF) {
    isEthernetConnected = false;
    Serial.println("Error! The Ethernet cable is unplugged!");
    tone(PIEZO,800,200);
    delay(200);
    tone(PIEZO,400,200);
    delay(200);
    while (Ethernet.linkStatus() == LinkOFF) {
      digitalWrite(RED_LED,1);
      delay(500);
      digitalWrite(RED_LED,0);
      delay(500);
    }
    establishConnection();
  }
}

void establishConnection() {
  ethernet:
  Serial.println("Attempting to establish an Ethernet connection...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
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
  Serial.print("Ethernet connected with ip: ");
  Serial.println(Ethernet.localIP());
  isEthernetConnected = true;
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
  digitalWrite(RED_LED,1);
  tone(PIEZO,400,100);
  delay(100);
  digitalWrite(RED_LED,0);
  tone(PIEZO,400,100);
  delay(100);
  digitalWrite(RED_LED,1);
  tone(PIEZO,500,100);
  delay(100);
  digitalWrite(RED_LED,0);
  digitalWrite(GREEN_LED,0);
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
    Serial.println("Connecting...");
  }
  // If you get a connection, report back via serial:
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    if (IS_DEBUG_ENABLED) {
      Serial.println("Connected");
    }
    // Send HTTP request header
    client.println(HTTP_METHOD + " " + PATH_NAME + "?rfid=" + rfid + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP request header
  } else {
    // If you didn't get a connection to the server:
    if (IS_DEBUG_ENABLED) {
      Serial.println("Connection failed");
    }
    
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
      Serial.println("disconnecting.");
    }
    client.stop();
    if (getResponseBody(response).equalsIgnoreCase("true")) {
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
  Serial.println("Authorized access\n");
  tone(PIEZO,1000,200);
  digitalWrite(GREEN_LED,1);
  delay(200);
  digitalWrite(GREEN_LED,0);
  delay(1000);
}

void denied() {
  Serial.println("Access denied\n");
  tone(PIEZO,500,500);
  for(int i = 0; i < 3; i++) {
    digitalWrite(RED_LED,1);
    delay(100);
    digitalWrite(RED_LED,0);
    delay(100);
  }
  delay(1000);
}
