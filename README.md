DIY Pocket Piano
==============================
This started as a fork of the Critter and Guitari Arduino Pocket Piano which used to be a synth on an Arduino shield that you could buy from them. The shield is no longer made so I ordered some PCBs and all the necessary bits and pieces. Once assessmbled I started to mess with the code to add increased functionality. The Arduino Uno was not able to do everything I wanted so I have switched over to using a Teensy and SparkFun's Teensy Arduino Shield. I am currently working on designing a PCB that fits the Teensy directly. The main sketch is located in the "Teensy_Audio_Modes" folder. A major item that is missing right now is the BOM for my current build and the revisions I had to make to the PCB to get everything working correctly. 

Folders are as follows: 
1) Original CG Files: includes all the original files including PCB
2) Combined CG Files - Teensy and Arduino: Contains sketches for both Arduino and Teensy that have the simple play and octave arp sketches combined into one sketch
3) Photos: Contains photos from CG and photos of my current build
4) Solidworks Modeling: Contains Solidwork Part files and STLs for the parts I 3D printed for my current build
5) Teensy Intermediate Steps: Contains intermediate sketches I made while figuring out how to port this to Teensy. Files may or may not work
6) Teensy_Audio_Modes: The working file for using the Teensy synth
7) Teensy_Synth_PCB: The early stages of the PCB I am designing using KiCad. 
