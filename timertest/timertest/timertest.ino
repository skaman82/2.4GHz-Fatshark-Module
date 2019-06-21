#include <TVout.h>
#include <font6x8.h>
#include <font4x6.h>
#include <Wire.h>
#include "U8glib.h"
#include <EEPROM.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_FAST);  // Dev 0, Fast I2C / TWI

#define ButtonCenter          4
#define LED_pin               13
#define OSD_ctr1              5
#define OSD_ctr2              6
#define longpresstime         1000  // in ms
#define RST_pin               0 //NEW

int osd_mode = 0;
int menuactive = 0;
int BT_channel = 1;
int Old_FS_channel = 1;
int FS_channel = 1;
int FS_control = 0;
int SAVED_channel;
int ACT_channel = 1;
int BT_update = 0;
int pressedbut = 0;
int i_butt = 0;
word freq;
int lockmode;

byte voffset = 0;

byte refresh = 0; //NEW
byte osd_timeout; //NEW
byte lock_timeout; // NEW - do the same for lockmode later
byte display_setting = 0; //NEW setting for display modes. 0 = OLED+OSD | 1 = ONLY OSD | 2 = ONLY OLED (will require a reboot)

TVout TV;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void setup() {
  digitalWrite(RST_pin, HIGH); // pull reset high

  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }

  pinMode(LED_pin,      OUTPUT); //LED
  pinMode(OSD_ctr1,     OUTPUT); //ctrl1
  pinMode(OSD_ctr2,     OUTPUT); //ctrl2
  pinMode(ButtonCenter, INPUT_PULLUP); //Button
  pinMode(RST_pin,      OUTPUT); // NEW reset

  clearOLED();

  if (display_setting == 1) {  //NEW
    TV.begin(_PAL, 120, 90);
    osd_timeout = 100; //looptime setting for osd timeout
    lock_timeout = 500; //looptime setting for lockmode timeout
  }
  else {
    osd_timeout = 500; //looptime setting for osd timeout
    lock_timeout = 2500; //looptime setting for lockmode timeout
  }

  TV.clear_screen();

  digitalWrite(OSD_ctr1, HIGH);
  digitalWrite(OSD_ctr2, LOW);
  digitalWrite(LED_pin, HIGH);

  osd_mode = 1;
  menuactive = 0;
  lockmode = 0;
}


void clearOLED()
{
  u8g.firstPage();
  do
  {
  }
  while ( u8g.nextPage() );
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

byte buttoncheck()
{
  int i_butt = 0;
  byte buttonz = 0;
  if (digitalRead(ButtonCenter) != 1)
  {
    while (digitalRead(ButtonCenter) != 1)
    {
      delay(2);
      i_butt++;
    }
    buttonz = 1;
  }
  pressedbut = buttonz;
  return buttonz;
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void control() {
  if (pressedbut == 1) {
    callOSD();

    if (osd_mode == 1) {

      refresh = 1; //reset timer
      digitalWrite(RST_pin, LOW); // pull reset LOW for reset

    }
  }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void callOSD() {
  if (osd_mode != 1) {
    osd_mode = 1;
    return;
  }
  else {
    return;
  }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void osd() {
  if (osd_mode == 1) { //OSD ON
    digitalWrite(OSD_ctr1, HIGH);
    digitalWrite(OSD_ctr2, LOW);
    digitalWrite(LED_pin, LOW);
  }

  else if (osd_mode == 0) { //OSD OFF
    digitalWrite(OSD_ctr1, LOW);
    digitalWrite(OSD_ctr2, HIGH);
    digitalWrite(LED_pin, HIGH);
  }
  else {
    //osd_mode = 1;
  }
}


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void loop() {


  u8g.firstPage();
  do {

    // put your main code here, to run repeatedly:
    buttoncheck();
    control();
    osd();

    u8g.setFont(u8g_font_profont22r);

    if (refresh > 0) {
      u8g.setPrintPos(0, 24);
      u8g.print(osd_timeout - refresh);
    }
    else {
      u8g.setPrintPos(0, 24);
      u8g.print("OSD OFF");
    }

    TV.select_font(font6x8);
    TV.print(23, (27 + voffset), "OSD TEXT");



    //NEW OSD TIMER
    if ((menuactive != 1) && (osd_mode == 1)) {
      refresh++ ;

      if (refresh >= osd_timeout) {
        osd_mode = 0;
    #ifdef OSD
        TV.clear_screen();
        TV.tone(100, 150);
    #endif
        //save the last activechannel to EEPROM
        channelvalueEEP = ACT_channel;
        EEPROM.write(chanADDR, channelvalueEEP);
        refresh = 0;
      }

      else {
        osd_mode = 1;
      }
    }


  } while ( u8g.nextPage() );

}
