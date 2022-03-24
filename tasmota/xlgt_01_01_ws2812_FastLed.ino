/*
  xlgt_01_01_ws2812_FastLed.ino - led string support for Tasmota

  Copyright (C) 2021  Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#define USE_LIGHT
#define USE_WS2812

#ifdef USE_LIGHT
#ifdef USE_WS2812
/*********************************************************************************************\
 * WS2812 RGB / RGBW Leds using NeopixelBus library
 *
 * light_scheme  WS2812  3+ Colors  1+2 Colors  Effect
 * ------------  ------  ---------  ----------  -----------------
 *  0 (5)        yes     no         no          Clock
 *  1 (6)        yes     no         no          Incandescent
 *  2 (7)        yes     no         no          RGB
 *  3 (8)        yes     no         no          Christmas
 *  4 (9)        yes     no         no          Hanukkah
 *  5 (10)       yes     no         no          Kwanzaa
 *  6 (11)       yes     no         no          Rainbow
 *  7 (12)       yes     no         no          Fire
 *  8 (13)       yes     no         no          Stairs
\*********************************************************************************************/

#define XLGT_01_01             101

const uint8_t WS2812_SCHEMES_FASTLED = 9;      // Number of WS2812 schemes

const char kWs2812CommandsFastLed[] PROGMEM = "|"  // No prefix
 D_CMND_PIXELS "|" D_CMND_ROTATION "|" D_CMND_WIDTH ;

void (* const Ws2812CommandFastLed[])(void) PROGMEM = {
  &CmndPixelsFastLed, &CmndRotationFastLed, &CmndWidthFastLed };

#include "FastLED.h"


CRGB Leds[WS2812_MAX_LEDS];
uint8_t colorIndex[WS2812_MAX_LEDS];
uint8_t paletteIndex = 0;
uint8_t whichPalette = 0;

DEFINE_GRADIENT_PALETTE( greenblue_gp ) { 
  0,   0,  255, 245,
  46,  0,  21,  255,
  179, 12, 250, 0,
  255, 0,  255, 245
};

CRGBPalette16 purple_p = CRGBPalette16 (
    CRGB::DarkViolet,
    CRGB::DarkViolet,
    CRGB::DarkViolet,
    CRGB::DarkViolet,
    
    CRGB::Magenta,
    CRGB::Magenta,
    CRGB::Linen,
    CRGB::Linen,
    
    CRGB::Magenta,
    CRGB::Magenta,
    CRGB::DarkViolet,
    CRGB::DarkViolet,

    CRGB::DarkViolet,
    CRGB::DarkViolet,
    CRGB::Linen,
    CRGB::Linen
);

DEFINE_GRADIENT_PALETTE (heatmap_gp) {
    0,   0,   0,   0,   //black
  128, 255,   0,   0,   //red
  200, 255, 255,   0,   //bright yellow
  255, 255, 255, 255    //full white 
};

DEFINE_GRADIENT_PALETTE( orangepink_gp ) { 
    0,  255,  100,    0,     //orange
   90,  255,    0,  255,     //magenta
  150,  255,  100,    0,     //orange
  255,  255,  100,    0      //orange
};

DEFINE_GRADIENT_PALETTE( browngreen_gp ) { 
    0,    6,  255,    0,     //green
   71,    0,  255,  153,     //bluegreen
  122,  200,  200,  200,     //gray
  181,  110,   61,    6,     //brown
  255,    6,  255,    0      //green
};

CRGBPalette16 currentPalette(greenblue_gp);
CRGBPalette16 targetPalette(orangepink_gp);

CRGBPalette16 greenblue = greenblue_gp;
CRGBPalette16 purpule = purple_p;
CRGBPalette16 heatmap = heatmap_gp;

struct WS2812_FASTLED {
  uint8_t show_next = 1;
  uint8_t scheme_offset = 0;
  bool suspend_update = false;
} Ws2812FastLed;

/********************************************************************************************/

void Ws2812Clear(void)
{
  FastLED.clear();
  FastLED.show();
  Ws2812FastLed.show_next = 1;
}

void Ws2812SetColor(uint32_t led, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
  if (led) {
    Leds[led-1] = CRGB(red, green, blue);
  } 
  else {
    for (uint32_t i = 0; i < Settings->light_pixels; i++) {
      Leds[i] = CRGB(red, green, blue);
    }
  }
  if (!Ws2812FastLed.suspend_update) {
    FastLED.show();
    Ws2812FastLed.show_next = 1;
  }
}


/*********************************************************************************************\
 * Public - used by scripter only
\*********************************************************************************************/

void Ws2812ForceSuspend (void)
{
  Ws2812FastLed.suspend_update = true;
}

void Ws2812ForceUpdate (void)
{
  Ws2812FastLed.suspend_update = false;
  FastLED.show();
  Ws2812FastLed.show_next = 1;
}

/********************************************************************************************/


bool Ws2812SetChannels(void)
{
  uint8_t *cur_col = (uint8_t*)XdrvMailbox.data;

  Ws2812SetColor(0, cur_col[0], cur_col[1], cur_col[2], cur_col[3]);

  return true;
}

void Ws2812ShowScheme(void)
{
  uint32_t scheme = Settings->light_scheme - Ws2812FastLed.scheme_offset;
  uint16_t nb_pixels = Settings->light_pixels;

  switch (scheme) {
    case 0:  // Pattern SCHEME 5
      // Color each pixel from the palette using the index from colorIndex[]
      for (int i = 0; i < nb_pixels; i++) {
        Leds[i] = ColorFromPalette(greenblue, colorIndex[i]);
      }
      EVERY_N_MILLISECONDS(10){
        for (int i = 0; i < nb_pixels; i++) {
          colorIndex[i]++;
        }
      }
      FastLED.show();
      break;
    
    case 1:  // Pattern SCHEME 6
      //Switch on an LED at random, choosing a random color from the palette
      EVERY_N_MILLISECONDS(50){
        Leds[random8(0, nb_pixels - 1)] = ColorFromPalette(purple_p, random8(), 255, LINEARBLEND);
      }
      // Fade all LEDs down by 1 in brightness each time this is called
      fadeToBlackBy(Leds, nb_pixels, 1);
      
      FastLED.show();
      break;
    
    case 2: // Pattern SCHEME 7
      fill_palette(Leds, nb_pixels, paletteIndex, 255 / nb_pixels, heatmap, 255, LINEARBLEND);
      EVERY_N_MILLISECONDS(10){
        paletteIndex++;
      }
      FastLED.show();
      break;

    case 3: // Pattern SCHEME 8
        // Color each pixel from the palette using the index from colorIndex[]
        for (int i = 0; i < nb_pixels; i++) {
          Leds[i] = ColorFromPalette(currentPalette, colorIndex[i]);
        }

        nblendPaletteTowardPalette( currentPalette, targetPalette, 10 );

        switch (whichPalette) {
          case 0:
            targetPalette = orangepink_gp;
            break;
          case 1:
            targetPalette = browngreen_gp;
            break;
          case 2:
            targetPalette = greenblue_gp;
            break;
        }

        EVERY_N_SECONDS(5) {
          whichPalette++;
          if (whichPalette > 2) 
            whichPalette = 0;
        }
        
        EVERY_N_MILLISECONDS(10){
          for (int i = 0; i < nb_pixels; i++) {
            colorIndex[i]++;
          }
        }

        FastLED.show();
      break;

    default:
      break;
  }
}

void Ws2812ModuleSelected(void)
{
  //FastLED.addLeds<NEOPIXEL,Pin(GPIO_WS2812)>(Leds, WS2812_MAX_LEDS);
  FastLED.addLeds<NEOPIXEL,14>(Leds, WS2812_MAX_LEDS);

  Ws2812Clear();
  FastLED.setBrightness(50);

  //Fill the colorIndex array with random numbers
  for (int i = 0; i < WS2812_MAX_LEDS; i++) {
    colorIndex[i] = random8();
  }

  Ws2812FastLed.scheme_offset = Light.max_scheme +1;
  Light.max_scheme += WS2812_SCHEMES_FASTLED;

#if (USE_WS2812_CTYPE > NEO_3LED)
    TasmotaGlobal.light_type = LT_RGBW;
#else
    TasmotaGlobal.light_type = LT_RGB;
#endif
    TasmotaGlobal.light_driver = XLGT_01_01;
  
}


void CmndPixelsFastLed(void)
{
  if ((XdrvMailbox.payload > 0) && (XdrvMailbox.payload <= WS2812_MAX_LEDS)) {
    Settings->light_pixels = XdrvMailbox.payload;
    Settings->light_rotation = 0;
    Ws2812Clear();
    Light.update = true;
  }

  ResponseCmndNumber(Settings->light_pixels);

}


void CmndRotationFastLed(void)
{
  if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload < Settings->light_pixels)) {
    Settings->light_rotation = XdrvMailbox.payload;
  }
  ResponseCmndNumber(Settings->light_rotation);
}

void CmndWidthFastLed(void)
{
/*  if ((XdrvMailbox.index > 0) && (XdrvMailbox.index <= 4)) {
    if (1 == XdrvMailbox.index) {
      if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload <= 4)) {
        Settings->light_width = XdrvMailbox.payload;
      }
      ResponseCmndNumber(Settings->light_width);
    } else {
      if ((XdrvMailbox.payload >= 0) && (XdrvMailbox.payload < 32)) {
        Settings->ws_width[XdrvMailbox.index -2] = XdrvMailbox.payload;
      }
      ResponseCmndIdxNumber(Settings->ws_width[XdrvMailbox.index -2]);
    }
  }
*/
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xlgt01_01(uint8_t function)
{
  bool result = false;

  switch (function) {
    case FUNC_SET_CHANNELS:
      result = Ws2812SetChannels();
      break;
    case FUNC_SET_SCHEME:
      Ws2812ShowScheme();
      break;
    case FUNC_COMMAND:
      result = DecodeCommand(kWs2812CommandsFastLed, Ws2812CommandFastLed);
      break;
    case FUNC_MODULE_INIT:
      Ws2812ModuleSelected();
      break;
  }
  return result;
}



#endif  // USE_WS2812
#endif  // USE_LIGHT
