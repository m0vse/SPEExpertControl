# SPEExpertControl 

Copyright (C) 2025 Phil E Taylor (M0VSE)
Control SPE Expert Amplifiers

This is a "work in progress" application for Arduino Giga R1 and GigaDisplay to control SPE Expert Amplifiers.

Currently only the Expert 1K is supported. Only addition needed is a MAX3232 connected to the first UART for communications with the amplifier

Implemented:
Button handling
Main Receive Screen
TX Screen (partial)
Alarm History screen
System Messages
Warning Screen
Setup Options Screen
Setup Antenna Screen (partial)


To-Do:
The remaining screens
Web Interface
Direct selection of menu options 
Anything else I think of (suggestions welcome!)
Add support for other Expert amps

Building:
To build with Arduino IDE 2.x you must set your Sketchbook location in preferences to the directory containing the ui folder:

d:\Documents\Arduino\SPEExpertControl\SPE_Expert1K on my computer.