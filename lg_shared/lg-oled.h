

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
    display.ssd1306_command(SSD1306_DISPLAYON);

    return true;
}

bool displayOn(bool _bState)
{
    static unsigned long uDisplayTimeoutMs = 0;
    
    if (_bState == true)
    {
        display.ssd1306_command(SSD1306_DISPLAYON);
        uDisplayTimeoutMs = millis() + DISPLAY_ON_TIMEOUT_MS;
        return true;
    }
    else if (millis() > uDisplayTimeoutMs)
    {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        uDisplayTimeoutMs += DISPLAY_ON_TIMEOUT_MS;
        return false;
    }
    else
    {
        return true;
    }
}

void displayMsg(const AppState &_state, unsigned long _uTimeS)
{
    if (displayOn(false) == true)
    {
        if (_state.m_uFlags & APP_FLAG_GPSFIX)
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
        if (_state.m_uFlags & APP_FLAG_CHARGING)
        {
            display.print(" *");
        }
        
        display.println();

        display.print("t: ");
        display.print(_uTimeS);
        display.println();
    }
}

void clearDisplay()
{
    if (displayOn(false) == true)
    {
        display.clearDisplay();
        display.setCursor(0,0);
    }
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
