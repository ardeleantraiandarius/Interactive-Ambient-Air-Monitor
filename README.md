# Interactive Ambient Air Quality Monitor

An ESP32-based environmental monitor featuring real-time temperature and humidity tracking, a dynamic buzzer alarm, and an interactive WS2812B NeoPixel matrix for ambient visual feedback.

-Features
*Real-Time Monitoring: Reads temperature and humidity using a DHT11 sensor and displays data on a 0.96" OLED screen.
*Interactive RGB Matrix: 
 A 16-LED WS2812B NeoPixel matrix reflecting the room's temperature. A dedicated push button cycles through 3 modes:
  * `Temp Mode`: Colors react to temperature (Blue for cold, Green for optimal, Red for warm).
  * `Night Light Mode`: Emits a warm white glow, acting as a standard night light.
  * `Off`: Disables the LED matrix.
*Unit Toggle: A secondary push button allows the user to instantly switch the display units between Celsius (°C) and Fahrenheit (°F).
*Dynamic Alarm: A passive buzzer automatically triggers a subtle alarm if the temperature shifts by 1°C.

-Hardware Requirements
*ESP32 Development Board
*DHT11 Temperature & Humidity Sensor
*(minimum) 0.96" I2C OLED Display (SSD1306)
*(number of LEDs is not relevant) 16-LED WS2812B NeoPixel Matrix (5V)
*Passive Buzzer
*2x Tactile Push Buttons
*Breadboard & Jumper Wires

-Software & Libraries
This project is built using the Arduino IDE. Make sure to install the following dependencies via the Library Manager:
* `Adafruit GFX Library`
* `Adafruit SSD1306`
* `Adafruit NeoPixel`
* `DHT sensor library` (by Adafruit)

-How It Works
1.Boot: Upon startup, the ESP32 initializes the I2C bus, sensors, and display. It takes an initial temperature reading to set the "Alarm Reference" baseline.
2.Non-Blocking Logic: The system uses `millis()` for debouncing buttons and scheduling sensor reads (every 2 seconds), ensuring the UI and button presses remain instantly responsive.
3.Internal Logic: All mathematical comparisons for the alarm and color thresholds are kept strictly in Celsius to maintain system stability. Fahrenheit conversion happens instantly only at the display rendering level.
