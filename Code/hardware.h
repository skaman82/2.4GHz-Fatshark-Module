//#define debug

#define RX28 //RX28 (S-RX28) or SM186R
#define RSSI_mod //If RSSI mod is done on the rtc6711 chip (see datasheet)
#define PAL_FORMAT //PAL_FORMAT or NTSC_FORMAT for OSD (NTSC may not work)


//************V3 Hardware*******************
  #define CH1_pin      A3
  #define CH2_pin      A2
#ifdef RX28
  #define CH3_pin      A1
#endif
  #define BX_pin       A4 
  #define RSSI_pin     A0
  #define FS_pin1      2
  #define FS_pin2      8 //RXD0
  #define FS_pin3      9 //TXD0
  #define ButtonCenter 19 
  #define ButtonLeft   21
  #define ButtonRight  20
  #define ButtonUp     23
  #define ButtonDown   22
  #define BUZZ         15 //OC2A
  #define LED_pin      10 
  #define OSD_ctr1     11
  #define OSD_ctr2     12
  #define RST_pin       0 //NEW
