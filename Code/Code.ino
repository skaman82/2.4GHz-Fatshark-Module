// This Project is created by Albert Kravcov

// TO DOS OR FIXMEs for 1.0:
// • Array for RSS values in calibration and Bandscan

// STUFF FOR LATER VERSIONS
// • Autosearch
// • Display setting (OLED + OSD)
// • Serial setting (OLED + OSD)
// • Dockking compatibility???
// • Reorder menu items


const char verId[7] = "v1.0"; //VERSION INFO

#include <TVout.h>
#include <font8x8.h>
#include <font6x8.h>
#include <font4x6.h>
#include "hardware.h"
#include "logo.h"

#include <Wire.h>
#include "U8glib.h"

#include <EEPROM.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_FAST);  // Dev 0, Fast I2C / TWI

#include "bitmaps.h"

#define longpresstime         1000  // in ms
#define chanADDR                 1  // EEPROM Adress
#define lockmodeADDR             2  // EEPROM Adress
#define fscontrollADDR           3  // EEPROM Adress
#define displayADDR              4  // EEPROM Adress
#define RSSImaxADDR              5  // EEPROM Adress
#define RSSIminADDR              6  // EEPROM Adress
#define serialADDR               7  // EEPROM Adress

int channelvalueEEP;
int lockmodeEEP;
int fscontrollEEP;
int displayEEP;     //TODO: display settings
int RSSImaxEEP;     // RSSI calibration
int RSSIminEEP;     // RSSI calibration
int serialEEP;     // TODO serial communication
int max; //= 365; //0% RSSI
int min; // = 338; //100% RSSI
int findermode = 0;
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

byte voffset = 10;
byte rssiupdate = 0;
int refresh_osd = 0;
int refresh = 0;
int osd_timeout;
int lock_timeout;
byte display_setting; //NEW setting for display modes. 0 = ONLY OSD | 1 = OLED+OSD | 2 = ONLY OLED (will require a reboot)

unsigned char x, y;
unsigned char originx = 5;
unsigned char originy = 70;
unsigned char plotx = originx;
unsigned char ploty = 40;



TVout TV;

void setup() {
  //digitalWrite(RST_pin, HIGH); // pull reset pin high

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
  //pinMode(RST_pin,      OUTPUT); // NEW reset

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
  RSSImaxEEP = EEPROM.read(RSSImaxADDR)  * 2;
  RSSIminEEP = EEPROM.read(RSSIminADDR)  * 2;
  serialEEP = EEPROM.read(serialADDR); //TODO

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

  if (displayEEP > 2)
  {
    display_setting = 1; //setting the  default state of no valid value
  }

  display_setting = 1; //just for testing 0 = ONLY OSD | 1 = OLED+OSD | 2 = ONLY OLED

//TODO>
  //if (fscontrollEEP == 0) { // change to "serialEEP == 1" later
    //#define serial
    //#undef FS_pin2
    //#undef FS_pin3
    //Serial.begin(9600);
  //}

  clearOLED();

  if (display_setting >= 1) {
    showlogo();

    if (display_setting == 2) {
      delay(2000);
    }
  }


  if (display_setting <= 1) {  //NEW
#ifdef PAL_FORMAT
    TV.begin(_PAL, 128, 96); //original 128x96px, 160x120max (146x120)
#endif
#ifdef NTSC_FORMAT
    TV.begin(_NTSC, 128, 90);
#endif
    TV.delay_frame(1);
    display.output_delay = 55; //move the whole screen vertically

    TV.clear_screen();
    TV.bitmap(5, (20 + voffset), logo);

    osd_timeout = 100; //looptime setting for osd timeout
    lock_timeout = 300; //looptime setting for lockmode timeout
  }
  else {
    osd_timeout = 0; //looptime setting for osd timeout
    lock_timeout = 600; //looptime setting for lockmode timeout
  }

  if (display_setting <= 1) {

    digitalWrite(OSD_ctr1, HIGH);
    digitalWrite(OSD_ctr2, LOW);
    digitalWrite(LED_pin, HIGH);

    TV.select_font(font6x8);
    TV.print(15, (50 + voffset), "2G4 MODULE ");
    TV.print(verId);
    TV.select_font(font4x6);
    TV.print(29, (69 + voffset), "#RUININGTHEHOBBY");
    TV.delay(3000);
    TV.tone(500, 200);
    TV.clear_screen();

    osd_mode = 1;
  }
  else {
    tone(BUZZ, 500, 200);
    osd_mode = 0;
  }

  clearOLED();

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
  u8g.firstPage();
  do
  {
    // splashscreen goes here
    u8g.drawBitmapP(10, 20, 14, 18, logo_OLED);
    u8g.setFont(u8g_font_5x7r);
    u8g.setPrintPos(31, 55);
    u8g.print("2G4 MODULE ");
    u8g.print(verId);
  }
  while (u8g.nextPage());
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
  //#ifndef serial
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
  //#endif
}


void control() {
  if (pressedbut == 1) {

    if (lockmode == 0) {
      if (display_setting <= 1) {
        TV.tone(800, 80);
      }
      else {
        tone(BUZZ, 800, 80);
      }
    }
    else {
      if (display_setting <= 1) {
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
        if (display_setting <= 1) {
          TV.clear_screen();
        }
        menuactive = 1;
        menu();

      }
      else if (menuactive == 1) {
        if (display_setting <= 1) {
          TV.fill(BLACK);
          TV.delay(10);
        }
        //return;
      }
    }


  }

  if (pressedbut == 2) {

    if (lockmode == 0) {
      if (display_setting <= 1) {
        TV.tone(800, 80);
      }
      else {
        tone(BUZZ, 800, 80);
      }
    }
    else {
      if (display_setting <= 1) {
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
        if (display_setting <= 1) {
          TV.clear_screen();
        }
        callOSD();
      }
    }
  }

  if (pressedbut == 3) {

    if (lockmode == 0) {
      if (display_setting <= 1) {
        TV.tone(800, 80);
      }
      else {
        tone(BUZZ, 800, 80);
      }
    }
    else {
      if (display_setting <= 1) {
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
        if (display_setting <= 1) {
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
      if (display_setting <= 1) {
        TV.tone(600, 400);
      }
      else {
        tone(BUZZ, 600, 400);
      }

      callOSD();
      return;
    }
    else {
      if (display_setting <= 1) {
        TV.clear_screen();
      }
      //reboot_modal();
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
    if (display_setting <= 1) {
      digitalWrite(OSD_ctr1, HIGH);
      digitalWrite(OSD_ctr2, LOW);
      if (findermode == 0) {
        digitalWrite(LED_pin, HIGH);
      }
    }
  }

  else if (osd_mode == 0) {
    digitalWrite(OSD_ctr1, LOW);
    digitalWrite(OSD_ctr2, HIGH);
    digitalWrite(LED_pin, LOW);
  }
  else {
    //osd_mode = 1;
  }
}



//MAIN LOOP ************************

void loop() {

  u8g.firstPage();
  do {
    buttoncheck();

    if (fscontrollEEP == 1) {
      if (lockmode == 0) {
        fs_buttons();
      }
    }

    control();
    rx_update();
    channeltable();
    runlocktimer();
    osd();


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

    if (display_setting <= 1) {
      //do OSD stuff
    }

#endif

#ifndef debug

    if (display_setting <= 1) {
      //Main OSD code

      TV.select_font(font8x8);
      TV.print(58, (20 + voffset), freq);
      TV.select_font(font6x8);
      TV.println(95, (22 + voffset), "MHz");

      if (ACT_channel == 1) {
        TV.bitmap(18, (20 + voffset), bitmap_one_OSD);
      }
      else if (ACT_channel == 2) {
        TV.bitmap(18, (20 + voffset), bitmap_two_OSD);
      }
      else if (ACT_channel == 3) {
        TV.bitmap(18, (20 + voffset), bitmap_three_OSD);
      }
      else if (ACT_channel == 4) {
        TV.bitmap(18, (20 + voffset), bitmap_four_OSD);
      }
      else if (ACT_channel == 5) {
        TV.bitmap(18, (20 + voffset), bitmap_five_OSD);
      }
      else if (ACT_channel == 6) {
        TV.bitmap(18, (20 + voffset), bitmap_six_OSD);
      }
      else if (ACT_channel == 7) {
        TV.bitmap(18, (20 + voffset), bitmap_seven_OSD);
      }
      else if (ACT_channel == 8) {
        TV.bitmap(18, (20 + voffset), bitmap_eight_OSD);
      }

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

    if (fscontrollEEP == 0) {
      Serial.println(percentage, 0);
      Serial.println(ACT_channel);
    }

    if (display_setting <= 1) {
      TV.select_font(font4x6);
      //TV.draw_rect(58, (41 + voffset), 30, 8, BLACK, BLACK); //mask
      TV.print(58, (41 + voffset), "RSSI:");
      TV.println((percentage - 0), 0);

      if ((percentage < 100) && (percentage >= 10)) {
        TV.draw_rect(86, (41 + voffset), 4, 4, BLACK, BLACK); //cover the first RSSI number
      }

      else if (percentage < 10) {
        TV.draw_rect(82, (41 + voffset), 8, 4, BLACK, BLACK); //cover the second RSSI number
      }

      TV.draw_rect(58, (53 + voffset), 52, 8, WHITE, BLACK);
      TV.draw_rect(60, (55 + voffset), (percentage * 0.48) , 4, WHITE, WHITE);
    }

#endif

#endif

#ifndef debug
    if (display_setting <= 1) {
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

    if (display_setting >= 1) {
      u8g.setFont(u8g_font_profont22r);
      u8g.setPrintPos(50, 26);
      u8g.print(freq);
      u8g.setFont(u8g_font_5x7r);
      u8g.print(" MHz");

#ifdef RSSI_mod
      u8g.setFont(u8g_font_5x7r);
      u8g.setPrintPos(50, 40);
      u8g.print("RSSI:");
      u8g.print(percentage, 0);

      //rssibar = 100;

      u8g.drawBox(53, 48, (percentage * 0.60), 3); //
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
    }
  } while ( u8g.nextPage() );

}



//RSSI READOUT ************************ simple avg of 4 value

uint16_t _readRSSI() {

  rssiupdate++;
  volatile uint32_t sum = 0;
  sum = analogRead(RSSI_pin);
  //delay(2);
  sum += analogRead(RSSI_pin);
  //delay(2);
  sum += analogRead(RSSI_pin);
  //delay(2);
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

        if (display_setting <= 1) {
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

        u8g.drawBitmapP(38, 10, 7, 32, bitmap_nkizw);   //goggle
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(23, 55);
        u8g.print("GOGGLE CONTROL:");
        if (fscontrollEEP == 1) {
          u8g.print("ON");
        }
        else {
          u8g.print("OFF");
        }

        if (pressedbut == 1) {

          if (fscontrollEEP == 1) {
            fscontrollEEP = 0;
            EEPROM.write(fscontrollADDR, fscontrollEEP);
          }
          else {
            fscontrollEEP = 1;
            EEPROM.write(fscontrollADDR, fscontrollEEP);
          }

          if (display_setting <= 1) {
            TV.clear_screen();
          }
        }
        if (pressedbut == 2) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 2;
        }
      }

      else if (menu_position == 2) {

        buttoncheck();

        if (display_setting <= 1) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_hd85pj_OSD); //scan
          TV.print(38, (70 + voffset), "Bandscan");
        }

        u8g.drawBitmapP(38, 10, 7, 32, bitmap_hd85pj);   //scan
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(45, 55);
        u8g.print("BANDSCAN");

        if (pressedbut == 1) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          bandscan();
        }

        if (pressedbut == 2) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 3;
        }
        if (pressedbut == 3) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 1;
        }
      }

      else if (menu_position == 3) {

        buttoncheck();
        if (display_setting <= 1) {
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

        if (pressedbut == 1) {

          if (lockmodeEEP == 1) {
            lockmodeEEP = 0;
            EEPROM.write(lockmodeADDR, lockmodeEEP);
          }
          else {
            lockmodeEEP = 1;
            EEPROM.write(lockmodeADDR, lockmodeEEP);
          }

          if (display_setting <= 1) {
            TV.clear_screen();
          }
        }

        if (pressedbut == 2) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 4;
        }
        if (pressedbut == 3) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 2;
        }
      }

      else if (menu_position == 4) {

        buttoncheck();

        if (display_setting <= 1) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_ydywrn_OSD); //search
          TV.print(35, (70 + voffset), "Find mode");
        }

        u8g.drawBitmapP(38, 10, 7, 32, bitmap_ydywrn);   //search
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(45, 55);
        u8g.print("FIND MODE");


        if (pressedbut == 1) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          finder();
        }

        if (pressedbut == 2) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 5;
        }
        if (pressedbut == 3) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 3;
        }
      }

      else if (menu_position == 5) {

        buttoncheck();

        if (display_setting <= 1) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_949gqh_OSD); //tools
          TV.print(37, (70 + voffset), "Calibrate");
        }

        u8g.drawBitmapP(38, 10, 7, 32, bitmap_calib);   //tools
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(45, 55);
        u8g.print("CALIBRATE");

        if (pressedbut == 1) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          calibration();
        }

        if (pressedbut == 2) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 6;
        }
        if (pressedbut == 3) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 4;
        }
      }

      else if (menu_position == 6) {

        buttoncheck();

        if (display_setting <= 1) {
          TV.select_font(font4x6);
          TV.print(55, (0 + voffset), "MENU");
          TV.select_font(font6x8);
          TV.bitmap(35, (25 + voffset), bitmap_be8bbq_OSD); //exit
          TV.print(50, (70 + voffset), "EXIT");
        }

        u8g.drawBitmapP(38, 10, 7, 32, bitmap_be8bbq);   //exit
        u8g.setFont(u8g_font_5x7r);
        u8g.setPrintPos(55, 55);
        u8g.print("EXIT");


        if (pressedbut == 1) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 0;
          menuactive = 0;
          osd_mode = 0;
          exit = 1;
          return;
        }

        if (pressedbut == 3) {
          if (display_setting <= 1) {
            TV.clear_screen();
          }
          menu_position = 5;
        }
      }


    } while ( u8g.nextPage() );

  }
}



//CALIBRATION MENU ************************

void calibration() {     // Calibration wizzard
  clearOLED();
  byte exit = 0;
  int calstep = 1;

  while (exit == 0) {

    u8g.firstPage();
    do {
      if (calstep == 1) {
        buttoncheck();
        channeltable();
        //uint32_t rssi_value = _readRSSI();
        if (display_setting <= 1) {

          TV.select_font(font4x6);
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(14, (20 + voffset), "1:Select channel");

          TV.print(21, (40 + voffset), "< ");
          TV.print(ACT_channel);
          TV.print(":");
          TV.print(freq);
          TV.print(" MHz");
          TV.print(" >");

          TV.draw_rect(38, (65 + voffset), 48, 16, WHITE);
          TV.select_font(font4x6);
          TV.print(51, (71 + voffset), "NEXT >");

        }

        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(27, 26);
          u8g.print("1:Select channel");
          u8g.setPrintPos(33, 40);
          u8g.print("< ");

          u8g.print(ACT_channel);
          u8g.print(":");
          u8g.print(freq);

          u8g.print(" MHz");
          u8g.print(" >");

          u8g.setPrintPos(55, 55);
          u8g.print("NEXT >");
          u8g.drawFrame(49, 46, 40, 12);
        }


        if (ACT_channel <= 7) {
          if (pressedbut == 3) {
            ACT_channel += 1;
          }
        }
        if (ACT_channel >= 2) {

          if (pressedbut == 2) {
            ACT_channel -= 1;
          }
        }

        if (pressedbut == 1) {

          if (display_setting <= 1) {
            TV.clear_screen();
          }
          calstep = 2;
        }
      }

      else if (calstep == 2) {
        buttoncheck();
        uint32_t rssi_value = _readRSSI();

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(0, (20 + voffset), "2:Remove antenna and switch off the VTX");
          TV.draw_rect(38, (65 + voffset), 48, 16, WHITE);
          TV.select_font(font4x6);
          TV.print(51, (71 + voffset), "NEXT >");
        }


        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(20, 26);
          u8g.print("2:Remove antenna and");
          u8g.setPrintPos(25, 36);
          u8g.print("switch off the VTX");
          u8g.setPrintPos(55, 55);
          u8g.print("NEXT >");
          u8g.drawFrame(49, 46, 40, 12);
        }

        if (pressedbut == 1) {

          RSSImaxEEP = rssi_value / 2;
          EEPROM.write(RSSImaxADDR, RSSImaxEEP);

          if (display_setting <= 1) {
            TV.clear_screen();
          }
          calstep = 3;
        }
      }
      else if (calstep == 3) {
        buttoncheck();
        uint32_t rssi_value = _readRSSI();

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(0, (20 + voffset), "3:Put on the antenna and switch on the VTX");
          TV.draw_rect(38, (65 + voffset), 48, 16, WHITE);
          TV.select_font(font4x6);
          TV.print(51, (71 + voffset), "NEXT >");
        }

        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(18, 26);
          u8g.print("3:Put on the antenna");
          u8g.setPrintPos(16, 36);
          u8g.print("and switch on the VTX");
          u8g.setPrintPos(55, 55);
          u8g.print("NEXT >");
          u8g.drawFrame(49, 46, 40, 12);
        }

        if (pressedbut == 1) {

          RSSIminEEP = rssi_value / 2;
          EEPROM.write(RSSIminADDR, RSSIminEEP);

          if (display_setting <= 1) {
            TV.clear_screen();
          }
          calstep = 4;
        }
      }
      else if (calstep == 4) {
        buttoncheck();

        RSSIminEEP = EEPROM.read(RSSIminADDR) * 2;
        RSSImaxEEP = EEPROM.read(RSSImaxADDR) * 2;

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(8, (20 + voffset), "4:Calibration done!");
          TV.print(30, (40 + voffset), "Saved MIN:");
          TV.print(RSSIminEEP);
          TV.print(30, (50 + voffset), "Saved MAX:");
          TV.print(RSSImaxEEP);
          TV.draw_rect(38, (65 + voffset), 48, 16, WHITE);
          TV.select_font(font4x6);
          TV.print(51, (71 + voffset), "EXIT >");
        }

        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(20, 26);
          u8g.print("4:Calibration done!");

          u8g.setPrintPos(30, 38);
          u8g.print("MIN:");
          u8g.print(RSSIminEEP);
          u8g.print(" MAX:");
          u8g.print(RSSImaxEEP);

          u8g.setPrintPos(55, 55);
          u8g.print("EXIT >");
          u8g.drawFrame(49, 46, 40, 12);
        }


        if (pressedbut == 1) {

          if (display_setting <= 1) {
            TV.clear_screen();
          }

          max = RSSImaxEEP;
          min = RSSIminEEP;
          calstep = 0;

          exit = 1;
        }
      }
    } while ( u8g.nextPage() );
  }

}

//BANDSCAN MENU ************************

void bandscan() {

  clearOLED();

  int percentage1 = 0;
  int percentage2 = 0;
  int percentage3 = 0;
  int percentage4 = 0;
  int percentage5 = 0;
  int percentage6 = 0;
  int percentage7 = 0;
  int percentage8 = 0;


  byte exit = 0;
  while (exit == 0) {

    u8g.firstPage();
    do {

      menuactive = 1; //for debugging only
      buttoncheck();
      channeltable();

      if (display_setting <= 1) {
        TV.select_font(font4x6);
        TV.print(35, (0 + voffset), "BAND SCANNER");
        TV.draw_line(0, (70 + voffset), 123, (70 + voffset), WHITE);
      }



      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH1
      if (ACT_channel == 1) {

        uint32_t rssi_value1 = _readRSSI();
        percentage1 = map(rssi_value1, max, min, 0, 100);
        if (percentage1 > 100) {
          percentage1 = 100;
        }
        else if (percentage1 < 0) {
          percentage1 = 0;
        }
        if (display_setting <= 1) {
          TV.draw_rect(0, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(0, ((70 - (percentage1 * 0.46)) + voffset), 10, (percentage1 * 0.46), WHITE, WHITE);
          TV.print(0, (75 + voffset), "CH1");
          if ((percentage1 < 100) && (percentage1 >= 10)) {
            TV.print(2, ((63 - (percentage1 * 0.46)) + voffset), percentage1);
          }
          else if (percentage1 < 10) {
            TV.print(4, ((63 - (percentage1 * 0.46)) + voffset), percentage1);
          }
          else {
            TV.print(0, ((63 - (percentage1 * 0.46)) + voffset), percentage1);
          }
        }

        ACT_channel = 2;
      }

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH2
      else if (ACT_channel == 2) {

        uint32_t rssi_value2 = _readRSSI();
        percentage2 = map(rssi_value2, max, min, 0, 100);
        if (percentage2 > 100) {
          percentage2 = 100;
        }
        else if (percentage2 < 0) {
          percentage2 = 0;
        }
        if (display_setting <= 1) {
          TV.draw_rect(16, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(16, ((70 - (percentage2 * 0.46)) + voffset), 10, (percentage2 * 0.46), WHITE, WHITE);
          TV.print(16, (75 + voffset), "CH2");
          if ((percentage2 < 100) && (percentage2 >= 10)) {
            TV.print(18, ((63 - (percentage2 * 0.46)) + voffset), percentage2);
          }
          else if (percentage2 < 10) {
            TV.print(20, ((63 - (percentage2 * 0.46)) + voffset), percentage2);
          }
          else {
            TV.print(16, ((63 - (percentage2 * 0.46)) + voffset), percentage2);
          }
        }
        ACT_channel = 3;
      }

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH3
      else if (ACT_channel == 3) {

        uint32_t rssi_value3 = _readRSSI();
        percentage3 = map(rssi_value3, max, min, 0, 100);
        if (percentage3 > 100) {
          percentage3 = 100;
        }
        else if (percentage3 < 0) {
          percentage3 = 0;
        }

        if (display_setting <= 1) {
          TV.draw_rect(32, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(32, ((70 - (percentage3 * 0.46)) + voffset), 10, (percentage3 * 0.46), WHITE, WHITE);
          TV.print(32, (75 + voffset), "CH3");
          if ((percentage3 < 100) && (percentage3 >= 10)) {
            TV.print(34, ((63 - (percentage3 * 0.46)) + voffset), percentage3);
          }
          else if (percentage3 < 10) {
            TV.print(36, ((63 - (percentage3 * 0.46)) + voffset), percentage3);
          }
          else {
            TV.print(32, ((63 - (percentage3 * 0.46)) + voffset), percentage3);
          }
        }
        ACT_channel = 4;
      }

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH4
      else if (ACT_channel == 4) {

        uint32_t rssi_value4 = _readRSSI();
        percentage4 = map(rssi_value4, max, min, 0, 100);
        if (percentage4 > 100) {
          percentage4 = 100;
        }
        else if (percentage4 < 0) {
          percentage4 = 0;
        }
        if (display_setting <= 1) {
          TV.draw_rect(48, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(48, ((70 - (percentage4 * 0.46)) + voffset), 10, (percentage4 * 0.46), WHITE, WHITE);
          TV.print(48, (75 + voffset), "CH4");
          if ((percentage4 < 100) && (percentage4 >= 10)) {
            TV.print(50, ((63 - (percentage4 * 0.46)) + voffset), percentage4);
          }
          else if (percentage4 < 10) {
            TV.print(52, ((63 - (percentage4 * 0.46)) + voffset), percentage4);
          }
          else {
            TV.print(48, ((63 - (percentage4 * 0.46)) + voffset), percentage4);
          }
        }
        ACT_channel = 5;
      }

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH5
      else if (ACT_channel == 5) {

        uint32_t rssi_value5 = _readRSSI();
        percentage5 = map(rssi_value5, max, min, 0, 100);
        if (percentage5 > 100) {
          percentage5 = 100;
        }
        else if (percentage5 < 0) {
          percentage5 = 0;
        }
        if (display_setting <= 1) {
          TV.draw_rect(64, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(64, ((70 - (percentage5 * 0.46)) + voffset), 10, (percentage5 * 0.46), WHITE, WHITE);
          TV.print(64, (75 + voffset), "CH5");
          if ((percentage5 < 100) && (percentage5 >= 10)) {
            TV.print(66, ((63 - (percentage5 * 0.46)) + voffset), percentage5);
          }
          else if (percentage5 < 10) {
            TV.print(68, ((63 - (percentage5 * 0.46)) + voffset), percentage5);
          }
          else {
            TV.print(64, ((63 - (percentage5 * 0.46)) + voffset), percentage5);
          }
        }
        ACT_channel = 6;
      }

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH6
      else if (ACT_channel == 6) {

        uint32_t rssi_value6 = _readRSSI();
        percentage6 = map(rssi_value6, max, min, 0, 100);

        if (percentage6 > 100) {
          percentage6 = 100;
        }
        else if (percentage6 < 0) {
          percentage6 = 0;
        }

        if (display_setting <= 1) {
          TV.draw_rect(80, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(80, ((70 - (percentage6 * 0.46)) + voffset), 10, (percentage6 * 0.46), WHITE, WHITE);
          TV.print(80, (75 + voffset), "CH6");
          if ((percentage6 < 100) && (percentage6 >= 10)) {
            TV.print(82, ((63 - (percentage6 * 0.46)) + voffset), percentage6);
          }
          else if (percentage6 < 10) {
            TV.print(84, ((63 - (percentage6 * 0.46)) + voffset), percentage6);
          }
          else {
            TV.print(80, ((63 - (percentage6 * 0.46)) + voffset), percentage6);
          }
        }
        ACT_channel = 7;
      }

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH7
      else if (ACT_channel == 7) {

        uint32_t rssi_value7 = _readRSSI();
        percentage7 = map(rssi_value7, max, min, 0, 100);

        if (percentage7 > 100) {
          percentage7 = 100;
        }
        else if (percentage7 < 0) {
          percentage7 = 0;
        }
        if (display_setting <= 1) {
          TV.draw_rect(96, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(96, ((70 - (percentage7 * 0.46)) + voffset), 10, (percentage7 * 0.46), WHITE, WHITE);
          TV.print(96, (75 + voffset), "CH7");
          if ((percentage7 < 100) && (percentage7 >= 10)) {
            TV.print(98, ((63 - (percentage7 * 0.46)) + voffset), percentage7);
          }
          else if (percentage7 < 10) {
            TV.print(100, ((63 - (percentage7 * 0.46)) + voffset), percentage7);
          }
          else {
            TV.print(96, ((63 - (percentage7 * 0.46)) + voffset), percentage7);
          }
        }
        ACT_channel = 8;
      }

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CH8
      else if (ACT_channel == 8) {

        uint32_t rssi_value8 = _readRSSI();
        percentage8 = map(rssi_value8, max, min, 0, 100);

        if (percentage8 > 100) {
          percentage8 = 100;
        }
        else if (percentage8 < 0) {
          percentage8 = 0;
        }
        if (display_setting <= 1) {
          TV.draw_rect(112, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(112, ((70 - (percentage8 * 0.46)) + voffset), 10, (percentage8 * 0.46), WHITE, WHITE);
          TV.print(112, (75 + voffset), "CH8");
          if ((percentage8 < 100) && (percentage8 >= 10)) {
            TV.print(114, ((63 - (percentage8 * 0.46)) + voffset), percentage8);
          }
          else if (percentage8 < 10) {
            TV.print(116, ((63 - (percentage8 * 0.46)) + voffset), percentage8);
          }
          else {
            TV.print(112, ((63 - (percentage8 * 0.46)) + voffset), percentage8);
          }
        }
        ACT_channel = 1;
      }


      if (display_setting >= 1) {
        u8g.setPrintPos(35, 10);
        u8g.print("BAND SCANNER");
        u8g.drawHLine(15, 50, 100);


        int rssibar_ch1 = round(percentage1 * 0.3);
        int rssibar_ch2 = round(percentage2 * 0.3);
        int rssibar_ch3 = round(percentage3 * 0.3);
        int rssibar_ch4 = round(percentage4 * 0.3);
        int rssibar_ch5 = round(percentage5 * 0.3);
        int rssibar_ch6 = round(percentage6 * 0.3);
        int rssibar_ch7 = round(percentage7 * 0.3);
        int rssibar_ch8 = round(percentage8 * 0.3);


        u8g.setPrintPos(15, 59);
        u8g.print("C1");
        u8g.setPrintPos(28, 59);
        u8g.print("C2");
        u8g.setPrintPos(41, 59);
        u8g.print("C3");
        u8g.setPrintPos(54, 59);
        u8g.print("C4");
        u8g.setPrintPos(67, 59);
        u8g.print("C5");
        u8g.setPrintPos(80, 59);
        u8g.print("C6");
        u8g.setPrintPos(93, 59);
        u8g.print("C7");
        u8g.setPrintPos(106, 59);
        u8g.print("C8");


        if (percentage1 > 1) {
          u8g.drawBox(15, (51 - (rssibar_ch1)), 9, rssibar_ch1);
        }
        if (percentage1 <= 9) {
          u8g.setPrintPos(17, (48 - rssibar_ch1));
        }
        else {
          u8g.setPrintPos(15, (48 - rssibar_ch1));
        }
        u8g.print(percentage1);


        if (percentage2 > 1) {
          u8g.drawBox(28, (51 - (rssibar_ch2)), 9, rssibar_ch2);
        }
        if (percentage2 <= 9) {
          u8g.setPrintPos(30, (48 - rssibar_ch2));
        }
        else {
          u8g.setPrintPos(28, (48 - rssibar_ch2));
        }
        u8g.print(percentage2);


        if (percentage3 > 1) {
          u8g.drawBox(41, (51 - (rssibar_ch3)), 9, rssibar_ch3);
        }
        if (percentage3 <= 9) {
          u8g.setPrintPos(43, (48 - rssibar_ch3));
        }
        else {
          u8g.setPrintPos(41, (48 - rssibar_ch3));
        }
        u8g.print(percentage3);


        if (percentage4 > 1) {
          u8g.drawBox(54, (51 - (rssibar_ch4)), 9, rssibar_ch4);
        }
        if (percentage4 <= 9) {
          u8g.setPrintPos(56, (48 - rssibar_ch4));
        }
        else {
          u8g.setPrintPos(54, (48 - rssibar_ch4));
        }
        u8g.print(percentage4);


        if (percentage5 > 1) {
          u8g.drawBox(67, (51 - rssibar_ch5), 9, rssibar_ch5);
        }
        if (percentage5 <= 9) {
          u8g.setPrintPos(69, (48 - rssibar_ch5));
        }
        else {
          u8g.setPrintPos(67, (48 - rssibar_ch5));
        }
        u8g.print(percentage5);


        if (percentage6 > 1) {
          u8g.drawBox(80, (51 - (rssibar_ch6)), 9, rssibar_ch6);
        }
        if (percentage6 <= 9) {
          u8g.setPrintPos(82, (48 - rssibar_ch6));
        }
        else {
          u8g.setPrintPos(80, (48 - rssibar_ch6));
        }
        u8g.print(percentage6);


        if (percentage7 > 1) {
          u8g.drawBox(93, (51 - (rssibar_ch7)), 9, rssibar_ch7);
        }
        if (percentage7 <= 9) {
          u8g.setPrintPos(95, (48 - rssibar_ch7));
        }
        else {
          u8g.setPrintPos(93, (48 - rssibar_ch7));
        }
        u8g.print(percentage7);


        if (percentage8 > 1) {
          u8g.drawBox(106, (51 - (rssibar_ch8)), 9, rssibar_ch8);
        }
        if (percentage7 <= 9) {
          u8g.setPrintPos(108, (48 - rssibar_ch8));
        }
        else {
          u8g.setPrintPos(106, (48 - rssibar_ch8));
        }
        u8g.print(percentage8);
      }


      if (pressedbut == 1) {
        if (display_setting <= 1) {
          TV.clear_screen();
        }
        menuactive = 0; //for debugging only
        exit = 1;
        return;
      }

    } while ( u8g.nextPage() );

  }
}



//BUTTON LOCKING FUNCTION ************************

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
        if (display_setting <= 1) {
          TV.tone(600, 50);
        }
        else {
          tone(BUZZ, 600, 50);
        }
      }
      else {
        lockmode = 0;
      }
    }

    if (display_setting <= 1) {
      if ((lockmodeEEP == 1) && (lockmode == 1)) {
        TV.bitmap(104, (38 + voffset), bitmap_minilock_OSD);
      }
      else if ((lockmodeEEP == 1) && (lockmode == 0)) {
        TV.bitmap(104, (38 + voffset), bitmap_minilockopen_OSD);
      }
    }
  }
}




//FINDER MODE ************************

void finder() {
  u8g.setRot270();
  byte exit = 0;
  clearOLED();
  int buzzertimer = 0;
  findermode = 1;


  while (exit == 0) {
    u8g.firstPage();

    do {
      osd();
      buttoncheck();
      channeltable();
      digitalWrite(LED_pin, LOW);
      menuactive = 1;
      osd_mode = 1;

      int delaytime;

      uint32_t rssi_value = _readRSSI();
      int percentage = map(rssi_value, max, min, 0, 100);

      if (percentage > 100) {
        percentage = 100;
      }
      if (percentage < 0) {
        percentage = 0;
      }


      if (display_setting <= 1) {
        delaytime = ((102 - percentage) * 0.6);
      }
      else {
        delaytime = (101 - percentage);
      }


      if (display_setting <= 1) {
        TV.select_font(font4x6);
        TV.print(45, (0 + voffset), "FIND MODE");
        TV.select_font(font6x8);

        drawGraph();


        int newploty = 70 - (percentage * 0.5);
        TV.draw_line(plotx - 1, ploty, plotx, newploty, 1);
        ploty = newploty;

        if (plotx++ > 120) {
          TV.fill(0);
          plotx = originx + 1;
        }

        TV.print(3, 85, "RSSI:");
        TV.draw_rect(29, 84, 20,8, BLACK, BLACK);
        TV.print(percentage);
        TV.print(55, 85, "CH");
        TV.print(ACT_channel);
        TV.print(" ");
        TV.print(freq);
        TV.print("MHz");
      }


      if (display_setting >= 1) {
        u8g.setPrintPos(5, 20);
        u8g.print("CH");
        u8g.print(ACT_channel);
        u8g.print(" ");
        u8g.print(freq);
        u8g.print("MHz");

        u8g.drawFrame(12, 32, 40, 88);

        if (percentage > 1) {
          u8g.drawBox(16, (116 - (percentage * 0.8)), 32, (percentage * 0.8));
        }
        u8g.drawFrame(16, 115, 32, 1);

        if (percentage < 100 ) {
          u8g.setPrintPos(27, 100);
        }
        else {
          u8g.setPrintPos(24, 100);
        }

        if (percentage > 26 ) {
          u8g.setColorIndex(0);
          u8g.print(percentage);
          u8g.setColorIndex(1);
        }
        else {
          u8g.print(percentage);
        }
      }


      buzzertimer++;
      if (buzzertimer > delaytime) {
        if (display_setting <= 1) {
          TV.tone(800, 100);

        }
        else {
          tone(BUZZ, 800, 100);

        }
        digitalWrite(LED_pin, HIGH);
        buzzertimer = 0;
      }


      if (pressedbut == 1) {
        menuactive = 0; //for debugging only
        if (display_setting <= 1) {
          TV.clear_screen();
        }
        fixoled();
        findermode = 0;
        exit = 1;
        return;
      }

      if (pressedbut == 3) {
        if (ACT_channel <= 7) {
          ACT_channel += 1;
        }
        else {
          ACT_channel = 1;
        }
      }

      if (pressedbut == 2) {
        if (ACT_channel >= 2) {
          ACT_channel -= 1;
        }
        else {
          ACT_channel = 8;
        }
      }

    } while ( u8g.nextPage() );
  }
}




//REBOOT MODAL ************************

void reboot_modal() {
  byte exit = 0;
  clearOLED();

  while (exit == 0) {

    u8g.firstPage();
    do {
      osd();
      buttoncheck();
      menuactive = 1;
      osd_mode = 1;

      if (display_setting <= 1) {
        TV.print(10, (10 + voffset), "reboot needed");
      }
      if (display_setting >= 1) {
        u8g.setPrintPos(10, 10);
        u8g.print("reboot needed");
      }

      if (pressedbut == 1) {

        //restart
        //digitalWrite(RST_pin, LOW); // pull reset pin to low for reset

        //following code doesn't happen if the reset is done:
        menuactive = 0; //for debugging only
        if (display_setting <= 1) {
          TV.clear_screen();
        }
        exit = 1;
        return;
      }

    } while ( u8g.nextPage() );
  }
}




//RESTORE OLED ORIENTATION ************************

void fixoled() {
  u8g.undoRotation();
  u8g.firstPage();

  do {
  } while ( u8g.nextPage() );
}

void drawGraph() {
  TV.draw_line(originx, 20, originx, originy, 1);
  TV.draw_line(originx, originy, 120, originy, 1);

  for (byte y = originy; y > 20; y -= 4) {
    TV.set_pixel(originx - 1, y, 1);
    TV.set_pixel(originx - 2, y, 1);
  }

  for (byte x = originx; x < 120; x += 4) {
    TV.set_pixel(x, originy + 1, 1);
    TV.set_pixel(x, originy + 2, 1);
  }
}






//STORING STUFF FOR LATER
// u8g.drawBitmapP(5, 20, 7, 32, bitmap_dock);    //serial
// u8g.drawBitmapP(5, 20, 7, 32, bitmap_display);    //display
