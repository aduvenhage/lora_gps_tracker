

#include "lg-blink.h"
#include "lg-battery.h"
#include "lg-lora.h"
#include "lg-oled.h"
#include "lg-gps.h"
#include "lg-misc.h"



// constants
const uint8_t         APP_ADDR            = 1;
const int             WRN_PIN             = 6;
const int             STATUS_PIN          = 9;
const unsigned long   STATUS_TIMEOUT_MS   = 200;
const unsigned long   REPORT_TIMEOUT_MS   = 8000;
const unsigned long   DISPLAY_TIMEOUT_MS  = 1000;
const unsigned long   GPS_TIMEOUT_MS      = 3000;
const float           VBTY_FULL           = 4.10;    ///< full charge level
const int             BAD_TX_COUNT        = 5;


// globals
AppState            state;
int                 msgTxCount = 0;



// updates application state (read voltages, etc.)
void updateAppState()
{
  state.m_fVbty = readBatteryVoltage();
  state.m_fVcc = readSupplyVoltage();
  state.m_bPowerOn = state.m_fVcc > state.m_fVbty + 0.2;
  state.m_bBadLink = msgTxCount > BAD_TX_COUNT;

  const float fVbtyFull = state.m_bCharging ? VBTY_FULL : (VBTY_FULL-0.02);
  state.m_bCharging = (state.m_bPowerOn == true) && (state.m_fVbty < fVbtyFull);
}


// report app state
void reportAppState()
{
  static unsigned long uRadioTimeoutMs = REPORT_TIMEOUT_MS;
  static unsigned long uDisplayTimeoutMs = DISPLAY_TIMEOUT_MS;

  // radio status output
  if (millis() > uRadioTimeoutMs)
  {
    uRadioTimeoutMs += REPORT_TIMEOUT_MS + random(REPORT_TIMEOUT_MS/2);

    // report app state to HQ
    sendRadioMsg(state, APP_ADDR, RFADDR_BRDCST);
    msgTxCount = min(msgTxCount + 1, 1024);

    // debug
    if (Serial)
    {
      Serial.print("addr=");
      Serial.print(APP_ADDR, HEX);
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

  // oled
  if (millis() > uDisplayTimeoutMs)
  {
    uDisplayTimeoutMs += DISPLAY_TIMEOUT_MS;

    clearDisplay();
    displayMsg(state);  
    refreshDisplay();
  }
}


// read from lora
void readLora()
{
  uint8_t uSrcAddr = 0;
  uint8_t uDstAddr = 0;

  uint8_t n = recvRadioMsg(uSrcAddr, uDstAddr);
  if ( (n > 0) &&
       (uDstAddr == APP_ADDR) )
  {
    // flash status LED on any message
    tickOutput(STATUS_TIMEOUT_MS, STATUS_PIN, true, false);
    
    // decode messages from HQ
    if (checkRadioMsgType<HqResponse>(n) == true)
    {
      HqResponse response;
      bool bSuccess = getRadioMsg(response, n);
      if (bSuccess == true)
      {
        msgTxCount = 0;
      }
    }
  }
}


/// read and process GPS data
void readGps()
{
  static unsigned long uGpsTimeoutMs = GPS_TIMEOUT_MS;
  static bool gpsGood = false;
  static bool gpsEnabled = false;

  // power GPS off and on
  if ( (gpsGood == false) &&
       (millis() > uGpsTimeoutMs) )
  {
    gpsEnabled = true;
  }
  else if ( (gpsEnabled == true) &&
            (gpsGood == true) )
  {
    gpsEnabled = false;
    gpsGood = false;
    uGpsTimeoutMs = millis() + GPS_TIMEOUT_MS;
  }

  gpsOn(gpsEnabled);  

  static NmeaLocation location;
  if (readLocation(location) == true)
  {
    state.m_uGpsTimeS = (unsigned long)location.m_iTimeS;
    state.m_fLatitudeDeg = location.m_fLatitudeDeg;
    state.m_fLongitudeDeg = location.m_fLongitudeDeg;
    state.m_fAltitudeM = location.m_fAltitudeM;
    state.m_bGoodGpsFix = location.m_bGoodMsg;

    gpsGood |= location.m_bGoodMsg && gpsEnabled;
  }
}


void setup()
{
  // setup output pins
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);
  
  pinMode(WRN_PIN, OUTPUT);
  digitalWrite(WRN_PIN, LOW);

  // setup serial
  Serial.begin(19200);
  
  // setup peripherals
  setupDisplay();
  setupRadio();
  setupGps();
}


void loop()
{
  blink(500, 13);
  
  // read battery voltages, etc.
  updateAppState();

  // update WRN LED
  if (state.m_bBadLink == true)
  {
    blink(100, WRN_PIN);
  }
  else if (state.m_bPowerOn == false)
  {
    digitalWrite(WRN_PIN, HIGH);
  }
  else if (state.m_bCharging == true)
  {
    blink(1000, WRN_PIN);
  }
  else
  {
    digitalWrite(WRN_PIN, LOW);
  }

  // read external inputs
  readLora();
  readGps();

  // report app state
  reportAppState();
  
  // maintain output timers
  tickOutput(0, STATUS_PIN, false, false);
}



