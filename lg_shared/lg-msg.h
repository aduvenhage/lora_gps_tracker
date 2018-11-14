
#ifndef LG_MSG_H
#define LG_MSG_H

// constants
const uint8_t         RFPROTO_ID          = 42;       //< unique protocol ID
const uint8_t         RFADDR_HQ           = 0;       //< gateway address
const uint8_t         RFADDR_BRDCST       = 255;     //< broadcast address



#pragma pack(push)
#pragma pack(1)
struct AppState
{
  AppState()
    :m_uProtoId(RFPROTO_ID),
     m_uSrcAddr(0),
     m_uDstAddr(0),
     m_uGpsTimeS(0),
     m_fVbty(0),
     m_fVcc(0),
     m_fLatitudeDeg(0),
     m_fLongitudeDeg(0),
     m_fAltitudeM(0),
     m_bCharging(false),
     m_bPowerOn(false),
     m_bGoodGpsFix(false),
     m_bBadLink(false)
  {}

  uint8_t         m_uProtoId;
  uint8_t         m_uSrcAddr;
  uint8_t         m_uDstAddr;
  unsigned long   m_uGpsTimeS;
  float           m_fVbty;
  float           m_fVcc;
  float           m_fLatitudeDeg;
  float           m_fLongitudeDeg;
  float           m_fAltitudeM;
  bool            m_bCharging;
  bool            m_bPowerOn;
  bool            m_bGoodGpsFix;
  bool            m_bBadLink;
};
#pragma pack(pop)


#pragma pack(push)
#pragma pack(1)
struct HqResponse
{
    HqResponse()
        :m_uProtoId(RFPROTO_ID),
         m_uSrcAddr(0),
         m_uDstAddr(0),
         m_uTimeMs(0)
    {}
    
    uint8_t         m_uProtoId;
    uint8_t         m_uSrcAddr;
    uint8_t         m_uDstAddr;
    unsigned long   m_uTimeMs;
};
#pragma pack(pop)



#endif // #ifndef LG_MSG_H

