//#define debug

#define V3 //V1, V2 or V3
#define RX28 //RX28 (S-RX28) or SM186R
#define RSSI_mod //If RSSI mod is done on the rtc6711 chip (see datasheet)
#define OLED //V3 ONLY due space limitations
#define OSD 
#define PAL_FORMAT //PAL_FORMAT or NTSC_FORMAT for OSD (NTSC may not work)




//************V1*******************
#ifdef V1 //ATMEGA 328p version
  #define CH1_pin      A0
  #define CH2_pin      A1
#ifdef RX28
  #define CH3_pin      A2
#endif
  #define BX_pin       A3 
  #define RSSI_pin     A7
  #define FS_pin1      2
  #define FS_pin2      1 
  #define FS_pin3      0 
  #define ButtonCenter 4
  #define LED_pin      13
  #define OSD_ctr1     5
  #define OSD_ctr2     6
  #define BUZZ         11 

#endif

//************V2*******************
#ifdef V2 //ATMEGA 328p version with beeper and 3 buttons
  #define CH1_pin      A0
  #define CH2_pin      A1
#ifdef RX28
  #define CH3_pin      A2
#endif
  #define BX_pin       A3 
  #define RSSI_pin     A7
  #define FS_pin1      2
  #define FS_pin2      0 
  #define FS_pin3      1 
  #define ButtonCenter 4 
  #define ButtonLeft   8
  #define ButtonRight  10
  #define BUZZ         11 
  #define LED_pin      13
  #define OSD_ctr1     5
  #define OSD_ctr2     6

  //#undef OLED
#endif

//************V3*******************
#ifdef V3 //ATMEGA 644p 20MHz version with beeper and 5-way joystick
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
#endif
