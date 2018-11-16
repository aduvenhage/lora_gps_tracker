

#ifndef LG_OLED_H
#define LG_OLED_H


#include <Wire.h>
#include <Adafruit_SSD1306_32.h>
#include "lg-msg.h"


// constants
const unsigned long     DISPLAY_ON_TIMEOUT_MS      = 5000;      ///< [ms]
const int               DISPLAY_ENABLE_PIN         = 1;


// globals
Adafruit_SSD1306    display;


// setup OLED (displays oled logo briefly)
bool setupDisplay()
{
    pinMode(DISPLAY_ENABLE_PIN, INPUT);
    
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    delay(500);
    display.clearDisplay();
    display.display();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    return true;
}

void displayMsg(const AppState &_state)
{
    if (_state.m_bGoodGpsFix == true)
    {
        display.print("P: ");
    }
    else
    {
        display.print("N: ");
    }
    
    display.print(_state.m_fLatitudeDeg);
    display.print(",");
    display.print(_state.m_fLongitudeDeg);
    display.println();
    
    display.print("V: ");
    display.print(_state.m_fVbty);
    display.print(",");
    display.print(_state.m_fVcc);
    display.println();

    display.print("t: ");
    display.print(_state.m_uGpsTimeS);
    display.println();
}

bool displayOn(bool _bState)
{
    static unsigned long uOnTimeMs = millis();
    unsigned long uTimeMs = millis();
    
    if (_bState == true)
    {
        display.ssd1306_command(SSD1306_DISPLAYON);
        uOnTimeMs = uTimeMs;
        return true;
    }
    else if (uTimeMs - uOnTimeMs > DISPLAY_ON_TIMEOUT_MS)
    {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        uOnTimeMs = uTimeMs - DISPLAY_ON_TIMEOUT_MS;
        return false;
    }
}

void clearDisplay()
{
    display.clearDisplay();
    display.setCursor(0,0);
}

void refreshDisplay()
{
    bool bEnable = digitalRead(DISPLAY_ENABLE_PIN) == HIGH;
    bool bVisible = displayOn(bEnable);
    if (bVisible == true)
    {
        display.display();
    }
}



#endif // #ifndef LG_OLED_H
