

#include <Wire.h>
#include <Adafruit_SSD1306_32.h>
#include "lg-msg.h"


// constants
const int             OLED_RESET          = 4;


// globals
Adafruit_SSD1306    display(OLED_RESET);


// setup OLED (displays oled logo briefly)
bool setupDisplay()
{
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
