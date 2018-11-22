
#ifndef LG_GPS_H
#define LG_GPS_H


#include <SoftwareSerial.h>
#include "lg-msg.h"
#include "lg-blink.h"



// constants
const int               GPS_SERIAL_RX_PIN   = 11;
const int               GPS_SERIAL_TX_PIN   = 12;
const int               GPS_ENABLE_PIN      = 20;
const unsigned long     GPS_ENABLE_TIMEOUT  = 2000;


// globals
SoftwareSerial      gpsSerial(GPS_SERIAL_RX_PIN, GPS_SERIAL_TX_PIN);   // RX, TX



struct NmeaLocation
{
    NmeaLocation()
        :m_iTimeS(0),
         m_fLatitudeDeg(0.0),
         m_fLongitudeDeg(0.0),
         m_fAltitudeM(0.0),
         m_bGoodMsg(false)
    {}
    
    long    m_iTimeS;
    float   m_fLatitudeDeg;
    float   m_fLongitudeDeg;
    float   m_fAltitudeM;
    bool    m_bGoodMsg;
};


/// setup GPS pins, etc.
bool setupGps()
{
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, LOW);
    
    gpsSerial.begin(9600);
    return true;
}


/// try to read until line is idle for _uTimeOutMs, until buffer is full, or until NL or CR is received; returns immediately if no bytes available and _bCheckAvailable == true
int readln(Stream &_rSerial, char *_pBuf, unsigned int _uBufSize, unsigned long _uTimeOutMs, bool _bCheckAvailable)
{
    if ( (_bCheckAvailable == true) &&
        (_rSerial.available() == 0) )
    {
        return 0;
    }
    
    int n = 0;
    for (unsigned long t = millis(); millis() < t + _uTimeOutMs;)
    {
        if (_rSerial.available() > 0)
        {
            int ch = _rSerial.read();
            
            // stop if NL or CR characters are received
            if ( (ch == '\n') ||
                (ch == '\r') )
            {
                if (n > 0)  // ignores NL & CR if no other characters have been received
                {
                    break;
                }
            }
            else
            {
                _pBuf[n] = ch;
                n++;
                
                if (n >= _uBufSize)
                {
                    n--;    // wind back to make space for '\0'
                    break;
                }
                else
                {
                    t = millis();
                }
            }
        }
    }
    
    _pBuf[n] = '\0';
    return n;
}


/// $GNGGA,150923.000,2545.3763,S,02816.7662,E,1,07,4.8,1433.8,M,0.84,,07,04,001,12*71
bool decodeNmeaGpgga(NmeaLocation &_location, const char *_pszNmea, int n)
{
    bool bSuccess = false;
    
    if ( (n > 25) &&
         (strncmp(_pszNmea+3, "GGA", 3) == 0) )
    {
        const char *pCh = _pszNmea;
        int count = 0;
        
        for (;;)
        {
            pCh = strchr(pCh, ',');
            if ( (pCh == nullptr) ||
                (pCh - _pszNmea > n) )
            {
                break;
            }
            else
            {
                pCh++;
                if (count == 0)
                {
                    long iTime = strtol(pCh, nullptr, 10);
                    long iHH = iTime / 10000;
                    long iMM = (iTime - iHH * 10000) / 100;
                    long iSS = (iTime - iHH * 10000 - iMM * 100);
                    _location.m_iTimeS = iSS + iMM * 60 + iHH * 3600;
                }
                else if (count == 1)
                {
                    float dLat = strtod(pCh, nullptr);
                    long iLatDeg = (long)dLat / 100;
                    _location.m_fLatitudeDeg = iLatDeg + (dLat - iLatDeg*100) / 60.0;
                }
                else if (count == 2)
                {
                    if (*pCh == 'S')
                    {
                        _location.m_fLatitudeDeg *= -1;
                    }
                }
                else if (count == 3)
                {
                    float dLon = strtod(pCh, nullptr);
                    long iLonDeg = (long)dLon / 100;
                    _location.m_fLongitudeDeg = iLonDeg + (dLon - iLonDeg*100) / 60.0;
                }
                else if (count == 4)
                {
                    if (*pCh == 'W')
                    {
                        _location.m_fLongitudeDeg *= -1;
                    }
                }
                else if (count == 5)
                {
                    int iFix = strtol(pCh, nullptr, 10);
                    _location.m_bGoodMsg = (iFix == 1) || (iFix == 2);
                }
                else if (count == 8)
                {
                    _location.m_fAltitudeM = strtod(pCh, nullptr);
                }
                
                count++;
            }
        }
        
        bSuccess = (count > 6);
        _location.m_bGoodMsg &= bSuccess;
    }
    
    return bSuccess;
}


// read and decode GPS data from serial port (returns true if a good GPS message was read)
bool readLocation(NmeaLocation &_location)
{
    static char pszNmea[96] = {0};
    bool bSuccess = false;
    
    if (gpsSerial.available() > 0)
    {
        int n = readln(gpsSerial, pszNmea, sizeof(pszNmea), 10, true);
        if (n > 0)
        {
            bSuccess = decodeNmeaGpgga(_location, pszNmea, n);
        }
    }
    
    return bSuccess;
}


bool gpsOn(bool _bState)
{
    // set pin output (with timeout)
    tickOutput(GPS_ENABLE_TIMEOUT, GPS_ENABLE_PIN, _bState, false);
}


void gpsEcho()
{
    if (gpsSerial.available() > 0)
    {
        Serial.write(gpsSerial.read());
    }
    
    if (Serial.available() > 0)
    {
        gpsSerial.write(Serial.read());
    }
}

#endif // #ifndef LG_GPS_H
