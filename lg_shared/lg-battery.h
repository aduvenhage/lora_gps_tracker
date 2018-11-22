

#ifndef LG_BATTERY_H
#define LG_BATTERY_H



// constants
const int             BTY_APIN            = 1;
const int             BTY_R1              = 1000;
const int             BTY_R2              = 1000;
const float           BTY_VREF            = 3.30;
const float           BTY_VSCALE          = 1.002;

const int             VCC_APIN            = 0;
const int             VCC_R1              = 1000;
const int             VCC_R2              = 1000;
const float           VCC_VREF            = 3.30;
const float           VCC_VSCALE          = 1.002;


// sample analog pin (gets average of a few samples; returns float [0..1])
float sampleAnalogPin(int _iPin)
{
  float fValue = 0;
  const int n = 16;
  for (int i = 0; i < n; i++)
  {
    fValue += analogRead(_iPin);
    delay(1);
  }

  return fValue / n / 1023.0;
}


// read and scale battery voltage
float readBatteryVoltage()
{
  float fVb = sampleAnalogPin(BTY_APIN);
  fVb *= BTY_VSCALE * BTY_VREF * (BTY_R1 + BTY_R2) / BTY_R2;
  
  return fVb;
}


// read and scale supply voltage
float readSupplyVoltage()
{
  float fVcc = sampleAnalogPin(VCC_APIN);
  fVcc *= VCC_VSCALE * VCC_VREF * (VCC_R1 + VCC_R2) / VCC_R2;
  
  return fVcc;
}


#endif  // #ifndef LG_BATTERY_H
