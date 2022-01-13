# beer_stopwatch

needed parts: 
- 128x64 pixel I2C OLED Display thats compatible with Adafruit SSD_1306 library
- magnetic switch
- simple button
- strong magnet glued to the bottom of a glass
- ESP8266

1) Connect magnetic switch to GPIO16 and GND
2) Connect button to GPIO12 and GND
3) Connect display to 5V, GND, and I2C pins (D1: CLK, D2: DATA)
4) Start drinking
