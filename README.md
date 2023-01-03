# PocuterUtils - Pocuter Utility Library and Applications
PocuterUtils is a collection of applications and utility library for developing applications for the [Pocuter](https://pocuter.com/pocuter-one)  ecosystem

At this point in time, some of the apps require the [Kallistisoft fork of the Pocuter Library](https://github.com/kallistisoft/PocuterLib) 


***

## Developer Tools

**[convert-icon.sh](Tools/)**<br/>Utility for converting images to base64 strings suitable for inclusion in Pocuter App metadata files


***

## Utility Libraries
***[PocuterUtil::Keyboard](Libs/Keyboard)***<br/>Utility class for quickly implementing a keyboard interface. It supports compossible character sets, special handling for numeric, float, ip address, and hostname input. It also supports user-defined character sets and post-processing flag for creating 'smart' keyboards

***

## Utility Applications
**[SDCardUtil](Apps/SDCardUtil)**<br/>A Pocuter application for mounting/unmounting the sd card while the system is running (*requires pocuter library fork*)

**[KeyboardDemo](Apps/KeyboardDemo)**<br/>A Pocuter application demonstrating advanced usage of the PocuterUtil::Keyboard object
