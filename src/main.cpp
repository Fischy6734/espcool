#include <M5Unified.h>

// Family-friendly rainbow colors
const uint16_t rainbowColors[7] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_BLUE, TFT_PURPLE
};

// Function to play a simple melody
void playMelody() {
  // Simple family-friendly startup melody (C major scale)
  const int notes[] = {262, 294, 330, 349, 392, 440, 494};
  for (int i = 0; i < 7; i++) {
    M5.Speaker.tone(notes[i], 200);
    delay(250);
  }
  M5.Speaker.tone(523, 500);  // High C ending
  delay(600);
  M5.Speaker.stop();  // Turn off speaker when done
}

void setup() {
  M5.begin();  // Initialize all M5Stick S3 hardware
  
  // Set display orientation (portrait mode for M5Stick S3)
  M5.Lcd.setRotation(0);  // 0=portrait, 1=landscape
  
  // Show test message
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("Flasher Test\nSuccess!");

  // Draw rainbow bar at top
  drawRainbow();
  
  // Play melody to confirm speaker works
  playMelody();
}

void loop() {
  M5.update();  // Required to read button states with M5Unified

  // Toggle inverted display when BtnA is pressed
  if (M5.BtnA.wasPressed()) {
    static bool inverted = false;
    inverted = !inverted;
    M5.Lcd.invertDisplay(inverted);
  }

  // Animate bouncing ball (fits small screen)
  static int x = 10, y = 40, dx = 1, dy = 1;
  static unsigned long lastBallUpdate = 0;
  if (millis() - lastBallUpdate > 30) {
    M5.Lcd.fillCircle(x, y, 5, TFT_BLACK);  // Clear old ball position
    x += dx; y += dy;
    // Bounce off screen edges
    if (x <= 5 || x >= 130) dx *= -1;
    if (y <= 30 || y >= 235) dy *= -1;
    M5.Lcd.fillCircle(x, y, 5, TFT_WHITE);  // Draw new ball position
    lastBallUpdate = millis();
  }

  // Cycle rainbow top bar
  static int colorIdx = 0;
  M5.Lcd.fillRect(0, 25, 135, 5, rainbowColors[colorIdx++ % 7]);
  delay(150);
}

// Function to draw rainbow at top of screen
void drawRainbow() {
  // Draw small rainbow bar at top (fits 135px width)
  for (int i = 0; i < 7; i++) {
    M5.Lcd.fillRect(i * 19, 20, 19, 5, rainbowColors[i]);  // 7*19=133px total
  }
}
