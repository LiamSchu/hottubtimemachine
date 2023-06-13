# hottubtimemachine
Code to have Arduino Uno control hot tub heater (and other devices)


walkthrough video here:
[![Video Title](https://img.youtube.com/vi/sqngU6ImdXs/0.jpg)](https://www.youtube.com/watch?v=sqngU6ImdXs)


The purpose of this project was to replace the stock control and relay board of my Hotsprings Spa tub.  This code should work on any hot tub (I think) as I am building it with the intention of recycling it with other free tubs I get of marketplace.

I am using the stock thermistors (if your hot tub has thermocouples which vary the voltage instead of the resistance, you can use the same code but will need a voltage amplifier circuit).  I am also using the stock heater which should be cross compatible with any heater.  I did purchase a relay board - had to search online to find one that could handle 30 amps, and any relay board should be compatible with the code (do be aware of the current draw especially if the relay solenoid is fed from the arduino (max output 0.8 amps at 5V, 0.15A at 3.3V).  I have many future upgrade plans, but for now, this functions and should function for any hot tub, to read the temperature and turn on and off the heater.  It displays on an LCD display and also sends information back over the serial port (usb) to the computer which is helpful for determining how frequently the heater cycles at different temperatures.

This kit may be helpful as it has the LCD display, buttons, and breadboard but not necessary:
https://www.amazon.com/gp/product/B08351F76R/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1

parts:
LCD display,
thermistor,
buttons,
heater,
arduino uno (should work with other arduinos, but not tested)
resistors (10k, 2k and 220 ohm)
wire jumpers,
relay board,
power supply (usb or walwart for arduino, walwart for relay board).
