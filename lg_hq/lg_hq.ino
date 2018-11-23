

#include <lg-blink.h>
#include <lg-lora.h>
#include <lg-gps.h>
#include <lg-misc.h>

#include <avr/wdt.h>



// constants
const int             STATUS_PIN          = 9;
const int             STATUS_TIMEOUT_MS   = 200;      ///< how long radio/status light will stay on for


// Interrupt is called once a millisecond -- maintain output LEDs
SIGNAL(TIMER0_COMPA_vect) 
{
  static unsigned long count = 0;
  if (count % 10)
  {
    tickOutput(0, STATUS_PIN, false, false);
  }
  
  count++;
}


// read from lora
void readLora()
{
  uint8_t uSrcAddr = 0;
  uint8_t uDstAddr = 0;
  int iSnr = 0;

  uint8_t n = recvRadioMsg(uSrcAddr, uDstAddr, iSnr);
  if (n > 0)
  {
    // flash status LED
    tickOutput(STATUS_TIMEOUT_MS, STATUS_PIN, true, false);

    // decode message to serial port
    if (checkRadioMsgType<AppState>(n) == true)
    {
      AppState state;
      bool bSuccess = getRadioMsg(state, n);
      if (bSuccess == true)
      {
        bool bCharging = state.m_uFlags & APP_FLAG_CHARGING;
        bool bPowerOn = state.m_uFlags & APP_FLAG_POWERON;
        bool bGpsFix = state.m_uFlags & APP_FLAG_GPSFIX;
        bool bBadLink = state.m_uFlags & APP_FLAG_BADLINK;
        
        // reply to device
        HqResponse response;
        sendRadioMsg(response, RFADDR_HQ, uSrcAddr);
        
        // output data
        if (Serial)
        {
          Serial.print("addr=");
          Serial.print(state.m_uSrcAddr, HEX);
          Serial.print(", bty=");
          Serial.print(state.m_fVbty, 2);
          Serial.print(", lat=");
          Serial.print(state.m_fLatitudeDeg, 4);
          Serial.print(", lon=");
          Serial.print(state.m_fLongitudeDeg, 4);
          Serial.print(", chr=");
          Serial.print(bCharging ? "yes" : "no");
          Serial.print(", pwr=");
          Serial.print(bPowerOn ? "yes" : "no");
          Serial.print(", fix=");
          Serial.print(bGpsFix ? "yes" : "no");
          Serial.print(", snr=");
          Serial.print(iSnr);
          Serial.print(", badlink=");
          Serial.print(bBadLink ? "yes" : "no");
          Serial.println();
        }
      }
    }
  }
}


void setup()
{
  // setup output pins
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);
  
  // setup serial
  Serial.begin(19200);
  
  // setup peripherals
  setupRadio();
  setupGps();
  
  // Timer0 is already used for millis() - just interrupt somewhere to maintain LEDs
  cli();
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  sei();

  // start watchdog
  wdt_enable(WDTO_4S);
}


void loop()
{
  // debug
  blink(500, 13);
  
  // read external inputs
  readLora();

  // pet the dog
  wdt_reset();
}



