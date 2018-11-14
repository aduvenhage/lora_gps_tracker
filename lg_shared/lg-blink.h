


// blink any pin (50% duty cycle) -- function should be called from main loop to update state
void blink(unsigned long _uTimeMs, int _iPin)
{
  static unsigned long uLoopExpireTime[14] = {0};
  static bool bLoopLedOn[14] = {false};
  
  if (millis() > uLoopExpireTime[_iPin])
  {
    uLoopExpireTime[_iPin] = millis() + _uTimeMs;
    bLoopLedOn[_iPin] = !bLoopLedOn[_iPin];
    digitalWrite(_iPin, bLoopLedOn[_iPin] ? HIGH : LOW);
  }
}


// blink any pin (5% duty cycle) -- function should be called from main loop to update state
void flash(unsigned long _uTimeMs, int _iPin)
{
  static unsigned long uLoopExpireTime[14] = {0};
  static bool bLoopLedOn[14] = {false};
  
  if (millis() > uLoopExpireTime[_iPin])
  {
      uLoopExpireTime[_iPin] = millis() + (bLoopLedOn[_iPin] ? _uTimeMs : _uTimeMs/20);
      bLoopLedOn[_iPin] = !bLoopLedOn[_iPin];
      digitalWrite(_iPin, bLoopLedOn[_iPin] ? HIGH : LOW);
  }
}


// set pin with a timeout (pin resets to high if _bOutput is true or pin goes low after timeout) -- function should be called from main loop to update state
void tickOutput(unsigned long _uTimeMs, int _iPin, bool _bOutput, bool _bInverted)
{
  static unsigned long uLoopExpireTime[14] = {0};
  static bool bLoopLedOn[14] = {false};

  if (_bOutput == true)
  {
    bLoopLedOn[_iPin] = true;
    uLoopExpireTime[_iPin] = millis() + _uTimeMs;
  }
  else if (millis() > uLoopExpireTime[_iPin])
  {
    bLoopLedOn[_iPin] = false;
  }

  if (_bInverted == false)
  {
    digitalWrite(_iPin, bLoopLedOn[_iPin] ? HIGH : LOW);
  }
  else 
  {
    digitalWrite(_iPin, bLoopLedOn[_iPin] ? LOW : HIGH);
  }
}

