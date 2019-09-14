/* je nach taster saut ein Pünktle nach links oder nach rechts. */

/* mit Fast-GPIO-bibliothek, weil pololus APA102 auch im fast-modus wegen
    des bit-bangings noch etwas zu langsam ist

    supported led: Adafruit's DotStars - AKA the APA102
    supported platform: Arduino & compatibles - straight up arduino devices, uno, duo, leonardo, mega, nano, etc...
    Arduino ATMega328P based board (von: https://github.com/FastLED/FastLED/wiki/SPI-Hardware-or-Bit-banging)
        Hardware SPI - data (grün) 11, clock (gelb) 13
     Arduino ATMega1280/2560 based boards:
        Hardware SPI - data 51, clock 53

*/

#include <FastLED.h>
// Data pin that led data will be written out over
//      Hardware SPI - data (grün) 11, clock (gelb) 13
//      --> braucht nicht angegeben zu werden, da dies die HW vorgibt

#define NUM_LEDS 144
CRGB leds[NUM_LEDS];

const int LIMIT_LINKS = 50;   // led number where playing area starts
const int LIMIT_RECHTS = 100;
int tasterBreite = 5;         // length of landing position

const uint8_t taster1 = 8;    // input pin of left key, normally low, high if pressed
const uint8_t taster2 = 9;    // input pin of right key, normally low, high if pressed
int pos = (LIMIT_LINKS + LIMIT_RECHTS) / 2; // current position of dot
int del = 40;         // delay between loops = frame rate
int dir = 1;          // direction: 1 = right, -1 = left
int loslassen1 = 0;   // key still pressed?
int loslassen2 = 0;
int pointsLeft = 0;   // points of left user
int pointsRight = 0;
int fast = del;       // how fast dot is moving

void setup()
{
  FastLED.addLeds<APA102, BGR>(leds, NUM_LEDS); // leds sind nicht als RGB sondern als BGR angeordnet!
  memset8(leds, 0, NUM_LEDS * sizeof(CRGB));
  FastLED.setBrightness(15);
  drawField();
}


void loop()
{
  // delete previous cursor:
  if (pos <= LIMIT_LINKS + tasterBreite)
    leds[pos] = CRGB::Green;
  else if (pos >= LIMIT_RECHTS - tasterBreite)
    leds[pos] = CRGB::Magenta;
  else
    leds[pos] = CRGB::Black;

  // move to next position and mark it:
  pos = pos + dir;
  leds[pos] = CRGB::White;

  delay(fast);

  // beyond limits? user lost!
  if (pos < LIMIT_LINKS) {
    pointsRight++;
    redScreen();
  }
  if (pos > LIMIT_RECHTS) {
    pointsLeft++;
    redScreen();
  }

  // push button 1 was pressed:
  if ((digitalRead(taster1) == HIGH) && (loslassen1 == 0) && (dir == -1)) {
    // if it is within the allowed limits: change direction of movement and increase speed:
    if (pos >= LIMIT_LINKS && pos <= LIMIT_LINKS + tasterBreite) {
      dir = -dir;
      loslassen1 = 1;
      fast--;
    }
    else {
      // user pressed outside limits --> user lost
      pointsRight++;
      redScreen();
    }
  }
  if (digitalRead(taster1) == LOW) {
    loslassen1 = 0;
  }

  // push button 2 was pressed:
  if ((digitalRead(taster2) == HIGH) && (loslassen2 == 0) && (dir == 1)) {
    // if it is within the allowed limits: change direction of movement and increase speed:
    if (pos <= LIMIT_RECHTS && pos >= LIMIT_RECHTS - tasterBreite) {
      dir = -dir;
      loslassen2 = 1;
      fast--;
    }
    else {
      // user pressed outside limits --> user lost
      pointsLeft++;
      redScreen();
    }
  }
  if (digitalRead(taster2) == LOW) {
    loslassen2 = 0;
  }
  
  FastLED.show(); // show everything
}


void drawField() {
  // draw green 'landing area' on left and right border:
  for (int i = LIMIT_LINKS; i <= LIMIT_LINKS + tasterBreite; i++) {
    leds[i] = CRGB::Green;
    for (int i = 0; i < pointsRight; i++) {
      leds[i + 130] = CRGB::Blue;
    }
  }
  for (int i = LIMIT_RECHTS - tasterBreite; i <= LIMIT_RECHTS; i++) {
    leds[i] = CRGB::Magenta;

    for (int i = 0; i < pointsLeft; i++) {
      leds[i] = CRGB::Blue;
    }
  }
  
  // if someone reached 5 points --> display winner!
  if (pointsLeft == 5) {
    for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
      FastLED.show();
    }
    delay(500);
    pointsLeft = 0;
    redScreen();
  }

  if (pointsRight == 5) {
    for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB::Magenta;
      FastLED.show();
    }
    delay(500);
    pointsRight = 0;
    redScreen();
  }
  FastLED.show();

}

// flash red and initialize from beginning
void redScreen() {
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(1000);

  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  drawField();
  pos = (LIMIT_LINKS + LIMIT_RECHTS) / 2;
  dir = 1;
  fast = del;
}
