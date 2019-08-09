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

# OPERATION
…

# UPDATING THE FIRMWARE
You will need a FTDI to USB adapter to upload the module firmware. You may need to install the FTDI driver for it. Set the FTDI to 5V.

Steps to upload new firmware
1. Install Arduino IDE
2. Install the MightyCore hardware package from here: https://github.com/MCUdude/MightyCore (just follow the instructions)
3. Download the firmware and open the Code.INO file in Arduino
4. Select the hardware settings as shown in the image
5. Connect the FTDI tool according to the plug pinout as shown in the picture and select the COM port in Arduino IDE
6. If you have the rev 06 PCB open the JUMP1 jumper on the PCB to upload code (solder it together after your upload is done).
7. Upload the code
YOU ARE DONE :)

<img src="https://raw.githubusercontent.com/skaman82/2.4GHz-Fatshark-Module/master/img/PCB.png"/>
