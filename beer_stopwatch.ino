#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define TRIGGER 16 //Trigger pin
#define RESET 12 //Reset pin

// Instantiate a Bounce object
Bounce triggerDebouncer = Bounce(); 
Bounce resetDebouncer = Bounce();

int temp = 0;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  pinMode(TRIGGER, INPUT);
  triggerDebouncer.attach(TRIGGER);
  triggerDebouncer.interval(5);

  pinMode(RESET, INPUT);
  resetDebouncer.attach(RESET);
  resetDebouncer.interval(5);
  
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Zum Starten Taste drücken");
  display.display();
  display.setTextSize(3);
}

bool startState = LOW;
bool runningState = LOW;
bool beerTaken = LOW;

unsigned long startMillis;
unsigned long currentMillis;
unsigned long elapsedMillis;
unsigned long countdownMillis = 3000;
long remainingMillis;

void loop() {
  resetDebouncer.update();

  if(resetDebouncer.rose() && runningState == LOW) {
    startState = HIGH;         // Toggle start button state
    startMillis = millis();
  }

  if(startState) {
    currentMillis = millis();
    remainingMillis = (countdownMillis - (currentMillis - startMillis));

    Serial.print("remainingMillis: ");
    Serial.print(remainingMillis);
    Serial.println("");    

    unsigned long durMS = (remainingMillis%1000);       //Milliseconds
    unsigned long durSS = (remainingMillis/1000)%60;    //Seconds
    
    Serial.print("Time: ");
    Serial.print(durSS);
    Serial.print(" : ");
    Serial.print(durMS);
    Serial.println("");

    if(remainingMillis <= 10){
      startState = LOW;
      runningState = HIGH;
    }else if(3000 >= remainingMillis >= 2000){
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("3");
      display.display();
    }else if(2000 >= remainingMillis >= 1000){
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("2");
      display.display();
    }else if(1000 >= remainingMillis >= 0){
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("1");
      display.display();
    }

    delay(10);
  }

  if(runningState) {
    currentMillis = millis();
    elapsedMillis = (currentMillis - startMillis);

    Serial.print("elapsedMillis: ");
    Serial.print(elapsedMillis);
    Serial.println("");
    
    unsigned long durMS = (elapsedMillis%1000);       //Milliseconds
    unsigned long durSS = (elapsedMillis/1000)%60;    //Seconds
    
    Serial.print("Time: ");
    Serial.print(durSS);
    Serial.print(" : ");
    Serial.print(durMS);
    Serial.println("");
    
    String durMilliSec = timeMillis(durSS,durMS);
    display.setCursor(0, 0);
    display.println(durMilliSec);
    
    delay(10);
  }
  
  triggerDebouncer.update();

  if (triggerDebouncer.rose() && runningState == LOW)
  {
    //FRÜHSTART
  }

  if(triggerDebouncer.rose() && runningState == HIGH){
    delay(10);
    if(triggerDebouncer.read() == HIGH){
      beerTaken = HIGH;
    }
  }

  if(triggerDebouncer.fell() && runningState == HIGH){
    delay(10);
    if(triggerDebouncer.read() == LOW){
      beerTaken = LOW;
      runningState = LOW;
    }
  }
  
}

String timeMillis(unsigned long Sectime,unsigned long MStime)
{
  String dataTemp = "";
  
  if (Sectime < 10)
  {
    dataTemp = dataTemp + "0" + String(Sectime)+ "s:";
  }
  else{
    dataTemp = dataTemp + String(Sectime)+ "s:";
  }
  
  dataTemp = dataTemp + String(MStime);

  Serial.print("String Time: ");
  Serial.println(dataTemp);
  
  return dataTemp;
}
