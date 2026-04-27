// src/main.cpp
#include <Arduino.h>
#include <M5StickC.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>


// Screen/menu constants
#define NUM_SCREENS 5
enum Screen { SCREEN_CLOCK = 0, SCREEN_JOKES, SCREEN_REACTION, SCREEN_KIDS, SCREEN_SETTINGS };

Screen currentScreen = SCREEN_CLOCK;

// Jokes for family-friendly fun
const char* jokes[] = {
  "Why did the tomato blush? Because it saw the salad dressing!",
  "What do you call a sleeping bull? A bulldozer!",
  "Why did the math book look sad? It had too many problems.",
  "Why don’t eggs tell jokes? They'd crack each other up!"
};
const int NUM_JOKES = sizeof(jokes) / sizeof(jokes[0]);
int jokeIndex = 0;

// UI / timing state
unsigned long lastScreenUpdate = 0;
unsigned long clockUpdateMs = 1000; // update clock every second

// WiFi / Time
const char* NTPSERVER = "pool.ntp.org";
const long  GMT_OFFSET_SEC = 0; // adjust to your timezone in seconds
const int   DAYLIGHT_OFFSET_SEC = 0;

// Reaction game state
bool reactionActive = false;
unsigned long reactionReadyTime = 0;
unsigned long reactionStartTime = 0;
unsigned long reactionDelayMin = 1200;
unsigned long reactionDelayMax = 3500;
int reactionScore = 0;

// Kids game state
int kidsScore = 0;
int targetNumber = 0;
int kidsPhase = 0; // 0..2

// Settings
bool nightMode = false;

// Helpers
void setupWiFiIfPossible() {
  if (WIFI_SSID[0] == '\0') return; // no credentials defined
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long t = millis();
  // wait up to 8 seconds for WiFi
  while (WiFi.status() != WL_CONNECTED && millis() - t < 8000) {
    delay(100);
  }
}

void initTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTPSERVER);
}

void beeper(int freq = 440, int ms = 150) {
  // Built-in M5 speaker call (works on many M5 variants)
  // If not available, you may replace with tone() on a buzzer pin
  // Using M5StickC library: M5.Speaker
  // We guard by try/catch style: ensure the API exists
  #if defined(M5)
  // Some platforms expose M5.Speaker; if not, this is a no-op
  #endif
  // Fallback: attempt Arduino tone on a common buzzer pin if you wired one
  // Not all boards: left as a no-op to avoid compile errors
}

// Fancy text wrap helper for small screens
void wrapPrint(const char* s, int maxCharsPerLine, int y) {
  String str = String(s);
  int len = str.length();
  int idx = 0;
  int x = 0;
  M5.Lcd.setCursor(0, y);
  while (idx < len) {
    int take = min(maxCharsPerLine, len - idx);
    String part = str.substring(idx, idx + take);
    M5.Lcd.print(part);
    idx += take;
    if (idx < len) {
      M5.Lcd.setCursor(0, y += 16);
    }
  }
}

// Screen rendering
void drawClockScreen() {
  // Clear with night/day style
  if (nightMode) {
    M5.Lcd.fillScreen(BLACK);
  } else {
    M5.Lcd.fillScreen(WHITE);
  }

  // Time display
  time_t now;
  struct tm* tm;
  time(&now);
  tm = localtime(&now);
  char timebuf[9];
  if (tm) {
    snprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
  } else {
    snprintf(timebuf, sizeof(timebuf), "00:00:00");
  }

  // Title
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(nightMode ? WHITE : BLACK);
  M5.Lcd.setCursor(4, 4);
  M5.Lcd.print("Time");

  // Time value
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(4, 28);
  M5.Lcd.print(timebuf);

  // Date line
  if (tm) {
    char datebuf[20];
    strftime(datebuf, sizeof(datebuf), "%Y-%m-%d", tm);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(4, 62);
    M5.Lcd.print(datebuf);
  }

  // Menu hint
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(4, 110);
  M5.Lcd.print("A/B to switch screens");
}

void drawJokesScreen() {
  if (nightMode) M5.Lcd.fillScreen(BLACK);
  else M5.Lcd.fillScreen(WHITE);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(nightMode ? WHITE : BLACK);
  M5.Lcd.setCursor(4, 8);
  M5.Lcd.print("Kid Jokes");

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(4, 34);
  M5.Lcd.print(jokes[jokeIndex]);

  // Show which joke
  M5.Lcd.setCursor(4, 60);
  M5.Lcd.print("Joke ");
  M5.Lcd.print(jokeIndex + 1);
  M5.Lcd.print(" / ");
  M5.Lcd.print(NUM_JOKES);

  // Next/back hints
  M5.Lcd.setCursor(4, 100);
  M5.Lcd.print("A: prev  B: next");
}

void drawReactionScreen() {
  if (nightMode) M5.Lcd.fillScreen(BLACK);
  else M5.Lcd.fillScreen(WHITE);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(nightMode ? WHITE : BLACK);
  M5.Lcd.setCursor(4, 8);
  M5.Lcd.print("Reaction Game");

  // Show status
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(4, 34);
  if (!reactionActive) {
    M5.Lcd.print("Tap to start → GO!");
  } else {
    M5.Lcd.print("GO! Tap any button!");
  }

  // Score
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(4, 60);
  M5.Lcd.print("Score: ");
  M5.Lcd.print(reactionScore);

  M5.Lcd.setCursor(4, 110);
  M5.Lcd.print("B to reset score");
}

void drawKidsScreen() {
  if (nightMode) M5.Lcd.fillScreen(BLACK);
  else M5.Lcd.fillScreen(WHITE);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(nightMode ? WHITE : BLACK);
  M5.Lcd.setCursor(4, 8);
  M5.Lcd.print("Kids Play");

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(4, 34);
  M5.Lcd.print("Guess the number 1-9");

  // Display target
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(4, 60);
  M5.Lcd.print(targetNumber);

  // Score
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(4, 110);
  M5.Lcd.print("Score: ");
  M5.Lcd.print(kidsScore);
  M5.Lcd.setCursor(60, 110);
  M5.Lcd.print("A: -  B: +"); // simple controls
}

void drawSettingsScreen() {
  if (nightMode) M5.Lcd.fillScreen(BLACK);
  else M5.Lcd.fillScreen(WHITE);

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(nightMode ? WHITE : BLACK);
  M5.Lcd.setCursor(4, 8);
  M5.Lcd.print("Settings");

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(4, 34);
  M5.Lcd.print("Night mode: ");
  M5.Lcd.print(nightMode ? "ON" : "OFF");

  M5.Lcd.setCursor(4, 52);
  M5.Lcd.print("WiFi: ");
  M5.Lcd.print(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");

  M5.Lcd.setCursor(4, 70);
  M5.Lcd.print("A: toggle Night mode");
  M5.Lcd.setCursor(4, 92);
  M5.Lcd.print("B: back to Clock");
}

cpp
 
// Continuation of main.cpp (setup and loop)
void setup() {
  // Initialize the display first
  M5.begin();

  // Basic UI setup
  M5.Lcd.setRotation(0);
  nightMode = false;

  // Clear the screen
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setCursor(10, 60);
  M5.Lcd.print("Booting...");

  delay(500);
  M5.Lcd.clear();

  // WiFi + Time setup (graceful)
  setupWiFiIfPossible();
  initTime();

  // First screen hint
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setCursor(4, 8);
  M5.Lcd.print("M5 Stick S3 Controller");
  M5.Lcd.setCursor(4, 28);
  M5.Lcd.print("A/B to switch screens");
  M5.Lcd.setCursor(4, 46);
  M5.Lcd.print("Hold both to select");

  // Initialize joke index
  jokeIndex = 0;

  // Initialize game states
  reactionActive = false;
  reactionScore = 0;
  kidsScore = 0;
  targetNumber = 5;
}

void loop() {
  // Basic UI navigation
  // Note: Depending on your exact M5Stick S3 library version, the button API names may differ slightly (BtnA/BtnB/wasPressed/isPressed)
  // Here we rely on common syntax: BtnA.wasPressed(), BtnB.wasPressed()
  if (M5.BtnA.wasPressed()) {
    currentScreen = static_cast<Screen>((currentScreen - 1 + NUM_SCREENS) % NUM_SCREENS);
  }
  if (M5.BtnB.wasPressed()) {
    currentScreen = static_cast<Screen>((currentScreen + 1) % NUM_SCREENS);
  }

  // Enter/Select: we approximate by both buttons pressed
  bool bothPressed = M5.BtnA.isPressed() && M5.BtnB.isPressed();
  if (bothPressed) {
    // Simple: advance to next screen when both pressed
    currentScreen = static_cast<Screen>((currentScreen + 1) % NUM_SCREENS);
    // Debounce a bit
    delay(400);
  }

  // Screen rendering loop
  switch (currentScreen) {
    case SCREEN_CLOCK:
      // Update once per second
      if (millis() - lastScreenUpdate > clockUpdateMs) {
        lastScreenUpdate = millis();
        drawClockScreen();
      }
      break;

    case SCREEN_JOKES:
      drawJokesScreen();
      break;

    case SCREEN_REACTION:
      drawReactionScreen();
      // Minimal interactive logic for reaction game
      if (!reactionActive) {
        // Start a new round when user presses A or B
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
          reactionActive = true;
          reactionReadyTime = millis() + random(reactionDelayMin, reactionDelayMax);
          M5.Lcd.setTextSize(2);
          M5.Lcd.setCursor(4, 34);
          M5.Lcd.print("Get Ready...");
        }
      } else {
        // Wait until GO
        if (millis() >= reactionReadyTime) {
          // GO event
          reactionStartTime = millis();
          M5.Lcd.setTextColor(WHITE, BLACK);
          M5.Lcd.setCursor(4, 34);
          M5.Lcd.print("GO!      ");
          beeper(660, 100);
          // Now wait for a button press
          if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
            unsigned long reactionMs = millis() - reactionStartTime;
            reactionScore += max(0, (1000 - (int)reactionMs)) / 10;
            M5.Lcd.setCursor(4, 60);
            M5.Lcd.print("Nice! Score +");
            M5.Lcd.print(reactionScore);
            // Reset
            reactionActive = false;
            delay(800);
            M5.Lcd.clear();
          }
        }
      }
      break;

    case SCREEN_KIDS:
      drawKidsScreen();
      // Simple kids logic: A decreases number, B increases number
      if (M5.BtnA.wasPressed()) {
        targetNumber--;
        if (targetNumber < 1) targetNumber = 9;
      }
      if (M5.BtnB.wasPressed()) {
        targetNumber++;
        if (targetNumber > 9) targetNumber = 1;
      }
      // Show new target by refreshing screen
      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(4, 60);
      M5.Lcd.print(targetNumber);
      // crude scoring when user guesses correctly with a quick press sequence
      if (targetNumber == 5 && kidsPhase == 0) {
        kidsScore += 1;
        kidsPhase = 1;
      }
      break;

    case SCREEN_SETTINGS:
      drawSettingsScreen();
      // Toggle night mode with A
      if (M5.BtnA.wasPressed()) {
        nightMode = !nightMode;
        // update UI
        M5.Lcd.clear();
        // Redraw the current settings screen with new mode
        drawSettingsScreen();
      }
      // Back to Clock with B
      if (M5.BtnB.wasPressed()) {
        currentScreen = SCREEN_CLOCK;
      }
      break;
  }

  delay(20); // small delay to avoid too fast loop
}
