#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int buttonPin = 12;  // Button connected to pin 12 for input
const int buzzerPin = 27;  // Buzzer connected to pin 27
const int eraseButtonPin = 13;  // Button connected to pin 13 for erasing characters
unsigned long pressStartTime;
unsigned long lastPressTime = 0;  // To track time since last press
bool isPressed = false;
bool longPressDetected = false;
String morseCode = "";  // Stores the sequence of dots and dashes
String sentence = "";   // Stores the full sentence

const int debounceDelay = 50;  // Debounce delay (50 ms)
const int minPressDuration = 100;  // Minimum press duration to register (100 ms)
const int longPressDuration = 500;  // Threshold for a long press (1000 ms)
const int decodeDelay = 1500;  // 2 seconds timeout to decode Morse code

// Morse code dictionary
const String morseDictionary[27] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", 
  "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "-...."
};
const char letters[27] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '  // Last one is space
};

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Button with pull-up
  pinMode(buzzerPin, OUTPUT);        // Buzzer as output
  pinMode(eraseButtonPin, INPUT_PULLUP); // Button for erasing characters
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display();
}

void loop() {
  int buttonState = digitalRead(buttonPin);
  int eraseButtonState = digitalRead(eraseButtonPin);  // Read erase button state
  
  // Detect button press and start timing
  if (buttonState == LOW && !isPressed && (millis() - lastPressTime > debounceDelay)) {
    pressStartTime = millis();  // Start timing the press
    isPressed = true;
    longPressDetected = false;  // Reset long press detection
  }

  // Handle long press detection while the button is still pressed
  if (isPressed && !longPressDetected && (millis() - pressStartTime >= longPressDuration)) {
    morseCode += "-";  // Register dash for long press
    displayMorseCode(morseCode);
    longPressDetected = true;  // Mark long press as handled
    lastPressTime = millis();  // Reset the last press time
    
    // Buzzer for dash (long beep)
    tone(buzzerPin, 1000);  // Start tone at 1kHz
    delay(600);             // Hold the tone for 600 ms
    noTone(buzzerPin);      // Stop the tone
  }

  // Handle button release
  if (buttonState == HIGH && isPressed) {
    unsigned long pressDuration = millis() - pressStartTime;

    // Register short press for dot
    if (!longPressDetected && pressDuration >= minPressDuration) {
      morseCode += ".";  // Short press for dot
      displayMorseCode(morseCode);
      lastPressTime = millis();  // Reset the last press time
      
      // Buzzer for dot (short beep)
      tone(buzzerPin, 1000);  // Start tone at 1kHz
      delay(200);             // Hold the tone for 200 ms
      noTone(buzzerPin);      // Stop the tone
    }

    isPressed = false;  // Reset press state
  }

  // Check if 2 seconds have passed since the last press for decoding
  if (millis() - lastPressTime > decodeDelay && morseCode.length() > 0) {
    char letter = decodeMorse(morseCode);  // Decode Morse code after 2 seconds
    if (letter != '?') {  // If valid Morse code, append the decoded letter to the sentence
      sentence += letter;
    }
    displaySentence(sentence);  // Update the OLED with the full sentence
    morseCode = "";  // Reset Morse code for the next letter
  }

  // Erase character when the erase button is pressed
  if (eraseButtonState == LOW) {
    if (sentence.length() > 0) {
      sentence.remove(sentence.length() - 1);  // Remove the last character
      displaySentence(sentence);  // Update the OLED with the new sentence
    }
    delay(300);  // Simple delay to prevent multiple erases from one press
  }
}

void displayMorseCode(String code) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(code);  // Display current Morse code input
  display.display();
}

char decodeMorse(String morse) {
  for (int i = 0; i < 27; i++) {
    if (morse == morseDictionary[i]) {
      return letters[i];
    }
  }
  return '?';  // Return '?' if Morse code is invalid
}

void displaySentence(String sentence) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(sentence);  // Show the full sentence being typed
  display.display();
}
