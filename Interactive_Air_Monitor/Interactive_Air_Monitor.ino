/*
 Project: Interactive Ambient Air Quality Monitor
 Description: An ESP32 environmental monitor using a DHT11 sensor.
 Features a dynamic buzzer alarm for temperature shifts and an interactive WS2812B NeoPixel matrix.
 Includes a physical button to toggle matrix states:
 State 0: Temp (Temperature-reactive colors: Blue/Green/Red)
 State 1: Daylight (Night Light Mode)
 State 2: Off (Matrix disabled)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

// PIN CONFIGURATION 
#define OLED_SDA 21 
#define OLED_SCL 22 
#define DHTPIN 13       
#define DHTTYPE DHT11   
#define BUZZER_PIN 12   
#define LED_PIN 14     
#define NUM_LEDS 16    
#define BUTTON_PIN 27   
#define BUTTON_TEMP_PIN 26

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// OBJECT INSTANTIATION
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel matrix(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// GLOBAL VARIABLES
float temperaturaReferinta = 0.0;
float tempCurenta = 0.0;
float umiditateCurenta = 0.0;
bool primaCitire = true;
bool afisareFahrenheit = false;

// Timing variables
unsigned long timpPrecedent = 0;
const long intervalSenzori = 2000; 

// Button and Matrix State logic
int modLumina = 0; // 0 = Temp, 1 = Daylight, 2 = OFF
int stareButon = HIGH;
int ultimaStareButon = HIGH;
unsigned long ultimulTimpDebounce = 0; 

// Button Temp logic
int stareButonTemp = HIGH;
int ultimaStareButonTemp = HIGH;
unsigned long ultimulTimpDebounceTemp = 0; 

const unsigned long timpDebounce = 50; 

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_TEMP_PIN, INPUT_PULLUP);

  // Initialize I2C for the onboard OLED Display
  Wire.begin(OLED_SDA, OLED_SCL);
  dht.begin();
  
  matrix.begin();
  matrix.setBrightness(50);
  matrix.show(); 

  // Start OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED initialization failed!"));
  }

  // Boot Screen
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("System Active");
  display.display();
  delay(2000); 
}

// Helper function to update NeoPixel colors based on the current mode
void updateLEDs() {
  uint32_t culoare;
  
  if (modLumina == 0) {
    // Mode 0: Temperature reactive
    if (tempCurenta < 22.0) {
      culoare = matrix.Color(0, 0, 50);    // Blue (Cold)
    } else if (tempCurenta >= 22.0 && tempCurenta <= 25.0) {
      culoare = matrix.Color(0, 50, 0);    // Green (Optimal)
    } else {
      culoare = matrix.Color(50, 0, 0);    // Red (Warm)
    }
  } 
  else if (modLumina == 1) {
    // Mode 1: Night Light
    culoare = matrix.Color(150, 70, 10);    // Warm White
  } 
  else {
    // Mode 2: Off
    culoare = matrix.Color(0, 0, 0);       // Off
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, culoare);
  }
  matrix.show();
}

void loop() {
  unsigned long timpCurent = millis();

  // BUTTON LOGIC (Debounce)
  int citireButon = digitalRead(BUTTON_PIN);

  if (citireButon != ultimaStareButon) {
    ultimulTimpDebounce = timpCurent; 
  }

  if ((timpCurent - ultimulTimpDebounce) > timpDebounce) {
    if (citireButon != stareButon) {
      stareButon = citireButon;
      
      // Trigger mode change only on button press (LOW)
      if (stareButon == LOW) {
        modLumina++;
        if (modLumina > 2) modLumina = 0; 
        updateLEDs(); // Update light mode
      }
    }
  }
  ultimaStareButon = citireButon;

  // BUTTON TEMP LOGIC (Debounce)
  int citireButonTemp = digitalRead(BUTTON_TEMP_PIN);

  if (citireButonTemp != ultimaStareButonTemp) {
    ultimulTimpDebounceTemp = timpCurent; 
  }

  if ((timpCurent - ultimulTimpDebounceTemp) > timpDebounce) {
    if (citireButonTemp != stareButonTemp) {
      stareButonTemp = citireButonTemp;
      
      // Toggle C/F on button press (LOW)
      if (stareButonTemp == LOW) {
        afisareFahrenheit = !afisareFahrenheit;
      }
    }
  }
  ultimaStareButonTemp = citireButonTemp;

  // SENSOR & ALARM LOGIC
  if (timpCurent - timpPrecedent >= intervalSenzori) {
    timpPrecedent = timpCurent;

    // Read Humidity and Temperature from DHT11
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Validation check
    if (isnan(h) || isnan(t)) {
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Sensor ERROR");
      display.display();
      return; 
    }

    umiditateCurenta = h;
    tempCurenta = t;

    // Set initial reference temperature
    if (primaCitire) {
      temperaturaReferinta = tempCurenta;
      primaCitire = false;
      updateLEDs(); 
    }

    // Dynamic Alarm Trigger (for 1.0 degree difference)
    if (abs(tempCurenta - temperaturaReferinta) >= 1.0) {
      for(int i = 0; i < 2; i++) {
        tone(BUZZER_PIN, 2500); 
        delay(100); 
        noTone(BUZZER_PIN);
        delay(50);
      }
      temperaturaReferinta = tempCurenta; // Update reference after alarm
    }

    // Auto-update colors if in Temp mode
    if (modLumina == 0) {
      updateLEDs(); 
    }

    float afisajTemp = afisareFahrenheit ? (tempCurenta * 1.8 + 32.0) : tempCurenta;
    float afisajRef = afisareFahrenheit ? (temperaturaReferinta * 1.8 + 32.0) : temperaturaReferinta;
    char unitate = afisareFahrenheit ? 'F' : 'C';

    // --- OLED DISPLAY UPDATES ---
    display.clearDisplay();
    display.setTextSize(1);
    
    // Line 1: Mode Indicator
    display.setCursor(0, 0);
    display.print("Mode: ");
    if(modLumina == 0) display.print("Temp");
    if(modLumina == 1) display.print("NightLight");
    if(modLumina == 2) display.print("Off");

    // Line 2: Temperature
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.print("T: "); display.print(afisajTemp, 1); display.print(" "); display.print(unitate);
    
    // Line 3: Humidity
    display.setCursor(0, 38);
    display.print("H: "); display.print(umiditateCurenta, 0); display.print(" %");

    // Line 4: Reference Temperature for Alarm
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print("Ref. TEMP: "); display.print(afisajRef, 1); display.print(" "); display.print(unitate);
    
    display.display();
  }
}