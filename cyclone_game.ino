//Cyclone Game - ESP32 Version (Modified to match Adafruit_NeoPixel behavior)
#include <FastLED.h>

// Pin Definitions for ESP32
#define BUTTON_PIN    19  // GPIO15 - Pushbutton input
#define PIXEL_PIN     5  //  - WS2812B data pin
#define BUZZER_PIN    18  // GPIO17 - Buzzer output

// LED Configuration
#define NUM_LEDS      28  // 25 LEDs
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

// Brightness (0-255)
#define BRIGHTNESS    100

// Game speed constants
const float LEVEL       = 0.008;
const float FINAL_LEVEL = 0.02;
const float START_SPEED = 0.12;

// ── Buzzer Struct Definition ─────────────────────────────────
struct BuzzerNote { 
  int freq; 
  int dur; 
};

// ── State Machine ────────────────────────────────────────────
enum GameState {
  STATE_RUNNING,
  STATE_HIT_FLASH,
  STATE_GAME_OVER_CHASE,
  STATE_GAME_OVER_FLASH_ON,
  STATE_GAME_OVER_FLASH_OFF,
  STATE_GAME_OVER_HOLD,
  STATE_WIN_RAINBOW,
  STATE_WIN_CLEAR
};

GameState gameState = STATE_RUNNING;

// ── Colors ───────────────────────────────────────────────────
CRGB colors[12];

void initColors() {
  colors[0]  = CRGB::Red;
  colors[1]  = CRGB::Orange;
  colors[2]  = CRGB::Yellow;
  colors[3]  = CRGB::Green;
  colors[4]  = CRGB::Teal;
  colors[5]  = CRGB::Cyan;
  colors[6]  = CRGB::Blue;
  colors[7]  = CRGB::Purple;
  colors[8]  = CRGB::Magenta;
  colors[9]  = CRGB::Gold;
  colors[10] = CRGB::Aqua;
  colors[11] = CRGB::HotPink;
}

// ── Game Variables ───────────────────────────────────────────
int   num        = 0;
int   last_num   = 0;
int   now_color  = 0;
int   next_color = 1;
float speed      = START_SPEED;
bool  new_target   = true;
bool  button_state = false;
int   x, y, z;  // Target zone (3 LEDs)

// ── Timing ───────────────────────────────────────────────────
unsigned long lastPixelTime = 0;
unsigned long stateTimer    = 0;
int           flashCount    = 0;
int           chaseIndex    = 0;
int           rainbowJ      = 0;

// ── Non-blocking Buzzer Variables ────────────────────────────
BuzzerNote buzzerQueue[8];
int        buzzerQueueLen = 0;
int        buzzerQueueIdx = 0;
bool       buzzerActive   = false;
unsigned long buzzerEndTime = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];

// ── Function Prototypes ──────────────────────────────────────
void buzzerUpdate();
void buzzerPlay(BuzzerNote* notes, int len);
void buzzerHit();
void buzzerWin();
void buzzerGameOver();
void buzzerTick();
CRGB colorWheel(byte pos);  // Changed to return CRGB instead of uint32_t
void drawTarget();
bool onTarget(int p);
void resetGame();

// ── Buzzer Functions (Non-blocking) ──────────────────────────
void buzzerUpdate() {
  unsigned long now = millis();
  if (buzzerActive && now >= buzzerEndTime) {
    noTone(BUZZER_PIN);
    buzzerActive = false;
    buzzerQueueIdx++;
  }
  if (!buzzerActive && buzzerQueueIdx < buzzerQueueLen) {
    BuzzerNote n = buzzerQueue[buzzerQueueIdx];
    tone(BUZZER_PIN, n.freq, n.dur);
    buzzerEndTime = now + n.dur;
    buzzerActive  = true;
  }
}

void buzzerPlay(BuzzerNote* notes, int len) {
  for (int i = 0; i < len; i++) buzzerQueue[i] = notes[i];
  buzzerQueueLen = len;
  buzzerQueueIdx = 0;
  buzzerActive   = false;
  noTone(BUZZER_PIN);
}

void buzzerHit() {
  BuzzerNote n[] = {{600,60},{900,60},{1200,100}};
  buzzerPlay(n, 3);
}

void buzzerWin() {
  BuzzerNote n[] = {{523,100},{659,100},{784,100},{1047,250}};
  buzzerPlay(n, 4);
}

void buzzerGameOver() {
  BuzzerNote n[] = {{400,150},{300,150},{200,300}};
  buzzerPlay(n, 3);
}

void buzzerTick() {
  if (!buzzerActive && buzzerQueueIdx >= buzzerQueueLen) {
    tone(BUZZER_PIN, 1200, 5);
  }
}

// ── Rainbow helper (FIXED - returns CRGB instead of uint32_t) ──
CRGB colorWheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return CRGB(255 - pos * 3, 0, pos * 3);
  }
  if (pos < 170) {
    pos -= 85;
    return CRGB(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return CRGB(pos * 3, 255 - pos * 3, 0);
}

// ── Draw target zone (3 LEDs) ────────────────────────────────
void drawTarget() {
  leds[x] = CRGB::White;
  leds[y] = colors[next_color];
  leds[z] = CRGB::White;
}

bool onTarget(int p) {
  return (p == x || p == y || p == z);
}

// ── Reset Game ───────────────────────────────────────────────
void resetGame() {
  num        = 0;
  last_num   = 0;
  speed      = START_SPEED;
  next_color = 1;
  now_color  = 0;
  new_target = true;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  gameState  = STATE_RUNNING;
  Serial.println("Game reset.");
}

// ── Setup ────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Cyclone Game Starting on ESP32...");
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize colors
  initColors();
  
  // Initialize LEDs
  FastLED.addLeds<LED_TYPE, PIXEL_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  
  randomSeed(analogRead(0));  // ESP32 doesn't have A0 like Arduino, use GPIO pin
  
  Serial.println("WS2812B 25-LED game ready!");
  Serial.print("Using GPIO pins - Button: ");
  Serial.print(BUTTON_PIN);
  Serial.print(", LED: ");
  Serial.print(PIXEL_PIN);
  Serial.print(", Buzzer: ");
  Serial.println(BUZZER_PIN);
}

// ── Main Loop ────────────────────────────────────────────────
void loop() {
  buzzerUpdate();  // Handle non-blocking buzzer

  bool btnPressed = (digitalRead(BUTTON_PIN) == LOW);
  if (btnPressed && !button_state) button_state = true;

  unsigned long now = millis();

  if (gameState == STATE_RUNNING) {
    // Generate new target zone if needed
    if (new_target) {
      y = random(1, NUM_LEDS - 1);
      x = y - 1;
      z = y + 1;
      new_target = false;
      Serial.printf("New target → x:%d  y:%d  z:%d\n", x, y, z);
    }

    drawTarget();

    // Move the chaser LED
    if ((now - lastPixelTime) >= (unsigned long)(speed * 1000)) {
      lastPixelTime = now;

      // Clear previous LED
      if (num > 0) {
        last_num = num - 1;
        leds[last_num] = CRGB::Black;
      }

      // Redraw target if it was cleared
      if (onTarget(last_num)) drawTarget();

      // Draw current chaser LED
      if (num < NUM_LEDS) {
        leds[num] = colors[now_color];
        buzzerTick();  // Quick beep on LED change
        num++;
      }

      // Reset to beginning if reached end
      if (num == NUM_LEDS) {
        leds[num - 1] = CRGB::Black;
        num = 0;
      }

      FastLED.show();

      // Check for button press on target
      if (onTarget(last_num) && btnPressed) {
        button_state = false;
        Serial.printf("HIT!  chaser:%d  target:%d-%d-%d\n", last_num, x, y, z);
        buzzerHit();
        
        // Flash with next color
        fill_solid(leds, NUM_LEDS, colors[next_color]);
        FastLED.show();
        
        num        = 0;
        stateTimer = now;
        gameState  = STATE_HIT_FLASH;
      }
      else if (!onTarget(last_num) && btnPressed) {
        button_state = false;
        Serial.printf("MISS! chaser:%d  target:%d-%d-%d\n", last_num, x, y, z);
        buzzerGameOver();
        
        // Flash with current color
        fill_solid(leds, NUM_LEDS, colors[now_color]);
        FastLED.show();
        
        num        = 0;
        chaseIndex = 0;
        stateTimer = now;
        gameState  = STATE_GAME_OVER_CHASE;
      }
    }
  }
  else if (gameState == STATE_HIT_FLASH) {
    if (now - stateTimer >= 500) {
      // Clear and increase difficulty
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();

      speed -= LEVEL;
      next_color = (next_color + 1) % 12;
      now_color  = (now_color + 1) % 12;
      new_target = true;
      lastPixelTime = now;
      Serial.printf("Speed: %.4f seconds between steps\n", speed);

      if (speed <= FINAL_LEVEL) {
        buzzerWin();
        rainbowJ   = 0;
        stateTimer = now;
        gameState  = STATE_WIN_RAINBOW;
      } else {
        gameState = STATE_RUNNING;
      }
    }
  }
  else if (gameState == STATE_GAME_OVER_CHASE) {
    if (now - stateTimer >= 25) {
      stateTimer = now;
      if (chaseIndex < NUM_LEDS) {
        leds[chaseIndex] = CRGB::Black;
        FastLED.show();
        chaseIndex++;
      } else {
        flashCount = 0;
        fill_solid(leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        stateTimer = now;
        gameState  = STATE_GAME_OVER_FLASH_ON;
      }
    }
  }
  else if (gameState == STATE_GAME_OVER_FLASH_ON) {
    if (now - stateTimer >= 400) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      stateTimer = now;
      gameState  = STATE_GAME_OVER_FLASH_OFF;
    }
  }
  else if (gameState == STATE_GAME_OVER_FLASH_OFF) {
    if (now - stateTimer >= 400) {
      flashCount++;
      if (flashCount < 3) {
        fill_solid(leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        stateTimer = now;
        gameState  = STATE_GAME_OVER_FLASH_ON;
      } else {
        fill_solid(leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        stateTimer = now;
        gameState  = STATE_GAME_OVER_HOLD;
      }
    }
  }
  else if (gameState == STATE_GAME_OVER_HOLD) {
    if (now - stateTimer >= 1000) {
      resetGame();
    }
  }
  else if (gameState == STATE_WIN_RAINBOW) {
    if (now - stateTimer >= 12) {
      stateTimer = now;
      for (int i = 0; i < NUM_LEDS; i++) {
        int idx = (i * 256 / NUM_LEDS) + rainbowJ;
        leds[i] = colorWheel(idx & 255);  // FIXED: Direct assignment now works
      }
      FastLED.show();
      rainbowJ++;
      if (rainbowJ >= 512) {
        stateTimer = now;
        gameState  = STATE_WIN_CLEAR;
      }
    }
  }
  else if (gameState == STATE_WIN_CLEAR) {
    if (now - stateTimer >= 800) {
      resetGame();
    }
  }

  yield();  // Required for ESP32
}