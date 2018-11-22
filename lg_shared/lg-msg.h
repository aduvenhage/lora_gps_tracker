
#ifndef LG_MSG_H
#define LG_MSG_H



// constants
const uint8_t         RFPROTO_ID          = 42;       //< unique protocol ID
const uint8_t         RFADDR_HQ           = 0;       //< gateway address
const uint8_t         RFADDR_BRDCST       = 255;     //< broadcast address


const uint8_t   APP_FLAG_CHARGING   = 1;
const uint8_t   APP_FLAG_POWERON    = 2;
const uint8_t   APP_FLAG_GPSFIX     = 4;
const uint8_t   APP_FLAG_BADLINK    = 8;


#pragma pack(push)
#pragma pack(1)
struct AppState
{
  AppState()
    :m_uProtoId(RFPROTO_ID),
     m_uSrcAddr(0),
     m_uDstAddr(0),
     m_fVbty(0),
     m_fLatitudeDeg(0),
     m_fLongitudeDeg(0),
     m_uFlags(0)
  {}

  AppState(uint8_t _uSrcAddr, uint8_t _uDstAddr)
    :m_uProtoId(RFPROTO_ID),
     m_uSrcAddr(_uSrcAddr),
     m_uDstAddr(_uDstAddr),
     m_fVbty(0),
     m_fLatitudeDeg(0),
     m_fLongitudeDeg(0),
     m_uFlags(0)
  {}

  uint8_t         m_uProtoId;
  uint8_t         m_uSrcAddr;
  uint8_t         m_uDstAddr;
  float           m_fVbty;
  float           m_fLatitudeDeg;
  float           m_fLongitudeDeg;
  uint8_t         m_uFlags;
};
#pragma pack(pop)


#pragma pack(push)
#pragma pack(1)
struct HqResponse
{
    HqResponse()
        :m_uProtoId(RFPROTO_ID),
         m_uSrcAddr(0),
         m_uDstAddr(0)
    {}

    HqResponse(uint8_t _uSrcAddr, uint8_t _uDstAddr)
        :m_uProtoId(RFPROTO_ID),
         m_uSrcAddr(_uSrcAddr),
         m_uDstAddr(_uDstAddr)
    {}

    uint8_t         m_uProtoId;
    uint8_t         m_uSrcAddr;
    uint8_t         m_uDstAddr;
};
#pragma pack(pop)



#endif // #ifndef LG_MSG_H

