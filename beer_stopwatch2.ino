#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define RESET 26
#define FAKE_GLASS 25
#define REFRESH_INTERVAL 100

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const short w = 4;    // optimum = 4
const short h = 16;   // optimum = 16
const short r = 2;
const short space = 4;

const short marginX = 4;
const short marginY = 8;
const short digitHeight = 3*w+2*h;
const short digitWidth = 2*w+h;

const short colonRadius = 3;

unsigned int timeOfLastRefresh = 0;


// Segment definitions (A to G segments)
const uint8_t segment[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

void drawSegment(int x, int y, int seg, bool on) {
  switch (seg) {
    case 0: // A
      display.fillRoundRect(x+w, y, h, w, r, on ? SSD1306_WHITE : SSD1306_BLACK);
      break;
    case 1: // B
      display.fillRoundRect(x+h+w, y+w, w, h, r, on ? SSD1306_WHITE : SSD1306_BLACK);
      break;
    case 2: // C
      display.fillRoundRect(x+h+w, y+h+2*w, w, h, r, on ? SSD1306_WHITE : SSD1306_BLACK);
      break;
    case 3: // D
      display.fillRoundRect(x+w, y+2*h+2*w, h, w, r, on ? SSD1306_WHITE : SSD1306_BLACK);
      break;
    case 4: // E
      display.fillRoundRect(x, y+h+2*w, w, h, r, on ? SSD1306_WHITE : SSD1306_BLACK);
      break;
    case 5: // F
      display.fillRoundRect(x, y+w, w, h, r, on ? SSD1306_WHITE : SSD1306_BLACK);
      break;
    case 6: // G
      display.fillRoundRect(x+w, y+h+w, h, w, r, on ? SSD1306_WHITE : SSD1306_BLACK);
      break;
  }
}

void drawDigit(int digit, int x, int y) {
  for (int i = 0; i < 7; i++) {
    drawSegment(x, y, i, segment[digit][i]);
  }
}

void drawColon(int x, int y, int r) {
  int y1 = y-13;
  int y2 = y1+12;

  display.fillRoundRect(x-3, y1, 2*r, 2*r, r, SSD1306_WHITE);
  display.fillRoundRect(x-3, y2, 2*r, 2*r, r, SSD1306_WHITE);
}

void printTime(unsigned long ms, bool withoutColon = false){

  if((millis()-timeOfLastRefresh) < REFRESH_INTERVAL){
    return;
  }

  int sec = ms/1000;
  int rest = (ms%1000);

  display.clearDisplay();

  drawDigit(sec/10, marginX, marginY);
  drawDigit(sec%10, marginX+digitWidth+space, marginY);

  if(!withoutColon){
    drawColon(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, colonRadius);
  }

  drawDigit(rest/100, SCREEN_WIDTH-marginX-digitWidth*2-space, marginY);
  drawDigit((rest%100)/10, SCREEN_WIDTH-marginX-digitWidth, marginY);

  display.display();
  timeOfLastRefresh = millis();
}

void printDigit(unsigned short digit){
  if((millis()-timeOfLastRefresh) < REFRESH_INTERVAL){
    return;
  }

  display.clearDisplay();

  drawDigit(digit, (SCREEN_WIDTH/2)-(digitWidth/2), marginY);

  display.display();
  timeOfLastRefresh = millis();
}

void checkGlass(){
  short glassState = digitalRead(FAKE_GLASS);
  short resetState = digitalRead(RESET);

  if(glassState == HIGH){
    return;
  }

  Serial.println("Ready");
  display.clearDisplay();
  display.setCursor(28,24);
  display.println("Bereit");
  display.display();

  while(true){
    glassState = digitalRead(FAKE_GLASS);
    resetState = digitalRead(RESET);

    if(glassState == HIGH && resetState == LOW){
      return;
    }
  }
}

void countDown(){
  short glassState = digitalRead(FAKE_GLASS);
  unsigned short countdown = 3000;
  unsigned int startTime = millis();

  while(true){
    glassState = digitalRead(FAKE_GLASS);

    if(glassState == LOW){
      Serial.println("Early start");

      display.clearDisplay();
      display.setCursor(4, 24);
      display.println("Fruehstart");
      display.display();
      
      while(digitalRead(RESET) != LOW){
        
        delay(100);
      }

      return;
    }

    if((startTime+countdown)-millis() > 2000){
      printDigit(3);
    } else if ((startTime+countdown)-millis() > 1000){
      printDigit(2);
    } else if((startTime+countdown)-millis() > 50){
      printDigit(1);
    } else {
      return;
    }
  }
}

void runGame(){
  bool glassLifted = false;
  short resetState = digitalRead(RESET);
  short glassState = digitalRead(FAKE_GLASS);
  unsigned int startTime = 0;
  unsigned int reactionTime = 0;
  unsigned int endTime = 0;

  startTime = millis();
  Serial.println("Start");

  while(true){
    resetState = digitalRead(RESET);
    glassState = digitalRead(FAKE_GLASS);

    printTime(millis()-startTime);

    if(resetState == LOW){
      return;
    }

    if(glassState == HIGH && glassLifted){
      endTime = millis();

      Serial.print("Finished after ");
      Serial.print(endTime-startTime);
      Serial.println("ms");
      
      while(true){
        resetState = digitalRead(RESET);

        if(resetState == LOW){
          return;
        }

        printTime(endTime-startTime);
        delay(500);
        display.clearDisplay();
        display.display();
        delay(500);
      }
    }

    if(glassState == LOW && !glassLifted){
      glassLifted = true;
      reactionTime = millis()-startTime;

      Serial.print("Glass lifted after ");
      Serial.print(reactionTime);
      Serial.println("ms");

      delay(100);
    }
  }
}

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);

    display.clearDisplay();
    display.display();
  }

  Serial.begin(115200);
  Serial.println("Start");

  pinMode(RESET, INPUT_PULLUP);
  pinMode(FAKE_GLASS, INPUT_PULLUP);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.display();
}

void loop() {
  checkGlass();
  countDown();
  runGame();
}
