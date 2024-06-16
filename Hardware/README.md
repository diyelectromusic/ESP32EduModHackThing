# ESP32EduModHackThing

* Blog series: https://diyelectromusic.wordpress.com/2024/05/07/educational-diy-synth-thing/
* PCB Design Notes: https://diyelectromusic.wordpress.com/2024/05/07/esp32-wroom-educational-modular-synth-thing-pcb-design/
* PCB Build and Test Notes: https://diyelectromusic.wordpress.com/2024/05/19/esp32-wroom-educational-modular-synth-thing-pcb-design-build-guide/

Bill of Materials:
- ESP32 WROOM Educational Modular Synth Thing PCB.
- 24-pin DIP adaptor PCB (see note on footprint error below).
- ESP32 WROOM DevKit.
- 1 x CD4067B 24-pin DIP 16-way analog multiplexer (see note below on footprint error!).
- 1x H11L1 6-pin DIP opto-isolator.
- 1x TDA7052A 8-pin DIP BTL audio amplifier with DC volume control.
- 1x L7805 TO-220 5V voltage regulator.
- Resistors: 1 x 220R, 11 x 470R, 5 x 1K, 4 x 10K, 5 x 100K, 3 x 680K, 6 x 1M, 1 x 2M
- Variable Resistors: 13 x 10K linear (PCB Mount: RK09 footprint), 1 x 100K linear (PCB Mount: RK09 footprint)
- 4 x 2N3904 NPN transistors
- 1 x BC557 PNP transistor
- 5 x 1N4148 signal diodes
- 6 x BAT43 Schottky diode
- 1 x 3mm LED
- Ceramic Capacitors: 10 x 68 nF, 7 x 100nF
- Electrolytic Capacitors: 2 x 1uF, 2 x 10uF, 1 x 100uF
- 15 x 2-way pin header sockets.
- 2 x 4-way pin header sockets.
- Jumper header pins.
- 2 x 15-way pin header sockets.
- Optional: 24-pin DIP socket (see note below on footprint error!).
- Optional: 8-pin DIP socket.
- Optional: 6-pin DIP socket.

Errata:
- The 24-pin DIP footprint is incorrect and requires an adaptor board.
- There is no board space to allow the two power supply electrolytics to bend over the PCB.
- The footprint for the 2N3904 transistors is too small for easy soldering.
- Using GPIO 2, 4, 15 as analog input values causes issues - see design notes for details.

This repository also contains:
* 24-pin DIP narrow to wide adaptor.
* Front panel design.
* Connector panel design.
* STL file for a 3D Printed box for both panels.
* STL file for a 3D Printed tool to help screw in spacers.

If you like what you see, you can buy me a Ko-Fi - https://ko-fi.com/diyelectromusic

#  A Word of Caution!

Please note - I consider myself a novice at pcb design!

**Don't spend your own time and resources on any of these designs without knowing what you are doing.  They are not "off the shelf products" ready to go!**

Do not use this with expensive computer, audio, music or electronic equipment, it is for hobby use only.  For more details, see https://diyelectromusic.wordpress.com/pcbs/

# License

All information is provided AS IS with no implied fit for purpose as detailed in the included MIT License.

All content and code (c) diyelectromusic (Kevin)
