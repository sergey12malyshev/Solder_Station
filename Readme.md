Проект паяльной станции на микроконтроллере Atmega8a из хлама

Проведен рефакторинг (привел типы данных и функции к классическому СИ), добавлен режим индикации NO_SOLDER при отсутсвии или неисправности паяльника, добавлен heartbeet-светодиод
## Инструментарий
*PlatformIO* + *VS Code*

Настройка: https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/
## Прошивка ПО
USBISP(USBASP) + Khazama AVR Programmer

## Hardware
- МК Atmega8a 16MHz, 1KB RAM, 8KB Flash
- ОУ LM358
- Индикатор BA56-12SRWA

<img src= "https://github.com/sergey12malyshev/Solder_Station/blob/develop/image/work.jpg" width=30% height=30%> <img src= "https://github.com/sergey12malyshev/Solder_Station/blob/develop/image/noSolder.jpg" width=30% height=30%>
<img src= "https://github.com/sergey12malyshev/Solder_Station/blob/develop/image/hard.jpg" width=40% height=40%>

## Полезная информация
Исходник: https://www.allaboutcircuits.com/projects/do-it-yourself-soldering-station-with-an-atmega8/

Перевод статьи: https://radioprog.ru/post/193


