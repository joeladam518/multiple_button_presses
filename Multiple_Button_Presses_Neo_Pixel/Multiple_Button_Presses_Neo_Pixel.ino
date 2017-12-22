#include <Adafruit_NeoPixel.h>

#define buttonPin 9    // analog input pin to use as a digital input
#define NEO_PIN 6      // NeoPixel Data Strand
#define LED_COUNT 3    // # of neopixels in strand
#define PIX_LEVEL 255  // Neo Pixel Brightness Level

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t dark = strip.Color(0, 0, 0);
int C_CB;                // int to store the check button inside a loop
boolean C_Break = false; // if true start breaking out of Rainbow loop

//==============================================================================

//  MULTI-CLICK:  One Button, Multiple Events

// Button timing variables
int debounce = 20;         // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 250;           // max ms between clicks for a double click event
int holdTime = 1000;       // ms hold period: how long to wait for press+hold event
int longHoldTime = 3000;   // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal = HIGH;  // value read from button
boolean buttonLast = HIGH; // buffered value of the button's previous state
boolean DCwaiting = false; // whether we're waiting for a double click (down)
boolean DConUp = false;    // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;   // whether it's OK to do a single click
long downTime = -1;        // time the button was pressed down
long upTime = -1;          // time the button was released
boolean ignoreUp = false;  // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false;         // when held, whether to wait for the up event
boolean holdEventPast = false;     // whether or not the hold event happened already
boolean longHoldEventPast = false; // whether or not the long hold event happened already

//==============================================================================

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

void rainbow(uint8_t wait)
{
    uint16_t i, j;

    for (j=0; j<256; j++) {
        
        C_CB = checkButton();
        
        if (C_CB == 2) {
            clearPixels();
            C_Break = true;
            break;
        }
    
        for (i=0; i<strip.numPixels(); i++) {
            
            if (C_Break == true) {
                break;
            }
      
            strip.setPixelColor(i, Wheel((i+j) & 255));
      
            C_CB = checkButton();
            
            if (C_CB == 2) {
                clearPixels();
                C_Break = true;
                break;
            }
        }
    
        strip.show();
    
        if (C_Break == true) {
            break;
        }
    
        delay(wait);
    }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) 
{
    uint16_t i, j;

    for (j = 0; j < 256 * 5; j++) { 
        
        for (i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        
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

//==============================================================================

void clickEvent() 
{
    int randNumber = random(0, 256);
    uint32_t color = strip.getPixelColor(0);
    uint32_t randColor = Wheel(randNumber);
    uint16_t n = strip.numPixels();
  
    if (color == dark) {
        
        strip.setBrightness(PIX_LEVEL);
        
        for (int i = 0; i < n; i++) {
            strip.setPixelColor(i, randColor);
            strip.show();
            delay(125);
        }
    }

    if (color != dark) {
        
        strip.setBrightness(PIX_LEVEL);
    
        for (int i = (n - 1); i >= 0; i--) {
            strip.setPixelColor(i, dark);
            strip.show();
            delay(125);
        }
    }
}

void doubleClickEvent() 
{
    strip.setBrightness(PIX_LEVEL);
    
    while (C_Break == false) {
        rainbow(50);
    }
  
    clearPixels();
}

void holdEvent() 
{
    //... 
}

void longHoldEvent() 
{
    //...
}

//==============================================================================

int checkButton() 
{    
    int event = 0;
    buttonVal = digitalRead(buttonPin);

    
    if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce) {
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
    } else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce) {     
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
    if ( buttonVal == HIGH && (millis()-upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2) {
        event = 1;
        DCwaiting = false;
    }

    // Test for hold
    if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
        
        // Trigger "normal" hold
        if (not holdEventPast) {
            event = 3;
            waitForUp = true;
            ignoreUp = true;
            DConUp = false;
            DCwaiting = false;
            //downTime = millis();
            holdEventPast = true;
        }
        
        // Trigger "long" hold
        if ((millis() - downTime) >= longHoldTime) {
            if (not longHoldEventPast) {
                event = 4;
                longHoldEventPast = true;
            }
        }
    }

    buttonLast = buttonVal;

    return event;
}

//==============================================================================

void setup() {
    // Set button input pin
    pinMode(buttonPin, INPUT);
    digitalWrite(buttonPin, HIGH );
    
    randomSeed(analogRead(0));
    
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

    if (b == 3) {
        holdEvent();
    }

    if (b == 4) {
        longHoldEvent();
    }
}
