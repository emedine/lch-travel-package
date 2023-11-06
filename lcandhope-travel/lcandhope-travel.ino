#include <FastLED.h>
 
// strip data
#define NUM_LEDS 115 

#define LED_TYPE   WS2812
#define COLOR_ORDER   GRB
 
// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 12
#define CLOCK_PIN 13

// CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

#define VOLTS          12
#define MAX_MA       4000

// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4

// hue, speed, and brighness vars
float bright = 55;
int brightLimit = 65;
int speed = 25;
float fadeAmt = 5;

// Rotary Encoder Params
/*
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
*/

// hardware controls
int inputA0 = 0; // analogRead(A0); // color
int inputA1 = 1; // analogRead(A1); // style
int inputA2 = 2; // analogRead(A2); // speed
int inputA3 = 3; // analogRead(A3); // bright 

float hue_val = 15;
int style_id = 1;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

bool fadeIn = true;

/// for TWINKLETOWN

CRGBArray<NUM_LEDS> leds;
// Overall twinkle density.

// 0 (VERY slow) to 8 (VERY fast).  
// 4, 5, and 6 are recommended, default is 4.
#define TWINKLE_SPEED 4
// 0 (NONE lit) to 8 (ALL lit at once).  
// Default is 5.
#define TWINKLE_DENSITY 5

// How often to change color palettes.
#define SECONDS_PER_PALETTE  30
// Background color for 'unlit' pixels
// Can be set to CRGB::Black if desired.
CRGB gBackgroundColor = CRGB::Black; 
// Example of dim incandescent fairy light background color
// CRGB gBackgroundColor = CRGB(CRGB::FairyLight).nscale8_video(16);

// If AUTO_SELECT_BACKGROUND_COLOR is set to 1,
// then for any palette where the first two entries 
// are the same, a dimmed version of that color will
// automatically be used as the background color.
#define AUTO_SELECT_BACKGROUND_COLOR 0

// If COOL_LIKE_INCANDESCENT is set to 1, colors will 
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 1

CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;

 
// Define the array of leds
// CRGB leds[NUM_LEDS];

void setup() { 
    Serial.println("initializing");
  delay(3000); //safety startup delay


  FastLED.setMaxPowerInVoltsAndMilliamps( VOLTS, MAX_MA);


    // twinkletown init

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip);

  chooseNextColorPalette(gTargetPalette);

  // Set rotary encoder pins as inputs
  /*
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
  // Read the initial state of CLK
	lastStateCLK = digitalRead(CLK);
	// Call updateEncoder() when any high/low changed seen on interrupt 0 (pin 2), or interrupt 1 (pin 3)
	attachInterrupt(0, updateEncoder, CHANGE);
	attachInterrupt(1, updateEncoder, CHANGE);
  */

  // FastLED.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
  FastLED.setBrightness(bright);
  Serial.begin(9600);
  Serial.println("launching LEDs");
  fadeall();
}
 

 
void loop() { 
  inputA0 = analogRead(A0); // color
  inputA1 = analogRead(A1); // style
  inputA2 = analogRead(A2); // brightness -- speed
  inputA3 = analogRead(A3); // speed
  // Rescale to potentiometer's voltage (from 0V to 5V):
  float voltage = floatMap(inputA0, 0, 1023, 0, 5); // we kind of don't care about this
  // normalize inputs
  // int adcreading0 = inputA0 >> 2;
  // int adcreading1 = inputA1 >> 2;
  // change hue val
  hue_val = map(inputA0, 0, 1000, 0, 255);
  // check for new style
  style_id = map(inputA1, 0, 1000, 1, 7); 
  // check for speed
  speed = map(inputA2, 0, 1000, 1, 100); 
  // print out the value you read:

/*
  Serial.println("--- new color -----");
  Serial.println("raw input A0");
  Serial.println(inputA0);
  Serial.println("color mapped");
  Serial.println(hue_val);

  Serial.println("--- new style -----");
  Serial.println("raw input A1");
  Serial.println(inputA1);
  Serial.println("style mapped");
  Serial.println(style_id);
*/


switch (style_id) {
  case 1:
    doTwinkly();
    break;
  case 2:
    doJuggle();
    break;
  case 3:
    doBpm();
    break;
  case 4:
    doRainbow();
    // doSinelon();
    break;
  case 5:
    doCylon();
    break;
  case 6:
    doBreathe();
    break;
  default:
    doNone();
    break;
}

  // delay(10);
  // doTwinkly();
  doCylon();
  //doBreathe();
  // fadeall();
  // doBreathe();
    // fastLED function params
  FastLED.show();  
  // insert a delay to keep the framerate modest
  /*
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  */
  

}

/// rotary encoder functions
/*
void updateEncoder(){
	// Read the current state of CLK
	currentStateCLK = digitalRead(CLK);

	// If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		if (digitalRead(DT) != currentStateCLK) {
			counter --;
			currentDir ="CCW";
		} else {
			// Encoder is rotating CW so increment
			counter ++;
			currentDir ="CW";
		}

		Serial.print("Direction: ");
		Serial.print(currentDir);
		Serial.print(" | Counter: ");
		Serial.println(counter);
	}
	// Serial.println("Rotary Detected: " + counter);
	// Remember last CLK state
	lastStateCLK = currentStateCLK;
}

*/

void doNone(){
    fadeall();
}
void doBreathe(){

    // fade in 
    if(fadeIn){

      for(int i = 0; i < NUM_LEDS; i++) {
        // check to see if we're changing color or style
        doVibeCheck();
        // update LEDs
        leds[i] = CHSV(hue_val, 255, bright);
        leds[i].maximizeBrightness(bright);
          
      }
       bright+=fadeAmt;

    } else if (!fadeIn){
     
      for(int i = 0; i < NUM_LEDS; i++) {
        doVibeCheck();
        // update LEDs
        leds[i] = CHSV(hue_val, 255, bright);
        leds[i].maximizeBrightness(bright);
        
      }
      bright-=fadeAmt;
    }

        // Show the leds
  FastLED.show();
  delay(1);

  if(bright <=0 ){
    bright = 1;
    fadeIn = true;
     Serial.println("--- fade breathe in -----");
  }

  if(bright > brightLimit){
    bright = brightLimit;
    fadeIn = false;
         Serial.println("--- fade breathe out -----");
  }
}


void doCylon(){
  bright = 100;
  // First slide the led in one direction
    for(int i = 0; i < NUM_LEDS; i++) {

        // check to see if we're changing color or style
        doVibeCheck();
        // update LEDs
        // Set the i'th led to our hue value 
        leds[i] = CHSV(hue_val, 255, bright);
        // Show the leds
        FastLED.show(); 
        // now that we've shown the leds, reset the i'th led to black
        // leds[i] = CRGB::Black;
        fadeall();
          
        // Wait a little bit before we loop around and do it again
        delay(speed);
    }
   //  Serial.print("y");
 
    // Now go in the other direction.  
    for(int i = (NUM_LEDS)-1; i >= 0; i--) {
      // check to see if we're changing color or style
        // check to see if we're changing color or style
        doVibeCheck();
        // update LEDs
        // Set the i'th led to red 
        leds[i] = CHSV(hue_val, 255, bright);
        // Show the leds
        FastLED.show();
        // now that we've shown the leds, reset the i'th led to black
        // leds[i] = CRGB::Black;
        fadeall();
        // Wait a little bit before we loop around and do it again


        delay(speed);
    }
}


void doRainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void doSinelon()
{
        // check to see if we're changing color or style
        doVibeCheck();

  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( hue_val, 255, 192);
}

void doBpm()
{
          // check to see if we're changing color or style
        doVibeCheck();
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, hue_val+(i*2), beat-hue_val+(i*10));
  }
}

void doJuggle() {
  // check to see if we're changing color or style
   doVibeCheck();
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = hue_val; // 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(hue_val, 200, 255);
    dothue += 32;
  }
}




void doTwinkly(){
     EVERY_N_SECONDS( SECONDS_PER_PALETTE ) { 
    chooseNextColorPalette( gTargetPalette ); 
  }
  
  EVERY_N_MILLISECONDS( 10 ) {
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 12);
  }

  drawTwinkles( leds);
  
  FastLED.show();
}



//  This function loops over each pixel, calculates the 
//  adjusted 'clock' that this pixel should use, and calls 
//  "CalculateOneTwinkle" on each pixel.  It then displays
//  either the twinkle color of the background color, 
//  whichever is brighter.
void drawTwinkles( CRGBSet& L)
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;
  
  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if AUTO_SELECT_BACKGROUND_COLOR == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if( (AUTO_SELECT_BACKGROUND_COLOR == 1) &&
      (gCurrentPalette[0] == gCurrentPalette[1] )) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();
  
  for( CRGB& pixel: L) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color, 
      // use the new color.
      pixel = c;
    } else if( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      pixel = blend( bg, c, deltabright * 8);
    } else { 
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      pixel = bg;
    }
  }
}


///////////////////////// UTILS ////////////////////////////////

/// this checks for new potentiometer value and updates our params
void doVibeCheck(){
  inputA0 = analogRead(A0); // color
  inputA1 = analogRead(A1); // style
  inputA2 = analogRead(A2); // speed/bright
  // change hue val
  hue_val = map(inputA0, 0, 1000, 0, 255);
    // check for new style
  style_id = map(inputA1, 0, 1000, 1, 7); 
  // check for current style-- if it's cylon, adjust spee
  if(style_id == 5){
      speed = map(inputA2, 0, 1000, 1, 100); 

  } else {
    speed = 25;
  }

}



float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void fadeall() { 
  for(int i = 0; i < NUM_LEDS; i++) { 
    leds[i].nscale8(250); 
    
    } 
}

//  This function takes a time in pseudo-milliseconds,
//  figures out brightness = f( time ), and also hue = f( time )
//  The 'low digits' of the millisecond time are used as 
//  input to the brightness wave function.  
//  The 'high digits' are used to select a color, so that the color
//  does not change over the course of the fade-in, fade-out
//  of one cycle of the brightness wave function.
//  The 'high digits' are also used to determine whether this pixel
//  should light at all during this cycle, based on the TWINKLE_DENSITY.
CRGB computeOneTwinkle( uint32_t ms, uint8_t salt)
{
  uint16_t ticks = ms >> (8-TWINKLE_SPEED);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);
  
  uint8_t bright = 0;
  if( ((slowcycle8 & 0x0E)/2) < TWINKLE_DENSITY) {
    bright = attackDecayWave8( fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if( bright > 0) {
    c = ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
    if( COOL_LIKE_INCANDESCENT == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}


// This function is like 'triwave8', which produces a 
// symmetrical up-and-down triangle sawtooth waveform, except that this
// function produces a triangle wave with a faster attack and a slower decay:
//
//     / \ 
//    /     \ 
//   /         \ 
//  /             \ 
//

uint8_t attackDecayWave8( uint8_t i)
{
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}

// This function takes a pixel, and if its in the 'fading down'
// part of the cycle, it adjusts the color a little bit like the 
// way that incandescent bulbs fade toward 'red' as they dim.
void coolLikeIncandescent( CRGB& c, uint8_t phase)
{
  if( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}

// A mostly red palette with green accents and white trim.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM =
{  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, 
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green };

// A mostly (dark) green palette with red berries.
#define Holly_Green 0x00580c
#define Holly_Red   0xB00402
const TProgmemRGBPalette16 Holly_p FL_PROGMEM =
{  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Red 
};

// A red and white striped palette
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM =
{  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
   CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A mostly blue palette with white accents.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM =
{  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
   HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
   QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
   CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight };

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
{  0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0xE0F0FF };

// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red    0xB80400
#define C9_Orange 0x902C02
#define C9_Green  0x046002
#define C9_Blue   0x070758
#define C9_White  0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM =
{  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
   C9_Orange, C9_Red,    C9_Orange, C9_Red,
   C9_Green,  C9_Green,  C9_Green,  C9_Green,
   C9_Blue,   C9_Blue,   C9_Blue,
   C9_White
};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM =
{
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
};


// Add or remove palette names from this list to control which color
// palettes are used, and in what order.
const TProgmemRGBPalette16* ActivePaletteList[] = {
  &RetroC9_p,
  &BlueWhite_p,
  &RainbowColors_p,
  &FairyLight_p,
  &RedGreenWhite_p,
  &PartyColors_p,
  &RedWhite_p,
  &Snow_p,
  &Holly_p,
  &Ice_p  
};


// Advance to the next color palette in the list (above).
void chooseNextColorPalette( CRGBPalette16& pal)
{
  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);
  static uint8_t whichPalette = -1; 
  whichPalette = addmod8( whichPalette, 1, numberOfPalettes);

  pal = *(ActivePaletteList[whichPalette]);
}
