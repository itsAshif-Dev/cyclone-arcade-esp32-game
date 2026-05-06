# 🌀 Cyclone LED Game — ESP32

A fun and addictive **Cyclone arcade game** built with ESP32 and 28 WS2812B LEDs.  
A light chases around the LED strip — press the button at the right moment to hit the target zone and win!

---

## 🎮 How to Play

1. Power on the ESP32
2. A **colored LED** starts moving around the LED ring
3. A **white target zone** (3 LEDs) is placed at a random position
4. **Press the button** when the moving LED reaches the target zone
5. ✅ **Hit** → Speed increases, new target appears — can you keep up?
6. ❌ **Miss** → Game over! Red flash animation plays, then restarts

> The game gets **faster and harder** with every successful hit!  
> Reach the final speed and you **WIN** — rainbow animation plays! 🌈

---

## 🔧 Components Used

| Component         | Quantity | Details                     |
|-------------------|----------|-----------------------------|
| ESP32             | 1        | Main microcontroller        |
| WS2812B LED Strip | 1        | 28 LEDs (NeoPixel type)     |
| Push Button       | 1        | Player input                |
| Buzzer            | 1        | Sound feedback              |
| Jumper Wires      | Few      | For connections             |
| Breadboard        | 1        | For prototyping             |
| USB Cable         | 1        | For programming ESP32       |

---

## 📌 Pin Connections

| Component       | ESP32 GPIO Pin |
|-----------------|----------------|
| Push Button     | GPIO 19        |
| WS2812B Data    | GPIO 5         |
| Buzzer          | GPIO 18        |

---

## 📚 Libraries Required

Install these libraries in Arduino IDE before uploading:

- **FastLED** — `Sketch → Include Library → Manage Libraries → Search "FastLED"`

---

## 💻 How to Upload the Code

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Go to `File → Preferences`
   - Add this URL in "Additional Board Manager URLs":  
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Go to `Tools → Board → Board Manager` → Search **ESP32** → Install
3. Install **FastLED** library
4. Open `cyclone_game.ino`
5. Select board: `Tools → Board → ESP32 Dev Module`
6. Select the correct COM port
7. Click **Upload** ✅

---

## 🎵 Sound Effects

| Event       | Sound                        |
|-------------|------------------------------|
| LED tick    | Quick beep on each LED step  |
| Hit target  | 3-tone rising beep           |
| Game over   | 3-tone falling beep          |
| Win         | 4-note victory melody 🎶     |

---

## 📸 Project Photos

<!-- Add your photos here after uploading -->
> *Circuit photo and project image coming soon!*

---

## 👤 Made By

**Ashif**  
Technical Intern @ OPSTA World Skills Pvt. Limited  
📧 Ashif.dev0@gmail.com  
🔗 [GitHub Profile](https://github.com/itsAshif-Dev)
