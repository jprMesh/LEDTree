#include <Adafruit_WS2801.h>
//#include "SPI.h"
#ifdef __AVR_ATtiny85__
    #include <avr/power.h>
#endif

// CONSTANTS
#define NUM_PIXELS 82
#define NUM_MODES 3
#define RAINBOW_DELAY 100
#define SPIRAL_DELAY 10

// GLOBALS
uint8_t dataPin  = 1;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 0;    // Green wire on Adafruit Pixels
uint8_t buttonPin = 3;   // Blue wire on tree

uint32_t time = 0;
uint8_t show_mode = 0;
uint8_t debouncing_state = 0;
uint8_t button_state = 0;

/** SETUP *********************************************************************/
// Initialize strip
Adafruit_WS2801 strip = Adafruit_WS2801(NUM_PIXELS, dataPin, clockPin);

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
    clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif
    
    pinMode(buttonPin, INPUT_PULLUP);
    strip.begin();
    strip.show();
}

/** LOOP **********************************************************************/
void loop() {
    update_state();
    if (show_mode == 0) {
        if ((millis() - time) > RAINBOW_DELAY) {
            rainbow();
            time = millis();
        }
    } else if (show_mode == 1) {
        if ((millis() - time) > SPIRAL_DELAY) {
            color_spiral();
            time = millis();
        }
    }
    strip.show();
}

/** ROUTINES ******************************************************************/
void color_spiral() {
    static int colors[6] = {0b100, 
                            0b110, 
                            0b010, 
                            0b011, 
                            0b001, 
                            0b101};
    static int color_index = 0;
    static int color = Color(255, 0, 0);
    static int j = 0;
    if (j < strip.numPixels()) { // Spiral up
        for (int i=0; i < strip.numPixels(); ++i) {
            if (i < j) {
                strip.setPixelColor(i, color);
            } else {
                strip.setPixelColor(i, Color(0,0,0));
            }
        }
    } else { // Blink
        if (j % 10 == 0) {
            for (int i=0; i < strip.numPixels(); ++i) {
                strip.setPixelColor(i, color);
            }
        } else if (j % 20 == 0) {
            for (int i=0; i < strip.numPixels(); ++i) {
                strip.setPixelColor(i, Color(0,0,0));
            }
        }
    }
    ++j;
    if (j > 140) {
        j = 0;
        ++color_index;
        if (color_index >= 6) {
            color_index = 0;
        }
        color = Color(((colors[color_index] & 0b100) >> 2) * 255,
                      ((colors[color_index] & 0b010) >> 1) * 255,
                      ((colors[color_index] & 0b001) >> 0) * 255);
    }
}

void rainbow() {
    static int j = 0;
    for (int i=0; i < strip.numPixels(); ++i) {
        strip.setPixelColor(i, Wheel( (i + j) % 255));
    }
    j = (j+1) & 0xFF;
}

/** HELPERS *******************************************************************/
bool debounce() {
    // global debouncing_state
    // global button_state
    static int PRESSED_STATE = 10;
    static int SAMPLES_PRESSED = 2;
    static int SAMPLES_RELEASE = 5;
    int debouncer;
    if (digitalRead(buttonPin)) {
        debouncing_state += PRESSED_STATE/SAMPLES_PRESSED;
        if (debouncing_state >= PRESSED_STATE) {
            debouncing_state = PRESSED_STATE;
            debouncer = 1;
        }
    } else {
        debouncing_state -= PRESSED_STATE/SAMPLES_RELEASE;
        if (debouncing_state <= 0) {
            debouncing_state = 0;
            debouncer = 0;
        }
    }
    if (button_state == 0 && debouncer == 1) {
        button_state = debouncer;
        return true;
    }
    button_state = debouncer;
    return false;
}

void update_state() {
    if (debounce()) {
        if (show_mode = NUM_MODES - 1) {
            show_mode = 0;
        } else {
            show_mode += 1;
        }
    }
}

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
    uint32_t c;
    c = r;
    c <<= 8;
    c |= g;
    c <<= 8;
    c |= b;
    return c;
}

//Input a value 0 to 255 to get a color value.
//The colors are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
    if (WheelPos < 85) {
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170; 
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}
