
// This Project is created by Albert Kravcov

// TO DOS & FIXMEs:
// • Find OSD button bug not enabaling the osd on first press
// X Main Menu structure 
// • Finder Screen
// • Lock Mode
// X Calibrate menu


#include <TVout.h>
#include <font6x8.h>
#include <font4x6.h>
#include "hardware.h"
#include "logo.h"

#include <Wire.h>
#include "U8glib.h"

#include <EEPROM.h>

//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI
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

int channelvalueEEP;
int lockmodeEEP;    //TODO: lock mode setting (all buttons deactivated)
int fscontrollEEP;  //TODO: FS Buttons disabled
int displayEEP;     //TODO: OLED disabled
int RSSImaxEEP;     // RSSI calibration
int RSSIminEEP;     // RSSI calibration
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

unsigned long previousOsdMillis = 0;

#ifdef V3
unsigned long BTinterval = 6000;
unsigned long BTinterval_FIXED = 6000;
#endif

#ifndef V3
unsigned long BTinterval = 4000;
unsigned long BTinterval_FIXED = 4000;
#endif



#ifdef OSD
TVout TV;
#endif

void setup() {

  //Serial.begin(9600);
  //Serial.println("Hello");

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
#ifndef SM186R
  pinMode(CH3_pin,      OUTPUT); //module c3
#endif
  pinMode(BX_pin,       OUTPUT); //module BX
  pinMode(FS_pin1,      INPUT); //google c1
  pinMode(FS_pin2,      INPUT); //google c2
  pinMode(FS_pin3,      INPUT); //google c3
  pinMode(ButtonCenter, INPUT_PULLUP); //Button

#ifdef V2
  pinMode(ButtonLeft,   INPUT_PULLUP); //Button left
  pinMode(ButtonRight,  INPUT_PULLUP); //Button right
  pinMode(BUZZ,         OUTPUT); //Buzzer
#endif

#ifdef V3
  pinMode(ButtonLeft,   INPUT_PULLUP); //Button left
  pinMode(ButtonRight,  INPUT_PULLUP); //Button right
  pinMode(ButtonUp,     INPUT_PULLUP); //Button left
  pinMode(ButtonDown,   INPUT_PULLUP); //Button right
  //pinMode(BUZZ,         OUTPUT); //Buzzer
#endif

  channelvalueEEP = EEPROM.read(chanADDR);

  lockmodeEEP = EEPROM.read(chanADDR);
  fscontrollEEP = EEPROM.read(fscontrollADDR);
  displayEEP = EEPROM.read(displayADDR);

  RSSImaxEEP = EEPROM.read(RSSImaxADDR)  * 2;
  RSSIminEEP = EEPROM.read(RSSIminADDR)  * 2;

  max = RSSImaxEEP;
  min = RSSIminEEP;



  if (channelvalueEEP <= 8)
  {
    SAVED_channel = channelvalueEEP;
  }
  if (channelvalueEEP > 8)
  {
    SAVED_channel = 1;
  }



#ifdef OLED
  clearOLED();
  showlogo();
#endif

#ifdef OSD
#ifdef PAL_FORMAT
#ifdef V3
  TV.begin(_PAL, 128, 96); //original 128x96px, 160x120max
#endif
#ifndef V3
  TV.begin(_PAL, 112, 78);
#ifdef debug
  //TV.begin(_PAL, 120, 86);
#endif
#endif
#endif

#ifdef NTSC_FORMAT
  TV.begin(_NTSC, 120, 90);
#endif


#ifdef V3
  TV.delay_frame(1);
  display.output_delay = 55; //move the whole screen vertically
#endif

  TV.clear_screen();
  TV.bitmap(2, 20, logo);

#endif

  digitalWrite(OSD_ctr1, HIGH);
  digitalWrite(OSD_ctr2, LOW);
  digitalWrite(LED_pin, HIGH);

#ifndef OSD
  tone(11, 500, 200);
#endif

#ifdef OSD
  TV.select_font(font6x8);
  TV.print(25, 50, "2G4 MODULE");
  TV.select_font(font4x6);
  TV.print(24, 69, "#RUININGTHEHOBBY");
  TV.delay(3000);
  TV.tone(500, 200);
  TV.clear_screen();
#endif


#ifdef OLED
  clearOLED();
#endif

  osd_mode = 1;


  //bandscan(); //for debugging only

}


void clearOLED()
{
#ifdef OLED
  u8g.firstPage();
  do
  {
  }
  while ( u8g.nextPage() );
#endif

}

void showlogo()
{
#ifdef OLED
  u8g.firstPage();
  do
  {
    // splashscreen goes here
    u8g.drawBitmapP(5, 20, 14, 18, logo_OLED);
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

#ifndef V1
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
#endif

#ifdef V3
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
#endif
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
#ifdef OSD
    TV.tone(800, 80);
#endif
#ifndef OSD
    tone(11, 800, 80);
#endif

#ifdef V1

    if (menuactive == 0) {
      //  button short action here
      Old_FS_channel = FS_channel;
      BT_update = 1;
      FS_control = 0;

      if (osd_mode == 1) {
        BTinterval += 1000;


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
#endif

#ifndef V1

    if (menuactive == 0) {
#ifdef OSD
      TV.clear_screen();
#endif
      menuactive = 1;
      menu();
      //bandscan();
    }
    else if (menuactive == 1) {
      TV.fill(BLACK);
      TV.delay(10);
      //menuactive = 0;
      return;
    }

#endif
  }

  if (pressedbut == 2) {

#ifdef OSD
    TV.tone(800, 80);
#endif
#ifndef OSD
    tone(11, 800, 80);
#endif


    if (menuactive == 0) {
      //  button short action here
      Old_FS_channel = FS_channel;
      BT_update = 1;
      FS_control = 0;

      if (osd_mode == 1) {
        BTinterval += 500;


        if (BT_channel >= 2) {
          BT_channel -= 1;
        }

        else {
          BT_channel = 8;
        }
      }
      else {
#ifdef OSD
        TV.clear_screen();
#endif
        callOSD();
      }
    }


  }

  if (pressedbut == 3) {

#ifdef OSD
    TV.tone(800, 80);
#endif
#ifndef OSD
    tone(11, 800, 80);
#endif

    if (menuactive == 0) {
      //  button short action here
      Old_FS_channel = FS_channel;
      BT_update = 1;
      FS_control = 0;

      if (osd_mode == 1) {
        BTinterval += 500;


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

  }

  if (pressedbut == 6) {

#ifndef V1
    if (menuactive == 0) {
#ifdef OSD
      TV.clear_screen();
#endif
      menuactive = 1;
      calibration();
    }
    else if (menuactive == 1) {
      //TV.fill(BLACK);
      //TV.delay(10);
      //menuactive = 0;
      //return;

    }

    else {
      TV.fill(BLACK);
    }
#endif

#ifdef V1
    if (menuactive == 0) {
#ifdef OSD
      TV.clear_screen();
#endif
      menuactive = 1;
      calibration();
    }
    else if (menuactive == 1) {
      TV.fill(BLACK);
      TV.delay(10);
      menuactive = 0;
      return;

    }

    else {
      TV.fill(BLACK);
    }
#endif
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
    exit;
  }
  else {
    exit;
  }
}

void osd() {

  if (osd_mode == 1) {
    digitalWrite(OSD_ctr1, HIGH);
    digitalWrite(OSD_ctr2, LOW);
    digitalWrite(LED_pin, LOW);


  }

  else if (osd_mode == 0) {
    digitalWrite(OSD_ctr1, LOW);
    digitalWrite(OSD_ctr2, HIGH);
    digitalWrite(LED_pin, HIGH);


  }
  else {
    osd_mode = 1;
  }

}



void loop() {

#ifdef OLED
  u8g.firstPage();
  do {
#endif
    buttoncheck();
    fs_buttons();
    control();
    rx_update();
    channeltable();

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

#ifdef OSD
    TV.select_font(font6x8);
    TV.print(5, 2, "Fp1:");
    TV.print(p1_value);
    TV.print(" Fp2:");
    TV.print(p2_value);
    TV.print(" Fp3:");
    TV.println(p3_value);

    TV.print(5, 10, "chEEP:");
    TV.println(channelvalueEEP);

    TV.print(5, 20, "BT_CH:");
    TV.println(BT_channel);

    TV.print(5, 30, "FS_CH:");
    TV.println(FS_channel);

    TV.print(5, 40, "OLD_FS_CH:");
    TV.println(Old_FS_channel);

    TV.print(5, 50, "ACT_CH:");
    TV.println(ACT_channel);

    TV.print(5, 60, freq);
    TV.println(" MHz");

    TV.print(5, 70, "RSSI:");
    TV.println(percentage, 0);

    if ((percentage < 100) && (percentage >= 10)) {
      TV.draw_rect(42, 70, 8, 6, BLACK, BLACK); //cover the first number
    }

    else if (percentage < 10) {
      TV.draw_rect(36, 70, 8, 6, BLACK, BLACK); //cover the second number
    }

    float rssibar = percentage * 0.81; //scalin the bar a bit down

    TV.select_font(font4x6);
    TV.draw_rect(5, 80, rssibar, 5, WHITE, WHITE);

    if (percentage < 100) {
      TV.draw_rect(rssibar, 80, (100 - rssibar), 5, BLACK, BLACK);
    }

    TV.draw_line(5, 85, 81, 85, WHITE);

    TV.draw_rect(0, 0, 159, 119, WHITE);

#endif

#endif

#ifndef debug

#ifdef OSD
    //Main OSD code
    TV.select_font(font6x8);
    TV.print(23, 27, "CH");
    TV.println(ACT_channel);
    TV.draw_rect(18, 20, 26, 20, WHITE);

    TV.draw_rect(44, 20, 55, 20, WHITE);
    TV.print(49, 27, freq);
    TV.println(" MHz");

#endif

#ifdef RSSI_mod
    uint32_t rssi_value = _readRSSI();
    float percentage = map(rssi_value, max, min, 0, 100);
    if (percentage > 100) {
      percentage = 100;
    }
    else if (percentage < 0) {
      percentage = 0;
    }

    float rssibar = percentage * 0.81; //scalin the bar a bit down

#ifdef OSD
    TV.select_font(font4x6);
    TV.print(18, 50, "RSSI ");
    TV.println((percentage - 0), 0);

    if ((percentage < 100) && (percentage >= 10)) {
      TV.draw_rect(46, 50, 4, 4, BLACK, BLACK); //cover the first RSSI number
    }

    else if (percentage < 10) {
      TV.draw_rect(42, 50, 8, 4, BLACK, BLACK); //cover the second RSSI number
    }

    TV.draw_rect(18, 58, rssibar, 5, WHITE, WHITE);
    if (percentage < 100) {
      TV.draw_rect((18 + rssibar), 58, (100 - rssibar), 5, BLACK, BLACK);
    }
    TV.draw_line(18, 64, 100, 64, WHITE);

    TV.print(1, 53, rssi_value);

#endif

#endif

#endif

    //osd timer >>
#ifndef debug
    if ((menuactive != 1) && (osd_mode == 1)) {
      //FIXME TIMER
      unsigned long currentOsdMillis = millis();

      if (currentOsdMillis - previousOsdMillis > BTinterval) {

#ifdef OSD
        TV.clear_screen();
        TV.tone(100, 150);
#endif
        osd_mode = 0; //coment out for debugging the OSD
        BTinterval = BTinterval_FIXED; //set the timeout-interval back to 2sek.

        //save the last activechannel to EEPROM
        channelvalueEEP = ACT_channel;
        EEPROM.write(chanADDR, channelvalueEEP);
        previousOsdMillis = currentOsdMillis;
      }
      else {
        osd_mode = 1;
      }
    }
#endif

#ifdef OLED
    u8g.setFont(u8g_font_profont22r);
    u8g.setPrintPos(42, 24);
    u8g.print(freq);
    u8g.setFont(u8g_font_5x7r);
    u8g.print(" MHz");

    const long testval = millis();
    u8g.setPrintPos(42, 33);
    u8g.print(testval);

#ifdef RSSI_mod
    u8g.setFont(u8g_font_5x7r);
    u8g.setPrintPos(42, 42);
    //u8g.print("RSSI: ");
    //u8g.print(percentage, 0);

    u8g.drawBox(45, 48, (rssibar * 0.90), 5); //
    u8g.drawFrame(42, 45, 78, 11);
    //u8g.setColorIndex(0);
    //u8g.drawVLine(53,48,5);
    //u8g.drawVLine(61,48,5);
    //u8g.drawVLine(69,48,5);
    //u8g.drawVLine(77,48,5);
    //u8g.drawVLine(85,48,5);
    //u8g.drawVLine(93,48,5);
    //u8g.drawVLine(101,48,5);
    //u8g.drawVLine(109,48,5);

    u8g.setColorIndex(1);

#endif

    if (ACT_channel == 1) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_one);
    }
    if (ACT_channel == 2) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_two);
    }
    if (ACT_channel == 3) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_three);
    }
    if (ACT_channel == 4) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_four);
    }
    if (ACT_channel == 5) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_five);
    }
    if (ACT_channel == 6) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_six);
    }
    if (ACT_channel == 7) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_seven);
    }
    if (ACT_channel == 8) {
      u8g.drawBitmapP(0, 8, 5, 48, bitmap_eight);
    }

#ifdef OSD
#ifdef V3

    //TV.draw_rect(0, 0, 119, 89, WHITE);

    //TV.delay_frame(1);
    //display.output_delay = 55;
    //TV.delay_frame(1);

#endif
#endif

  } while ( u8g.nextPage() );
#endif
}


//simple avg of 4 value
uint16_t _readRSSI() {
  volatile uint32_t sum = 0;
  analogRead(RSSI_pin);
  delay(10);
  sum = analogRead(RSSI_pin);
  //delay(10);
  sum += analogRead(RSSI_pin);
  //delay(10);
  sum += analogRead(RSSI_pin);
  //delay(10);
  sum += analogRead(RSSI_pin);
  return sum / 4;
}


void menu() {
  menuactive = 1;
  int menu_position = 1;
  osd_mode = 1;

  byte exit = 0;
  while (exit == 0)
  {


    if (menu_position == 1) {
      buttoncheck();
      control();
      osd();


#ifdef OSD
      TV.select_font(font4x6);
      TV.print(55, 0, "MENU");
      TV.select_font(font6x8);
      TV.bitmap(35, 30, bitmap_nkizw_OSD); //goggle
      TV.print(15, 80, "Goggle control:");
      TV.print("ON");
#endif

      if (pressedbut == 1) {
#ifdef OSD
        TV.clear_screen();
#endif
      }
      if (pressedbut == 4) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 2;
      }
    }

    else if (menu_position == 2) {

      buttoncheck();
      control();
      osd();

      //do stuff pos2
      TV.select_font(font4x6);
      TV.print(55, 0, "MENU");
      TV.select_font(font6x8);
      TV.bitmap(35, 30, bitmap_hd85pj_OSD); //scan
      TV.print(38, 80, "Bandscan");

      if (pressedbut == 1) {
#ifdef OSD
        TV.clear_screen();
#endif
        bandscan();
      }

      if (pressedbut == 4) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 3;
      }
      if (pressedbut == 5) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 1;
      }
    }

    else if (menu_position == 3) {

      buttoncheck();
      control();
      osd();

      TV.select_font(font4x6);
      TV.print(55, 0, "MENU");
      TV.select_font(font6x8);
      TV.bitmap(35, 30, bitmap_w113l_OSD); //lock
      TV.print(39, 80, "Lock:");
      TV.print("OFF");

      if (pressedbut == 4) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 4;
      }
      if (pressedbut == 5) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 2;
      }
    }

    else if (menu_position == 4) {

      buttoncheck();
      control();
      osd();

      //do stuff pos4
      TV.select_font(font4x6);
      TV.print(55, 0, "MENU");
      TV.select_font(font6x8);
      TV.bitmap(35, 30, bitmap_ydywrn_OSD); //search
      TV.print(35, 80, "Find mode");

      if (pressedbut == 4) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 5;
      }
      if (pressedbut == 5) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 3;

      }
    }

    else if (menu_position == 5) {

      buttoncheck();
      control();
      osd();

      //do stuff pos5
      TV.select_font(font4x6);
      TV.print(55, 0, "MENU");
      TV.select_font(font6x8);
      TV.bitmap(35, 30, bitmap_949gqh_OSD); //tools
      TV.print(37, 80, "Calibrate");


      if (pressedbut == 1) {
#ifdef OSD
        TV.clear_screen();
#endif
        calibration();
      }

      if (pressedbut == 4) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 6;
      }
      if (pressedbut == 5) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 4;

      }
    }

    else if (menu_position == 6) {

      buttoncheck();
      control();
      osd();

      //do stuff pos6
      TV.select_font(font4x6);
      TV.print(55, 0, "MENU");
      TV.select_font(font6x8);
      TV.bitmap(35, 30, bitmap_be8bbq_OSD); //exit
      TV.print(50, 80, "EXIT");

      if (pressedbut == 1) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 0;
        menuactive = 0;
        exit = 1;
      }

      if (pressedbut == 5) {
#ifdef OSD
        TV.clear_screen();
#endif
        menu_position = 5;
      }
    }
  }
}




void calibration() {     // Calibration wizzard

  menuactive = 1;

  int calstep = 1;
  osd_mode = 1;

  while (calstep == 1) {
    buttoncheck();
    control();
    osd();
    channeltable();

#ifdef OSD

    TV.select_font(font4x6);
    TV.print(30, 0, "RSSI CALIBRATION");
    TV.select_font(font6x8);
    TV.print(14, 30, "1:Select channel");
    uint32_t rssi_value = _readRSSI();
    // TV.print(0, 55, rssi_value);
    TV.print(21, 55, "< ");
    TV.print(ACT_channel);
    TV.print(":");
    TV.print(freq);
    TV.print(" MHz");
    TV.print(" >");

    TV.draw_rect(38, 75, 48, 16, WHITE);
    TV.select_font(font4x6);
    TV.print(51, 81, "NEXT >");

#endif

#ifndef V1
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

#endif

    if (pressedbut == 1) {

#ifdef OSD
      TV.clear_screen();
#endif

      calstep = 2;
    }
  }

  while (calstep == 2) {
    buttoncheck();
    control();

    TV.print(30, 0, "RSSI CALIBRATION");
    TV.select_font(font6x8);

    TV.print(0, 30, "2:Remove antenna and switch off the VTX");
    uint32_t rssi_value = _readRSSI();
    // RSSImaxEEP = EEPROM.read(RSSImaxADDR) * 2;
    // TV.print(50, 55, rssi_value);
    // TV.print(50, 65, RSSImaxEEP);

    TV.draw_rect(38, 75, 48, 16, WHITE);
    TV.select_font(font4x6);
    TV.print(51, 81, "NEXT >");

    if (pressedbut == 1) {

      RSSImaxEEP = rssi_value / 2;
      EEPROM.write(RSSImaxADDR, RSSImaxEEP);

#ifdef OSD
      TV.clear_screen();
#endif
      calstep = 3;
    }
  }
  while (calstep == 3) {
    buttoncheck();
    control();

    TV.print(30, 0, "RSSI CALIBRATION");
    TV.select_font(font6x8);

    TV.print(0, 30, "3:Put on the antenna and switch on the VTX");
    uint32_t rssi_value = _readRSSI();
    // TV.print(50, 55, rssi_value);
    // RSSIminEEP = EEPROM.read(RSSIminADDR) * 2;
    // TV.print(50, 65, RSSIminEEP);

    TV.draw_rect(38, 75, 48, 16, WHITE);
    TV.select_font(font4x6);
    TV.print(51, 81, "NEXT >");

    if (pressedbut == 1) {

      RSSIminEEP = rssi_value / 2;
      EEPROM.write(RSSIminADDR, RSSIminEEP);

#ifdef OSD
      TV.clear_screen();
#endif
      calstep = 4;
    }
  }
  while (calstep == 4) {
    buttoncheck();
    control();

    TV.print(30, 0, "RSSI CALIBRATION");
    TV.select_font(font6x8);

    TV.print(8, 30, "4:Calibration done!");

    RSSIminEEP = EEPROM.read(RSSIminADDR) * 2;
    RSSImaxEEP = EEPROM.read(RSSImaxADDR) * 2;

    TV.print(30, 50, "Saved Max:");
    TV.print(RSSIminEEP);

    TV.print(30, 60, "Saved Min:");
    TV.print(RSSImaxEEP);

    TV.draw_rect(38, 75, 48, 16, WHITE);
    TV.select_font(font4x6);
    TV.print(51, 81, "EXIT >");

    if (pressedbut == 1) {

#ifdef OSD
      TV.clear_screen();
#endif
      calstep = 0;
      menuactive = 0;
      osd_mode = 0;
      //softReset();

      max = RSSImaxEEP;
      min = RSSIminEEP;

      return;
    }
  }


}


void bandscan() {

  osd_mode = 1;
  menuactive = 1;

  while (menuactive == 1) {
    buttoncheck();
    control();
    osd();
    channeltable();

    TV.select_font(font4x6);
    TV.print(35, 0, "BAND SCANNER ");


    ACT_channel = 1;

    uint32_t rssi_value = _readRSSI();

    float percentage = map(rssi_value, max, min, 0, 100);
    if (percentage > 100) {
      percentage = 100;
    }
    else if (percentage < 0) {
      percentage = 0;
    }


    float rssibar1 = percentage * 0.81; //scalin the bar a bit down
    TV.print(0, 90, "CH1");
    TV.draw_rect(0, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(0, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);

    ACT_channel = 2;
    TV.delay(10);
    rssi_value = _readRSSI();
    float rssibar2 = percentage * 0.81; //scalin the bar a bit down
    TV.print(16, 90, "CH2");
    TV.draw_rect(16, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(16, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);

    ACT_channel = 3;
    TV.delay(10);
    rssi_value = _readRSSI();
    float rssibar3 = percentage * 0.81; //scalin the bar a bit down
    TV.print(32, 90, "CH3");
    TV.draw_rect(32, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(32, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);

    ACT_channel = 4;
    TV.delay(10);
    rssi_value = _readRSSI();
    float rssibar4 = percentage * 0.81; //scalin the bar a bit down
    TV.print(48, 90, "CH4");
    TV.draw_rect(48, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(48, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);

    ACT_channel = 5;
    TV.delay(10);
    rssi_value = _readRSSI();
    float rssibar5 = percentage * 0.81; //scalin the bar a bit down
    TV.print(64, 90, "CH5");
    TV.draw_rect(64, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(64, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);

    ACT_channel = 6;
    TV.delay(10);
    rssi_value = _readRSSI();
    float rssibar6 = percentage * 0.81; //scalin the bar a bit down
    TV.print(80, 90, "CH6");
    TV.draw_rect(80, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(80, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);


    ACT_channel = 7;
    TV.delay(10);
    rssi_value = _readRSSI();
    float rssibar7 = percentage * 0.81; //scalin the bar a bit down
    TV.print(96, 90, "CH7");
    TV.draw_rect(96, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(96, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);

    ACT_channel = 8;
    TV.delay(10);
    rssi_value = _readRSSI();
    float rssibar8 = percentage * 0.81; //scalin the bar a bit down
    TV.print(112, 90, "CH8");
    TV.draw_rect(112, 35, 10, 50, BLACK, BLACK);
    TV.draw_rect(112, (85 - rssi_value), 10, rssi_value, WHITE, WHITE);

    TV.print(0, 0, rssi_value);


    if (pressedbut == 1) {
#ifdef OSD
      TV.clear_screen();
#endif
      menuactive = 0;
      osd_mode = 0;
      exit;


    }

  }

}
