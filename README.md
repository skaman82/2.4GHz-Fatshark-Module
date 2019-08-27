<img src="https://raw.githubusercontent.com/skaman82/2.4GHz-Fatshark-Module/master/img/img.jpg"/>

# 2.4GHz-Fatshark-Module
I created this module to fly my 2.4 wings casually without the need to set up the bulky 2.4 ground station. The range is absolutely plenty - up to 8km with just a pair of CPL Singularity antennas, even more with a patch. This is not meant to be a replacement for a good ground station for real LR flights.

RX28 based Fatshark RX module

- 8 channels (2414, 2432, 2450, 2468, 2490, 2510, 2390, 2370 MHz)
- RSSI display
- Fatshark button control
- Bandscan
- Finder mode
- RSSI calibration
- OSD menus
- OLED screen

Tested with following VTX models:</br>
• TBS Unify 2G4</br>
• Lawmate 2.4</br>
• FuriousFPV 2.4 VTX</br>
• Cheap low power chinese transmitter modules </br>
(no audio compatibility issues unlike the latest RX 2.4 module on the market)


# OPERATION
<b>HOME SCREEN:</b> You can change the channels by pressing the UP/DOWN keys - it works also with the google buttons (if the feature is active). If using the joystick you have to press UP/DOWN once to bring the OSD up - then you can change the channel. After no button press is registered the OSD will automatically disappear. </br>
</br>
By pressing the CENTER button you can bring up the <b>MENU</b>. With UP/DOWN you can switch between different menu items. Select EXIT to return to the home screen.</br>
</br>
<b>GOGGLE CONTROL:</b> Enables fatshark button control.</br>
</br>
<b>BANDSCAN:</b> Scans all channels and displays the RSSI values in a graph.</br>
</br>
<b>LOCK MODE:</b> If activated (lock icon in the home screen) the module will lock up after approx. 10 seconds. All controls are disabled to prevent chanel changes on acidental button press. You can temporarily UNLOCK the module by pressing the CENTER button for 3-5 seconds (the lock icon will open). After unlocking the module it will return to locked state after approx. 10 seconds.</br>
</br>
<b>FIND MODE:</b>Will show the RSSI value in a BAR (oled) or a plot (OSD). Depending on the RSSI value the beeper will beep more often and the OSD LED will blink as well indicating the direction. You can change the channel in the FIND MODE - so be sure you have the correct one. Use a directional antenna to find your model with this function.</br>
</br>
<b>AUTOSEARCH:</b> Press the CENTER button for 2 seconds to call the autosearch function. Autosearch will find the strongest channel and switch to it automatically.</br>
</br>
<b>CALIBRATION:</b> You have to run calibration to have the correct RSSI values. Just follow the instructions in the wizzard.</br>





# UPDATING THE FIRMWARE
You will need a FTDI to USB adapter to upload the module firmware. You may need to install the FTDI driver for it. Set the FTDI to 5V.

Steps to upload new firmware
1. Install Arduino IDE
2. Install the Arduino Libraries from the "Libs" folder.
3. Install the MightyCore hardware package from here: https://github.com/MCUdude/MightyCore (just follow the instructions)
4. Download the firmware and open the Code.INO file from the "Code" folder in Arduino
5. Select the hardware settings in Arduino IDE as shown in the image below
6. Connect the FTDI tool according to the plug pinout as shown in the picture and select the COM port in Arduino IDE
7. If you have the rev 06 PCB open the JUMP1 jumper on the PCB to upload code (solder it together after your upload is done).
8. Upload the code </br>
YOU ARE DONE :)

<img src="https://raw.githubusercontent.com/skaman82/2.4GHz-Fatshark-Module/master/img/PCB.png"/>
