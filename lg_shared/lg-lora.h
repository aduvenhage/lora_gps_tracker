
#ifndef LG_LORA_H
#define LG_LORA_H


#include <RH_RF95.h>
#include <SPI.h>
#include "lg-msg.h"


// constants
const int             RFM95_CS            = 5;
const int             RFM95_RST           = 3;
const int             RFM95_INT           = 0;
const float           RF95_FREQ           = 434.0;


// globals
RH_RF95             radio(RFM95_CS, RFM95_INT);
uint8_t             radioBuffer[64];



// initialise radio
bool setupRadio()
{
  bool bSuccess = false;
    
  // setup LoRa
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(500);

  // test LoRa
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  bSuccess = radio.init();
    
  if (bSuccess == true)
  {
    radio.setFrequency(RF95_FREQ);
    radio.setTxPower(23, false);
    radio.setModemConfig(RH_RF95::Bw125Cr48Sf4096);
  }

  return bSuccess;
}


template <typename msg_type>
void sendRadioMsg(msg_type &_msg, uint8_t _uSrcAddr, uint8_t _uDstAddr)
{
  _msg.m_uSrcAddr = _uSrcAddr;
  _msg.m_uDstAddr = _uDstAddr;
  
  radio.send((uint8_t*)&_msg, sizeof(_msg));
  radio.waitPacketSent();
  radio.setModeRx();
}


uint8_t recvRadioMsg(uint8_t &_uSrcAddr, uint8_t &_uDstAddr, int &_iSnr)
{
  uint8_t n = 0;

  if (radio.available() == true)
  {
    n = sizeof(radioBuffer);
    bool bSuccess = radio.recv(radioBuffer, &n);
    if (bSuccess == true)
    {
      uint8_t uProtoId = radioBuffer[0];
      _uSrcAddr = radioBuffer[1]; 
      _uDstAddr = radioBuffer[2];
      _iSnr = radio.lastSNR();

      if (uProtoId != RFPROTO_ID)
      {
        n = 0;
      }
    }
  }
      
  return n;
}

template <typename msg_type>
bool checkRadioMsgType(uint8_t _uSize)
{
  return (sizeof(msg_type) == _uSize) &&
         (radioBuffer[0] == RFPROTO_ID);
}


template <typename msg_type>
bool getRadioMsg(msg_type &_msg, uint8_t _uSize)
{
  if (checkRadioMsgType<msg_type>(_uSize) == true)
  {
    memcpy(&_msg, radioBuffer, sizeof(_msg));
    return true;
  }
  else
  {
    return false;
  }
}



#endif // #ifndef LG_LORA_H
