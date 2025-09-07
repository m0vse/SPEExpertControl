# SPEExpertControl 

![main-screen](https://github.com/user-attachments/assets/f7b14f2b-df57-4a4f-8e39-0634fccd2e02)

Copyright (C) 2025 Phil E Taylor (M0VSE)

Controler for SPE Expert Amplifiers

This is a "work in progress" application for Arduino Giga R1 and GigaDisplay to control SPE Expert Amplifiers.

Currently only the Expert 1K is supported. Only addition needed is a MAX3232 connected to the first UART for communications with the amplifier

Implemented:

Button handling and all current screens


To-Do:

The remaining screens

Web Interface

Direct selection of menu options 

Anything else I think of (suggestions welcome!)

Add support for other Expert amps

Make Bootup splash screen actually do something

Add Power-On option (GPIO->DTR)



Building:
To build with Arduino IDE 2.x you must set your Sketchbook location in preferences to the directory containing the ui folder:

d:\Documents\Arduino\SPEExpertControl\SPE_Expert1K on my computer.
