

#include <lg-blink.h>
#include <lg-lora.h>
#include <lg-gps.h>
#include <lg-misc.h>



// constants
const int             STATUS_PIN          = 9;
const int             STATUS_TIMEOUT_MS   = 200;



// read from lora
void readLora()
{
  uint8_t uSrcAddr = 0;
  uint8_t uDstAddr = 0;

  uint8_t n = recvRadioMsg(uSrcAddr, uDstAddr);
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
        // reply to device
        HqResponse response;
        response.m_uTimeMs = millis();
        
        sendRadioMsg(response, RFADDR_HQ, uSrcAddr);
        
        // output data
        if (Serial)
        {
          Serial.print("addr=");
          Serial.print(state.m_uSrcAddr, HEX);
          Serial.print(", chr=");
          Serial.print(state.m_bCharging ? "yes" : "no");
          Serial.print(", pwr=");
          Serial.print(state.m_bPowerOn ? "yes" : "no");
          Serial.print(", bty=");
          Serial.print(state.m_fVbty, 2);
          Serial.print(", vcc=");
          Serial.print(state.m_fVcc, 2);
          Serial.print(", lat=");
          Serial.print(state.m_fLatitudeDeg, 4);
          Serial.print(", lon=");
          Serial.print(state.m_fLongitudeDeg, 4);
          Serial.print(", alt=");
          Serial.print(state.m_fAltitudeM, 2);
          Serial.print(", fix=");
          Serial.print(state.m_bGoodGpsFix);
          Serial.print(", t=");
          Serial.print(state.m_uGpsTimeS);
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
}


void loop()
{
  // debug
  blink(500, 13);
  
  // read external inputs
  readLora();

  // maintain output timers
  tickOutput(0, STATUS_PIN, false, false);
}



