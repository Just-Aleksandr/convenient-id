const express = require("express");
const app = express();
const dgram = require("dgram");
const os = require("os");
const cors = require("cors");
const MAIN_PORT = 3141;

app.use(express.json());
app.use(cors());

const db = require("./models");

// Routers
const usersRouter = require("./routes/users");
const scansRouter = require("./routes/scans");
const { json } = require("sequelize");
app.use("/users", usersRouter);
app.use("/scans", scansRouter);

db.sequelize.sync().then(() => {
  app.listen(MAIN_PORT, () => {
    console.log("Server running on port " + MAIN_PORT);
  });
});

// Define the expected message
const EXPECTED_MESSAGE = "DISCOVER_SERVER";

// Create a UDP server
const server = dgram.createSocket("udp4");

// Define the port for receiving messages from Arduino
const UDP_PORT = 8888;

// Get the server's IP address (local network IP)
const serverIP = getEthernetIP();

// Start the UDP server to listen for incoming packets
server.on("message", (msg, rinfo) => {
  console.log(`Received message from ${rinfo.address}:${rinfo.port}`);
  // Respond only when the Arduino sends the correct message
  if (msg == EXPECTED_MESSAGE) {
    console.log("Valid message received. Sending response...");
    const responseMessage = `SERVER_IP:${serverIP}`;
    server.send(responseMessage, 0, responseMessage.length, rinfo.port, rinfo.address, (err) => {
      if (err) {
        console.error("Error sending response:", err);
      } else {
        console.log("Sent server IP response to Arduino");
      }
    });
  } else {
    console.log("Received message did not match the expected message. Ignoring...");
  }
});

// Start listening on the specified port
server.bind(UDP_PORT, () => {
  console.log(`Server is listening for incoming messages on port ${UDP_PORT}...`);
});

function getEthernetIP() {
  const interfaces = os.networkInterfaces();

  for (const ifaceName in interfaces) {
    const ifaceDetails = interfaces[ifaceName];
    for (const details of ifaceDetails) {
      if (
        details.family === "IPv4" && // Only IPv4 addresses
        !details.internal && // Exclude internal interfaces
        (ifaceName.toLowerCase().includes("ethernet") || // Match "Ethernet" in the name
          ifaceName.toLowerCase().includes("en") || // Match for Linux/Mac (e.g., "en0")
          ifaceName.toLowerCase().includes("wi-fi"))
      ) {
        return details.address;
      }
    }
  }

  return null; // No suitable interface found
}

const ethernetIP = getEthernetIP();
console.log("Ethernet IP:", ethernetIP);
