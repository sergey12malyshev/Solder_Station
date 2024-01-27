The project of a soldering station on the Atmega8a microcontroller from the trash

Refactoring was carried out (brought data types and functions to classical C), NO_SOLDER indication mode was added when a soldering iron is missing or entered, a Heartbeat LED was added.

## Toolkit
*PlatformIO* + *VS Code*

Configuring the development environment: https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/
## Program firmware
USBISP(USBASP) HW programmer + Khazama AVR SW Programmer

## Hardware
- Microcontroller Atmega8a 16MHz, 1KB RAM, 8KB Flash
- Operational amplifier LM358
- Indicator BA56-12SRWA

<img src= "https://github.com/sergey12malyshev/Solder_Station/blob/develop/image/work.jpg" width=30% height=30%> <img src= "https://github.com/sergey12malyshev/Solder_Station/blob/develop/image/noSolder.jpg" width=30% height=30%>
<img src= "https://github.com/sergey12malyshev/Solder_Station/blob/develop/image/hard.jpg" width=40% height=40%>

## Useful information
The original project: https://www.allaboutcircuits.com/projects/do-it-yourself-soldering-station-with-an-atmega8/

## About repo
Work in the repository is carried out through the Git-flow branching model

