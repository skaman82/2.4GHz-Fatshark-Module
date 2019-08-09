# 2.4GHz-Fatshark-Module
RX28 based Fatshark RX module

- 8 channels (2414, 2432, 2450, 2468, 2490, 2510, 2390, 2370 MHz)
- RSSI display
- Fatshark button control
- Bandscan
- Finder mode
- RSSI calibration
- OSD menus
- OLED screen

Tested with following VTX models:
• TBS Unify 2G4
• Lawmate
• Cheap low power chinese transmitter modules (no audio compatibility issues unlike the latest RX 2.4 module on the market)


# OPERATION
…


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
8. Upload the code
YOU ARE DONE :)

<img src="https://raw.githubusercontent.com/skaman82/2.4GHz-Fatshark-Module/master/img/PCB.png"/>
