#include <SPI.h>
#include <XPT2046_Bitbang.h> // A library for interfacing with the touch screen
#include <TFT_eSPI.h>   // main Display
TFT_eSPI tft = TFT_eSPI();
#define NUM_BUTTONS 8
TFT_eSPI_Button key[NUM_BUTTONS];
#define LCD_BACK_LIGHT_PIN 21

// ----------------------------
// Touch Screen pins
// The CYD touch uses some non default
// SPI pins

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
// ----------------------------

XPT2046_Bitbang ts(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);


// GPIO I/O pins on the Arduino connected to strobe, clock, data,
// pick on any I/O you want.
#define  STROBE_TM GPIO_NUM_16
#define  CLOCK_TM GPIO_NUM_4
#define  DIO_TM GPIO_NUM_17

#include <TM1638.h>
#include <TM16xxDisplay.h>
#include <TM16xxButtons.h>
#include <Adafruit_GFX.h>
#include <TM1640.h>
#include <TM16xxMatrixGFX.h>

// Define the module on data pin 8, clock pin 9 and strobe pin 7
// Define the buttons object for detecting button states
// also define a display object for using print()
TM1638 module(DIO_TM, CLOCK_TM, STROBE_TM);
TM16xxButtons buttons(&module);       // TM16xx button 
TM16xxDisplay display(&module, 8);    // TM16xx object, 8 digits


void setup()
{
  Serial.begin(115200);
  Serial.println(F("TM16xxButtons example"));
  Serial.println();

  Serial.println(F("clear"));
  module.clearDisplay();              // clear display
  module.setupDisplay(true, 0);       // set intensity 0-7, 7=highest

  // Show some text.
  // Note: the TM16xxDisplay provides the println() function which is used in the callback functions below.
  Serial.println(F("txt"));
  module.setDisplayToString("boobs");
  delay(800);
  module.clearDisplay();

  // Attach the button callback functions that are defined below
  buttons.attachRelease(fnRelease);
  buttons.attachClick(fnClick);
  buttons.attachDoubleClick(fnDoubleclick);
  buttons.attachLongPressStart(fnLongPressStart);
  buttons.attachLongPressStop(fnLongPressStop);
  buttons.attachDuringLongPress(fnLongPress);

  // Start the SPI for the touch screen and init the TS library
  ts.begin();
  
  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape
  pinMode(LCD_BACK_LIGHT_PIN, OUTPUT); // set pin as output
  // backlight on full brightness
  digitalWrite(LCD_BACK_LIGHT_PIN,1);

  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&FreeMono18pt7b);
  drawButtons();
}
void drawButtons() {
  uint16_t bWidth = TFT_HEIGHT/(NUM_BUTTONS/2);
  uint16_t bHeight = TFT_WIDTH/2;
  // Generate buttons with different size X deltas
  for (int i = 0; i < NUM_BUTTONS; i++) {
    key[i].initButton(&tft,
                      bWidth * (i%4) + bWidth/2,
                      bHeight * (i/4) + bHeight/2,
                      bWidth,
                      bHeight,
                      TFT_BLACK, // Outline
                      TFT_BLUE, // Fill
                      TFT_BLACK, // Text
                      "",
                      1);

    key[i].drawButton(false, String(i+1));
  }
}

void loop()
{
  updateTFT();
  static unsigned long ulTime=millis();
  uint32_t dwButtons=buttons.tick();

  if(dwButtons)
    display.setDisplayToHexNumber(dwButtons, 0, false);
  else
  {
    // For best doubleclick detection, the loop() needs to be as fast as possible. 
    // So instead of calling delay(100), we check if 100ms has passed.
    if(millis()-ulTime>10)
    {
      char text[17];
      //sprintf(text, "%lu", millis());
      ltoa(millis(), text, 10);   // DECIMAL = base 10
      module.setDisplayToString(text);
      ulTime=millis();
    }
  }

}

void updateTFT() {
  TouchPoint p = ts.getTouch();
  tft.drawPixel(p.x,p.y,TFT_RED);
  // Adjust press state of each key appropriately
  for (uint8_t b = 0; b < NUM_BUTTONS; b++) {
    if ((p.zRaw > 0) && key[b].contains(p.x, p.y)) {
      key[b].press(true);  // tell the button it is pressed
      module.setupDisplay(true, b);
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < NUM_BUTTONS; b++) {
    // If button was just pressed, redraw inverted button
    if (key[b].justPressed()) {
      Serial.printf("Button %d pressed\n", b);
      key[b].drawButton(true, String(b+1));
    }

    // If button was just released, redraw normal color button
    if (key[b].justReleased()) {
      Serial.printf("Button %d released\n", b);
      Serial.println("Button " + (String)b + " released");
      key[b].drawButton(false, String(b+1));
    }
  }
}



// function to show some indicative blinks
void Blink(int nDelay=500, int nTimes=1)
{
}





//
// Button callback functions
//

// The Release function will be called when a button was released.
// It can be used for fast actions when no click or double click needs to be detected.
void fnRelease(byte nButton)
{
  // using isPressed or is LongPressed a shift-key can be implemented
  if(buttons.isLongPressed(0))
    Serial.print(F("Button 0 still longpressed. "));
  else if(buttons.isPressed(0))
    Serial.print(F("Button 0 still pressed. "));

  Serial.print(F("Button "));
  Serial.print(nButton);
  Serial.println(F(" release."));
  display.println(F("rels  "));
  Blink(10,1);
} // release


// This function will be called when a button was pressed 1 time (without a second press).
void fnClick(byte nButton)
{
  Serial.print(F("Button "));
  Serial.print(nButton);
  Serial.println(F(" click."));
  display.println(F("sclk  "));
  Blink(100,1);
} // click


// This function will be called when a button was pressed 2 times in a short timeframe.
void fnDoubleclick(byte nButton)
{
  Serial.print(F("Button "));
  Serial.print(nButton);
  Serial.println(F(" doubleclick."));
  display.println(F("dclk  "));
  Blink(200,2);
} // doubleclick


// This function will be called once, when a button is pressed for a long time.
void fnLongPressStart(byte nButton)
{
  Serial.print(F("Button "));
  Serial.print(nButton);
  Serial.println(F(" longPress start"));
  display.println(F("strt  "));
  Blink(50,1);
} // longPressStart


// This function will be called often, while a button is pressed for a long time.
void fnLongPress(byte nButton)
{
  Serial.print(F("Button "));
  Serial.print(nButton);
  Serial.println(F(" longPress..."));
} // longPress


// This function will be called once, when a button is released after beeing pressed for a long time.
void fnLongPressStop(byte nButton)
{
  Serial.print(F("Button "));
  Serial.print(nButton);
  Serial.println(F(" longPress stop"));
  display.println(F("stop  "));
  Blink(500,1);
} // longPressStop
