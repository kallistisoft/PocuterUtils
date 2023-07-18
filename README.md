# PocuterUtils -- Pocuter Utility Libraries, CLI Tools, and Applications
PocuterUtils is a collection of applications, command-line tools, and utility libraries for developing software for the [Pocuter](https://pocuter.com/pocuter-one) ecosystem

At this point in time, some of the apps require the [Kallistisoft fork of the Pocuter Library](https://github.com/kallistisoft/PocuterLib) 


***

## Developer Tools

**[convert-icon.sh](Tools/)**<br/>Utility for converting images to base64 strings suitable for inclusion in Pocuter App metadata files

**[pocuter-deploy](./Apps/CodeUploader/tools/)**<br/>Utility for compiling, packaging, and uploading applications to the [**'Code Upload Server'**](./Apps/CodeUploader/)

***NOTE: The 'pocuter-deploy' tool has been integrated into the [Official Pocuter GitHub Repository](https://github.com/pocuter/pocuter-deploy), all future updates will posted to that repository!***


***

## Utility Libraries
***[PocuterUtil::Keyboard](Libs/Keyboard)***<br/>Utility class for quickly implementing a keyboard interface. It supports compossible character sets, special handling for numeric, float, ip address, and hostname input. It also supports user-defined character sets and real-time post-processing for creating 'smart' keyboards

***

## Utility Applications
[**'Code Upload Server'**](./Apps/CodeUploader/)<br/>A Pocuter application for installing applications via the web, features an HTML5 web interface and [command line tool](./Apps/CodeUploader/tools/)

***NOTE: The 'Code Upload Server' has been integrated into the [Official Pocuter GitHub Repository](https://github.com/pocuter/Pocuter-One_Apps/tree/main/CodeUploader), all future updates will posted to that repository!***

**[SDCardUtil](Apps/SDCardUtil)**<br/>A Pocuter application for mounting/unmounting the sd card while the system is running (*requires pocuter library fork*)

**[KeyboardDemo](Apps/KeyboardDemo)**<br/>A Pocuter application demonstrating advanced usage of the PocuterUtil::Keyboard object
