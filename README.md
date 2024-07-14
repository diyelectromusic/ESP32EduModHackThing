# ESP32EduModHackThing

![Photo of the ESP32EduModHackThing](images/ESP32EduModHackThing.JPG)

ESP32 based, hacked together modular-synth like thing to help learn the basics of analog synthesis and hook up to breadboards for experimenting with circuits.

* Blog series: https://diyelectromusic.wordpress.com/2024/05/07/educational-diy-synth-thing/

## Getting Started

Once the hardware is ready (see below) here are some links to get started:
* One-sheet Getting Started Guide: [Download Here](Docs/EduModSynthThing-GettingStarted.pdf).
* Some first experiments: https://diyelectromusic.com/2024/06/17/educational-diy-synth-thing-part-5/

## Hardware

**Be sure to read the Errata and limitations in the design notes**

Contents of the [Hardware](/Hardware) directory:
* Main PCB - 100x100mm main PCB.
* 24pin DIP Adaptor - small adaptor PCB as a workaround to the errata about the 24-pin DIP socket.
* Front Panel - 100x100mm PCB files that can be used as a front panel.

Note: The above does not include any connections for MIDI, audio, or power.  See the build notes for details.

Additional hardware (optional):
* Connector Panel - 50x100mm PCB files that can be used as an additional accessories/connectors panel.
* 3D Printer STL file - 150x100x35mm approx enclosure for a front panel and connector panel.
* 3D Printer STL file for a low quality tool to help screw in the hex-spacers.
* Alternative Connector Panel - 50x100mm PCB files for accessories/connectors including a JST SH 4-way header for power and MIDI compatible with my Raspberry Pi Pico Touch Keyboard.
* JST SH 4-pin breakout board - breakout board for use with the alternative connecor panel.

![Photo of the ESP32EduModHackThing in its box](images/ESP32EduModHackThingBox.JPG)

Main PCB:
* PCB Design Notes: https://diyelectromusic.wordpress.com/2024/05/07/esp32-wroom-educational-modular-synth-thing-pcb-design/
* PCB Build and Test Notes: https://diyelectromusic.wordpress.com/2024/05/19/esp32-wroom-educational-modular-synth-thing-pcb-design-build-guide/

## Code Directories

* HardwareTestCode - contains a series of sketches that can be used to test specific elements of the board.
* Firmware - contains the current release of the core firmware to perform all the functionss as described in the blog series.

## License, Use, Feedback, Contributions

Released under a MIT license.  No fitness for any purpose suggested or implied.  Use at your own risk.  I am not an electronics person - I'm just messing around.

**Do not plug this into anything else and certainly not anything expensive or wanted.**

Contributions are not being accepted.  This is just a repository for the design, but suggestions and conversations about it welcome on my blog or via social media.

Kevin

_All code and designs Copyright (c) Kevin (diyelectromusic)_
