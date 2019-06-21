
// This Project is created by Albert Kravcov

// TO DOS & FIXMEs:
// X Main Menu structure
// X Calibrate function
// X Bandscan function
// X FS Button Control setting
// X Lock Mode function
// X Find OSD button bug not enabaling the OSD on first press

// • Finder Screen
// • OLED screens & menus


#include <TVout.h>
#include <font6x8.h>
#include <font4x6.h>
#include "hardware.h"
#include "logo.h"

#include <Wire.h>
#include "U8glib.h"

#include <EEPROM.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_FAST);  // Dev 0, Fast I2C / TWI

#ifdef OLED
#include "bitmaps.h"
#endif

#define longpresstime         1000  // in ms

#define chanADDR                 1  // EEPROM Adress
#define lockmodeADDR             2  // EEPROM Adress
#define fscontrollADDR           3  // EEPROM Adress
#define displayADDR              4  // EEPROM Adress
#define RSSImaxADDR              5  // EEPROM Adress
#define RSSIminADDR              6  // EEPROM Adress
#define serialADDR               7  // EEPROM Adress

int channelvalueEEP;
int lockmodeEEP;    //TODO: lock mode setting (all buttons deactivated)
int fscontrollEEP;  //TODO: FS Buttons disabled
int displayEEP;     //TODO: display settings
int RSSImaxEEP;     // RSSI calibration
int RSSIminEEP;     // RSSI calibration
int serialEEP;     // serial communication
int max; //= 365; //0% RSSI
int min; // = 338; //100% RSSI

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


byte voffset = 10; //20
byte rssiupdate = 0; //NEW
int refresh_osd = 0; //NEW
int refresh = 0; //NEW
int osd_timeout; //NEW
int lock_timeout; // NEW - do the same for lockmode later
byte display_setting; //NEW setting for display modes. 0 = OLED+OSD | 1 = ONLY OSD | 2 = ONLY OLED (will require a reboot)


TVout TV;

void setup() {
  digitalWrite(RST_pin, HIGH); // pull reset pin high
  //Serial.begin(19200);

#ifdef OLED
  // assign default color value
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
#endif

  pinMode(LED_pin,      OUTPUT); //LED
  pinMode(OSD_ctr1,     OUTPUT); //ctrl1
  pinMode(OSD_ctr2,     OUTPUT); //ctrl2
  pinMode(CH1_pin,      OUTPUT); //module c1
  pinMode(CH2_pin,      OUTPUT); //module c2
  pinMode(BX_pin,       OUTPUT); //module BX
  pinMode(FS_pin1,      INPUT); //google c1
  pinMode(FS_pin2,      INPUT); //google c2
  pinMode(FS_pin3,      INPUT); //google c3
  pinMode(ButtonCenter, INPUT_PULLUP); //Button
  pinMode(ButtonLeft,   INPUT_PULLUP); //Button left
  pinMode(ButtonRight,  INPUT_PULLUP); //Button right
  pinMode(ButtonUp,     INPUT_PULLUP); //Button left
  pinMode(ButtonDown,   INPUT_PULLUP); //Button right
  pinMode(BUZZ,         OUTPUT); //Buzzer
  pinMode(RST_pin,      OUTPUT); // NEW reset

#ifdef RSSI_mod
  pinMode(RSSI_pin, INPUT);
#endif

#ifndef SM186R
  pinMode(CH3_pin,      OUTPUT); //module c3
#endif

  channelvalueEEP = EEPROM.read(chanADDR);
  lockmodeEEP = EEPROM.read(lockmodeADDR);
  fscontrollEEP = EEPROM.read(fscontrollADDR);
  displayEEP = EEPROM.read(displayADDR); //TODO
  serialEEP = EEPROM.read(serialADDR); //TODO
  RSSImaxEEP = EEPROM.read(RSSImaxADDR)  * 2;
  RSSIminEEP = EEPROM.read(RSSIminADDR)  * 2;

  max = RSSImaxEEP;
  min = RSSIminEEP;


  if (fscontrollEEP >= 2) {
    fscontrollEEP = 0; //setting the  default state of no valid value
  }

  if (lockmodeEEP >= 2) {
    lockmodeEEP = 0; //setting the  default state of no valid value
  }

  if (channelvalueEEP <= 8)
  {
    SAVED_channel = channelvalueEEP;
  }
  if (channelvalueEEP > 8)
  {
    SAVED_channel = 1; //setting the  default state of no valid value
  }

  display_setting = 0; //just for testing

  clearOLED();

#ifdef OLED
  showlogo();
#endif

#ifdef PAL_FORMAT

  if (display_setting == 0) {  //NEW
#ifdef PAL_FORMAT
    TV.begin(_PAL, 128, 96); //original 128x96px, 160x120max (146x120)
#endif
#ifdef NTSC_FORMAT
    TV.begin(_NTSC, 120, 90);
#endif
    TV.delay_frame(1);
    display.output_delay = 55; //move the whole screen vertically

    TV.clear_screen();
    TV.bitmap(2, (20 + voffset), logo);

    osd_timeout = 100; //looptime setting for osd timeout
    lock_timeout = 300; //looptime setting for lockmode timeout
  }
  else {
    osd_timeout = 100; //looptime setting for osd timeout
    lock_timeout = 300; //looptime setting for lockmode timeout
  }
#endif

  if (display_setting == 0) {

    digitalWrite(OSD_ctr1, HIGH);
    digitalWrite(OSD_ctr2, LOW);
    digitalWrite(LED_pin, HIGH);

    TV.select_font(font6x8);
    TV.print(25, (50 + voffset), "2G4 MODULE");
    TV.select_font(font4x6);
    TV.print(24, (69 + voffset), "#RUININGTHEHOBBY");
    TV.delay(3000);
    TV.tone(500, 200);
    TV.clear_screen();

    osd_mode = 1;
  }
  else {
    tone(BUZZ, 500, 200);
    osd_mode = 0;
  }

#ifdef OLED
  clearOLED();
#endif

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

void showlogo()
{
#ifdef OLED
  u8g.firstPage();
  do
  {
    // splashscreen goes here
    u8g.drawBitmapP(10, 20, 14, 18, logo_OLED);
    u8g.setFont(u8g_font_5x7r);
    u8g.setPrintPos(23, 55);
    u8g.print("#RUININGTHEHOBBY");
  }
  while (u8g.nextPage());
#endif
}

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

    if (i_butt > (longpresstime / 2))
    {
      buttonz += 5;
    }
  }

  if (digitalRead(ButtonLeft) != 1)
  {
    while (digitalRead(ButtonLeft) != 1)
    {
      delay(2);
      i_butt++;
    }
    buttonz = 2;
  }

  if (digitalRead(ButtonRight) != 1)
  {
    while (digitalRead(ButtonRight) != 1)
    {
      delay(2);
      i_butt++;
    }
    buttonz = 3;
  }

  if (digitalRead(ButtonUp) != 1)
  {
    while (digitalRead(ButtonUp) != 1)
    {
      delay(2);
      i_butt++;
    }
    buttonz = 4;
  }

  if (digitalRead(ButtonDown) != 1)
  {
    while (digitalRead(ButtonDown) != 1)
    {
      delay(2);
      i_butt++;
    }
    buttonz = 5;
  }

  pressedbut = buttonz;
  return buttonz;
}


void fs_buttons() {
  // FS Button mapping:
  //CH1: p1-LOW,  p2-LOW,   p3-LOW
  //CH2: p1-HIGH, p2-LOW,   p3-LOW
  //CH3: p1-LOW,  p2-HIGH,  p3-LOW
  //CH4: p1-HIGH, p2-HIGH,  p3-LOW
  //CH5: p1-LOW,  p2-LOW,   p3-HIGH
  //CH6: p1-HIGH, p2-LOW,   p3-HIGH
  //CH7: p1-LOW,  p2-HIGH,  p3-HIGH
  //CH8: p1-HIGH, p2-HIGH,  p3-HIGH

  if ((digitalRead(FS_pin1) == LOW) && (digitalRead(FS_pin2) == LOW) && (digitalRead(FS_pin3) == LOW)) {
    FS_channel = 1;
  }
  else if ((digitalRead(FS_pin1) == HIGH) && (digitalRead(FS_pin2) == LOW) && (digitalRead(FS_pin3) == LOW)) {
    FS_channel = 2;
  }
  else if ((digitalRead(FS_pin1) == LOW) && (digitalRead(FS_pin2) == HIGH) && (digitalRead(FS_pin3) == LOW)) {
    FS_channel = 3;
  }
  else if ((digitalRead(FS_pin1) == HIGH) && (digitalRead(FS_pin2) == HIGH) && (digitalRead(FS_pin3) == LOW)) {
    FS_channel = 4;
  }
  else if ((digitalRead(FS_pin1) == LOW) && (digitalRead(FS_pin2) == LOW) && (digitalRead(FS_pin3) == HIGH)) {
    FS_channel = 5;
  }
  else if ((digitalRead(FS_pin1) == HIGH) && (digitalRead(FS_pin2) == LOW) && (digitalRead(FS_pin3) == HIGH)) {
    FS_channel = 6;
  }
  else if ((digitalRead(FS_pin1) == LOW) && (digitalRead(FS_pin2) == HIGH) && (digitalRead(FS_pin3) == HIGH)) {
    FS_channel = 7;
  }
  else if ((digitalRead(FS_pin1) == HIGH) && (digitalRead(FS_pin2) == HIGH) && (digitalRead(FS_pin3) == HIGH)) {
    FS_channel = 8;
  }

  else {
    FS_channel = 1;
    //BT_update = 1;
  }
}


void control() {
  if (pressedbut == 1) {

    if (lockmode == 0) {
      if (display_setting == 0) {
        TV.tone(800, 80);
      }
      else {
        tone(BUZZ, 800, 80);
      }
    }
    else {
      if (display_setting == 0) {
        TV.tone(100, 100);
        TV.delay(200);
        TV.tone(100, 100);
      }
      else {
        tone(BUZZ, 100, 100);
        delay(200);
        tone(BUZZ, 100, 100);
      }
    }


    if (lockmode == 0) {
      if (menuactive == 0) {
#ifdef OSD
        if (display_setting == 0) {
          TV.clear_screen();
        }
        menuactive = 1;
        menu();
        //bandscan();
      }
      else if (menuactive == 1) {
        if (display_setting == 0) {
          TV.fill(BLACK);
          TV.delay(10);
        }
        //menuactive = 0;
        return;
      }
    }

#endif
  }

  if (pressedbut == 2) {

    if (lockmode == 0) {
      if (display_setting == 0) {
        TV.tone(800, 80);
      }
      else {
        tone(BUZZ, 800, 80);
      }
    }
    else {
      if (display_setting == 0) {
        TV.tone(100, 100);
        TV.delay(200);
        TV.tone(100, 100);
      }
      else {
        tone(BUZZ, 100, 100);
        delay(200);
        tone(BUZZ, 100, 100);
      }
    }
    if (lockmode == 0) {
      if (menuactive == 0) {
        Old_FS_channel = FS_channel;
        BT_update = 1;
        FS_control = 0;

        if (osd_mode == 1) {
          refresh_osd = 1; //reset  OSD timer
          refresh = 1; //reset  lock timer

          if (BT_channel >= 2) {
            BT_channel -= 1;
          }

          else {
            BT_channel = 8;
          }
        }

        else {
          callOSD();
        }
      }
      else {
        if (display_setting == 0) {
          TV.clear_screen();
        }
        callOSD();
      }
    }
  }

  if (pressedbut == 3) {

    if (lockmode == 0) {
      if (display_setting == 0) {
        TV.tone(800, 80);
      }
      else {
        tone(BUZZ, 800, 80);
      }
    }
    else {
      if (display_setting == 0) {
        TV.tone(100, 100);
        TV.delay(200);
        TV.tone(100, 100);
      }
      else {
        tone(BUZZ, 100, 100);
        delay(200);
        tone(BUZZ, 100, 100);
      }
    }

    if (lockmode == 0) {
      if (menuactive == 0) {
        Old_FS_channel = FS_channel;
        BT_update = 1;
        FS_control = 0;

        if (osd_mode == 1) {

          refresh_osd = 1; //reset  osd timer
          refresh = 1; //reset timer

          if (BT_channel <= 7) {
            BT_channel += 1;
          }

          else {
            BT_channel = 1;
          }
        }

        else {
          callOSD();
        }
      }
      else {
        if (display_setting == 0) {
          TV.clear_screen();
        }
        callOSD();
      }
    }
  }

  if (pressedbut == 6) {

    if (lockmode == 1) {
      lockmode = 0;
      menuactive = 0;
      if (display_setting == 0) {
        TV.tone(400, 450);
      }
      else {
        tone(BUZZ, 400, 450);
      }

      callOSD();
      return;
    }    

  }
}


//>>RXUPDATE
void rx_update() {

  if  (FS_channel != Old_FS_channel) { //if a change is detected (FS buttons)
    BT_update = 0;
    FS_control = 1;
    if (osd_mode == 0) { //call OSD
      callOSD();
      Old_FS_channel = FS_channel; //store active FS button state to detect a change
    }
  }

  if (BT_update == 1) { //if a change is detected (module button)
    ACT_channel = BT_channel;
  }

  if (BT_update == 0) {

    if  ((SAVED_channel != 0) && (FS_control == 0)) { //if a saved channel is found, seti it at startup
      ACT_channel = SAVED_channel;
    }
    else {
      ACT_channel = FS_channel; //or set the active channel according to FS button position
    }
  }

  else {
    ACT_channel = BT_channel; //if nothing from above applies, set tha active channel acording to module button state
  }
}


void channeltable() {
#ifdef RX28 //RX28 receiver channel settings

  if (ACT_channel == 1)  {
    freq = 2414;
    //2414
    digitalWrite(BX_pin, LOW); //BX
    digitalWrite(CH1_pin, LOW); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(CH3_pin, HIGH); //3
  }
  else if (ACT_channel == 2)  {
    freq = 2432;
    //2432
    digitalWrite(BX_pin, LOW); //BX
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, LOW); //2
    digitalWrite(CH3_pin, HIGH); //3
  }
  else if (ACT_channel == 3)  {
    freq = 2450;
    //2450
    digitalWrite(BX_pin, LOW); //BX
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(CH3_pin, LOW); //3
  }
  else if (ACT_channel == 4)  {
    freq = 2468;
    //2468
    digitalWrite(BX_pin, LOW); //BX
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(CH3_pin, HIGH); //3
  }
  else if (ACT_channel == 5)  {
    freq = 2490;
    //2490
    digitalWrite(BX_pin, HIGH); //BX
    digitalWrite(CH1_pin, LOW); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(CH3_pin, HIGH); //3
  }
  else if (ACT_channel == 6)  {
    freq = 2510;
    //2510
    digitalWrite(BX_pin, HIGH); //BX
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, LOW); //2
    digitalWrite(CH3_pin, HIGH); //3
  }
  else if (ACT_channel == 7)  {
    freq = 2390;
    //2390
    digitalWrite(BX_pin, HIGH); //BX
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(CH3_pin, LOW); //3
  }
  else if (ACT_channel == 8)  {
    freq = 2370;
    //2370
    digitalWrite(BX_pin, HIGH); //BX
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(CH3_pin, HIGH); //3
  }
#endif


#ifdef SM186R //SM186R receiver channel settings

  if (ACT_channel == 1)  {
    freq = 2414;
    //2414
    digitalWrite(CH1_pin, LOW); //1
    digitalWrite(CH2_pin, LOW); //2
    digitalWrite(BX_pin, LOW); //BX
  }
  else if (ACT_channel == 2)  {
    freq = 2432;
    //2432
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, LOW); //2
    digitalWrite(BX_pin, LOW); //BX
  }
  else if (ACT_channel == 3)  {
    freq = 2450;
    //2450
    digitalWrite(CH1_pin, LOW); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(BX_pin, LOW); //BX
  }
  else if (ACT_channel == 4)  {
    freq = 2468;
    //2468
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(BX_pin, LOW); //BX
  }
  else if (ACT_channel == 5)  {
    freq = 2490;
    //2490
    digitalWrite(CH1_pin, LOW); //1
    digitalWrite(CH2_pin, LOW); //2
    digitalWrite(BX_pin, HIGH); //BX
  }
  else if (ACT_channel == 6)  {
    freq = 2510;
    //2510
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, LOW); //2
    digitalWrite(BX_pin, HIGH); //BX
  }
  else if (ACT_channel == 7)  {
    freq = 2390;
    //2390
    digitalWrite(CH1_pin, LOW); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(BX_pin, HIGH); //BX
  }
  else if (ACT_channel == 8)  {
    freq = 2370;
    //2370
    digitalWrite(CH1_pin, HIGH); //1
    digitalWrite(CH2_pin, HIGH); //2
    digitalWrite(BX_pin, HIGH); //BX
  }

#endif
}

void callOSD() {

  if (osd_mode != 1) {
    osd_mode = 1;
    return;
  }
  else {
    return;
  }
}

void osd() {

  if (osd_mode == 1) {
    if (display_setting == 0) {
      digitalWrite(OSD_ctr1, HIGH);
      digitalWrite(OSD_ctr2, LOW);
      digitalWrite(LED_pin, LOW);
    }
  }

  else if (osd_mode == 0) {
    digitalWrite(OSD_ctr1, LOW);
    digitalWrite(OSD_ctr2, HIGH);
    digitalWrite(LED_pin, HIGH);
  }
  else {
    //osd_mode = 1;
  }
}



//MAIN LOOP ************************

void loop() {

#ifdef OLED
  u8g.firstPage();
  do {
#endif
    buttoncheck();
    // Serial.println("HelloW");

    if (fscontrollEEP == 1) {
      if (lockmode == 0) {
        fs_buttons();
      }
    }

    control();
    rx_update();
    channeltable();
    runlocktimer();

#ifdef OSD
    osd();
#endif

#ifdef debug
    //Debug OSD output

    osd_mode = 1;
    uint32_t rssi_value = _readRSSI();
    float percentage = map(rssi_value, max, min, 0, 100);

    if (percentage > 100) {
      percentage = 100;
    }
    if (percentage < 0) {
      percentage = 0;
    }

    int p1_value = digitalRead(FS_pin1);
    int p2_value = digitalRead(FS_pin2);
    int p3_value = digitalRead(FS_pin3);

    if (display_setting == 0) {
      //do OSD stuff
    }

#endif

#ifndef debug

    if (display_setting == 0) {
      //Main OSD code
      TV.select_font(font6x8);
      TV.print(23, (27 + voffset), "CH");
      TV.println(ACT_channel);
      TV.draw_rect(18, (20 + voffset), 26, 20, WHITE);

      TV.draw_rect(44, (20 + voffset), 55, 20, WHITE);
      TV.print(49, (27 + voffset), freq);
      TV.println(" MHz");
    }

#ifdef RSSI_mod
    uint32_t rssi_value = _readRSSI();
    float percentage = map(rssi_value, max, min, 0, 100);
    if (percentage > 100) {
      percentage = 100;
    }
    else if (percentage < 0) {
      percentage = 0;
    }

    float rssibar = percentage * 0.81; //scaling the bar a bit down

    if (display_setting == 0) {
      TV.select_font(font4x6);
      TV.print(18, (50 + voffset), "RSSI ");
      TV.println((percentage - 0), 0);

      if ((percentage < 100) && (percentage >= 10)) {
        TV.draw_rect(46, (50 + voffset), 4, 4, BLACK, BLACK); //cover the first RSSI number
      }

      else if (percentage < 10) {
        TV.draw_rect(42, (50 + voffset), 8, 4, BLACK, BLACK); //cover the second RSSI number
      }

      TV.draw_rect(18, (58 + voffset), rssibar, 5, WHITE, WHITE);
      if (percentage < 100) {
        TV.draw_rect((18 + rssibar), (58 + voffset), (100 - rssibar), 5, BLACK, BLACK);
      }
      TV.draw_line(18, (64 + voffset), 100, (64 + voffset), WHITE);

      //TV.print(1, (53 + voffset), rssi_value); //for debugging only
    }

#endif

#endif

#ifndef debug
    if (display_setting == 0) {
      if ((menuactive != 1) && (osd_mode == 1)) {
        //NEW OSD TIMER
        refresh_osd++ ;

        if (refresh_osd >= osd_timeout) {
          osd_mode = 0;

          TV.clear_screen();
          TV.tone(100, 150);

          //saving the last activechannel to EEPROM
          channelvalueEEP = ACT_channel;
          EEPROM.write(chanADDR, channelvalueEEP);
          refresh_osd = 0;
        }
        else {
          osd_mode = 1;
        }
      }
    }
#endif

#ifdef OLED
    u8g.setFont(u8g_font_profont22r);
    u8g.setPrintPos(50, 26);
    u8g.print(freq);
    u8g.setFont(u8g_font_5x7r);
    u8g.print(" MHz");

#ifdef RSSI_mod

    u8g.setFont(u8g_font_5x7r);
    u8g.setPrintPos(50, 39);
    u8g.print("RSSI:");
    u8g.print(percentage, 0);

    //rssibar = 100;

    u8g.drawBox(53, 48, (rssibar * 0.60), 3); //
    u8g.drawFrame(50, 45, 66, 9);
    //u8g.setColorIndex(0);
    //u8g.drawVLine(53,48,5);
    //u8g.drawVLine(61,48,5);
    //u8g.drawVLine(69,48,5);
    //u8g.drawVLine(77,48,5);
    //u8g.drawVLine(85,48,5);
    //u8g.drawVLine(93,48,5);
    //u8g.drawVLine(101,48,5);
    //u8g.drawVLine(109,48,5);
    //u8g.setColorIndex(1);
#endif

    if (ACT_channel == 1) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_one);
    }
    else if (ACT_channel == 2) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_two);
    }
    else if (ACT_channel == 3) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_three);
    }
    else if (ACT_channel == 4) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_four);
    }
    else if (ACT_channel == 5) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_five);
    }
    else if (ACT_channel == 6) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_six);
    }
    else if (ACT_channel == 7) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_seven);
    }
    else if (ACT_channel == 8) {
      u8g.drawBitmapP(12, 12, 4, 41, bitmap_eight);
    }

    if ((lockmodeEEP == 1) && (lockmode == 1)) {
      u8g.drawBitmapP(109, 32, 2, 8, bitmap_lock);     //mini lock icon
    }
    else if ((lockmodeEEP == 1) && (lockmode == 0)) {
      u8g.drawBitmapP(109, 32, 2, 8, bitmap_lockopen);     //mini lock icon
    }

  } while ( u8g.nextPage() );
#endif
}



//RSSI READOUT ************************ simple avg of 4 value

uint16_t _readRSSI() {

  rssiupdate++;
  volatile uint32_t sum = 0;
  sum = analogRead(RSSI_pin);
  delay(1);
  sum += analogRead(RSSI_pin);
  delay(10);
  sum += analogRead(RSSI_pin);
  delay(10);
  sum += analogRead(RSSI_pin);
  return sum / 4;
}



//MAIN MENU ************************

void menu() {

  int menu_position = 1;
  byte exit = 0;
  clearOLED();

  while (exit == 0) {

    u8g.firstPage();
    do {
      osd();
      menuactive = 1;
      osd_mode = 1;

      if (menu_position == 1) {
        buttoncheck();

        if (display_setting == 0) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_nkizw_OSD); //goggle
          TV.print(15, (70 + voffset), "Goggle control:");
          if (fscontrollEEP == 1) {
            TV.print("ON");
          }
          else {
            TV.print("OFF");
          }
        }
#ifdef OLED
        u8g.drawBitmapP(38, 10, 7, 32, bitmap_nkizw);   //goggle
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(20, 55);
        u8g.print("GOGGLE CONTROLL:");
        if (fscontrollEEP == 1) {
          u8g.print("ON");
        }
        else {
          u8g.print("OFF");
        }
#endif
        if (pressedbut == 1) {

          if (fscontrollEEP == 1) {
            fscontrollEEP = 0;
            EEPROM.write(fscontrollADDR, fscontrollEEP);
          }
          else {
            fscontrollEEP = 1;
            EEPROM.write(fscontrollADDR, fscontrollEEP);
          }

          if (display_setting == 0) {
            TV.clear_screen();
          }
        }
        if (pressedbut == 2) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 2;
        }
      }

      else if (menu_position == 2) {

        buttoncheck();

        if (display_setting == 0) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_hd85pj_OSD); //scan
          TV.print(38, (70 + voffset), "Bandscan");
        }
#ifdef OLED
        u8g.drawBitmapP(38, 10, 7, 32, bitmap_hd85pj);   //scan
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(45, 55);
        u8g.print("BANDSCAN");
#endif
        if (pressedbut == 1) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          bandscan();
        }

        if (pressedbut == 2) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 3;
        }
        if (pressedbut == 3) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 1;
        }
      }

      else if (menu_position == 3) {

        buttoncheck();
        if (display_setting == 0) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_w113l_OSD); //lock
          TV.print(39, (70 + voffset), "Lock:");

          if (lockmodeEEP == 1) {
            TV.print("ON");
          }
          else {
            TV.print("OFF");
          }
        }
#ifdef OLED
        u8g.drawBitmapP(38, 10, 7, 32, bitmap_w113l);   //lock
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(35, 55);
        u8g.print("LOCK MODE:");

        if (lockmodeEEP == 1) {
          u8g.print("ON");
        }
        else {
          u8g.print("OFF");
        }
#endif
        if (pressedbut == 1) {

          if (lockmodeEEP == 1) {
            lockmodeEEP = 0;
            EEPROM.write(lockmodeADDR, lockmodeEEP);
          }
          else {
            lockmodeEEP = 1;
            EEPROM.write(lockmodeADDR, lockmodeEEP);
          }

          if (display_setting == 0) {
            TV.clear_screen();
          }
        }

        if (pressedbut == 2) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 4;
        }
        if (pressedbut == 3) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 2;
        }
      }

      else if (menu_position == 4) {

        buttoncheck();

        if (display_setting == 0) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_ydywrn_OSD); //search
          TV.print(35, (70 + voffset), "Find mode");
        }
#ifdef OLED
        u8g.drawBitmapP(38, 10, 7, 32, bitmap_ydywrn);   //search
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(45, 55);
        u8g.print("FIND MODE");
#endif

        if (pressedbut == 2) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 5;
        }
        if (pressedbut == 3) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 3;
        }
      }

      else if (menu_position == 5) {

        buttoncheck();

        if (display_setting == 0) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_949gqh_OSD); //tools
          TV.print(37, (70 + voffset), "Calibrate");
        }

#ifdef OLED
        u8g.drawBitmapP(38, 10, 7, 32, bitmap_calib);   //tools
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(45, 55);
        u8g.print("CALIBRATE");
#endif


        if (pressedbut == 1) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          calibration();
        }

        if (pressedbut == 2) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 6;
        }
        if (pressedbut == 3) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 4;
        }
      }

      else if (menu_position == 6) {

        buttoncheck();

        if (display_setting == 0) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_be8bbq_OSD); //exit
          TV.print(50, (70 + voffset), "EXIT");
        }
#ifdef OLED
        u8g.drawBitmapP(38, 10, 7, 32, bitmap_be8bbq);   //exit
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(55, 55);
        u8g.print("EXIT");
#endif

        if (pressedbut == 1) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 0;
          menuactive = 0;
          osd_mode = 0;
          exit = 1;
          return;
        }

        if (pressedbut == 3) {
          if (display_setting == 0) {
            TV.clear_screen();
          }
          menu_position = 5;
        }
      }

#ifdef OLED
    } while ( u8g.nextPage() );
#endif
  }
}



//CALIBRATION MENU ************************

void calibration() {     // Calibration wizzard

  byte exit = 0;
  while (exit == 0) {

    int calstep = 1;

    while (calstep == 1) {
      buttoncheck();
      channeltable();

      if (display_setting == 0) {

        TV.select_font(font4x6);
        TV.print(30, (0 + voffset), "RSSI CALIBRATION");
        TV.select_font(font6x8);
        TV.print(14, (30 + voffset), "1:Select channel");
        uint32_t rssi_value = _readRSSI();
        TV.print(21, (50 + voffset), "< ");
        TV.print(ACT_channel);
        TV.print(":");
        TV.print(freq);
        TV.print(" MHz");
        TV.print(" >");

        TV.draw_rect(38, (75 + voffset), 48, 16, WHITE);
        TV.select_font(font4x6);
        TV.print(51, (81 + voffset), "NEXT >");

      }

      if (ACT_channel <= 7) {
        if (pressedbut == 4) {
          ACT_channel += 1;
        }
      }
      if (ACT_channel >= 2) {

        if (pressedbut == 5) {
          ACT_channel -= 1;
        }
      }

      if (pressedbut == 1) {

        if (display_setting == 0) {
          TV.clear_screen();
        }
        calstep = 2;
      }
    }

    while (calstep == 2) {
      buttoncheck();
      uint32_t rssi_value = _readRSSI();

      if (display_setting == 0) {
        TV.print(30, (0 + voffset), "RSSI CALIBRATION");
        TV.select_font(font6x8);
        TV.print(0, (30 + voffset), "2:Remove antenna and switch off the VTX");
        TV.draw_rect(38, (75 + voffset), 48, 16, WHITE);
        TV.select_font(font4x6);
        TV.print(51, (81 + voffset), "NEXT >");
      }
      if (pressedbut == 1) {

        RSSImaxEEP = rssi_value / 2;
        EEPROM.write(RSSImaxADDR, RSSImaxEEP);

        if (display_setting == 0) {
          TV.clear_screen();
        }
        calstep = 3;
      }
    }
    while (calstep == 3) {
      buttoncheck();
      uint32_t rssi_value = _readRSSI();

      if (display_setting == 0) {
        TV.print(30, (0 + voffset), "RSSI CALIBRATION");
        TV.select_font(font6x8);
        TV.print(0, (30 + voffset), "3:Put on the antenna and switch on the VTX");
        TV.draw_rect(38, (75 + voffset), 48, 16, WHITE);
        TV.select_font(font4x6);
        TV.print(51, (81 + voffset), "NEXT >");
      }
      if (pressedbut == 1) {

        RSSIminEEP = rssi_value / 2;
        EEPROM.write(RSSIminADDR, RSSIminEEP);

        if (display_setting == 0) {
          TV.clear_screen();
        }
        calstep = 4;
      }
    }
    while (calstep == 4) {
      buttoncheck();

      RSSIminEEP = EEPROM.read(RSSIminADDR) * 2;
      RSSImaxEEP = EEPROM.read(RSSImaxADDR) * 2;

      if (display_setting == 0) {
        TV.print(30, (0 + voffset), "RSSI CALIBRATION");
        TV.select_font(font6x8);
        TV.print(8, (30 + voffset), "4:Calibration done!");
        TV.print(30, (50 + voffset), "Saved Max:");
        TV.print(RSSIminEEP);
        TV.print(30, (60 + voffset), "Saved Min:");
        TV.print(RSSImaxEEP);
        TV.draw_rect(38, (75 + voffset), 48, 16, WHITE);
        TV.select_font(font4x6);
        TV.print(51, (81 + voffset), "EXIT >");
      }
      if (pressedbut == 1) {

        if (display_setting == 0) {
          TV.clear_screen();
        }

        max = RSSImaxEEP;
        min = RSSIminEEP;
        calstep = 0;
        
        exit = 1;
      }
    }
  }

}

//BANDSCAN MENU ************************

void bandscan() {

  byte exit = 0;
  while (exit == 0) {

    menuactive = 1; //for debugging only
    buttoncheck();
    channeltable();

    if (display_setting == 0) {
      TV.select_font(font4x6);
      TV.print(35, (0 + voffset), "BAND SCANNER ");
      TV.draw_line(0, (70 + voffset), 123, (70 + voffset), WHITE);
    }
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH1
    if (ACT_channel == 1) {

      uint32_t rssi_value1 = _readRSSI();
      float percentage1 = map(rssi_value1, max, min, 0, 100);
      if (percentage1 > 100) {
        percentage1 = 100;
      }
      else if (percentage1 < 0) {
        percentage1 = 0;
      }
      if (display_setting == 0) {
        TV.draw_rect(0, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(0, ((70 - (percentage1 * 0.46)) + voffset), 10, (percentage1 * 0.46), WHITE, WHITE);
        TV.print(0, (75 + voffset), "CH1");
        if ((percentage1 < 100) && (percentage1 >= 10)) {
          TV.print(2, ((63 - (percentage1 * 0.46)) + voffset), percentage1, 0);
        }
        else if (percentage1 < 10) {
          TV.print(4, ((63 - (percentage1 * 0.46)) + voffset), percentage1, 0);
        }
        else {
          TV.print(0, ((63 - (percentage1 * 0.46)) + voffset), percentage1, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 2;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH2
    else if (ACT_channel == 2) {

      uint32_t rssi_value2 = _readRSSI();
      float percentage2 = map(rssi_value2, max, min, 0, 100);
      if (percentage2 > 100) {
        percentage2 = 100;
      }
      else if (percentage2 < 0) {
        percentage2 = 0;
      }
      if (display_setting == 0) {
        TV.draw_rect(16, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(16, ((70 - (percentage2 * 0.46)) + voffset), 10, (percentage2 * 0.46), WHITE, WHITE);
        TV.print(16, (75 + voffset), "CH2");
        if ((percentage2 < 100) && (percentage2 >= 10)) {
          TV.print(18, ((63 - (percentage2 * 0.46)) + voffset), percentage2, 0);
        }
        else if (percentage2 < 10) {
          TV.print(20, ((63 - (percentage2 * 0.46)) + voffset), percentage2, 0);
        }
        else {
          TV.print(16, ((63 - (percentage2 * 0.46)) + voffset), percentage2, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 3;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH3
    else if (ACT_channel == 3) {

      uint32_t rssi_value3 = _readRSSI();
      float percentage3 = map(rssi_value3, max, min, 0, 100);
      if (percentage3 > 100) {
        percentage3 = 100;
      }
      else if (percentage3 < 0) {
        percentage3 = 0;
      }

      if (display_setting == 0) {
        TV.draw_rect(32, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(32, ((70 - (percentage3 * 0.46)) + voffset), 10, (percentage3 * 0.46), WHITE, WHITE);
        TV.print(32, (75 + voffset), "CH3");
        if ((percentage3 < 100) && (percentage3 >= 10)) {
          TV.print(34, ((63 - (percentage3 * 0.46)) + voffset), percentage3, 0);
        }
        else if (percentage3 < 10) {
          TV.print(36, ((63 - (percentage3 * 0.46)) + voffset), percentage3, 0);
        }
        else {
          TV.print(32, ((63 - (percentage3 * 0.46)) + voffset), percentage3, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 4;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH4
    else if (ACT_channel == 4) {

      uint32_t rssi_value4 = _readRSSI();
      float percentage4 = map(rssi_value4, max, min, 0, 100);
      if (percentage4 > 100) {
        percentage4 = 100;
      }
      else if (percentage4 < 0) {
        percentage4 = 0;
      }
      if (display_setting == 0) {
        TV.draw_rect(48, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(48, ((70 - (percentage4 * 0.46)) + voffset), 10, (percentage4 * 0.46), WHITE, WHITE);
        TV.print(48, (75 + voffset), "CH4");
        if ((percentage4 < 100) && (percentage4 >= 10)) {
          TV.print(50, ((63 - (percentage4 * 0.46)) + voffset), percentage4, 0);
        }
        else if (percentage4 < 10) {
          TV.print(52, ((63 - (percentage4 * 0.46)) + voffset), percentage4, 0);
        }
        else {
          TV.print(48, ((63 - (percentage4 * 0.46)) + voffset), percentage4, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 5;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH5
    else if (ACT_channel == 5) {

      uint32_t rssi_value5 = _readRSSI();
      float percentage5 = map(rssi_value5, max, min, 0, 100);
      if (percentage5 > 100) {
        percentage5 = 100;
      }
      else if (percentage5 < 0) {
        percentage5 = 0;
      }
      if (display_setting == 0) {
        TV.draw_rect(64, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(64, ((70 - (percentage5 * 0.46)) + voffset), 10, (percentage5 * 0.46), WHITE, WHITE);
        TV.print(64, (75 + voffset), "CH5");
        if ((percentage5 < 100) && (percentage5 >= 10)) {
          TV.print(66, ((63 - (percentage5 * 0.46)) + voffset), percentage5, 0);
        }
        else if (percentage5 < 10) {
          TV.print(68, ((63 - (percentage5 * 0.46)) + voffset), percentage5, 0);
        }
        else {
          TV.print(64, ((63 - (percentage5 * 0.46)) + voffset), percentage5, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 6;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH6
    else if (ACT_channel == 6) {

      uint32_t rssi_value6 = _readRSSI();
      float percentage6 = map(rssi_value6, max, min, 0, 100);

      if (percentage6 > 100) {
        percentage6 = 100;
      }
      else if (percentage6 < 0) {
        percentage6 = 0;
      }

      if (display_setting == 0) {
        TV.draw_rect(80, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(80, ((70 - (percentage6 * 0.46)) + voffset), 10, (percentage6 * 0.46), WHITE, WHITE);
        TV.print(80, (75 + voffset), "CH6");
        if ((percentage6 < 100) && (percentage6 >= 10)) {
          TV.print(82, ((63 - (percentage6 * 0.46)) + voffset), percentage6, 0);
        }
        else if (percentage6 < 10) {
          TV.print(84, ((63 - (percentage6 * 0.46)) + voffset), percentage6, 0);
        }
        else {
          TV.print(80, ((63 - (percentage6 * 0.46)) + voffset), percentage6, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 7;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH7
    else if (ACT_channel == 7) {

      uint32_t rssi_value7 = _readRSSI();
      float percentage7 = map(rssi_value7, max, min, 0, 100);

      if (percentage7 > 100) {
        percentage7 = 100;
      }
      else if (percentage7 < 0) {
        percentage7 = 0;
      }
      if (display_setting == 0) {
        TV.draw_rect(96, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(96, ((70 - (percentage7 * 0.46)) + voffset), 10, (percentage7 * 0.46), WHITE, WHITE);
        TV.print(96, (75 + voffset), "CH7");
        if ((percentage7 < 100) && (percentage7 >= 10)) {
          TV.print(98, ((63 - (percentage7 * 0.46)) + voffset), percentage7, 0);
        }
        else if (percentage7 < 10) {
          TV.print(100, ((63 - (percentage7 * 0.46)) + voffset), percentage7, 0);
        }
        else {
          TV.print(96, ((63 - (percentage7 * 0.46)) + voffset), percentage7, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 8;
    }

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH8
    else if (ACT_channel == 8) {

      uint32_t rssi_value8 = _readRSSI();
      float percentage8 = map(rssi_value8, max, min, 0, 100);

      if (percentage8 > 100) {
        percentage8 = 100;
      }
      else if (percentage8 < 0) {
        percentage8 = 0;
      }
      if (display_setting == 0) {
        TV.draw_rect(112, (13 + voffset), 14, 67, BLACK, BLACK);
        TV.draw_rect(112, ((70 - (percentage8 * 0.46)) + voffset), 10, (percentage8 * 0.46), WHITE, WHITE);
        TV.print(112, (75 + voffset), "CH8");
        if ((percentage8 < 100) && (percentage8 >= 10)) {
          TV.print(114, ((63 - (percentage8 * 0.46)) + voffset), percentage8, 0);
        }
        else if (percentage8 < 10) {
          TV.print(116, ((63 - (percentage8 * 0.46)) + voffset), percentage8, 0);
        }
        else {
          TV.print(112, ((63 - (percentage8 * 0.46)) + voffset), percentage8, 0);
        }
      }
      //TV.print("%");
      ACT_channel = 1;
    }


    if (pressedbut == 1) {
      if (display_setting == 0) {
        TV.clear_screen();
      }
      menuactive = 0; //for debugging only
      exit = 1;
    }
  }
}


void runlocktimer() {
  buttoncheck();
  control();
  osd();

  if (lockmodeEEP == 1)  {
    if ((lockmode == 0) && (menuactive == 0)) {
      refresh++ ;

      if (refresh >= lock_timeout) {
        lockmode = 1;
        refresh = 0;
      }
      else {
        lockmode = 0;
      }
    }

    if (display_setting == 0) {
      if ((lockmodeEEP == 1) && (lockmode == 1)) {
        TV.bitmap(92, (48 + voffset), bitmap_minilock_OSD);
      }
      else if ((lockmodeEEP == 1) && (lockmode == 0)) {
        TV.bitmap(92, (48 + voffset), bitmap_minilockopen_OSD);
      }
    }
  }
}

//STORING STUFF FOR LATER
// u8g.drawBitmapP(5, 20, 7, 32, bitmap_dock);    //serial
// u8g.drawBitmapP(5, 20, 7, 32, bitmap_display);    //display
