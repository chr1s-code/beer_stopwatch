#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
//#define TRIGGER 14 //Trigger pin
//#define RESET 13 //Reset pin
#define TRIGGER 14 //Trigger pin
#define RESET 12 //Reset pin

// Instantiate a Bounce object
Bounce triggerDebouncer = Bounce(); 
Bounce resetDebouncer = Bounce();

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  pinMode(TRIGGER, INPUT_PULLUP);
  triggerDebouncer.attach(TRIGGER);
  triggerDebouncer.interval(5);

  pinMode(RESET, INPUT_PULLUP);
  resetDebouncer.attach(RESET);
  resetDebouncer.interval(5);
  
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  
  display.clearDisplay();
  //display.setRotation(2); 
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Bereit");
  Serial.println("ready for startup");
  display.display();
}

String timeMillis(unsigned short Sectime,unsigned short MStime)
{
  String dataTemp = "";
  
  if (Sectime < 10)
  {
    dataTemp = dataTemp + "0" + String(Sectime)+ ":";
  }
  else{
    dataTemp = dataTemp + String(Sectime)+ ":";
  }
  
  dataTemp = dataTemp + String(MStime) + "s";

//  Serial.print("String Time: ");
//  Serial.println(dataTemp);
  
  return dataTemp;
}

// Returns true if x is in range [low..high], else false
bool inRange(unsigned low, unsigned high, unsigned x)
{
    return  ((x-low) <= (high-low));
}

void writeToDisplay(String str, unsigned short textSize = 3){
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setCursor(0, 0);
  display.print(str);
  display.display();
}

bool startState = LOW;
bool runningState = LOW;
bool beerTaken = LOW;

unsigned short startMillis;
unsigned short currentMillis;
unsigned short elapsedMillis;
unsigned short countdownMillis = 3000;
unsigned short remainingMillis;

void loop() {
  resetDebouncer.update();
  triggerDebouncer.update();

  if(resetDebouncer.rose() && runningState == LOW) {
    if(triggerDebouncer.read() == LOW){
      startState = HIGH;         // Toggle start button state
      startMillis = millis();
    }else{
      Serial.println("ERROR: GLASS NOT IN POSITION");
      
      writeToDisplay("Glas fehlt", 2);
      delay(2000);
      writeToDisplay("Bereit", 2);
    }
  }

  if(startState) {
    currentMillis = millis();
    remainingMillis = (countdownMillis - (currentMillis - startMillis));

//    Serial.print("remainingMillis: ");
//    Serial.print(remainingMillis);
//    Serial.println(" ");    

    unsigned long durMS = (remainingMillis%1000);       //Milliseconds
    unsigned long durSS = (remainingMillis/1000)%60;    //Seconds
    
    Serial.print("remaining time: ");
    Serial.print(durSS);
    Serial.print(" : ");
    Serial.print(durMS);
    Serial.println(" ");

    if(remainingMillis <= 50){
      startState = LOW;
      runningState = HIGH;

      writeToDisplay("GO", 4);
      Serial.println("###### GO ######");
      
      startMillis = millis();
    }else if(inRange(2000, 3000, remainingMillis)){
      writeToDisplay("3");

//      Serial.println("3");
    }else if(inRange(1000, 2000, remainingMillis)){
      writeToDisplay("2");

//      Serial.println("2");
    }else if(inRange(0, 1000, remainingMillis)){
      writeToDisplay("1");

//      Serial.println("1");
    }

    delay(5);
  }

  if(runningState) {
    currentMillis = millis();
    elapsedMillis = (currentMillis - startMillis);

//    Serial.print("elapsedMillis: ");
//    Serial.print(elapsedMillis);
//    Serial.println("");
    
    unsigned long durMS = (elapsedMillis%1000);       //Milliseconds
    unsigned long durSS = (elapsedMillis/1000)%60;    //Seconds
    
//    Serial.print("elapsed time: ");
//    Serial.print(durSS);
//    Serial.print(" : ");
//    Serial.print(durMS);
//    Serial.println(" ");
    
    String durMilliSec = timeMillis(durSS,durMS);
    if(elapsedMillis >= 10000){
      String shame = "Ernsthaft? " + durMilliSec + "?";
      writeToDisplay(shame, 2);
      Serial.println(shame);
    }else{
      writeToDisplay(durMilliSec);
      Serial.print("Elapsed Time: ");
      Serial.println(durMilliSec);
    }
    
    delay(5);
  }

  if (triggerDebouncer.rose() && runningState == LOW && startState == HIGH)
  {
    startState = LOW;
    writeToDisplay("FRUEHSTART", 2);
    Serial.println("###### FALSE START ######");
  }

  if(triggerDebouncer.rose() && runningState == HIGH){
    if(triggerDebouncer.read() == HIGH){
      beerTaken = HIGH;
      Serial.println("Beer lifted");
    }
    delay(5);
  }

  if(triggerDebouncer.fell() && runningState == HIGH){
    if(triggerDebouncer.read() == LOW){
      beerTaken = LOW;
      runningState = LOW;
    }
  }
}
