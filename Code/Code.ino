// This Project is created by Albert Kravcov
//
//
//
//
//
//
//
//

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

const unsigned char *chan_bitmaps[8] = { bitmap_one, bitmap_two, bitmap_three, bitmap_four, bitmap_five, bitmap_six, bitmap_seven, bitmap_eight };
const unsigned char *chan_bitmaps_osd[8] = { bitmap_one_OSD, bitmap_two_OSD, bitmap_three_OSD, bitmap_four_OSD, bitmap_five_OSD, bitmap_six_OSD, bitmap_seven_OSD, bitmap_eight_OSD };

typedef struct {
  int freq;
  int bx;
  int ch1;
  int ch2;
  int ch3;
} channel_t;

channel_t rx28_chan_table[8] = {
  { 2414, LOW, LOW, HIGH, HIGH },
  { 2432, LOW, HIGH, LOW, HIGH },
  { 2450, LOW, HIGH, HIGH, LOW },
  { 2468, LOW, HIGH, HIGH, HIGH },
  { 2490, HIGH, LOW, HIGH, HIGH },
  { 2510, HIGH, HIGH, LOW, HIGH },
  { 2390, HIGH, HIGH, HIGH, LOW },
  { 2370, HIGH, HIGH, HIGH, HIGH }
};

channel_t sm186R_chan_table[8] = {
  { 2414, LOW, LOW, LOW, 0 },
  { 2432, LOW, HIGH, LOW, 0 },
  { 2450, LOW, LOW, HIGH, 0 },
  { 2468, LOW, HIGH, HIGH, 0 },
  { 2490, HIGH, LOW, LOW, 0 },
  { 2510, HIGH, HIGH, LOW, 0 },
  { 2390, HIGH, LOW, HIGH, 0 },
  { 2370, HIGH, HIGH, HIGH, 0 }
};



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

  //TODO>
  display_setting = 1; //just for testing 0 = ONLY OSD | 1 = OLED+OSD | 2 = ONLY OLED

  //TODO>
  if (fscontrollEEP == 0) { // change to "serialEEP == 1" later
    Serial.begin(9600);
  }

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
  // FS Button mapping:
  //CH1: p1-LOW,  p2-LOW,   p3-LOW
  //CH2: p1-HIGH, p2-LOW,   p3-LOW
  //CH3: p1-LOW,  p2-HIGH,  p3-LOW
  //CH4: p1-HIGH, p2-HIGH,  p3-LOW
  //CH5: p1-LOW,  p2-LOW,   p3-HIGH
  //CH6: p1-HIGH, p2-LOW,   p3-HIGH
  //CH7: p1-LOW,  p2-HIGH,  p3-HIGH
  //CH8: p1-HIGH, p2-HIGH,  p3-HIGH

  int chan = 0;

  if (digitalRead(FS_pin1) == HIGH)
    chan = 1;

  if (digitalRead(FS_pin2) == HIGH)
    chan |= 1 << 1;

  if (digitalRead(FS_pin3) == HIGH)
    chan |= 1 << 2;

  FS_channel = chan + 1;
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
      autosearch();
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
  if (ACT_channel < 1 || ACT_channel > 8)
    ACT_channel = 1;

#ifdef RX28 //RX28 receiver channel settings
  channel_t ch = rx28_chan_table[ACT_channel - 1];
#elif SM186R //SM186R receiver channel settings
  channel_t ch = sm186R_chan_table[ACT_channel - 1];
#endif

  freq = ch.freq;
  digitalWrite(BX_pin, ch.bx); //BX
  digitalWrite(CH1_pin, ch.ch1); //1
  digitalWrite(CH2_pin, ch.ch2); //2

#ifdef RX28
  digitalWrite(CH3_pin, ch.ch3); //3
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

      if (ACT_channel >= 1 && ACT_channel <= 8)
        TV.bitmap(18, (20 + voffset), chan_bitmaps_osd[ACT_channel - 1]);
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
      //Serial.println(percentage, 0);
      //Serial.println(ACT_channel);
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

      if (ACT_channel >= 1 && ACT_channel <= 8)
        u8g.drawBitmapP(12, 12, 4, 41, chan_bitmaps[ACT_channel - 1]);

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


  ACT_channel = 1;

  double values[8];
  int maxIndex = 0;
  int minIndex = 0;

  byte rssireadin = 0;

  double rssi_value1 = 0;
  double rssi_value2 = 0;
  double rssi_value3 = 0;
  double rssi_value4 = 0;
  double rssi_value5 = 0;
  double rssi_value6 = 0;
  double rssi_value7 = 0;
  double rssi_value8 = 0;


  while (exit == 0) {

    u8g.firstPage();
    do {
      buttoncheck();
      channeltable();

      double rssi_max;
      double rssi_min;

      if (calstep == 1) {

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(0, (20 + voffset), "1:Remove antenna and switch off the VTX");
          TV.draw_rect(38, (65 + voffset), 48, 16, WHITE);
          TV.select_font(font4x6);
          TV.print(51, (71 + voffset), "NEXT >");
        }

        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(20, 26);
          u8g.print("1:Remove antenna and");
          u8g.setPrintPos(25, 36);
          u8g.print("switch off the VTX");
          u8g.setPrintPos(55, 55);
          u8g.print("NEXT >");
          u8g.drawFrame(49, 46, 40, 12);
        }

        if (pressedbut == 1) {
          calstep = 2;
        }
      }

      else if (calstep == 2) {

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(0, (20 + voffset), "calibrating");
        }

        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(20, 26);
          u8g.print("calibrating");
        }


        //Fill up the array with RSSi values for each channel - repeated 10 times for accuracy

        if (rssireadin < 10) {

          if (ACT_channel == 1) {
            rssi_value1 = _readRSSI();
            values[0] = rssi_value1;
            Serial.print("1: "); Serial.println(values[0]);
            ACT_channel = 2;
          }

          else if (ACT_channel == 2) {
            rssi_value2 = _readRSSI();
            values[1] = rssi_value2;
            Serial.print("2: "); Serial.println(values[1]);
            ACT_channel = 3;
          }

          else if (ACT_channel == 3) {
            rssi_value3 = _readRSSI();
            values[2] = rssi_value3;
            Serial.print("3: "); Serial.println(values[2]);
            ACT_channel = 4;
          }

          else if (ACT_channel == 4) {
            rssi_value4 = _readRSSI();
            values[3] = rssi_value4;
            Serial.print("4: "); Serial.println(values[3]);
            ACT_channel = 5;
          }

          else if (ACT_channel == 5) {
            rssi_value5 = _readRSSI();
            values[4] = rssi_value5;
            Serial.print("5: "); Serial.println(values[4]);
            ACT_channel = 6;
          }

          else if (ACT_channel == 6) {

            rssi_value6 = _readRSSI();
            values[5] = rssi_value6;
            Serial.print("6: "); Serial.println(values[5]);
            ACT_channel = 7;
          }

          else if (ACT_channel == 7) {
            rssi_value7 = _readRSSI();
            values[6] = rssi_value7;
            Serial.print("7: "); Serial.println(values[6]);
            ACT_channel = 8;
          }

          else if (ACT_channel == 8) {
            rssi_value8 = _readRSSI();
            values[7] = rssi_value8;
            Serial.print("8: "); Serial.println(values[7]);

            rssireadin = rssireadin + 1;
            ACT_channel = 1;
          }
        }

        //Getting the max/min values and the position in the array

        if (rssireadin == 10) {

          rssi_max = values[0];

          for (int i = 1; i < 8; i++) {
            if (values[i] > rssi_max) {
              maxIndex = i;
              rssi_max = values[i]; //determine max value in the array
            }
          }

          rssireadin = 11;

        }

        //print out the results
        if (rssireadin >= 11) {

          Serial.print("rssi_max: ");
          Serial.println(rssi_max);

          rssireadin = rssireadin + 1;
        }


        if (rssireadin == 20) {
          RSSImaxEEP = rssi_max / 2;

          EEPROM.write(RSSImaxADDR, RSSImaxEEP);
          Serial.print("rssi_max saved! ");

          if (display_setting <= 1) {
            TV.clear_screen();
          }
          calstep = 3;
        }
      }

      else if (calstep == 3) {

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(0, (20 + voffset), "2:Put on the antenna and switch on the VTX");
          TV.draw_rect(38, (65 + voffset), 48, 16, WHITE);
          TV.select_font(font4x6);
          TV.print(51, (71 + voffset), "NEXT >");
        }

        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(18, 26);
          u8g.print("2:Put on the antenna");
          u8g.setPrintPos(16, 36);
          u8g.print("and switch on the VTX");
          u8g.setPrintPos(55, 55);
          u8g.print("NEXT >");
          u8g.drawFrame(49, 46, 40, 12);
        }

        if (pressedbut == 1) {

          if (display_setting <= 1) {
            TV.clear_screen();
          }
          rssireadin = 1;

          calstep = 4;

          Serial.print("readin: "); Serial.println(rssireadin);
          Serial.print("step: "); Serial.println(calstep);
        }
      }



      else if (calstep == 4) {

        channeltable();

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(0, (20 + voffset), "calibrating");
        }


        if (display_setting >= 1) {
          u8g.setPrintPos(30, 10);
          u8g.print("RSS CALIBRATION");
          u8g.setPrintPos(20, 26);
          u8g.print("calibrating");
        }


        //Fill up the array with RSSi values for each channel - repeated 10 times for accuracy

        if (rssireadin < 10) {

          if (ACT_channel == 1) {
            rssi_value1 = _readRSSI();
            values[0] = rssi_value1;
            Serial.print("1: "); Serial.println(values[0]);
            ACT_channel = 2;
          }

          else if (ACT_channel == 2) {
            rssi_value2 = _readRSSI();
            values[1] = rssi_value2;
            Serial.print("2: "); Serial.println(values[1]);
            ACT_channel = 3;
          }

          else if (ACT_channel == 3) {
            rssi_value3 = _readRSSI();
            values[2] = rssi_value3;
            Serial.print("3: "); Serial.println(values[2]);
            ACT_channel = 4;
          }

          else if (ACT_channel == 4) {
            rssi_value4 = _readRSSI();
            values[3] = rssi_value4;
            Serial.print("4: "); Serial.println(values[3]);
            ACT_channel = 5;
          }

          else if (ACT_channel == 5) {
            rssi_value5 = _readRSSI();
            values[4] = rssi_value5;
            Serial.print("5: "); Serial.println(values[4]);
            ACT_channel = 6;
          }

          else if (ACT_channel == 6) {

            rssi_value6 = _readRSSI();
            values[5] = rssi_value6;
            Serial.print("6: "); Serial.println(values[5]);
            ACT_channel = 7;
          }

          else if (ACT_channel == 7) {
            rssi_value7 = _readRSSI();
            values[6] = rssi_value7;
            Serial.print("7: "); Serial.println(values[6]);
            ACT_channel = 8;
          }

          else if (ACT_channel == 8) {
            rssi_value8 = _readRSSI();
            values[7] = rssi_value8;
            Serial.print("8: "); Serial.println(values[7]);

            rssireadin = rssireadin + 1;
            ACT_channel = 1;
          }
        }

        //Getting the max/min values and the position in the array

        if (rssireadin == 10) {

          rssi_min = values[0];

          for (int i = 1; i < 8; i++) {
            if (values[i] < rssi_min) {
              rssi_min = values[i]; //determine min value in the array
              minIndex = i;
            }
          }

          rssireadin = 11;

        }

        //print out the results
        if (rssireadin >= 11) {

          Serial.print("rssi_min: ");
          Serial.println(rssi_min);

          rssireadin = rssireadin + 1;
        }


        if (rssireadin == 20) {
          RSSIminEEP = rssi_min / 2;

          EEPROM.write(RSSIminADDR, RSSIminEEP);
          Serial.print("rssi_min saved! ");

          if (display_setting <= 1) {
            TV.clear_screen();
          }
          calstep = 5;
        }


      }



      else if (calstep == 5) {

        RSSIminEEP = EEPROM.read(RSSIminADDR) * 2;
        RSSImaxEEP = EEPROM.read(RSSImaxADDR) * 2;

        if (display_setting <= 1) {
          TV.print(30, (0 + voffset), "RSSI CALIBRATION");
          TV.select_font(font6x8);
          TV.print(8, (20 + voffset), "3:Calibration done!");
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
          u8g.print("3:Calibration done!");

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

  int chan_rssi[8];
  memset(chan_rssi, 0, 8 * sizeof(int));

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

      {
        int idx = ACT_channel - 1;

        int rssi = map(_readRSSI(), max, min, 0, 100);

        if (rssi > 100) {
          rssi = 100;
        }
        if (rssi < 0) {
          rssi = 0;
        }

        chan_rssi[idx] = rssi;

        if (display_setting <= 1) {
          int x = 16 * idx;
          char chan_label[10];
          sprintf(chan_label, "CH%d", idx - 1);

          TV.draw_rect(x, (13 + voffset), 14, 56, BLACK, BLACK);
          TV.draw_rect(x, ((70 - (rssi * 0.46)) + voffset), 10, (rssi * 0.46), WHITE, WHITE);
          TV.print(x, (75 + voffset), chan_label);
          if ((rssi < 100) && (rssi >= 10)) {
            TV.print(x + 2, ((63 - (rssi * 0.46)) + voffset), rssi);
          }
          else if (rssi < 10) {
            TV.print(x + 4, ((63 - (rssi * 0.46)) + voffset), rssi);
          }
          else {
            TV.print(x, ((63 - (rssi * 0.46)) + voffset), rssi);
          }
        }
      }

      if (++ACT_channel > 8)
        ACT_channel = 1;

      if (display_setting >= 1) {
        u8g.setPrintPos(35, 10);
        u8g.print("BAND SCANNER");
        u8g.drawHLine(15, 50, 100);

        for (unsigned int idx = 0; idx < 8; idx++) {
          int rssi = chan_rssi[idx];

          int rssibar = round(rssi * 0.3);

          int x = 15 + idx * 13;

          char chan_label[10];
          sprintf(chan_label, "C%d", idx + 1);

          u8g.setPrintPos(x, 59);
          u8g.print(chan_label);

          if (rssi > 1) {
            u8g.drawBox(x, (51 - (rssibar)), 9, rssibar);
          }
          if (rssi <= 9) {
            u8g.setPrintPos(x + 2, (48 - rssibar));
          }
          else {
            u8g.setPrintPos(x, (48 - rssibar));
          }
          u8g.print(rssi);
        }
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
        TV.draw_rect(29, 84, 20, 8, BLACK, BLACK);
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



//AUTOSEARCH MODAL ************************

void autosearch() {
  byte exit = 0;

  ACT_channel = 1;

  double values[8];
  int maxIndex = 0;
  int minIndex = 0;

  byte rssireadin = 0;

  double rssi_value1 = 0;
  double rssi_value2 = 0;
  double rssi_value3 = 0;
  double rssi_value4 = 0;
  double rssi_value5 = 0;
  double rssi_value6 = 0;
  double rssi_value7 = 0;
  double rssi_value8 = 0;

  clearOLED();

  while (exit == 0) {

    u8g.firstPage();
    do {
      osd();
      buttoncheck();
      channeltable();
      osd_mode = 1;

      if (display_setting <= 1) {
        TV.print(10, (10 + voffset), "Autosearch...");
      }
      if (display_setting >= 1) {
        u8g.setPrintPos(10, 10);
        u8g.print("Autosearch...");
      }

      //Fill up the array with RSSi values for each channel - repeated 10 times for accuracy

      if (rssireadin < 10) {

        if (ACT_channel == 1) {
          rssi_value1 = _readRSSI();
          values[0] = rssi_value1;
          //Serial.print("1: "); Serial.println(values[0]);
          ACT_channel = 2;
        }

        else if (ACT_channel == 2) {
          rssi_value2 = _readRSSI();
          values[1] = rssi_value2;
          //Serial.print("2: "); Serial.println(values[1]);
          ACT_channel = 3;
        }

        else if (ACT_channel == 3) {
          rssi_value3 = _readRSSI();
          values[2] = rssi_value3;
          //Serial.print("3: "); Serial.println(values[2]);
          ACT_channel = 4;
        }

        else if (ACT_channel == 4) {
          rssi_value4 = _readRSSI();
          values[3] = rssi_value4;
          //Serial.print("4: "); Serial.println(values[3]);
          ACT_channel = 5;
        }

        else if (ACT_channel == 5) {
          rssi_value5 = _readRSSI();
          values[4] = rssi_value5;
          //Serial.print("5: "); Serial.println(values[4]);
          ACT_channel = 6;
        }

        else if (ACT_channel == 6) {

          rssi_value6 = _readRSSI();
          values[5] = rssi_value6;
          //Serial.print("6: "); Serial.println(values[5]);
          ACT_channel = 7;
        }

        else if (ACT_channel == 7) {
          rssi_value7 = _readRSSI();
          values[6] = rssi_value7;
          //Serial.print("7: "); Serial.println(values[6]);
          ACT_channel = 8;
        }

        else if (ACT_channel == 8) {
          rssi_value8 = _readRSSI();
          values[7] = rssi_value8;
          //Serial.print("8: "); Serial.println(values[7]);

          rssireadin = rssireadin + 1;
          ACT_channel = 1;
        }
      }

      //Getting the max/min values and the position in the array

      if (rssireadin == 10) {

        double rssi_max = values[0];
        double rssi_min = values[0];

        for (int i = 1; i < 8; i++) {
          if (values[i] > rssi_max) {
            maxIndex = i;
            rssi_max = values[i]; //determine max value in the array
          }
          if (values[i] < rssi_min) {
            rssi_min = values[i]; //determine min value in the array
            minIndex = i;
          }
        }

        rssireadin = 11;

      }

      //print out the results
      if (rssireadin >= 11) {

        if (display_setting >= 1) {
          u8g.setPrintPos(10, 30);
          u8g.print("Best CH: ");
          u8g.print(minIndex + 1);
        }

        rssireadin = rssireadin + 1;
      }

      //go back and switch to the best channel
      if (rssireadin >= 50)  {

        if (display_setting <= 1) {
          TV.clear_screen();
        }

        menuactive = 0; //for debugging only
        ACT_channel = minIndex + 1;
        BT_channel = minIndex + 1;
        rssireadin = 0;

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


//FINDER OSD PLOT ************************

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




// STUFF FOR LATER VERSIONS
// • Display setting (OLED + OSD)
// • Serial setting (OLED + OSD)
// • Dockking compatibility???
// • Reorder menu items


//STORING STUFF FOR LATER
// u8g.drawBitmapP(5, 20, 7, 32, bitmap_dock);    //serial
// u8g.drawBitmapP(5, 20, 7, 32, bitmap_display);    //display
