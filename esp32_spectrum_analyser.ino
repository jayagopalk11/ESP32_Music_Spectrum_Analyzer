#include "audio_reactive.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define EEPROM_SIZE 5
#define LED_PIN     2
#define M_WIDTH     16
#define M_HEIGHT    16
#define NUM_LEDS    (M_WIDTH * M_HEIGHT)

#define EEPROM_BRIGHTNESS   0
#define EEPROM_GAIN         1
#define EEPROM_SQUELCH      2
#define EEPROM_PATTERN      3
#define EEPROM_DISPLAY_TIME 4
//SCL --> GPIO 18 EPS32
//SDA --> GPIO 23 EPS32
#define TFT_DC    16     // TFT DC  pin is connected to NodeMCU pin D1 (GPIO5)
#define TFT_RST   17     // TFT RST pin is connected to NodeMCU pin D2 (GPIO4)
#define TFT_CS    4     // TFT CS  pin is connected to NodeMCU pin D8 (GPIO15)

uint8_t numBands;
uint8_t barWidth;
uint8_t pattern;
uint8_t brightness;
uint16_t displayTime;
bool autoChangePatterns = false;

Adafruit_ST7789 tft =  Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


uint8_t peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t prevFFTValue[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t barHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


uint8_t colorTimer = 0;

void setup() {
  Serial.begin(115200);
  tft.init(240, 240, SPI_MODE2);
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(200);
  tft.fillScreen(ST77XX_BLACK);
  setupAudio();

  if (M_WIDTH == 8) numBands = 8;
  else numBands = 16;
  barWidth = M_WIDTH / numBands;
  
}

void testfillrectsval(uint16_t color1, uint16_t color2,int x, int y) {
  tft.fillScreen(ST77XX_BLACK);
  y = 240-y;
  tft.fillRect(x,y,14,240, color1);
  tft.drawRect(x,y,14,240, color2);

}

  
void loop() {
 
  uint8_t divisor = 1;                                                    // If 8 bands, we need to divide things by 2
  if (numBands == 8) divisor = 2;                                         // and average each pair of bands together
  
  for (int i = 0; i < 16; i += divisor) {
    uint8_t fftValue;
    
    if (numBands == 8) fftValue = (fftResult[i] + fftResult[i+1]) / 2;    // Average every two bands if numBands = 8
    else fftValue = fftResult[i];

    fftValue = ((prevFFTValue[i/divisor] * 3) + fftValue) / 4;            // Dirty rolling average between frames to reduce flicker
    barHeights[i/divisor] = fftValue / (255 / M_HEIGHT);                  // Scale bar height
    
    if (barHeights[i/divisor] > peak[i/divisor])                          // Move peak up
      peak[i/divisor] = min(M_HEIGHT, (int)barHeights[i/divisor]);
      
    prevFFTValue[i/divisor] = fftValue;                                   // Save prevFFTValue for averaging later
    
  }

  //Draw the pattern
  //tft.fillScreen(ST77XX_BLACK);
  for (int band = 0; band < numBands; band++) {
     uint8_t barHeight = barHeights[band];
     //Serial.print(String(barHeight)+",");
     uint8_t xcoord = (uint8_t)(band * 14);                               // Scale the band 14 times across x and y Axis
     uint8_t height = (uint8_t)((barHeight * 14));
     uint8_t ycoord = (uint8_t)(240 - height);
     tft.drawFastVLine(xcoord, 0, 240, ST77XX_BLACK);                     // Draw a black line to clear previous traces 
     tft.drawFastVLine(xcoord, ycoord, height, ST77XX_WHITE);             // Draw the plot in white colour in negative y axis
     
     //uint8_t ycoord = (uint8_t)(240 - (barHeight * 14));
     //tft.drawFastVLine(xcoord, ycoord, 239, ST77XX_YELLOW);
     //tft.drawFastVLine(xcoord, ycoord, 20, ST77XX_BLACK);
  }
  //tft.fillScreen(ST77XX_BLACK);
  //Serial.println("");
}
