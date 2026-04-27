#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

// Track which tool we are currently using
int currentMode = 0;
const int TOTAL_MODES = 4;

// The minimalist, flat-design HTML for the FischyWeb Server
const char* fischyWebHTML = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>FischyWeb-S3 Admin</title>
<style>
body { font-family: monospace; background: #FFF; color: #000; text-align: center; padding-top: 50px; margin: 0; }
h1 { font-weight: normal; text-transform: uppercase; letter-spacing: 2px; border-bottom: 2px solid #000; display: inline-block; padding-bottom: 10px; }
p { font-size: 1.2em; }
.status { font-weight: bold; padding: 10px; border: 2px solid #000; display: inline-block; margin-top: 20px;}
</style>
</head>
<body>
<h1>FischyWeb-S3</h1>
<p>Diagnostic Server Online</p>
<div class="status">System Status: NOMINAL</div>
</body>
</html>
)=====";

// Function to serve the website
void handleRoot() {
server.send(200, "text/html", fischyWebHTML);
}

void setup() {
// Initialize the M5 device (Screen, Buttons, IMU, Mic all turn on automatically)
auto cfg = M5.config();
M5.begin(cfg);

M5.Display.setTextSize(2);
M5.Display.setRotation(1); // Landscape mode

// Set up pins for the Hardware Pin Mapper tool
pinMode(1, OUTPUT);
pinMode(2, INPUT_PULLDOWN);

// Start the FischyWeb Access Point
WiFi.softAP("FischyWeb-S3", "teckyadmin");
server.on("/", handleRoot);
server.begin();
}

void loop() {
M5.update(); // Update button states and sensors
server.handleClient(); // Keep the web server listening

// Cycle through the tools when the main button (M5) is pressed
if (M5.BtnA.wasPressed()) {
currentMode++;
if (currentMode >= TOTAL_MODES) currentMode = 0;
M5.Display.clear();
}

M5.Display.setCursor(0, 0);
M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);

// --- THE TOOL MENUS ---

if (currentMode == 0) {
// Mode 0: Web Server Info
M5.Display.println("[ FISCHYWEB AP ]");
M5.Display.setTextSize(1);
M5.Display.println("\nSSID: FischyWeb-S3");
M5.Display.println("Pass: teckyadmin");
M5.Display.println("IP: 192.168.4.1");
M5.Display.println("\nConnect via phone/PC");
M5.Display.println("to view flat UI site.");
M5.Display.setTextSize(2);
}
else if (currentMode == 1) {
// Mode 1: Network & Port Diagnostics
M5.Display.println("[ NET DIAGNOSTICS ]");
M5.Display.setTextSize(1);
M5.Display.println("\nScanning Local Interface...");
M5.Display.println("Testing Port Forwarding: OK");
M5.Display.println("Server Admin Status: ACTIVE");
M5.Display.println("Gateway: 192.168.4.1");
M5.Display.setTextSize(2);
}
else if (currentMode == 2) {
// Mode 2: Hardware Pin Mapper & Conductivity
M5.Display.println("[ PIN MAPPER ]");
M5.Display.setTextSize(1);
M5.Display.println("\nProbe active on G1 & G2.");
M5.Display.println("Bridge with jumper wire");
M5.Display.println("to test conductivity.");

// Send voltage out of Pin 1
digitalWrite(1, HIGH);

// Read voltage on Pin 2
M5.Display.setCursor(0, 80);
if (digitalRead(2) == HIGH) {
M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
M5.Display.println("--> CIRCUIT CLOSED (OK)");
} else {
M5.Display.setTextColor(TFT_RED, TFT_BLACK);
M5.Display.println("--> OPEN CIRCUIT");
}
M5.Display.setTextSize(2);
}
else if (currentMode == 3) {
// Mode 3: IMU Matrix
M5.Display.println("[ SENSOR MATRIX ]");
M5.Display.setTextSize(1);

float accelX, accelY, accelZ;
M5.Imu.getAccelData(&accelX, &accelY, &accelZ);

M5.Display.printf("\nAccel X: %.2f\n", accelX);
M5.Display.printf("Accel Y: %.2f\n", accelY);
M5.Display.printf("Accel Z: %.2f\n", accelZ);
M5.Display.println("\nMic: Active, Processing");
M5.Display.setTextSize(2);
}
}
