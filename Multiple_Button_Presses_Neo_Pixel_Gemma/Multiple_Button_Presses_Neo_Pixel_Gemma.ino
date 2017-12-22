#include <Adafruit_NeoPixel.h>

#define buttonPin 0  // analog input pin to use as a digital input
#define NEO_PIN 1    // NeoPixel Data Strand
#define LED_COUNT 3  // # of neopixels in strand

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t dark = strip.Color(0, 0, 0);
int C_CB;                // int to store the check button inside a loop
boolean C_Break = false; // if true start breaking out of Rainbow loop

//==============================================================================

// Even Color Fading

// Set inital color value 
int redVal = 0;
int grnVal = 0;
int bluVal = 0;

// Initalize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

//==============================================================================

//  MULTI-CLICK:  One Button, Multiple Events

// Button timing variables
int debounce = 20;       // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 250;         // max ms between clicks for a double click event
int holdTime = 1000;     // ms hold period: how long to wait for press+hold event
int longHoldTime = 3000; // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal = HIGH;          // value read from button
boolean buttonLast = HIGH;         // buffered value of the button's previous state
boolean DCwaiting = false;         // whether we're waiting for a double click (down)
boolean DConUp = false;            // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;           // whether it's OK to do a single click
long downTime = -1;                // time the button was pressed down
long upTime = -1;                  // time the button was released
boolean ignoreUp = false;          // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false;         // when held, whether to wait for the up event
boolean holdEventPast = false;     // whether or not the hold event happened already
boolean longHoldEventPast = false; // whether or not the long hold event happened already

//==============================================================================

void clearPixels()
{
    colorWipe(dark, 1);
    strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) 
{
    if (WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}

int calculateStep(int prevValue, int endValue)
{
    int step = endValue - prevValue; // What is the overall gap?
    
    // If its a non zero
    if (step) {       
        step = 1020/step; // Divide by 1020 
    }

    return step;
}

int calculateVal(int step, int val, int i)
{
    // if step is non 0 and its time to change value
    if ((step) && i % step == 0) { 
        if (step > 0) {
            val += 1;
        }

        if (step < 0) {
            val -= 1;
        }
    }
  
    // Defensive driving: make val stays in the range 0-255
    if (val > 255) {
        val = 255;
    }

    if (val < 0) {
        val = 0;
    }
  
    return val;
}

void crossFade(int pixel, int color[3], int wait)
{
    int R = (color[0] * 255) / 100;
    int G = (color[1] * 255) / 100;
    int B = (color[2] * 255) / 100;

    int stepR = calculateStep(prevR, R);
    int stepG = calculateStep(prevG, G);
    int stepB = calculateStep(prevB, B);
  
    for (int i = 0; i <= 1020; i++) {
        C_CB = checkButton();
        
        if (C_CB == 2) {
            clearPixels();
            C_Break = true;
            break;
        }
    
        redVal = calculateVal(stepR, redVal, i);
        grnVal = calculateVal(stepG, grnVal, i);
        bluVal = calculateVal(stepB, bluVal, i);
    
        uint32_t setColor = strip.Color(redVal, grnVal, bluVal);
        colorWipe(setColor, 1);
    
        delay(wait);
    
        if (C_Break == true) {
            break;
        }
    }

    prevR = redVal;
    prevG = grnVal;
    prevB = bluVal;
}

// Events to trigger

void clickEvent() 
{ 
    int randNumber3 = random(0, 256);
    uint32_t color = strip.getPixelColor(0);
    uint32_t randColor = Wheel(randNumber3);
    uint16_t n = strip.numPixels();
  
    if (color == dark) {
        for (int i = 0; i < n; i++) {
            strip.setPixelColor(i, randColor);
            strip.show();
            delay(125);
        }
    }
  
    if (color != dark) {
        for(int i = (n-1); i >= 0; i--) {
            strip.setPixelColor(i, dark);
            strip.show();
            delay(125);
        }
    } 
}

void doubleClickEvent() 
{
    // Color Arrays
    int black[3] = { 0, 0, 0 };
    int white[3] = { 100, 80, 20 };
    int red[3] = { 100, 0, 0 };
    int pink[3] = { 100, 0, 40 };
  
    while (C_Break == false) {
    
        if (C_CB == 2) {
            clearPixels();
            C_Break = true;
            break;
        }
    
        int randomNumber1 = random(0,3);
        int randomNumber2 = random(0,3);
    
        if (randomNumber1 == 0) {
            crossFade(randomNumber2, red, 0);
            crossFade(randomNumber2, black, 2);
            clearPixels();
        } else if (randomNumber1 == 1) {
            crossFade(randomNumber2, pink, 0);
            crossFade(randomNumber2, black, 2);
            clearPixels();
        } else if (randomNumber1 == 2) {
            crossFade(randomNumber2, white, 0);
            crossFade(randomNumber2, black, 2);
            clearPixels();
        }
    
        if (C_Break == true) {
          break;
        }
    }
    
    clearPixels();
}

int checkButton() 
{    
    int event = 0;
    buttonVal = digitalRead(buttonPin);
    
    if (
        buttonVal == LOW 
    &&  buttonLast == HIGH 
    &&  (millis() - upTime) > debounce
    ) {
        // Button pressed down

        downTime = millis();
        ignoreUp = false;
        waitForUp = false;
        singleOK = true;
        holdEventPast = false;
        longHoldEventPast = false;
    
        if ((millis()-upTime) < DCgap && DConUp == false && DCwaiting == true) {
            DConUp = true;
        } else {
            DConUp = false;
        }

        DCwaiting = false;

    } else if (
        buttonVal == HIGH 
    &&  buttonLast == LOW 
    &&  (millis() - downTime) > debounce
    ) {
        // Button released        
        
        if (not ignoreUp) {
            upTime = millis();
            
            if (DConUp == false) { 
                DCwaiting = true;
            } else {
                event = 2;
                DConUp = false;
                DCwaiting = false;
                singleOK = false;
            }
        }
    }

    // Test for normal click event: DCgap expired
    if (
        buttonVal == HIGH 
    &&  (millis() - upTime) >= DCgap 
    &&  DCwaiting == true 
    &&  DConUp == false 
    &&  singleOK == true 
    &&  event != 2
    ) {
        event = 1;
        DCwaiting = false;
    }
    
    buttonLast = buttonVal;
    
    return event;
}

//==============================================================================

void setup() {
    // Set button input pin
    pinMode(buttonPin, INPUT);
    digitalWrite(buttonPin, HIGH );

    randomSeed(analogRead(2));

    strip.begin();
    strip.show();
}

void loop() {
    // Get button event and act accordingly
    int b = checkButton();
    
    if (b == 1) {
        clickEvent();
    }
    
    if (b == 2) {
        C_CB = -1;
        C_Break = false;
        doubleClickEvent();
    }
}
