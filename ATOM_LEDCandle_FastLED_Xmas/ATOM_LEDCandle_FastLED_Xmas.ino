#include "M5Atom.h"
//#include <FastLED.h>
//#include <random>
#include <algorithm>
//#include <vector>
#include <array>

#define RGB(r, g, b)  (((g) << 16) + ((r) << 8) + (b))

//const int NUM_LEDS = 25;
const int LED_PIN = 27;
static CRGB leds[NUM_LEDS];
float fIntensity = random(10, 90)/100.0;

// 炎色 https://mtool.work/colordic/color/c44303
// RGB:(251, 100, 24)
uint8_t dR = 251;
uint8_t dG = 100;
uint8_t dB =  24;

using namespace std;
mt19937 get_rand_mt;
array<int,24> data = { 0,  1,  2,  3,  4,
                       5,  6,  7,  8,  9,
                      10, 11,     13, 14,
                      15, 16, 17, 18, 19,
                      20, 21, 22, 23, 24, 
                     };
array<CRGB,5> color = { 0x00FF00, // Red (GRB)
                        0x0000FF, // Blue
                        0xFFFF00, // Yellow
                        0x008080, // Purple
                        0x000000, // White
                      };

uint8_t pmode = 0;
unsigned long ctTime = 0;

void setup()
{
  M5.begin(true, false, false); // Serial, I2C, LED

  FastLED.addLeds<WS2812, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(80);
}

void loop()
{
  M5.update();
  if (M5.Btn.wasPressed())
  {
    pmode++;
    if (pmode + 1 > 2) pmode = 0;
  }
  
  Serial.println(pmode);
  
  if (pmode == 0){ ledCandle(); } else
  if (pmode == 1){ christmasTree(); }
  
  delay(100);
}

void christmasTree()
{
  if (millis() - ctTime > 1000)
  {
    blinkChristmasTree();
    ctTime = millis();
  }
}

void blinkChristmasTree()
{
  shuffle(begin(data), end(data), get_rand_mt);
//  shuffle(begin(color), end(color), get_rand_mt);
  
  for (uint32_t i=0; i<25; i++)
  {
//    leds[i] = RGB(xR, xG, xB);
    leds[i] = 0x800000; // Green

    // 任意の4箇所は特定の色にする
    for (uint32_t j=0; j<4; j++) 
      if (i==data[j])
//        leds[i] = RGB(128, 128, 0);
        leds[i] = color[j];
        
    FastLED.show();
  }
  
//  delay(1000);
//
//  // 中央は金色にする
//  leds[12] = 0xD7FF00; // Gold
//  FastLED.show();
  
//  delay(900);
}

void ledCandle()
{
  fIntensity = f1Fluctuation(fIntensity);
  
  uint8_t xR = (uint8_t)(dR*fIntensity);
  uint8_t xG = (uint8_t)(dG*fIntensity);
  uint8_t xB = (uint8_t)(dB*fIntensity);
  for (uint32_t i = 0; i < 13;i++)
  {
    //M5.dis.drawpix(i, dispColor(xG, xR, xB));
    //M5.dis.drawpix(i, RGB(xR, xG, xB));
    leds[i] = RGB(xR, xG, xB);
    FastLED.show();
  }
  
  xR = (uint8_t)(dR*(1 - fIntensity));
  xG = (uint8_t)(dG*(1 - fIntensity));
  xB = (uint8_t)(dB*(1 - fIntensity));
  for (uint32_t i = 13; i < 25;i++)
  {
    //M5.dis.drawpix(i, dispColor(xG, xR, xB));
    //M5.dis.drawpix(i, RGB(xR, xG, xB));
    leds[i] = RGB(xR, xG, xB);
    FastLED.show();
  }
}

float f1Fluctuation(float x)
{
  if(x < 0.5){
    x = x + 2*pow(x, 2);
  } 
  else
  {
    x = x - 2*pow((1.0 - x), 2);
  }
  
  if(x < 0.05 || x > 0.995){
    x = random(10, 90) / 100.0;
  }
  
  return x;
}

//CRGB dispColor(uint8_t g, uint8_t r, uint8_t b)
//{
//  return (CRGB)((g << 16) | (r << 8) | b);
//}
