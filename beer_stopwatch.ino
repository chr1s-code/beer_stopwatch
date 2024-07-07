#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define TRIGGER 14        //Trigger pin
#define START_BUTTON 12   //Reset pin
#define COUNTDOWN 2999

// Instantiate a Bounce object
Bounce2::Button startButton = Bounce2::Button();

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long startTime = 0;
unsigned long startTimeDrinking = 0;
unsigned long reactionTime = 0;
unsigned long drinkingTime = 0;
bool gameStarted = false;
bool startupComplete = false;
bool glassLifted = false;

void writeToDisplay(String str) {
  writeToDisplay(str, 0, 0, 3);
}

void writeToDisplay(String str, unsigned short textSize = 3) {
  writeToDisplay(str, 0, 0, textSize);
}

void writeToDisplay(String str, unsigned short cursorX, unsigned short cursorY, unsigned short textSize) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setCursor(cursorX, cursorY);
  display.print(str);
  display.display();
}

void writeNumberToDisplay(int num, unsigned short textSize = 3) {
  char str[8];
  itoa(num, str, 10);

  writeToDisplay(str, 64, 32, textSize);
}

String prettifyDuration(unsigned long duration) {
  unsigned long durMS = (duration % 1000);       //Milliseconds
  unsigned long durSS = (duration / 1000) % 60;  //Seconds

  String dataTemp = "";


  if (durSS == 0) {
    return String(durMS) + "ms";
  }

  if (durSS < 10) {
    dataTemp = dataTemp + "0" + String(durSS) + ":";
  } else {
    dataTemp = dataTemp + String(durSS) + ":";
  }

  dataTemp = dataTemp + String(durMS) + "s";

  return dataTemp;
}

void printEndScreen(unsigned long rt, unsigned long dt) {
  Serial.println("RESULTS:");
  Serial.print("reactionTime: ");
  Serial.print(rt);
  Serial.println("ms");
  
  Serial.print("drinkingTime: ");
  Serial.print(dt);
  Serial.println("ms");

  Serial.print("total: ");
  Serial.print(rt + dt);
  Serial.println("ms");
  Serial.println("-------------------------------------------------------------------");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Gesamtzeit");
  display.println(prettifyDuration(rt + dt));
  display.setCursor(0, 48);
  display.setTextSize(1);
  display.println("Reaktion: " + prettifyDuration(rt));
  display.display();
}

void startSequence() {
  unsigned short delayTime = 10;
  unsigned long timeSinceButtonPressed = millis();
  unsigned long now = millis();
  unsigned long remainingCountdownTime = 0;
  Serial.println("Countdown started...");

  for (int i = COUNTDOWN; i > 0; i - delayTime) {
    if (digitalRead(TRIGGER) == HIGH) {
      gameStarted = false;
      writeToDisplay("FRUEHSTART", 2);
      Serial.println("###### FALSE START ######");

      return;
    }

    now = millis();
    remainingCountdownTime = (COUNTDOWN - (now - timeSinceButtonPressed));

    if (remainingCountdownTime <= 50) {
      startupComplete = true;

      writeToDisplay("GO", 4);
      Serial.println("###### GO ######");

      return;
    }

    //Serial.println(remainingCountdownTime);

    writeNumberToDisplay(((int)remainingCountdownTime / 1000) + 1, 4);

    delay(delayTime);
  }
}

void displayTestMessage() {
  display.clearDisplay();
  //display.setRotation(2);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("A");
  display.setCursor(0, 16);
  display.println("B");
  display.setCursor(0, 32);
  display.println("C");
  display.setCursor(0, 48);
  display.println("D");
  display.display();

  delay(2000);
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIGGER, INPUT_PULLUP);

  startButton.attach(START_BUTTON, INPUT_PULLUP);
  startButton.interval(5);
  startButton.setPressedState(LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  //display.setRotation(2);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 32);
  display.println("Bereit");
  Serial.println("Ready for startup...");
  display.display();

  //displayTestMessage();
}

void loop() {
  startButton.update();

  if (!gameStarted && startButton.pressed() && digitalRead(TRIGGER) == LOW) {
    gameStarted = true;

    Serial.println("Game started...");

    startSequence();
    startTime = millis();
  }

  if (!gameStarted && startButton.pressed() && digitalRead(TRIGGER) == HIGH) {
    Serial.println("ERROR: GLASS NOT IN POSITION");

    writeToDisplay("Glas fehlt", 0, 32, 2);
    delay(1000);
    writeToDisplay("Bereit", 0, 32, 2);
  }

  if (gameStarted && startupComplete) {
    //Serial.println(millis() - startTime);
    writeToDisplay(prettifyDuration(millis() - startTime), 3);

    if (!glassLifted && digitalRead(TRIGGER) == HIGH) {  //Stop reaction timer and start drinking timer
      reactionTime = millis() - startTime;
      glassLifted = true;
      startTimeDrinking = millis();

      Serial.println("Beer lifted");
    }

    if (glassLifted && digitalRead(TRIGGER) == LOW) {  //Stop drinking timer
      drinkingTime = millis() - startTimeDrinking;
      gameStarted = false;
      startupComplete = false;
      glassLifted = false;

      printEndScreen(reactionTime, drinkingTime);
    }
  }
}
