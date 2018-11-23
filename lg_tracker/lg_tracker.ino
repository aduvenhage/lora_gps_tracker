

#include "lg-blink.h"
#include "lg-battery.h"
#include "lg-lora.h"
#include "lg-oled.h"
#include "lg-gps.h"
#include "lg-misc.h"

#include <avr/wdt.h>



// constants
const uint8_t         APP_ADDR            = 8;
const int             WRN_PIN             = 6;
const int             STATUS_PIN          = 9;
const unsigned long   STATUS_TIMEOUT_MS   = 200;      ///< how long radio/status pin will stay on for
const unsigned long   REPORT_TIMEOUT_MS   = 15000;    ///< time between radio reports
const unsigned long   GPS_TIMEOUT_MS      = 45000;    ///< time between GPS checks
const unsigned long   RX_TIMEOUT_MS       = 4000;     ///< time to wait for HQ response
const float           VBTY_FULL           = 4.10;     ///< full charge level
const uint8_t         BAD_TX_COUNT        = 4;        ///< number of times HQ response may fail 


// globals
float               fVbty = 0;
float               fVcc = 0;
float               fLongitudeDeg = 0;
float               fLatitudeDeg = 0;
bool                bPowerOn = false;
bool                bCharging = false;
bool                bBadLink = false;
bool                bGoodGpsFix = false;
unsigned long       uGpsTimeS = 0;
uint8_t             uTxCount = 0;



// setup timer3 and attach LED timer callback
void setupLedTimer()
{
  const unsigned long CPU_FREQ = 16000000;
  const unsigned long ISR_FREQ = 100;
  
  cli(); // disable all interrupts
  
  // initialize Timer1
  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = 0;
  
  OCR3A = CPU_FREQ/256/ISR_FREQ;
  TCCR3B |= (1 << WGM32);   // CTC mode
  TCCR3B |= (1 << CS32);    // 256 prescaler
  TCCR3C = 0; // not forcing output compare
  TIMSK3 |= (1 << OCIE3A);  // enable timer compare interrupt
  
  sei(); // enable all interrupts
}


// LED timer ISR (compare interrupt service routine)
ISR(TIMER3_COMPA_vect)
{
  // update WRN LED
  if (bBadLink == true)
  {
    blink(200, WRN_PIN);
  }
  else if (bPowerOn == false)
  {
    flash(2000, WRN_PIN);
  }
  else if (bCharging == true)
  {
    blink(1000, WRN_PIN);
  }
  else if (bPowerOn == true)
  {
    digitalWrite(WRN_PIN, HIGH);
  }
  else
  {
    digitalWrite(WRN_PIN, LOW);
  }
  
  tickOutput(0, STATUS_PIN, false, false);
}


// build app state message
AppState buildAppState()
{
  AppState state;
  
  state.m_fVbty = fVbty;
  state.m_fLatitudeDeg = fLatitudeDeg;
  state.m_fLongitudeDeg = fLongitudeDeg;
  state.m_uFlags |= bCharging ? APP_FLAG_CHARGING : 0;
  state.m_uFlags |= bPowerOn ? APP_FLAG_POWERON : 0;
  state.m_uFlags |= bGoodGpsFix ? APP_FLAG_GPSFIX : 0;
  state.m_uFlags |= bBadLink ? APP_FLAG_BADLINK : 0;

  return state;
}


// updates application state (read voltages, etc.)
void updateAppState()
{
  const float fVbtyFull = bCharging ? VBTY_FULL : (VBTY_FULL-0.02);
  
  fVbty = readBatteryVoltage();
  fVcc = readSupplyVoltage();
  bPowerOn = fVcc > fVbty + 0.2;
  bBadLink = uTxCount >= BAD_TX_COUNT;
  bCharging = (bPowerOn == true) && (fVbty < fVbtyFull);
}


// report app state (through radio and serial)
void reportAppState()
{
  // report app state to HQ
  auto state = buildAppState();
  sendRadioMsg(state, APP_ADDR, RFADDR_BRDCST);

  // debug
  if (Serial)
  {
    Serial.print(F("addr="));
    Serial.print(APP_ADDR, HEX);
    Serial.print(F(", chr="));
    Serial.print(bCharging ? F("yes") : F("no"));
    Serial.print(F(", pwr="));
    Serial.print(bPowerOn ? F("yes") : F("no"));
    Serial.print(F(", bty="));
    Serial.print(fVbty, 2);
    Serial.print(F(", vcc="));
    Serial.print(fVcc, 2);
    Serial.print(F(", lat="));
    Serial.print(fLatitudeDeg, 4);
    Serial.print(F(", lon="));
    Serial.print(fLongitudeDeg, 4);
    Serial.print(F(", fix="));
    Serial.print(bGoodGpsFix);
    Serial.print(F(", t="));
    Serial.print(uGpsTimeS);
    Serial.println();
  }
}


// display app state on OLED
void displayAppState()
{
  clearDisplay();
  
  auto state = buildAppState();
  displayMsg(state, uGpsTimeS);  
  
  refreshDisplay();
}


// read from lora
bool readLora()
{
  bool bSuccess = false;
  uint8_t uSrcAddr = 0;
  uint8_t uDstAddr = 0;
  int iSnr = 0;

  unsigned long uLoopTimeout = millis() + RX_TIMEOUT_MS;
  while ( (millis() < uLoopTimeout) &&
          (bSuccess == false) )
  {
    uint8_t n = recvRadioMsg(uSrcAddr, uDstAddr, iSnr);
    if ( (n > 0) &&
         (uDstAddr == APP_ADDR) )
    {
      // flash status LED on any message
      tickOutput(STATUS_TIMEOUT_MS, STATUS_PIN, true, false);
      
      // decode messages from HQ
      if (checkRadioMsgType<HqResponse>(n) == true)
      {
        HqResponse response;
        bSuccess = getRadioMsg(response, n);
      }
    }
  }

  return bSuccess;
}


/// read and process GPS data
bool readGps()
{
  static unsigned long uGpsTimeoutMs = 0;
  static bool gpsEnabled = false;

  // try to enable GPS after every timeout
  if (millis() > uGpsTimeoutMs)
  {
    // flag bad GPS fix if still enabled after timeout
    if (gpsEnabled == true)
    {
      bGoodGpsFix = false;
    }
    
    uGpsTimeoutMs = millis() + GPS_TIMEOUT_MS;
    gpsEnabled = true;
  }

  // NOTE: circuit has a PNP resistor controlling power to GPS
  gpsOn(!gpsEnabled);  

  static NmeaLocation location;
  if (readLocation(location) == true)
  {
    // take GPS fix and reset
    if (location.m_bGoodMsg == true)
    {
      fLatitudeDeg = location.m_fLatitudeDeg;
      fLongitudeDeg = location.m_fLongitudeDeg;
      uGpsTimeS = (unsigned long)location.m_iTimeS;
      
      bGoodGpsFix = true;
      gpsEnabled = false;
    }
  }

  return gpsEnabled;
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
  setupLedTimer();
  
  // start watchdog
  wdt_enable(WDTO_8S);
}


void loop()
{
  static bool bGpsEnabled = false;
  static unsigned long uReportTimeoutMs = 0;
  static unsigned long uDisplayTimeoutMs = 0;
  
  bool bReportState = false;
  
  // debug
  blink(500, 13);
    
  // read battery voltages, etc.
  updateAppState();

  // read GPS
  bool bGpsState = readGps();
  if (bGpsState != bGpsEnabled)
  {
    if (bGpsState == false)
    {
      bReportState = true;
    }
    
    bGpsEnabled = bGpsState;
  }

  // check timeouts
  if (millis() > uReportTimeoutMs)
  {
      bReportState = true;
  }

  // report app state
  if (bReportState == true)
  {
    reportAppState();
    uReportTimeoutMs = millis() + REPORT_TIMEOUT_MS + random(REPORT_TIMEOUT_MS/2);
    
    if (uTxCount <= BAD_TX_COUNT)
    {
      uTxCount++;
    }

    if (readLora() == true)
    {
      uTxCount = 0;
    }
    
    radio.sleep();
  }

  // maintain OLED state
  displayAppState();

  // pet the dog
  wdt_reset();
}



